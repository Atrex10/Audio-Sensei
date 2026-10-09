#include "freertos-task.h"

#include <Arduino.h>

#include "mainHeader.h"

QueueHandle_t commandQueueHandle;
TaskHandle_t processCommandHandle;

AckController ackController;

void sendCmd2Queue(InputType type, unsigned char id, char value, bool isr) {
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

    Serial.println("Freertos task started");

    while(1) {
        if (xQueueReceive(commandQueueHandle, &recvCmd, portMAX_DELAY) == pdPASS) {
            #ifdef DEBUG
            Serial.println("Received command");
            #endif
            
            while (ackController.readCommState().ackState != FREE) {} // wait for the ackController to be free

            idCounter++;  // increment the id counter

            bool waitForData = recvCmd.type == READ;

            ackController.reportCommand(idCounter, waitForData);  // if read command report with data request

            sendJson(recvCmd.type, recvCmd.id, recvCmd.value, idCounter);  // send the json to the pc

            // read ack
            JsonDocument ackJson = waitAndReadAck();

            // handle errors
            if (ackJson["type"] == "timeout") {
                Serial.println("Ack timed out, moving on");
                ackController.reportAckSkip();
            } else if (ackJson["type"] == "error") {
                Serial.println("Ack parsing error, moving on");
                ackController.reportAckSkip();
            }

            if (!ackController.reportAck(ackJson)) {
                // if ack invalid, skip it anyways
                #ifdef DEBUG
                Serial.println("Received ack, it is invalid. Skipping");
                #endif
                ackController.reportAckSkip();
            } else if (waitForData) {
                #ifdef DEBUG
                Serial.println("Received ack, interpretting the data");
                #endif
                
                // write the data to state buffer
                if (!stateBuffer.writeStateBufferJson(ackController.readAckData())) {
                    #ifdef DEBUG
                    Serial.println("Failed to write json to state buffer");
                    #endif
                }

            } else {
                #ifdef DEBUG
                Serial.println("Received ack, nothing worth attention");
                #endif
            }
        }
    }
}


// FreeRTOS task and queue setups
void setupTask() {
    xTaskCreate (
    processCommandTask,     // funciton realizing the task
    "Serial_data_task",     // text task name
    8192,                   // stack size (words)
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
