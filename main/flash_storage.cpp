#include "flash_storage.h"
#include "Debug.h"

// Magic number to identify valid settings
#define SETTINGS_MAGIC 0x54454C45  // "TELE" in hex

// Global settings storage
static TelescopeSettings currentSettings;
static bool flashStorageInitialized = false;

bool initFlashStorage() {
    Debug.println("Initializing settings storage...");
    Debug.println("Note: Using simplified storage approach for mbed core compatibility");
    
    // Initialize default settings
    currentSettings.magic = SETTINGS_MAGIC;
    currentSettings.parkPitch = 0.0;
    currentSettings.parkRoll = 0.0;
    currentSettings.tolerance = 2.0;
    currentSettings.cal_ax_offset = 0.0;
    currentSettings.cal_ay_offset = 0.0;
    currentSettings.cal_az_offset = 0.0;
    currentSettings.cal_gx_offset = 0.0;
    currentSettings.cal_gy_offset = 0.0;
    currentSettings.cal_gz_offset = 0.0;
    currentSettings.cal_timestamp = 0;
    currentSettings.checksum = 0;
    
    Debug.println("⚠ Persistent storage not available with current setup");
    Debug.println("Settings will use RAM storage (lost on power cycle)");
    Debug.println("Consider using Adafruit nRF52 board package for persistent storage");
    
    flashStorageInitialized = true;
    return false; // Return false to indicate no persistent storage
}

bool isFlashStorageAvailable() {
    return false; // Not available in this configuration
}

uint32_t calculateChecksum(const TelescopeSettings& settings) {
    // Simple checksum calculation (excluding the checksum field itself)
    uint32_t checksum = 0;
    const uint8_t* data = (const uint8_t*)&settings;
    size_t dataSize = sizeof(TelescopeSettings) - sizeof(uint32_t); // Exclude checksum field
    
    for (size_t i = 0; i < dataSize; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

bool saveSettingsToFlash(const TelescopeSettings& settings) {
    Debug.println("saveSettingsToFlash: Persistent storage not available");
    return false;
}

bool loadSettingsFromFlash(TelescopeSettings& settings) {
    Debug.println("loadSettingsFromFlash: Persistent storage not available");
    return false;
}

bool eraseSettingsFlash() {
    Debug.println("eraseSettingsFlash: Persistent storage not available");
    return false;
}

// Helper functions for the existing preference interface
bool saveFloatToFlash(const char* key, float value) {
    if (!flashStorageInitialized) return false;
    
    String keyStr = String(key);
    
    if (keyStr == "parkPitch") {
        currentSettings.parkPitch = value;
    } else if (keyStr == "parkRoll") {
        currentSettings.parkRoll = value;
    } else if (keyStr == "tolerance") {
        currentSettings.tolerance = value;
    } else if (keyStr == "cal_ax_offset") {
        currentSettings.cal_ax_offset = value;
    } else if (keyStr == "cal_ay_offset") {
        currentSettings.cal_ay_offset = value;
    } else if (keyStr == "cal_az_offset") {
        currentSettings.cal_az_offset = value;
    } else if (keyStr == "cal_gx_offset") {
        currentSettings.cal_gx_offset = value;
    } else if (keyStr == "cal_gy_offset") {
        currentSettings.cal_gy_offset = value;
    } else if (keyStr == "cal_gz_offset") {
        currentSettings.cal_gz_offset = value;
    } else if (keyStr == "cal_timestamp") {
        currentSettings.cal_timestamp = (uint32_t)value;
    } else {
        Debug.println("Unknown preference key: " + keyStr);
        return false;
    }
    
    Debug.println("RAM: Saved " + String(key) + " = " + String(value, 4) + " (not persistent)");
    return true; // Saved to RAM, not persistent
}

float loadFloatFromFlash(const char* key, float defaultValue) {
    if (!flashStorageInitialized) return defaultValue;
    
    String keyStr = String(key);
    float value = defaultValue;
    
    if (keyStr == "parkPitch") {
        value = currentSettings.parkPitch;
    } else if (keyStr == "parkRoll") {
        value = currentSettings.parkRoll;
    } else if (keyStr == "tolerance") {
        value = currentSettings.tolerance;
    } else if (keyStr == "cal_ax_offset") {
        value = currentSettings.cal_ax_offset;
    } else if (keyStr == "cal_ay_offset") {
        value = currentSettings.cal_ay_offset;
    } else if (keyStr == "cal_az_offset") {
        value = currentSettings.cal_az_offset;
    } else if (keyStr == "cal_gx_offset") {
        value = currentSettings.cal_gx_offset;
    } else if (keyStr == "cal_gy_offset") {
        value = currentSettings.cal_gy_offset;
    } else if (keyStr == "cal_gz_offset") {
        value = currentSettings.cal_gz_offset;
    } else if (keyStr == "cal_timestamp") {
        value = (float)currentSettings.cal_timestamp;
    } else {
        Debug.println("Unknown preference key: " + keyStr + ", using default");
        return defaultValue;
    }
    
    Debug.println("RAM: Loaded " + String(key) + " = " + String(value, 4));
    return value;
}

bool clearAllFlashSettings() {
    if (!flashStorageInitialized) return false;
    
    Debug.println("Clearing all RAM settings...");
    
    // Reset to defaults
    currentSettings.magic = SETTINGS_MAGIC;
    currentSettings.parkPitch = 0.0;
    currentSettings.parkRoll = 0.0;
    currentSettings.tolerance = 2.0;
    currentSettings.cal_ax_offset = 0.0;
    currentSettings.cal_ay_offset = 0.0;
    currentSettings.cal_az_offset = 0.0;
    currentSettings.cal_gx_offset = 0.0;
    currentSettings.cal_gy_offset = 0.0;
    currentSettings.cal_gz_offset = 0.0;
    currentSettings.cal_timestamp = 0;
    
    Debug.println("✓ All RAM settings cleared (not persistent)");
    return true;
}