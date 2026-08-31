#include "mainHeader.h"
#include <Arduino.h>

#include "inputs.h"
#include "freertos-task.h"

// variables
// INPUTS DEFINITIONS

// For each input device, set <device>Count variable to number of devices you have.
// Then in the array below remove/add instances to match number of devices.
// Last adjust the pins to which the devices are connectet.
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

bool checkInputActivity(int value) {
    return value != 0;
}


void setup() {
    Serial.begin(115200);  // only for testing, will be moved to separate class/file
    while (!Serial) {;}  // wait for serial
    delay(2000);
    Serial.println("Test");

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
}

void loop() {
    int value;
    // temporary loop

    // check the encoders
    for (int i=0; i < encoderCount; i++) {
        value = encoders[i].readAndReset();
        if (checkInputActivity(value)) {
            sendCmd2Queue(ENCODER, i, value);
            Serial.println("encoder sent");
        }
    }

    // check the buttons
    for (int i=0; i < buttonCount; i++) {
        value = buttons[i].readAndReset();
        if (checkInputActivity(value)) {
            sendCmd2Queue(BUTTON, i, value);
            Serial.println("button sent");
        }
    }

    Serial.print("Queue status. Used: ");
    Serial.print(uxQueueMessagesWaiting(commandQueueHandle));
    Serial.print(", available: ");
    Serial.println(uxQueueSpacesAvailable(commandQueueHandle));
    delay(100);
}
