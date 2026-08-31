#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#define QUEUE_SLOTS 10
#define QUEUE_FULL_TIMEOUT_MS 10

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

// queue and task handles
extern QueueHandle_t commandQueueHandle;
extern TaskHandle_t processCommandHandle;

void sendCmd2Queue(InputType type, char id, char value, bool isr=false);

void processCommandTask(void *pvParameters);

void setupTask();
void setupQueue();

#endif
