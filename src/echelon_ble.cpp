#ifdef ARDUINO
#include "echelon_ble.h"

#define ECHELON_ADV_UUID     "0bf669f0-45f2-11e7-9598-0800200c9a66"
#define ECHELON_SERVICE_UUID "0bf669f1-45f2-11e7-9598-0800200c9a66"
#define ECHELON_WRITE_UUID   "0bf669f2-45f2-11e7-9598-0800200c9a66"
#define ECHELON_READ_UUID    "0bf669f3-45f2-11e7-9598-0800200c9a66"
#define ECHELON_NOTIFY_UUID  "0bf669f4-45f2-11e7-9598-0800200c9a66"

EchelonBLE::EchelonBLE(const char* deviceName) : _deviceName(deviceName) {}

void EchelonBLE::begin() {
    BLEDevice::init(_deviceName);
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(this);

    // 1. Create the Advertising Service (0x69f0) so the tablet discovers the bike
    BLEService* pAdvService = _pServer->createService(ECHELON_ADV_UUID);
    pAdvService->start();

    // 2. Create the Primary Communication Service (0x69f1)
    BLEService* pCommService = _pServer->createService(ECHELON_SERVICE_UUID);

    _pWriteChar = pCommService->createCharacteristic(
        ECHELON_WRITE_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    _pWriteChar->setCallbacks(this);

    _pReadChar = pCommService->createCharacteristic(
        ECHELON_READ_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    _pNotifyChar = pCommService->createCharacteristic(
        ECHELON_NOTIFY_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCommService->start();

    // 3. Start Advertising both UUIDs
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(ECHELON_ADV_UUID);
    pAdvertising->addServiceUUID(ECHELON_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    BLEDevice::startAdvertising();

    Serial.printf("[BLE] Advertising as '%s'\n", _deviceName);
}

uint8_t EchelonBLE::computeChecksum(const uint8_t* data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

void EchelonBLE::onConnect(BLEServer* pServer) {
    _deviceConnected = true;
    _streamingEnabled = true; // Default to active on connect
    _sessionStartMs = millis();
    _lastSentResistance = -1;
    Serial.println("[BLE] Echelon Tablet Connected!");
}

void EchelonBLE::onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
    _streamingEnabled = false;
    Serial.println("[BLE] Tablet Disconnected. Restarting advertising...");
    delay(200);
    BLEDevice::startAdvertising();
}

void EchelonBLE::sendAckOnReadChar(const uint8_t* payload, size_t len) {
    if (!_deviceConnected || !_pReadChar) return;
    _pReadChar->setValue(const_cast<uint8_t*>(payload), len);
    _pReadChar->notify();
}

void EchelonBLE::onWrite(BLECharacteristic* pCharacteristic) {
    String val = pCharacteristic->getValue();
    size_t len = val.length();
    if (len < 4) return;

    const uint8_t* rx = reinterpret_cast<const uint8_t*>(val.c_str());
    if (rx[0] != 0xF0) return;

    uint8_t cmd = rx[1];
    switch (cmd) {
        case 0xA1: {
            // Device Info request -> Respond with standard Echelon firmware ACK
            uint8_t resp[9] = {0xF0, 0xA1, 0x05, 0x01, 0x02, 0x08, 0x02, 0x00, 0x00};
            resp[8] = computeChecksum(resp, 8);
            sendAckOnReadChar(resp, sizeof(resp));
            break;
        }
        case 0xB0: {
            // Sensor stream enable (0x01) / disable (0x00)
            _streamingEnabled = (rx[3] == 0x01);
            _sessionStartMs = millis();
            _lastSentResistance = -1; // Force immediate resistance push
            Serial.printf("[BLE] Telemetry stream %s\n", _streamingEnabled ? "ENABLED" : "DISABLED");
            break;
        }
        case 0xB1: {
            // Tablet sets target resistance level (1 - 32)
            int targetLevel = constrain((int)rx[3], 1, 32);
            Serial.printf("[BLE] Tablet requested Resistance Level: %d\n", targetLevel);
            if (_resCallback) {
                _resCallback(targetLevel);
            }
            notifyResistance(targetLevel);
            break;
        }
        default:
            Serial.printf("[BLE] Received cmd 0x%02X (len %d)\n", cmd, (int)len);
            break;
    }
}

void EchelonBLE::notifyResistance(int level) {
    if (!_deviceConnected || !_streamingEnabled) return;

    uint8_t pkt[5] = {0xF0, 0xD2, 0x01, (uint8_t)constrain(level, 1, 32), 0x00};
    pkt[4] = computeChecksum(pkt, 4);

    _pNotifyChar->setValue(pkt, sizeof(pkt));
    _pNotifyChar->notify();
    _lastSentResistance = level;
}

void EchelonBLE::notifyCadenceAndTime(uint16_t rpm, uint16_t elapsedSeconds) {
    if (!_deviceConnected || !_streamingEnabled) return;

    uint8_t pkt[13] = {
        0xF0, 0xD1, 0x09,
        0x00, 0x00, 0x00, 0x00, // Distance/stroke counters
        (uint8_t)((elapsedSeconds >> 8) & 0xFF),
        (uint8_t)(elapsedSeconds & 0xFF),
        (uint8_t)((rpm >> 8) & 0xFF),
        (uint8_t)(rpm & 0xFF),
        0x00,
        0x00 // Checksum placeholder
    };
    pkt[12] = computeChecksum(pkt, 12);

    _pNotifyChar->setValue(pkt, sizeof(pkt));
    _pNotifyChar->notify();
}

void EchelonBLE::update(int currentResistance, uint16_t currentCadenceRpm) {
    if (!_deviceConnected || !_streamingEnabled) return;

    // 1. Immediately push a notification whenever resistance changes
    if (currentResistance != _lastSentResistance) {
        notifyResistance(currentResistance);
    }

    // 2. Broadcast heartbeat telemetry (0xD1 + 0xD2) every 1000ms
    uint32_t now = millis();
    if (now - _lastTelemetryMs >= 1000) {
        _lastTelemetryMs = now;
        uint16_t elapsedSec = (now - _sessionStartMs) / 1000;
        notifyCadenceAndTime(currentCadenceRpm, elapsedSec);
        notifyResistance(currentResistance);
    }
}
#endif // ARDUINO
