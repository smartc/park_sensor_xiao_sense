#include "position_sensor.h"
#include "helpers.h"
#include <math.h>

// Use the same approach as the working example
#include "LSM6DS3.h"
#include "Wire.h"

// Create IMU instance exactly like the working example
LSM6DS3 imu(I2C_MODE, 0x6A);

// Calibration offsets for LSM6DS3TR-C
float ax_offset = 0.0, ay_offset = 0.0, az_offset = 0.0;
float gx_offset = 0.0, gy_offset = 0.0, gz_offset = 0.0;

// Low-pass filter alpha value (0-1, lower = more filtering)
const float alpha = 0.8;
float filtered_ax = 0, filtered_ay = 0, filtered_az = 0;

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
    
    // Apply low-pass filter to reduce noise
    filtered_ax = alpha * filtered_ax + (1.0 - alpha) * ax;
    filtered_ay = alpha * filtered_ay + (1.0 - alpha) * ay;
    filtered_az = alpha * filtered_az + (1.0 - alpha) * az;
    
    // Calculate magnitude and validate
    float magnitude = sqrt(filtered_ax * filtered_ax + filtered_ay * filtered_ay + filtered_az * filtered_az);
    if (magnitude < 0.1) {  // Was POSITION_MAGNITUDE_THRESHOLD
        Debug.println("Warning: Low accelerometer magnitude detected: " + String(magnitude, 4));
        return false;
    }
    
    // Calculate pitch and roll from accelerometer data
    pitch = atan2(-filtered_ax, sqrt(filtered_ay * filtered_ay + filtered_az * filtered_az)) * 180.0 / PI;
    roll = atan2(filtered_ay, filtered_az) * 180.0 / PI;
    
    // Validate calculated values
    if (!isValidPosition(pitch, roll)) {
        Debug.println("Error: Invalid pitch/roll values calculated from LSM6DS3TR-C");
        return false;
    }
    
    return true;
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
    
    Debug.println("LSM6DS3TR-C (XIAO Sense Plus) - Calibration data saved to memory");
    Debug.println("Note: Settings will be lost on power cycle (mbed core limitation)");
    Debug.println("Accelerometer offsets: X=" + String(ax_offset, 4) + 
                  " Y=" + String(ay_offset, 4) + " Z=" + String(az_offset, 4));
    Debug.println("Gyroscope offsets: X=" + String(gx_offset, 4) + 
                  " Y=" + String(gy_offset, 4) + " Z=" + String(gz_offset, 4));
}