#ifndef POSITION_SENSOR_H
#define POSITION_SENSOR_H

#include <Wire.h>
// Use the same library as the working example
#include "LSM6DS3.h"
#include "Debug.h"
#include "constants.h"

// LSM6DS3 object for built-in IMU using Seeed library (same as working example)
extern LSM6DS3 imu;

// Function prototypes
bool initPositionSensor();
bool readPosition(float &pitch, float &roll);
void calibrateSensor();
void loadCalibration();
void saveCalibration();
bool hasStoredCalibration();

// NEW: Filter control functions for improved responsiveness
void setFiltering(bool enable);
void setFilterAlpha(float new_alpha);

// Calibration values (using float for LSM6DS3TR-C)
extern float ax_offset, ay_offset, az_offset;
extern float gx_offset, gy_offset, gz_offset;

// Filter control variables
extern bool use_filtering;
extern float alpha;  // Removed const so we can change it

#endif // POSITION_SENSOR_H