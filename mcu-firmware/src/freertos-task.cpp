#include "freertos-task.h"

#include <Arduino.h>

#include "mainHeader.h"

void sendCmd2Queue(InputType type, char id, char value) {
    InputCommand newCmd;

    newCmd.type = type;
    newCmd.id = id;
    newCmd.value = value;

    xQueueSend(commandQueue, &newCmd, pdMS_TO_TICKS(10));  // wait max 10ms if queue full
}


void processCommandTask(void *pvParameters) {
    InputCommand recvCmd;

    for (;;) {
        if (xQueueReceive(commandQueue, &recvCmd, portMAX_DELAY) == pdPASS) {
            #ifdef DEBUG
            Serial.println("Received command");
            #endif
            // TODO: write jsoning and sending the command
        }
    }
}
