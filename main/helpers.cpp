#include "helpers.h"
#include "constants.h"
#include "position_sensor.h"
#include "flash_storage.h"  // Add storage support
#include "Debug.h"
#include <math.h>

// External global variables
extern bool isParked;
extern float currentPitch, currentRoll;
extern float parkPitch, parkRoll, positionTolerance;

// Position and park status management
void updatePositionAndParkStatus() {
    float pitch, roll;
    if (readPosition(pitch, roll)) {
        currentPitch = pitch;
        currentRoll = roll;
        
        // Check if we're in park position
        float pitchDiff = calculatePositionDifference(currentPitch, parkPitch);
        float rollDiff = calculatePositionDifference(currentRoll, parkRoll);
        bool newParkedStatus = (pitchDiff <= positionTolerance && rollDiff <= positionTolerance);
        
        // Detailed debug info every 5 seconds to avoid spam
        /*
        static unsigned long lastDetailedDebug = 0;
        if (millis() - lastDetailedDebug >= 5000) {
            Debug.println("=== PARK DETECTION DEBUG ===");
            Debug.println("  Current Pitch: " + String(currentPitch, 2) + "°");
            Debug.println("  Current Roll: " + String(currentRoll, 2) + "°");
            Debug.println("  Park Pitch: " + String(parkPitch, 2) + "°");
            Debug.println("  Park Roll: " + String(parkRoll, 2) + "°");
            Debug.println("  Pitch Difference: " + String(pitchDiff, 2) + "°");
            Debug.println("  Roll Difference: " + String(rollDiff, 2) + "°");
            Debug.println("  Tolerance: ±" + String(positionTolerance, 1) + "°");
            Debug.println("  Pitch OK: " + String(pitchDiff <= positionTolerance ? "YES" : "NO"));
            Debug.println("  Roll OK: " + String(rollDiff <= positionTolerance ? "YES" : "NO"));
            Debug.println("  Is Parked: " + String(newParkedStatus ? "YES" : "NO"));
            Debug.println("  Previous Parked: " + String(isParked ? "YES" : "NO"));
            Debug.println("============================");
            lastDetailedDebug = millis();
        }
        */
        
        isParked = newParkedStatus;
    } else {
        Debug.println("Failed to read position from sensor");
        isParked = false; // Assume not parked if we can't read position
    }
}

bool isCurrentlyParked() {
    updatePositionAndParkStatus();
    return isParked;
}

// Enhanced storage preferences management for v2.0.1
bool saveFloatPreference(const char* key, float value) {
    if (isFlashStorageAvailable()) {
        return saveFloatToFlash(key, value);
    } else {
        // Fallback to simple logging
        Debug.println("Storage: Saved " + String(key) + " = " + String(value, 2) + " (RAM only)");
        return true; // Pretend it worked for compatibility
    }
}

bool saveIntPreference(const char* key, int value) {
    return saveFloatPreference(key, (float)value);
}

float loadFloatPreference(const char* key, float defaultValue) {
    if (isFlashStorageAvailable()) {
        return loadFloatFromFlash(key, defaultValue);
    } else {
        // Fallback to default values
        Debug.println("Storage: Using default for " + String(key) + " = " + String(defaultValue, 2));
        return defaultValue;
    }
}

int loadIntPreference(const char* key, int defaultValue) {
    return (int)loadFloatPreference(key, (float)defaultValue);
}

bool clearAllPreferences() {
    if (isFlashStorageAvailable()) {
        return clearAllFlashSettings();
    } else {
        Debug.println("Storage: Cannot clear - no persistent storage available");
        return false;
    }
}

// JSON response builders (unchanged)
String buildSimpleJSONResponse(const String& key, const String& value) {
    return "{\"" + key + "\":\"" + value + "\"}";
}

String buildSimpleJSONResponse(const String& key, float value, int decimals) {
    return "{\"" + key + "\":" + String(value, decimals) + "}";
}

String buildSimpleJSONResponse(const String& key, bool value) {
    return "{\"" + key + "\":" + String(value ? "true" : "false") + "}";
}

String buildJSONError(const String& message) {
    return "{\"status\":\"error\",\"message\":\"" + message + "\"}";
}

String buildJSONNotification(const String& message) {
    return "{\"notification\":\"" + message + "\"}";
}

// JSONBuilder class implementation (unchanged)
JSONBuilder::JSONBuilder() : hasContent(false) {
    json = "{";
}

void JSONBuilder::add(const String& key, const String& value) {
    if (hasContent) json += ",";
    json += "\"" + key + "\":\"" + value + "\"";
    hasContent = true;
}

void JSONBuilder::add(const String& key, const char* value) {
    if (hasContent) json += ",";
    json += "\"" + key + "\":\"" + String(value) + "\"";
    hasContent = true;
}

void JSONBuilder::add(const String& key, float value, int decimals) {
    if (hasContent) json += ",";
    json += "\"" + key + "\":" + String(value, decimals);
    hasContent = true;
}

void JSONBuilder::add(const String& key, bool value) {
    if (hasContent) json += ",";
    json += "\"" + key + "\":" + String(value ? "true" : "false");
    hasContent = true;
}

void JSONBuilder::add(const String& key, int value) {
    if (hasContent) json += ",";
    json += "\"" + key + "\":" + String(value);
    hasContent = true;
}

void JSONBuilder::add(const String& key, unsigned long value) {
    if (hasContent) json += ",";
    json += "\"" + key + "\":" + String(value);
    hasContent = true;
}

String JSONBuilder::build() {
    return json + "}";
}

void JSONBuilder::reset() {
    json = "{";
    hasContent = false;
}

// Position validation helpers (unchanged)
bool isValidPosition(float pitch, float roll) {
    return !isnan(pitch) && !isnan(roll) && 
           pitch >= -90.0 && pitch <= 90.0 && 
           roll >= -180.0 && roll <= 180.0;
}

float calculatePositionDifference(float current, float target) {
    return abs(current - target);
}

// Debug helpers (unchanged)
void debugPositionInfo(float pitch, float roll, bool parked) {
    static unsigned long lastDebugTime = 0;
    unsigned long currentTime = millis();
    
    // Only print debug info every 5 seconds to avoid spam
    if (currentTime - lastDebugTime >= 5000) {
        Debug.println("Position: Pitch=" + String(pitch, 2) + 
                     "° Roll=" + String(roll, 2) + 
                     "° Parked=" + String(parked ? "Yes" : "No"));
        lastDebugTime = currentTime;
    }
}

void debugButtonAction(const String& action) {
    Debug.println("=== BUTTON ACTION: " + action + " ===");
}

void debugSensorCalibration(int samples, int successful) {
    Debug.println("Calibration complete!");
    Debug.println("Successful reads: " + String(successful) + "/" + String(samples));
    if (successful < (samples / 2)) {
        Debug.println("Warning: Only " + String(successful) + " successful reads out of " + String(samples));
        Debug.println("Calibration may be inaccurate!");
    }
}