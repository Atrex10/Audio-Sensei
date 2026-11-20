#ifndef SIGNAL_LEDS_H
#define SIGNAL_LEDS_H

#include <mainHeader.h>
#include <Arduino.h>
#include <timers.h>

enum LedFunction {
    L_INPUT,
    L_BUSY,
    L_ERROR
};

class SignalLeds {
public:
    SignalLeds(const int ledPins[3]) {
        for (int i = 0; i < 3; i++) {
            pins[i] = ledPins[i]; }
    }

    void begin() {
        #ifdef USE_SIG_LEDS
        for (int i = 0; i < 3; i++) {
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], LOW);
            timers[i] = xTimerCreate("led", pdMS_TO_TICKS(100), pdFALSE,
                                     (void*)(intptr_t)pins[i], ledOff);
        }
        #endif
    }

    void flash(LedFunction led, uint32_t durationMs) {
        #ifdef USE_SIG_LEDS
        digitalWrite(pins[led], HIGH);
        xTimerChangePeriod(timers[led], pdMS_TO_TICKS(durationMs), 0);  // also (re)starts it
        #endif
    }

    void turnOn(LedFunction led) {
        #ifdef USE_SIG_LEDS
        digitalWrite(pins[led], HIGH);
        #endif
    }

    void turnOff(LedFunction led) {
        #ifdef USE_SIG_LEDS
        digitalWrite(pins[led], LOW);
        #endif
    }

    void toggle(LedFunction led) {
        #ifdef USE_SIG_LEDS
        digitalWrite(pins[led], !digitalRead(pins[led]));
        #endif
    }

private:
    #ifdef USE_SIG_LEDS
    static void ledOff(TimerHandle_t t) {
        digitalWrite((int)(intptr_t)pvTimerGetTimerID(t), LOW);
    }

    int pins[3];
    TimerHandle_t timers[3];
    #endif
};

#endif
