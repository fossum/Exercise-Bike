#include "rotary_encoder.h"

// Quadrature lookup table: rejects invalid state jumps (contact bounce)
static const int8_t ENC_STATES[16] DRAM_ATTR = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

RotaryEncoder::RotaryEncoder(int pinA, int pinB, int pinSW, int minLevel, int maxLevel, int stepsPerClick)
    : _pinA(pinA),
      _pinB(pinB),
      _pinSW(pinSW),
      _minLevel(minLevel),
      _maxLevel(maxLevel),
      _stepsPerClick(stepsPerClick > 0 ? stepsPerClick : 1),
      _level(minLevel),
      _subStepAccum(0),
      _prevState(0),
      _changed(false),
      _lastButtonPressMs(0),
      _lastButtonState(HIGH) {}

void RotaryEncoder::begin() {
#ifdef ARDUINO
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);

    if (_pinSW >= 0) {
        pinMode(_pinSW, INPUT_PULLUP);
    }

    // Read initial pin states
    _prevState = (digitalRead(_pinA) << 1) | digitalRead(_pinB);

    // Attach interrupts on both pins using argument passing
    attachInterruptArg(digitalPinToInterrupt(_pinA), isrRouter, this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(_pinB), isrRouter, this, CHANGE);
#endif
}

void IRAM_ATTR RotaryEncoder::isrRouter(void* arg) {
    static_cast<RotaryEncoder*>(arg)->handleInterrupt();
}

void IRAM_ATTR RotaryEncoder::handleInterrupt() {
#ifdef ARDUINO
    uint8_t currState = (digitalRead(_pinA) << 1) | digitalRead(_pinB);
#else
    uint8_t currState = 0;
#endif
    uint8_t idx = ((_prevState << 2) | currState) & 0x0F;
    _prevState = currState;

    int8_t delta = ENC_STATES[idx];
    if (delta != 0) {
        _subStepAccum = _subStepAccum + delta;

        if (_subStepAccum >= _stepsPerClick) {
            _subStepAccum = 0;
            if (_level < _maxLevel) {
                _level = _level + 1;
                _changed = true;
            }
        } else if (_subStepAccum <= -_stepsPerClick) {
            _subStepAccum = 0;
            if (_level > _minLevel) {
                _level = _level - 1;
                _changed = true;
            }
        }
    }
}

bool RotaryEncoder::hasChanged() {
    if (_changed) {
        _changed = false;
        return true;
    }
    return false;
}

int RotaryEncoder::getLevel() const {
    return _level;
}

void RotaryEncoder::setLevel(int level) {
#ifdef ARDUINO
    noInterrupts();
#endif
    if (level < _minLevel) {
        _level = _minLevel;
    } else if (level > _maxLevel) {
        _level = _maxLevel;
    } else {
        _level = level;
    }
    _subStepAccum = 0;
    _changed = true;
#ifdef ARDUINO
    interrupts();
#endif
}

bool RotaryEncoder::wasButtonPressed() {
    if (_pinSW < 0) return false;

#ifdef ARDUINO
    bool reading = digitalRead(_pinSW);
    uint32_t now = millis();

    if (reading == LOW && _lastButtonState == HIGH && (now - _lastButtonPressMs > 200)) {
        _lastButtonPressMs = now;
        _lastButtonState = LOW;
        return true;
    } else if (reading == HIGH) {
        _lastButtonState = HIGH;
    }
#endif
    return false;
}