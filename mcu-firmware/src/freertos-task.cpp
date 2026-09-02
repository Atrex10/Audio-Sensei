#include "freertos-task.h"

#include <Arduino.h>

#include "mainHeader.h"
#include "serial-comm.h"

QueueHandle_t commandQueueHandle;
TaskHandle_t processCommandHandle;


void sendCmd2Queue(InputType type, char id, char value, bool isr) {
    InputCommand newCmd;

    newCmd.type = type;
    newCmd.id = id;
    newCmd.value = value;

    if (!isr) {
        xQueueSend(commandQueueHandle, &newCmd, pdMS_TO_TICKS(QUEUE_FULL_TIMEOUT_MS));  // wait max x ms if queue full
    } else {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(commandQueueHandle, &newCmd, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


// FreeRTOS json sending task func
void processCommandTask(void *pvParameters) {
    InputCommand recvCmd;

    #ifdef DEBUG
    Serial.println("Freertos task started");
    #endif

    while(1) {
        if (xQueueReceive(commandQueueHandle, &recvCmd, portMAX_DELAY) == pdPASS) {
            #ifdef DEBUG
            Serial.println("Received command");
            #endif
            
            idCounter++;
            sendJson(recvCmd.type, recvCmd.id, recvCmd.value, idCounter);
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
    1,                      // task priority (higher is more important)
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
