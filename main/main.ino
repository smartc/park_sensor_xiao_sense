// Telescope Park Sensor v2.0.1 - XIAO Sense Edition (mbed core)
// XIAO nRF52840 Sense with Built-in LSM6DS3TR-C IMU
// Software Interface Only - No External Components Required
// Enhanced storage system for v2.0.1

#include "Debug.h"
#include "constants.h"
#include "helpers.h"
#include "position_sensor.h"
#include "serial_interface.h"
#include "led_control.h"
#include "flash_storage.h"

// Device Information definitions (updated to v2.0.1)
const char* DEVICE_MANUFACTURER = "Corey Smart";
const char* DEVICE_VERSION = "2.0.1";
const char* DEVICE_NAME = "Telescope Park Sensor XIAO Sense";

// Position variables
bool isParked = false;
float currentPitch = 0.0;
float currentRoll = 0.0;
float parkPitch = 0.0;
float parkRoll = 0.0;
float positionTolerance = 2.0;  // DEFAULT_POSITION_TOLERANCE equivalent

// Timing variables
unsigned long lastSensorRead = 0;

// Function prototypes
void loadDeviceSettings();

void setup() {
    // Initialize serial interface FIRST
    initSerial();
    
    Debug.println("Starting Telescope Park Sensor v" + String(DEVICE_VERSION) + " for XIAO Sense...");
    Debug.println("Platform: XIAO nRF52840 Sense (mbed core)");
    Debug.println("IMU: Built-in LSM6DS3TR-C");
    Debug.println("Interface: Software Only (No External Components)");
    
    // Initialize storage system
    if (initFlashStorage()) {
        Debug.println("✓ Persistent storage initialized - settings will persist across reboots");
    } else {
        Debug.println("⚠ Using RAM storage - settings will be lost on reboot");
    }
    
    // Initialize LED (fixed active-low logic)
    initLED();
    
    // Load saved settings
    loadDeviceSettings();
    
    // Initialize position sensor (built-in LSM6DS3TR-C)
    if (!initPositionSensor()) {
        Debug.println("Failed to initialize built-in IMU!");
        Serial.println(buildJSONError("Built-in IMU initialization failed - check hardware"));
        
        // Enter LED error pattern (this will loop forever)
        ledErrorPattern();
    }
    
    Debug.println("Setup complete!");
    Serial.println("Device ready - type <00> for commands");
    Serial.println("XIAO Sense v2.0.1 features: Built-in IMU, Software interface, Enhanced storage");
    
    // Initial position reading and LED update
    updatePositionAndParkStatus();
    updateLEDStatus(isParked);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Handle serial commands
    handleSerialCommands();
    
    // Read sensor periodically (every 50ms)
    if (currentMillis - lastSensorRead >= 50) {
        updatePositionAndParkStatus();
        updateLEDStatus(isParked);
        lastSensorRead = currentMillis;
    }
    
    // Small delay to prevent issues
    delay(10);
    
    // Yield to allow background tasks (important for mbed)
    yield();
}

void loadDeviceSettings() {
    parkPitch = loadFloatPreference("parkPitch", 0.0);
    parkRoll = loadFloatPreference("parkRoll", 0.0);
    positionTolerance = loadFloatPreference("tolerance", 2.0);
    
    Debug.println("Loaded park position: Pitch=" + String(parkPitch, 2) + 
                  "° Roll=" + String(parkRoll, 2) + "° Tolerance=±" + String(positionTolerance, 1) + "°");
    
    if (isFlashStorageAvailable()) {
        Debug.println("Settings loaded from persistent storage");
    } else {
        Debug.println("Using default/RAM settings");
    }
}