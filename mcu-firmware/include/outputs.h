#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define CLEARING_SPACES "                    "
#define MAX_CLEARING_SPACES 20

extern LiquidCrystal_I2C lcd;

extern const uint8_t playingAnimationFrames[4][4][8];

class UserInterface{
private:
    LiquidCrystal_I2C* lcd;

    void loadPlayingFrameChars(int frameNum) {
        lcd->createChar(0, const_cast<uint8_t*>(playingAnimationFrames[frameNum][0]));
        lcd->createChar(1, const_cast<uint8_t*>(playingAnimationFrames[frameNum][1]));
        lcd->createChar(2, const_cast<uint8_t*>(playingAnimationFrames[frameNum][2]));
        lcd->createChar(3, const_cast<uint8_t*>(playingAnimationFrames[frameNum][3]));
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

        loadPlayingFrameChars(0);
    }

    void displayPlayingAnimation(int x=14, int y=0) {
        lcd->setCursor(x, y);
        lcd->write(0);
        lcd->setCursor(x+1, y);
        lcd->write(1);
        lcd->setCursor(x, y+1);
        lcd->write(2);
        lcd->setCursor(x+1, y+1);
        lcd->write(3);
    }

    void displayText(String text, int row, int maxChars = 16) {
        lcd->setCursor(0, row);
        lcd->print(trimString(text, maxChars));
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
