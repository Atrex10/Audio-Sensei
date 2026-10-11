#include "mainHeader.h"
#include <Arduino.h>
#include <Ticker.h>

#include "inputs.h"
#include "freertos-task.h"
#include "serial-comm.h"
#include "state-buffer.h"
#include "outputs.h"

// variables
// INPUTS DEFINITIONS

// For each input device, set <device>Count variable to number of devices you have.
// Then in the array below remove/add instances to match number of devices.
// Lastly adjust the pins to which the devices are connected.
// Notice: the order of device definitions matters, this is how they are going to
// identify to the daemon on the pc (first defined will have id 0, second id 1 and so on)

// TODO: add support for potentiometers

// encoders
const int encoderCount = 4;
Encoder encoders[encoderCount] = {
    Encoder(6, 10),
    Encoder(7, 11),
    Encoder(8, 12),
    Encoder(9, 13),
};

// buttons
const int buttonCount = 10;
Button buttons[buttonCount] = {
    // encoder buttons
    Button(21),
    Button(20),
    Button(19),
    Button(18),
    // regular buttons
    Button(0),
    Button(1),
    Button(2),
    Button(3),
    Button(4),
    Button(5)
};


// VARIABLES
const int updateInputsInterval = 100;  // ms

const int updateOutputsInterval = 500;  // ms, how often the outputs loop executes, set to lowest from below
// the cycles refer to how many times the above interval
// will trigger before the given function executes
// 200ms interval & 20 cycles executes every 4000ms
const int updateOutputsStateBufferCycles = 10;
const int renderOutputsLcdCycles = 10;  //  no need to be faster than state buffer update (above)
const int incrementOutputsAnimationCycles = 1;

uint32_t updateOutputsCycleCounter = 0;


unsigned long idCounter = 0;

// OBJECTS
Ticker updateInputsTicker;

Ticker updateOutputsTicker;

// lcd object definition
LiquidCrystal_I2C lcd(0x27, 16, 2);
UserInterface userInterface(&lcd);  // user interface object

// state buffer
StateBuffer stateBuffer;


// FUNCTIONS
bool checkInputActivity(int value) {
    return value != 0;
}

void sendReadCmd2Queue(bool isr = false) {
    sendCmd2Queue(READ, 0, 0, isr);
}

void updateInputs() {
    int value;

    // check encoders
    for (int i=0; i<encoderCount; i++) {
        if (!encoders[i].inputGrouperTimerExpired()) {
            value = encoders[i].read();

            if (checkInputActivity(value)) {
                #ifdef DEBUG
                Serial.println("Detected encoder turn");
                #endif
                encoders[i].startInputGrouperTimer();  // start the input group
            }
        } else {
            // time to send the group
            value = encoders[i].readAndReset();
            sendCmd2Queue(ENCODER, i, value, true);

            #ifdef DEBUG
            Serial.println("Encoder group sent to queue");
            #endif
        }
    }

    // check buttons
    for (int i=0; i<buttonCount; i++) {
        value = buttons[i].readAndReset();
        if (checkInputActivity(value)) {
            sendCmd2Queue(BUTTON, i, value, true);

            #ifdef DEBUG
            Serial.println("Detected button press");
            #endif
        }
    }
}

void updateOutputs() {
    if (updateOutputsCycleCounter % updateOutputsStateBufferCycles == 0) {
        sendReadCmd2Queue(true);
    }
    if (updateOutputsCycleCounter % renderOutputsLcdCycles == 0) {
        userInterface.updateText();
    }
    if (updateOutputsCycleCounter % incrementOutputsAnimationCycles == 0) {
        userInterface.updateAnimation();
    }

    updateOutputsCycleCounter++;
}

// SETUP & LOOP
void setup() {
    // state buffer setup
    stateBuffer.begin();

    // user interface setup
    userInterface.begin();

    // inputs setup
    for (int i=0; i<encoderCount; i++) {
        encoders[i].begin();
    }

    for (int i=0; i<buttonCount; i++) {
        buttons[i].begin();
    }

    // task and queue setup
    setupTask();
    setupQueue();

    // tickers configuration
    updateInputsTicker.attach_ms(updateInputsInterval, updateInputs);
    updateOutputsTicker.attach_ms(updateOutputsInterval, updateOutputs);

    // serial setup
    beginSerial();
}

void loop() {
}
