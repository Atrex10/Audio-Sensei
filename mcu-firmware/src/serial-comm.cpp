#include "serial-comm.h"

void beginSerial() {
    Serial.begin(115200);
    while (!Serial) {;}  // wait for serial to initialize
    // #ifdef DEBUG
    Serial.println("Serial started");
    // #endif
}

void sendJson(uint8_t inputType, uint8_t inputId, int8_t inputValue, unsigned int id) {
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
    case ENCODER:
        doc["input"] = "encoder";
        break;
    
    case BUTTON:
        doc["input"] = "button";
        break;
    
    case POTMR:
        doc["input"] = "potentiometer";
        break;
    
    default:
        doc["input"] = "read";
        break;
    }

    // send the json through terminal
    serializeJson(doc, Serial);
    Serial.print('\n');
}

JsonDocument waitAndReadAck(int timeoutMs) {
    unsigned long startMillis = millis();

    while (1) {
        // check for timeout
        if (millis() - startMillis >= timeoutMs) {
            JsonDocument timeoutDoc;
            timeoutDoc["type"] = "timeout";
            return timeoutDoc;
        };

        if (Serial.available() > 0) {
            // read serial until new line ('\n')
            String ackString = Serial.readStringUntil('\n');

            // trim and validate
            ackString.trim();
            if (ackString.length() == 0) {
                Serial.println("Read empty ack");
                continue;
            }

            JsonDocument ackDoc;

            DeserializationError error = deserializeJson(ackDoc, ackString);

            if (error) {
                Serial.print("JSON parsing error: ");
                Serial.println(error.f_str());

                JsonDocument errorDoc;
                errorDoc["type"] = "error";
                return errorDoc;
            }

            return ackDoc;
        }
    }
}
