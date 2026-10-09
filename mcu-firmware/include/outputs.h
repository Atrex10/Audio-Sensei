#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <FreeRTOS.h>
#include <task.h>

#include "mainHeader.h"
#include "state-buffer.h"

#define CLEARING_SPACES "                    "
#define MAX_CLEARING_SPACES 20

#define ANIMATION_TOTAL_FRAMES 4

#define UPDATE_LCD_ANIMATION_BYTE 0x01
#define UPDATE_LCD_TEXT_BYTE 0x02

extern LiquidCrystal_I2C lcd;

extern const uint8_t playingAnimationFrames[2][4][8];


class LcdUserInterface{
private:
    LiquidCrystal_I2C* lcd;

    volatile uint8_t currentPlayingAnimationFrame = 0;

    void loadPlayingFrameChars() {
        // load regular vinyl quarters
        lcd->createChar(0, const_cast<uint8_t*>(playingAnimationFrames[0][0]));
        lcd->createChar(1, const_cast<uint8_t*>(playingAnimationFrames[0][1]));
        lcd->createChar(2, const_cast<uint8_t*>(playingAnimationFrames[0][2]));
        lcd->createChar(3, const_cast<uint8_t*>(playingAnimationFrames[0][3]));
        // load marked vinyl quarters
        lcd->createChar(4, const_cast<uint8_t*>(playingAnimationFrames[1][0]));
        lcd->createChar(5, const_cast<uint8_t*>(playingAnimationFrames[1][1]));
        lcd->createChar(6, const_cast<uint8_t*>(playingAnimationFrames[1][2]));
        lcd->createChar(7, const_cast<uint8_t*>(playingAnimationFrames[1][3]));

        lcd->home();
    }

    void displayPlayingAnimationFrame(int x, int y, int frameNum) {
        // this function defines how animation frames look
        // it does not have any strict structure, all it has to do 
        // is display the given frame 

        // ANIMATION SPECIFIC CODE
        lcd->setCursor(x, y);
        lcd->write(0);
        lcd->write(1);
        lcd->setCursor(x, y+1);
        lcd->write(2);
        lcd->write(3);

        switch (frameNum) {
        case 0:
            lcd->setCursor(x, y);
            lcd->write(4);
            break;
        
        case 1:
            lcd->setCursor(x+1, y);
            lcd->write(5);
            break;

        case 3:
            lcd->setCursor(x, y+1);
            lcd->write(6);
            break;
        
        case 2:
            lcd->setCursor(x+1, y+1);
            lcd->write(7);
            break;
        
        default: break;}
        // ANIMATION SPECIFIC CODE
    }

    String trimString(const String& str, size_t maxLength) {
        if (maxLength == 0 || str.length() == 0) {
            return "---";  // if empty string return a placeholder
        }
        if (str.length() > maxLength) {
            return str.substring(0, maxLength - 1) + "-";
        }
        return str;
    }

public:
    void begin(LiquidCrystal_I2C* lcdInstance) {
        lcd = lcdInstance;

        // wire begin
        Wire.setSDA(16);
        Wire.setSCL(17);
        Wire.begin();

        lcd->init();
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->noBacklight();

        loadPlayingFrameChars();
    }

    void displayStandBy() {
        lcd->clear();
        displayPlayingAnimation();
        lcd->setCursor(0, 0);
        lcd->print("---");
        lcd->setCursor(0, 1);
        lcd->print("---");
    }

    void displayText(String text, int row, int maxChars = 16) {
        lcd->setCursor(0, row);
        lcd->print(trimString(text, maxChars));
    }

    void displayPlayingAnimation(int x=14, int y=0) {
        displayPlayingAnimationFrame(x, y, currentPlayingAnimationFrame);
    }

    void incrementPlayingAnimation() {
        currentPlayingAnimationFrame++;
        if (currentPlayingAnimationFrame > ANIMATION_TOTAL_FRAMES - 1) {
            currentPlayingAnimationFrame = 0; }  // reset the counter
    }

    void clearDisplay() {
        lcd->clear();
    }

    void clearDisplaySection(int x, int y, int dx, int dy) {
        if (dx > MAX_CLEARING_SPACES) {
            dx = MAX_CLEARING_SPACES;
        }

        for (int i=y; i<y+dy; i++) {
            lcd->setCursor(x, i);
            lcd->print(CLEARING_SPACES + (MAX_CLEARING_SPACES - dx));  // read only desired amount from the end
        }
    }

    void setBacklight(bool state) {
        lcd->setBacklight(state);
    }
};

class UserInterface {
private:
    // objects
    LiquidCrystal_I2C* lcd;

    LcdUserInterface lcdUI;

    TaskHandle_t lcdTaskHandle = NULL;


    // FreeRTOS task functions
    void lcdTaskRun(void *pvParameters) {
        uint32_t ulNotifiedValue;

        while (1) {
            if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotifiedValue, portMAX_DELAY) == pdTRUE) {
                if (ulNotifiedValue & UPDATE_LCD_ANIMATION_BYTE) {
                    // ANIMATION UPDATE
                    lcdUI.displayPlayingAnimation();
                }

                if (ulNotifiedValue & UPDATE_LCD_TEXT_BYTE) {
                    // TEXT UPDATE
                    // TODO: implement change sensing to avoid rerendering
                    PlaybackState playbackStateBuffer = stateBuffer.readStateBuffer();  // read the data from global state buffer

                    Serial.println(playbackStateBuffer.artist);

                    // lcdUI.setBacklight(playbackStateBuffer.state);

                    // lcdUI.clearDisplaySection(0, 0, 14, 2);  // clear everything except animation

                    // lcdUI.displayText(playbackStateBuffer.title, 0, 14);
                    // lcdUI.displayText(playbackStateBuffer.artist, 1, 14);
                }
            }
        }
    }

    static void lcdTaskWrapper(void* pvParameters) {
        UserInterface* instance = static_cast<UserInterface*>(pvParameters);

        instance->lcdTaskRun(pvParameters);
    }

    void triggerActionFromIsr(uint8_t actionByte) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (lcdTaskHandle != NULL) {
            xTaskNotifyFromISR(lcdTaskHandle, actionByte, eSetBits, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

public:
    UserInterface (LiquidCrystal_I2C* lcdInstance) : lcd(lcdInstance) {
        lcd = lcdInstance;
    }

    void begin() {
        // begin the lcd UI
        lcdUI.begin(lcd);
        lcdUI.displayStandBy();

        // create freertos task
        xTaskCreate (
        lcdTaskWrapper,         // funciton realizing the task
        "LCD_control_task",     // text task name
        2048,                   // stack size (bytes)
        this,                   // args for task
        0,                      // task priority (higher is more important)
        &lcdTaskHandle          // task handle
        );
    }

    void updateAnimation() {
        lcdUI.incrementPlayingAnimation();
        triggerActionFromIsr(UPDATE_LCD_ANIMATION_BYTE);
    }

    void updateText() {
        triggerActionFromIsr(UPDATE_LCD_TEXT_BYTE);
    }
};

#endif
