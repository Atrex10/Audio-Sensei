#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

typedef enum {
    ENCODER,
    BUTTON,
    POTMR
} InputType;

typedef struct {
    InputType type;
    char id;
    char value;
} InputCommand;

// queue handles
QueueHandle_t commandQueue;

void sendCmd2Queue(InputType type, char id, char value);

void processCommandTask(void *pvParameters);

void setupTask();
void setupQueue();

#endif
