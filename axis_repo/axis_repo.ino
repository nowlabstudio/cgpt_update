#include "Arduino.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Set up com p lete.");
}

void loop() {
    Serial.println("Loop running...");
    delay(1000);
}
