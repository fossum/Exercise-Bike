#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x05
#endif
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif
#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif
#endif

class RotaryEncoder {
public:
    /**
     * @param pinA          GPIO pin for Encoder Phase A (CLK)
     * @param pinB          GPIO pin for Encoder Phase B (DT)
     * @param pinSW         GPIO pin for Push Button (pass -1 if not wired)
     * @param minLevel      Minimum resistance level (default 1)
     * @param maxLevel      Maximum resistance level (default 32)
     * @param stepsPerClick Quadrature state transitions per detent click (default 1)
     */
    RotaryEncoder(int pinA, int pinB, int pinSW = -1, int minLevel = 1, int maxLevel = 32, int stepsPerClick = 1);

    void begin();

    // Returns true if the level changed since the last call to consumeChange()
    bool hasChanged();

    // Returns the current resistance level (clamped between minLevel and maxLevel)
    int getLevel() const;

    // Manually set the current level (e.g., when updated via BLE or startup)
    void setLevel(int level);

    // Returns true once per physical button press (debounced)
    bool wasButtonPressed();

private:
    int _pinA;
    int _pinB;
    int _pinSW;
    int _minLevel;
    int _maxLevel;
    int _stepsPerClick;

    volatile int _level;
    volatile int _subStepAccum;
    volatile uint8_t _prevState;
    volatile bool _changed;

    uint32_t _lastButtonPressMs;
    bool _lastButtonState;

    static void isrRouter(void* arg);
    void handleInterrupt();
};