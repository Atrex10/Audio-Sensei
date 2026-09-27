#ifndef STATE_BUFFER_H
#define STATE_BUFFER_H

#include <Arduino.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <semphr.h>
#include <ArduinoJson.h>

#include "mainHeader.h"

typedef struct {
    String title;
    String artist;
    String album;
    bool state;
} PlaybackState;

class StateBuffer {
    public:
        void begin() {
            // create the mutex
            xStateBufferMutex = xSemaphoreCreateMutex();

            #ifdef DEBUG
            if (xStateBufferMutex != NULL) {
                Serial.println("State buffer mutex created succesfuly");}
            #endif
        }

        bool writeStateBuffer(PlaybackState state) {
            if (xSemaphoreTake(xStateBufferMutex, portMAX_DELAY) == pdTRUE) {
                // CRITICAL SECTION
                memcpy((void*)&state, (void*)&stateBuffer, sizeof(PlaybackState));  // copy the new state into the buffer

                xSemaphoreGive(xStateBufferMutex);
                // END OF CRITICAL SENCTION
            } else {
                return false;
            }
            return true;
        }

        PlaybackState readStateBuffer() {
            if (xSemaphoreTake(xStateBufferMutex, portMAX_DELAY) == pdTRUE) {
                PlaybackState state;

                // CRITICAL SECTION
                memcpy((void*)&stateBuffer, (void*)&state, sizeof(PlaybackState));  // copy the buffer into state

                xSemaphoreGive(xStateBufferMutex);
                // END OF CRITICAL SENCTION

                return state;

            } else {
                // create and return empty state if taking mutex failed
                PlaybackState emptyState = {"", "", "", false};
                return emptyState;
            }
        }

        bool writeStateBufferJson(JsonDocument json) {
            PlaybackState newState;

            newState.title = json["title"].as<String>();
            newState.artist = json["artist"].as<String>();
            newState.album = json["album"].as<String>();
            newState.state = json["state"].as<String>();

            return writeStateBuffer(newState);  // copy the status code from lower function
        }

    private:
        volatile PlaybackState stateBuffer;

        SemaphoreHandle_t xStateBufferMutex = NULL;  // state buffer mutex handle
};

#endif
