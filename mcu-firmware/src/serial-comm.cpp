#include "serial-comm.h"

void beginSerial() {
    Serial.begin(115200);
    while (!Serial) {;}  // wait for serial to initialize
    delay(1000);
    Serial.println("Serial started");
}

void sendJson(uint8_t inputType, uint8_t inputId, uint8_t inputValue, unsigned int id) {
    JsonDocument doc;

    // set json fields
    doc["type"] = "command";
    doc["id"] = id;
    doc["status"] = "OK";
    doc["inputID"] = inputId;
    doc["value"] = inputValue;

    // set input json field
    switch (inputType)
    {
    case 0:
        doc["input"] = "encoder";
        break;
    
    case 1:
        doc["input"] = "button";
        break;
    
    case 2:
        doc["input"] = "potentiometer";
        break;
    
    default:
        doc["input"] = "read";
        break;
    }

    // send the json through terminal
    serializeJson(doc, Serial);
}
