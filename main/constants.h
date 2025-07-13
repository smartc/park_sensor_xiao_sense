#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifndef CONSTANTS_H
#define CONSTANTS_H

// Device Information - extern declarations
extern const char* DEVICE_MANUFACTURER;
extern const char* DEVICE_VERSION;
extern const char* DEVICE_NAME;

// GPIO Pin Definitions for XIAO nRF52840 Sense (mbed core)
// Using built-in LED - mbed core defines LED_BUILTIN (Red LED)
#define LED_PIN LED_BLUE    // Built-in Red LED for park status

// IMU is built-in - no pin definitions needed
// LSM6DS3TR-C is connected via internal I2C

// Serial Communication
#define SERIAL_BAUD_RATE 115200
#define SERIAL_TIMEOUT 1000
#define MAX_COMMAND_LENGTH 32

// LED Timing Constants
#define LED_BLINK_DURATION 200         // 200ms on/off for blinks
#define LED_BLINK_PAUSE 300            // 300ms pause between blinks
#define LED_ERROR_CYCLE_PAUSE 1000     // 1 second pause between error cycles

// Sensor Timing Constants
#define SENSOR_READ_INTERVAL 50        // Read sensor every 50ms (20Hz)
#define CALIBRATION_SAMPLES 500        // Number of samples for calibration
#define CALIBRATION_SAMPLE_DELAY 10    // Delay between calibration samples

// Default Values
#define DEFAULT_POSITION_TOLERANCE 2.0 // Default tolerance in degrees
#define POSITION_MAGNITUDE_THRESHOLD 0.1 // Minimum accelerometer magnitude

// LSM6DS3TR-C Constants (built-in IMU)
#define LSM6DS3_ADDRESS 0x6A           // I2C address for LSM6DS3TR-C
#define I2C_CLOCK_SPEED 100000         // 100kHz I2C clock

// Protocol Characters
#define CMD_START_CHAR '<'
#define CMD_END_CHAR '>'

// Storage Configuration for v2.0.1 - Enhanced storage system
#define USE_ENHANCED_STORAGE true
#define STORAGE_TYPE "Enhanced (RAM-based with v2.0.1 improvements)"

// Bluetooth Configuration (for future use)
#define BLE_DEVICE_NAME "TelescopeParkSensor"
#define BLE_SERVICE_UUID "12345678-1234-1234-1234-123456789abc"

// XIAO Sense Specific Features (v2.0.1)
#define HAS_BUILTIN_IMU true
#define HAS_EXTERNAL_BUTTON false
#define IMU_MODEL "LSM6DS3TR-C"
#define BOARD_MODEL "XIAO nRF52840 Sense"
#define CORE_TYPE "mbed"
#define FIRMWARE_VERSION "2.0.1"

// LED Behavior Documentation
#define LED_BEHAVIOR_DESCRIPTION "Red LED (active low): ON=Parked, OFF=Not Parked"

#endif // CONSTANTS_H

#endif // CONSTANTS_H