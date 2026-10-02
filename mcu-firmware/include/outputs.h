#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

extern const uint8_t playingAnimationFrames[4][4][8];

void begin();


class UserInterface{
private:
    LiquidCrystal_I2C lcd;

public:
    UserInterface(LiquidCrystal_I2C lcdInstance) : lcd(lcdInstance) {
        lcd = lcdInstance;
    }

    void begin() {
        lcd.init();
        lcd.clear();
        lcd.noBacklight();
        lcd.display();
    }
};


#endif
