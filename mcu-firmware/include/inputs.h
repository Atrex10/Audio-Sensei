#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include <RotaryEncoder.h>

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
            bool state = read();
            write(false);
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

class Encoder {
    public:
        // variables
        pin_size_t encoderPin1;
        pin_size_t encoderPin2;

        // pointer for dynamic instance (see begin())
        RotaryEncoder *encoder = nullptr;

        Encoder (pin_size_t pin1, pin_size_t pin2) {
            encoderPin1 = pin1;
            encoderPin2 = pin2;
        }

        void begin() {
            encoder = new RotaryEncoder(encoderPin1, encoderPin2, RotaryEncoder::LatchMode::FOUR3);

            attachInterruptParam(digitalPinToInterrupt(encoderPin1), isrWrapper, CHANGE, this);
            attachInterruptParam(digitalPinToInterrupt(encoderPin2), isrWrapper, CHANGE, this);
        }

        int read() {
            return encoder->getPosition();
        }

        void write(int turns) {
            encoder->setPosition(turns);
        }

        int readAndReset() {
            int turns = read();
            write(0);
            return turns;
        }

        bool inputGrouperTimerExpired() {  //  note: resets timerRedeemed flag
            if (timerRedeemed) {return false;}
            if (!timerRedeemed && (millis() - groupingStartMillis >= inputGroupingMs)) {
                timerRedeemed = true;
                return true;
            }

            return false;
        }

        void startInputGrouperTimer() {
            if (timerRedeemed) {
                groupingStartMillis = millis();
                timerRedeemed = false;
            }
        }

    
    private:
        const int inputGroupingMs = 300;
        volatile unsigned long groupingStartMillis = 0;

        volatile bool timerRedeemed = true;

        static void isrWrapper(void* arg) {
            Encoder* encoderInstance = static_cast<Encoder*>(arg);
            encoderInstance->interruptCallback();
        }

        void interruptCallback() {
            encoder->tick();
        }
};

#endif
