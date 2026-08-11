#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include <Encoder.h>

class Button {
    // variables
    pin_size_t buttonPin;

    public:
        Button(pin_size_t pin) {
            buttonPin = pin;
        }

        void begin(PinStatus interruptMode=FALLING) {
            pinMode(buttonPin, INPUT_PULLUP);
            attachInterruptParam(digitalPinToInterrupt(buttonPin), isrWrapper, interruptMode, this);
        }

        bool read() {
            return clicked;
        }

        void write(bool state) {
            clicked = state;
        }

        bool readAndReset() {
            bool state = clicked;
            clicked = false;
            return state;
        }

    private:
        volatile bool clicked = false;

        volatile int debounceLockoutMillis = 160;
        volatile unsigned long lastPressMillis = 0;

        static void isrWrapper(void* arg) {
            Button* buttonInstance = static_cast<Button*>(arg);
            buttonInstance->interruptCallback();
        }

        void interruptCallback() {
            if (millis() - lastPressMillis >= debounceLockoutMillis) {
                clicked = true;
                lastPressMillis = millis();
            }
        }
};

#endif
