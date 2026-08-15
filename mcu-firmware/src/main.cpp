#include <Arduino.h>

#include <inputs.h>

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

void setup() {
    Serial.begin(115200);  // only for testing, will be moved to separate class/file
    while (!Serial) {;}  // wait for serial
    Serial.println("Test");

    // inputs setup
    for (int i=0; i<encoderCount; i++) {
        encoders[i].begin();
    }

    for (int i=0; i<buttonCount; i++) {
        buttons[i].begin();
    }
}

void loop() {
    Serial.println("\n\n\nEncoders:");
    for (int i=0; i<encoderCount; i++) {
        Serial.println(encoders[i].read());
    }
    Serial.println("END");

    delay(100);
}
