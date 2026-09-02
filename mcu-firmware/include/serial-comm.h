#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <Arduino.h>

#include <ArduinoJson.h>

void beginSerial();

void sendJson(uint8_t inputType, uint8_t inputId, uint8_t inputValue, unsigned int id);

// TODO: write the class
class AckController {
    private:

    public:

};


#endif
