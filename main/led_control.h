#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "Debug.h"
#include "constants.h"
#include "Arduino.h"

// Function prototypes
void initLED();
void updateLEDStatus(bool isParked);
void ledErrorPattern();
void ledBlink(int count, int duration = 200, int pause = 300);

#endif // LED_CONTROL_H