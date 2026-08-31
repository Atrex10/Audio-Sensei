#include "freertos-task.h"

#include <Arduino.h>

#include "mainHeader.h"


void sendCmd2Queue(InputType type, char id, char value) {
    InputCommand newCmd;

    newCmd.type = type;
    newCmd.id = id;
    newCmd.value = value;

    xQueueSend(commandQueueHandle, &newCmd, pdMS_TO_TICKS(QUEUE_FULL_TIMEOUT_MS));  // wait max x ms if queue full
}


// FreeRTOS json sending task func
void processCommandTask(void *pvParameters) {
    InputCommand recvCmd;

    for (;;) {
        if (xQueueReceive(commandQueueHandle, &recvCmd, portMAX_DELAY) == pdPASS) {
            #ifdef DEBUG
            Serial.println("Received command");
            #endif
            // TODO: write jsoning and sending the command
        }
    }
}


// FreeRTOS task and queue setups
void setupTask() {
    xTaskCreate (
    processCommandTask,     // funciton realizing the task
    "Serial_data_task",     // text task name
    8192,                   // stack size (bytes)
    NULL,                   // args for task
    2,                      // task priority (higher is more important)
    &processCommandHandle   // task handle
    );
}

void setupQueue() {
    commandQueueHandle = xQueueCreate(QUEUE_SLOTS, sizeof(InputCommand));

    if (commandQueueHandle == NULL) {
        Serial.println("Error: failed to crate queue. Program halted");
        while (1);
    }
}
