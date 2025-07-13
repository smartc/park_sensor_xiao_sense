#include "led_control.h"

// LED state tracking
static bool currentLEDState = false;
static bool lastParkedState = false;

void initLED() {
    // Go back to using the RED LED (LED_BUILTIN) for park status
    // This should be more reliable
    
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Start with LED off (active low, not parked initially)
    Debug.println("nRF52840 - BLUE LED initialized on GPIO " + String(LED_BLUE));
    
    // Quick test blink to show LED is working
    digitalWrite(LED_BLUE, LOW);   // ON
    delay(200);
    digitalWrite(LED_BLUE, HIGH);  // OFF
    delay(200);
    digitalWrite(LED_BLUE, LOW);   // ON
    delay(200);
    digitalWrite(LED_BLUE, HIGH);  // OFF
    
    Debug.println("nRF52840 - RED LED test blink complete (2 blinks)");
    Debug.println("LED Behavior: BLUE ON = PARKED, BLUE OFF = NOT PARKED (active low)");
}

void updateLEDStatus(bool isParked) {
    // FIXED: LED is active LOW on XIAO nRF52840 Sense
    // LOW = LED ON, HIGH = LED OFF
    
    // Use the reliable RED LED (LED_BUILTIN)
    // LED ON when parked, LED OFF when not parked
    digitalWrite(LED_BLUE, isParked ? LOW : HIGH);
    
    currentLEDState = isParked;
    
    /*
    // Show status for debugging
    Debug.println("=== LED STATUS UPDATE (ACTIVE LOW) ===");
    Debug.println("  Input isParked: " + String(isParked ? "TRUE" : "FALSE"));
    Debug.println("  Park status: " + String(isParked ? "PARKED" : "NOT PARKED"));
    Debug.println("  BLUE LED should be: " + String(isParked ? "ON" : "OFF"));
    Debug.println("  BLUE LED set to: " + String(isParked ? "LOW (ON)" : "HIGH (OFF)"));
    Debug.println("  LED_BLUE pin: " + String(LED_BLUE));
    Debug.println("  Note: LED is active LOW on this board");
    Debug.println("=======================================");
    */
    
    lastParkedState = isParked;
}

void ledErrorPattern() {
    Debug.println("nRF52840 - Starting LED error pattern - sensor initialization failed");
    Debug.println("Using BLUE LED for error indication (same as park status)");
    
    // Use the same RED LED for error pattern
    // Run error pattern continuously until reset
    while (true) {
        // 3 red blinks
        ledBlink(3);
        
        // Pause
        delay(1000);  // LED_ERROR_CYCLE_PAUSE equivalent
        
        // 3 more red blinks
        ledBlink(3);
        
        // Longer pause before repeating
        delay(2000);  // LED_ERROR_CYCLE_PAUSE * 2 equivalent
        
        // Yield to other tasks (important for nRF52840)
        yield();
    }
}

void ledBlink(int count, int duration, int pause) {
    // Use RED LED for blinking (same as park status)
    // Active low: LOW = ON, HIGH = OFF
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_BLUE, LOW);   // ON
        delay(duration);
        digitalWrite(LED_BLUE, HIGH);  // OFF
        
        // Don't add pause after the last blink
        if (i < count - 1) {
            delay(pause);
        }
    }
}