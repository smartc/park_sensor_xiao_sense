#include "position_sensor.h"
#include "helpers.h"
#include "flash_storage.h" 
#include <math.h>

// Use the same approach as the working example
#include "LSM6DS3.h"
#include "Wire.h"

// Create IMU instance exactly like the working example
LSM6DS3 imu(I2C_MODE, 0x6A);

// Calibration offsets for LSM6DS3TR-C
float ax_offset = 0.0, ay_offset = 0.0, az_offset = 0.0;
float gx_offset = 0.0, gy_offset = 0.0, gz_offset = 0.0;

// FIXED: Much lighter filtering for better responsiveness
// Changed from 0.8 to 0.2 - now uses 80% new data, 20% old filtered data
float alpha = 0.2;  // Removed const so we can change it
float filtered_ax = 0, filtered_ay = 0, filtered_az = 0;

// Add option to disable filtering entirely for testing
bool use_filtering = true;

bool initPositionSensor() {
    Debug.println("Initializing built-in LSM6DS3TR-C IMU on XIAO Sense Plus...");
    Debug.println("Using Seeed Arduino LSM6DS3 library (working example approach)");
    
    // Initialize exactly like the working example
    if (imu.begin() != 0) {
        Debug.println("✗ Device error - IMU initialization failed");
        return false;
    } else {
        Debug.println("✓ Device OK! - IMU initialized successfully");
    }
    
    // Test basic readings to make sure it's working
    Debug.println("Testing basic sensor readings...");
    float ax = imu.readFloatAccelX();
    float ay = imu.readFloatAccelY(); 
    float az = imu.readFloatAccelZ();
    float temp = imu.readTempC();
    
    Debug.println("Initial readings:");
    Debug.println("  Accel X: " + String(ax, 4));
    Debug.println("  Accel Y: " + String(ay, 4));
    Debug.println("  Accel Z: " + String(az, 4));
    Debug.println("  Temperature: " + String(temp, 2) + "°C");
    
    if (ax == 0 && ay == 0 && az == 0) {
        Debug.println("⚠ All accelerometer readings are zero - sensor may not be working");
        return false;
    }
    
    Debug.println("✓ Sensor is providing valid data");
    Debug.println("Filter alpha: " + String(alpha, 2) + " (lower = more responsive)");
    Debug.println("Filter enabled: " + String(use_filtering ? "YES" : "NO"));
    
    // Load or perform calibration
    if (hasStoredCalibration()) {
        Debug.println("Found stored calibration data, loading...");
        loadCalibration();
        Debug.println("Stored calibration loaded successfully!");
    } else {
        Debug.println("No stored calibration found, performing initial calibration...");
        calibrateSensor();
        saveCalibration();
        Debug.println("Initial calibration complete and saved!");
    }

    Debug.println("✓ Built-in LSM6DS3TR-C IMU ready!");
    return true;
}

void calibrateSensor() {
    Debug.println("Calibrating LSM6DS3TR-C... Keep XIAO Sense Plus still!");
    Debug.println("This will take about 5 seconds...");
    
    float ax_sum = 0, ay_sum = 0, az_sum = 0;
    float gx_sum = 0, gy_sum = 0, gz_sum = 0;
    
    int successful_reads = 0;
    int total_samples = 500;  // Was CALIBRATION_SAMPLES
    int sample_delay = 10;    // Was CALIBRATION_SAMPLE_DELAY
    
    for (int i = 0; i < total_samples; i++) {
        // Read using Seeed library
        float ax = imu.readFloatAccelX();
        float ay = imu.readFloatAccelY();
        float az = imu.readFloatAccelZ();
        float gx = imu.readFloatGyroX();
        float gy = imu.readFloatGyroY();
        float gz = imu.readFloatGyroZ();
        
        // Check if readings are valid
        if (!isnan(ax) && !isnan(ay) && !isnan(az) && 
            !isnan(gx) && !isnan(gy) && !isnan(gz) &&
            abs(ax) < 10 && abs(ay) < 10 && abs(az) < 10) {
            
            ax_sum += ax;
            ay_sum += ay;
            az_sum += az;
            gx_sum += gx;
            gy_sum += gy;
            gz_sum += gz;
            
            successful_reads++;
        } else {
            Debug.println("Warning: Invalid reading during calibration at sample " + String(i));
        }
        
        delay(sample_delay);
        
        if (i % 50 == 0) {
            Debug.println("Calibration progress: " + String((i * 100) / total_samples) + "% (" + String(successful_reads) + " successful reads)");
        }
        
        yield(); // For mbed core
    }
    
    debugSensorCalibration(total_samples, successful_reads);
    
    // Calculate average offsets
    if (successful_reads > 0) {
        ax_offset = ax_sum / successful_reads;
        ay_offset = ay_sum / successful_reads;
        az_offset = (az_sum / successful_reads) - 1.0; // Subtract 1g (gravity)
        gx_offset = gx_sum / successful_reads;
        gy_offset = gy_sum / successful_reads;
        gz_offset = gz_sum / successful_reads;
    }
    
    Debug.println("LSM6DS3TR-C (XIAO Sense Plus) - Accelerometer offsets: X=" + String(ax_offset, 4) + 
                  " Y=" + String(ay_offset, 4) + " Z=" + String(az_offset, 4));
    Debug.println("LSM6DS3TR-C (XIAO Sense Plus) - Gyroscope offsets: X=" + String(gx_offset, 4) + 
                  " Y=" + String(gy_offset, 4) + " Z=" + String(gz_offset, 4));
}

bool readPosition(float &pitch, float &roll) {
    // Read accelerometer data using the same methods as working example
    float ax = imu.readFloatAccelX();
    float ay = imu.readFloatAccelY();
    float az = imu.readFloatAccelZ();
    
    // Check if readings are valid
    if (isnan(ax) || isnan(ay) || isnan(az)) {
        Debug.println("Error: Invalid readings from LSM6DS3TR-C");
        return false;
    }
    
    // Apply calibration offsets
    ax -= ax_offset;
    ay -= ay_offset;
    az -= az_offset;
    
    // FIXED: Apply lighter filtering or option to disable
    float final_ax, final_ay, final_az;
    
    if (use_filtering) {
        // Much lighter low-pass filter for better responsiveness
        filtered_ax = alpha * filtered_ax + (1.0 - alpha) * ax;
        filtered_ay = alpha * filtered_ay + (1.0 - alpha) * ay;
        filtered_az = alpha * filtered_az + (1.0 - alpha) * az;
        
        final_ax = filtered_ax;
        final_ay = filtered_ay;
        final_az = filtered_az;
    } else {
        // No filtering - use raw calibrated values
        final_ax = ax;
        final_ay = ay;
        final_az = az;
    }
    
    // Calculate magnitude and validate
    float magnitude = sqrt(final_ax * final_ax + final_ay * final_ay + final_az * final_az);
    if (magnitude < 0.1) {  // Was POSITION_MAGNITUDE_THRESHOLD
        Debug.println("Warning: Low accelerometer magnitude detected: " + String(magnitude, 4));
        return false;
    }
    
    // IMPROVED: Better pitch and roll calculation with proper handling
    // Standard aerospace convention pitch and roll calculations
    pitch = atan2(-final_ax, sqrt(final_ay * final_ay + final_az * final_az)) * 180.0 / PI;
    roll = atan2(final_ay, final_az) * 180.0 / PI;
    
    // Additional debug output every 2 seconds when debug enabled
    /*
    static unsigned long lastDebugOutput = 0;
    if (millis() - lastDebugOutput >= 2000) {
        Debug.println("Sensor data - Raw: ax=" + String(ax, 3) + " ay=" + String(ay, 3) + " az=" + String(az, 3));
        Debug.println("Sensor data - Final: ax=" + String(final_ax, 3) + " ay=" + String(final_ay, 3) + " az=" + String(final_az, 3));
        Debug.println("Calculated angles - Pitch=" + String(pitch, 2) + "° Roll=" + String(roll, 2) + "°");
        Debug.println("Filter enabled: " + String(use_filtering ? "YES" : "NO") + " Alpha: " + String(alpha, 2));
        lastDebugOutput = millis();
    }
    */

    // Validate calculated values
    if (!isValidPosition(pitch, roll)) {
        Debug.println("Error: Invalid pitch/roll values calculated from LSM6DS3TR-C");
        Debug.println("Pitch: " + String(pitch, 4) + " Roll: " + String(roll, 4));
        return false;
    }
    
    return true;
}

// Simple function to toggle filtering for testing
void setFiltering(bool enable) {
    use_filtering = enable;
    Debug.println("Filtering " + String(enable ? "ENABLED" : "DISABLED"));
    if (enable) {
        Debug.println("Filter alpha: " + String(alpha, 2) + " (lower = more responsive)");
    } else {
        Debug.println("Using raw sensor data (maximum responsiveness)");
    }
}

// Simple function to adjust filter strength
void setFilterAlpha(float new_alpha) {
    if (new_alpha >= 0.0 && new_alpha <= 1.0) {
        alpha = new_alpha;
        Debug.println("Filter alpha set to: " + String(alpha, 2));
        Debug.println("(0.0 = no filtering, 1.0 = maximum filtering)");
    } else {
        Debug.println("Invalid alpha value. Must be 0.0 to 1.0");
    }
}

bool hasStoredCalibration() {
    // Check if calibration values exist in our simple storage
    return (loadFloatPreference("cal_ax_offset", -999.0) != -999.0 && 
            loadFloatPreference("cal_ay_offset", -999.0) != -999.0 && 
            loadFloatPreference("cal_az_offset", -999.0) != -999.0);
}

void loadCalibration() {
    ax_offset = loadFloatPreference("cal_ax_offset", 0.0);
    ay_offset = loadFloatPreference("cal_ay_offset", 0.0);
    az_offset = loadFloatPreference("cal_az_offset", 0.0);
    gx_offset = loadFloatPreference("cal_gx_offset", 0.0);
    gy_offset = loadFloatPreference("cal_gy_offset", 0.0);
    gz_offset = loadFloatPreference("cal_gz_offset", 0.0);
    
    Debug.println("LSM6DS3TR-C (XIAO Sense Plus) - Loaded calibration offsets:");
    Debug.println("Accelerometer: X=" + String(ax_offset, 4) + 
                  " Y=" + String(ay_offset, 4) + " Z=" + String(az_offset, 4));
    Debug.println("Gyroscope: X=" + String(gx_offset, 4) + 
                  " Y=" + String(gy_offset, 4) + " Z=" + String(gz_offset, 4));
}

void saveCalibration() {
    saveFloatPreference("cal_ax_offset", ax_offset);
    saveFloatPreference("cal_ay_offset", ay_offset);
    saveFloatPreference("cal_az_offset", az_offset);
    saveFloatPreference("cal_gx_offset", gx_offset);
    saveFloatPreference("cal_gy_offset", gy_offset);
    saveFloatPreference("cal_gz_offset", gz_offset);
    
    // Also save a timestamp for reference
    saveFloatPreference("cal_timestamp", millis());
    
    Debug.println("LSM6DS3TR-C (XIAO Sense Plus) - Calibration data saved");
    // Remove the isFlashStorageAvailable() check, just always show RAM message
    Debug.println("⚠ Calibration saved to enhanced RAM only - will be lost on power cycle");
    Debug.println("Note: Enhanced RAM storage includes validation and checksums");
    Debug.println("Accelerometer offsets: X=" + String(ax_offset, 4) + 
                  " Y=" + String(ay_offset, 4) + " Z=" + String(az_offset, 4));
    Debug.println("Gyroscope offsets: X=" + String(gx_offset, 4) + 
                  " Y=" + String(gy_offset, 4) + " Z=" + String(gz_offset, 4));
}