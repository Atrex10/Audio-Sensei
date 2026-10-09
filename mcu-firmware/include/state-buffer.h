#ifndef STATE_BUFFER_H
#define STATE_BUFFER_H

#include <Arduino.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <ArduinoJson.h>

#include "mainHeader.h"

#define TEXT_FIELD_SIZE 24

typedef struct {
    String title = "";
    String artist = "";
    String album = "";
    bool state = false;
} PlaybackState;

class StateBuffer {
    public:
        void begin() {
            // create the mutex
            xMutex = xSemaphoreCreateMutex();

            #ifdef DEBUG
            if (xMutex != NULL) {
                Serial.println("State buffer mutex created succesfuly");}
            #endif
        }

        bool writeBuffer(const PlaybackState& in, TickType_t timeout = portMAX_DELAY) {
            if (!xMutex) return false;
            if (xSemaphoreTake(xMutex, timeout) != pdTRUE) return false;
            // CRITICAL SECTION
            copyToRaw(rawBuf.title, in.title);
            copyToRaw(rawBuf.artist, in.artist);
            copyToRaw(rawBuf.album, in.album);
            rawBuf.state = in.state;

            // END OF CRITICAL SECITON
            xSemaphoreGive(xMutex);
            return true;
        }

        bool readBuffer(PlaybackState& out, TickType_t timeout = portMAX_DELAY) {
            if (!xMutex) return false;

            RawPlaybackState snapshot;
            if (!xSemaphoreTake(xMutex, timeout) != pdTRUE) return false;
            // CRITICAL SECTION
            snapshot = rawBuf;  // quick copy under the lock
            // END OF CRITICAL SECITON
            xSemaphoreGive(xMutex);

            // building the Strings
            out.title = snapshot.title;
            out.artist = snapshot.artist;
            out.album = snapshot.album;
            out.state = snapshot.state;

            return true;
        }

        bool writeStateBufferJson(JsonDocument json) {
            PlaybackState newState;

            newState.title = json["title"] | "";
            newState.artist = json["artist"] | "";
            newState.album = json["album"] | "";
            newState.state = json["state"] | false;

            return writeBuffer(newState);  // copy the status code from lower function
        }

    private:
        // create a struct with char arrays for safe data storage
        struct RawPlaybackState {
            char title[TEXT_FIELD_SIZE] = {};
            char artist[TEXT_FIELD_SIZE] = {};
            char album[TEXT_FIELD_SIZE] = {};
            bool state = false;
        };

        static void copyToRaw(char (&dst)[TEXT_FIELD_SIZE], const String& src) {
            size_t n = src.length();
            if (n >= TEXT_FIELD_SIZE) {
                n = TEXT_FIELD_SIZE - 1;
                while (n > 0 && (static_cast<unsigned char>(src[n]) & 0xC0) == 0x80) n--;
            }
            memcpy(dst, src.c_str(), n);
            dst[n] = '\0';
        }

        RawPlaybackState rawBuf;

        SemaphoreHandle_t xMutex = nullptr;  // state buffer mutex handle
};

#endif
