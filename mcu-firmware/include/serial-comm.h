#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <semphr.h>

#include <ArduinoJson.h>

#include "freertos-task.h"

typedef enum {
    FREE,
    WAITING_FOR_ACK,
    WAITING_FOR_DATA
} AckState;

typedef struct {
    AckState ackState;
    char id;
} CommState;

void beginSerial();

void sendJson(uint8_t inputType, uint8_t inputId, uint8_t inputValue, unsigned int id);

JsonDocument waitAndReadAck(int timeoutMs=3000);

// TODO: write the class
class AckController {
    private:
        CommState commStateBuffer;

        JsonDocument ackDataBuffer;

    public:
        bool reportCommand(char id, bool waitForData=false) {
            // return false if comms already busy, true if report approved
            if (commStateBuffer.ackState != FREE) {
                return false;
            }

            commStateBuffer.id = id;

            if (!waitForData) {
                commStateBuffer.ackState = WAITING_FOR_ACK;
            } else {
                commStateBuffer.ackState = WAITING_FOR_DATA;
            }

            return true;
        }

        bool reportAck(JsonDocument ack) {
            // return true if ack valid and succesful

            // check validity
            if (ack["type"] != "ack") {return false;}
            if (ack["status"] != "OK") {return false;}
            
            // check id matching
            if (ack["id"] != commStateBuffer.id) {return false;}

            // if we care about the sent data, write it to buffer
            if (commStateBuffer.ackState == WAITING_FOR_DATA) {
                ackDataBuffer.set(ack["value"]);
            } else {
                ackDataBuffer = NULL;
            }

            return true;
        }

        void reportAckSkip() {
            // report the ack skip, will stop waiting for it
            commStateBuffer.ackState = FREE;
        }

        CommState readCommState() {
            return commStateBuffer;
        }
};


#endif
