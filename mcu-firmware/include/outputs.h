#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <FreeRTOS.h>
#include <task.h>

#define CLEARING_SPACES "                    "
#define MAX_CLEARING_SPACES 20

#define ANIMATION_TOTAL_FRAMES 4

#define UPDATE_LCD_ANIMATION_BYTE 0x01
#define UPDATE_LCD_TEXT_BYTE 0x02

extern LiquidCrystal_I2C lcd;

extern const uint8_t playingAnimationFrames[2][4][8];

class LcdFreeRTOSTask {
private:
    TaskHandle_t lcdTaskHandle;

    void lcdTaskRun(void *pvParameters) {
        uint32_t ulNotifiedValue;

        while (1) {
            if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotifiedValue, portMAX_DELAY) == pdTRUE) {
                if (ulNotifiedValue & UPDATE_LCD_ANIMATION_BYTE) {
                    // TODO: write lcd animation update
                    Serial.println("aohoi");
                }

                if (ulNotifiedValue & UPDATE_LCD_TEXT_BYTE) {
                    // TODO: write lcd text update
                    Serial.println("agag");
                }
            }
        }
    }

    static void lcdTaskWrapper(void* pvParameters) {
        LcdFreeRTOSTask* instance = static_cast<LcdFreeRTOSTask*>(pvParameters);

        instance->lcdTaskRun(pvParameters);
    }

public:
    void begin() {
        xTaskCreate (
        lcdTaskWrapper,         // funciton realizing the task
        "LCD_control_task",     // text task name
        2048,                   // stack size (bytes)
        NULL,                   // args for task
        2,                      // task priority (higher is more important)
        &lcdTaskHandle          // task handle
        );
    }

    void triggerActionFromIsr(uint8_t actionByte) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (lcdTaskHandle != NULL) {
            // Ustaw bit zdarzenia A bez nadpisywania pozostałych bitów
            xTaskNotifyFromISR(lcdTaskHandle, actionByte, eSetBits, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }


};


class UserInterface{
private:
    LiquidCrystal_I2C* lcd;

    uint8_t currentPlayingAnimationFrame = 0;
    
    LcdFreeRTOSTask lcdTask;

    int animationX = 14;
    int animationY = 0;

    void loadPlayingFrameChars() {
        // load regular vinyl quarters
        lcd->createChar(0, const_cast<uint8_t*>(playingAnimationFrames[0][0]));
        lcd->createChar(1, const_cast<uint8_t*>(playingAnimationFrames[0][1]));
        lcd->createChar(2, const_cast<uint8_t*>(playingAnimationFrames[0][2]));
        lcd->createChar(3, const_cast<uint8_t*>(playingAnimationFrames[0][3]));
        // load marked vinyl quarters
        lcd->createChar(4, const_cast<uint8_t*>(playingAnimationFrames[0][4]));
        lcd->createChar(5, const_cast<uint8_t*>(playingAnimationFrames[0][5]));
        lcd->createChar(6, const_cast<uint8_t*>(playingAnimationFrames[0][6]));
        lcd->createChar(7, const_cast<uint8_t*>(playingAnimationFrames[0][7]));

        lcd->home();
    }

    void displayPlayingAnimationFrame(int x, int y, int frameNum) {
        // this function defines how animation frames look
        // it does not have any strict structure, all it has to do 
        // is display the given frame 

        // ANIMATION SPECIFIC CODE
        if (frameNum == 1 || frameNum == 3) {
            lcd->setCursor(x, y);
            lcd->write(0);
            lcd->write(1);
            lcd->setCursor(x, y+1);
            lcd->write(2);
            lcd->write(3);
        } else if (frameNum == 0) {
            lcd->setCursor(x, y);
            lcd->write(4);
            lcd->write(1);
            lcd->setCursor(x, y+1);
            lcd->write(2);
            lcd->write(7);
        } else {
            lcd->setCursor(x, y);
            lcd->write(0);
            lcd->write(5);
            lcd->setCursor(x, y+1);
            lcd->write(6);
            lcd->write(3);
        }
        // ANIMATION SPECIFIC CODE
    }

    String trimString(const String& str, size_t maxLength) {
        if (maxLength == 0) {
            return "";
        }
        if (str.length() > maxLength) {
            return str.substring(0, maxLength - 1) + "-";
        }
        return str;
    }

public:
    UserInterface(LiquidCrystal_I2C* lcdInstance) : lcd(lcdInstance) {
        lcd = lcdInstance;
    }

    void begin() {
        // wire begin
        Wire.setSDA(16);
        Wire.setSCL(17);
        Wire.begin();

        lcd->init();
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->backlight();

        loadPlayingFrameChars();

        // start the freertos task
        lcdTask.begin();
    }

    void displayText(String text, int row, int maxChars = 16) {
        lcd->setCursor(0, row);
        lcd->print(trimString(text, maxChars));
    }

    void displayPlayingAnimation(int x=14, int y=0) {
        displayPlayingAnimationFrame(x, y, currentPlayingAnimationFrame);
        animationX = x;
        animationY = y;
    }

    void incrementPlayingAnimation() {
        currentPlayingAnimationFrame++;
        if (currentPlayingAnimationFrame > ANIMATION_TOTAL_FRAMES - 1) {
            currentPlayingAnimationFrame = 0; }  // reset the counter

        lcdTask.triggerActionFromIsr(UPDATE_LCD_ANIMATION_BYTE);
    }

    void clearDisplay() {
        lcd->clear();
    }

    void clearDisplaySection(int x, int y, int dx, int dy) {
        if (dx > MAX_CLEARING_SPACES) {
            dx = MAX_CLEARING_SPACES;
        }

        for (int i=0; i<y; i++) {
            lcd->setCursor(x, i);
            lcd->print(CLEARING_SPACES[MAX_CLEARING_SPACES - dx]);  // read only desired amount from the end
        }
    }
};

#endif
