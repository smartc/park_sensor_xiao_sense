#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include "Arduino.h"

// Storage structure for our settings
struct TelescopeSettings {
    uint32_t magic;           // Magic number to verify valid data
    float parkPitch;          // Park position pitch
    float parkRoll;           // Park position roll
    float tolerance;          // Position tolerance
    float cal_ax_offset;      // Calibration offsets
    float cal_ay_offset;
    float cal_az_offset;
    float cal_gx_offset;
    float cal_gy_offset;
    float cal_gz_offset;
    uint32_t cal_timestamp;   // Calibration timestamp
    uint32_t checksum;        // Simple checksum for data integrity
};

// Function prototypes
bool initFlashStorage();
bool saveSettingsToFlash(const TelescopeSettings& settings);
bool loadSettingsFromFlash(TelescopeSettings& settings);
bool eraseSettingsFlash();
uint32_t calculateChecksum(const TelescopeSettings& settings);
bool isFlashStorageAvailable();

// Helper functions for existing interface
bool saveFloatToFlash(const char* key, float value);
float loadFloatFromFlash(const char* key, float defaultValue);
bool clearAllFlashSettings();

#endif // FLASH_STORAGE_H