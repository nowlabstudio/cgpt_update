#include "Arduino.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Setup comp lete.");
}

void loop() {
    Serial.println("Loop running...");
    delay(1000);
}
