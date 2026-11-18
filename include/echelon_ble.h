#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <functional>

class EchelonBLE : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    using ResistanceCallback = std::function<void(int newLevel)>;

    EchelonBLE(const char* deviceName = "ECHEX-5s-109843");

    void begin();
    void update(int currentResistance, uint16_t currentCadenceRpm);
    void notifyResistance(int level);
    void notifyCadenceAndTime(uint16_t rpm, uint16_t elapsedSeconds);

    // Register a callback for when the tablet commands a new resistance level (1-32)
    void onTargetResistanceChanged(ResistanceCallback cb) { _resCallback = cb; }

    bool isConnected() const { return _deviceConnected; }

    // BLEServerCallbacks
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;

    // BLECharacteristicCallbacks
    void onWrite(BLECharacteristic* pCharacteristic) override;

private:
    const char* _deviceName;
    BLEServer* _pServer = nullptr;
    BLECharacteristic* _pWriteChar = nullptr;
    BLECharacteristic* _pReadChar = nullptr;
    BLECharacteristic* _pNotifyChar = nullptr;

    bool _deviceConnected = false;
    bool _streamingEnabled = false;
    int _lastSentResistance = -1;
    uint32_t _lastTelemetryMs = 0;
    uint32_t _sessionStartMs = 0;

    ResistanceCallback _resCallback = nullptr;

    uint8_t computeChecksum(const uint8_t* data, size_t len);
    void sendAckOnReadChar(const uint8_t* payload, size_t len);
};
#endif // ARDUINO
