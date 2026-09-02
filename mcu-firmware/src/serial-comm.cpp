#include "serial-comm.h"

void beginSerial() {
    Serial.begin(115200);
    while (!Serial) {;}  // wait for serial to initialize
    delay(1000);
    Serial.println("Serial started");
}



