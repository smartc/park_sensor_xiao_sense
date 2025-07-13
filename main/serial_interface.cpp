#include "serial_interface.h"
#include "position_sensor.h"
#include "helpers.h"

// Serial command buffer
String serialBuffer = "";
bool commandReady = false;
bool inCommand = false;

// External variables
extern bool isParked;
extern float currentPitch, currentRoll;
extern float parkPitch, parkRoll, positionTolerance;

// External device info
extern const char* DEVICE_NAME;
extern const char* DEVICE_VERSION;
extern const char* DEVICE_MANUFACTURER;

void initSerial() {
    Serial.begin(115200);  // SERIAL_BAUD_RATE
    Serial.setTimeout(1000);  // SERIAL_TIMEOUT
    
    // Wait for serial port to be ready (for debugging)
    // Note: nRF52840 USB serial is different from ESP32
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime) < 5000) {
        delay(10);
    }
    
    Serial.println();
    Serial.println("===========================================");
    Serial.println("Telescope Park Sensor - XIAO Sense Edition");
    Serial.println("Version: " + String(DEVICE_VERSION));
    Serial.println("Platform: XIAO nRF52840 Sense");
    Serial.println("IMU: Built-in LSM6DS3TR-C");
    Serial.println("===========================================");
    Serial.println("Command Protocol: <CODE> where CODE is 2-digit hex");
    Serial.println("Example: <00> for help");
    Serial.println("Type <00> for available commands");
    Serial.println();
    
    // Clear any existing buffer
    serialBuffer.reserve(32);  // MAX_COMMAND_LENGTH
    serialBuffer = "";
}

void handleSerialCommands() {
    // Read incoming serial data
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        
        if (inChar == '<') {  // CMD_START_CHAR
            serialBuffer = "";
            inCommand = true;
        } else if (inChar == '>' && inCommand) {  // CMD_END_CHAR
            if (serialBuffer.length() > 0) {
                commandReady = true;
            }
            inCommand = false;
        } else if (inCommand && inChar >= 32 && inChar <= 126) { // Printable characters only
            if (serialBuffer.length() < 31) {  // MAX_COMMAND_LENGTH - 1
                serialBuffer += inChar;
            }
        }
    }
    
    // Process command if ready
    if (commandReady) {
        serialBuffer.trim();
        serialBuffer.toUpperCase();
        
        if (serialBuffer.length() > 0) {
            Debug.println("Serial command received: " + serialBuffer);
            processSerialCommand(serialBuffer);
        }
        
        serialBuffer = "";
        commandReady = false;
    }
}

void sendSerialResponse(String response) {
    Serial.println(response);
    Debug.println("Serial response: " + response);
}

void sendSerialError(String error) {
    Serial.println(buildJSONError(error));
    Debug.println("Serial error: " + error);
}

void sendSerialAck(String command) {
    JSONBuilder json;
    json.add("status", "ack");
    json.add("command", command);
    Serial.println(json.build());
    Debug.println("Serial ACK: " + command);
}

void sendSerialJSONResponse(String jsonData) {
    JSONBuilder response;
    response.add("status", "ok");
    // Note: This is a simplified approach. For complex nested JSON,
    // we would need a more sophisticated approach
    String fullResponse = "{\"status\":\"ok\",\"data\":" + jsonData + "}";
    Serial.println(fullResponse);
    Debug.println("Serial JSON response: " + fullResponse);
}

void processSerialCommand(String command) {
    command.trim();
    command.toUpperCase();
    
    sendSerialAck(command);
    
    if (command == "00") {  // CMD_HELP
        printSerialHelp();
    }
    else if (command == "01") {  // CMD_GET_STATUS
        handleStatusCommand();
    }
    else if (command == "02") {  // CMD_GET_POSITION
        handlePositionCommand();
    }
    else if (command == "03") {  // CMD_IS_PARKED
        handleParkedCommand();
    }
    else if (command == "04") {  // CMD_SET_PARK
        handleSetParkCommand();
    }
    else if (command == "05") {  // CMD_GET_PARK
        handleGetParkCommand();
    }
    else if (command == "06") {  // CMD_CALIBRATE
        handleCalibrateCommand();
    }
    else if (command == "07") {  // CMD_TOGGLE_DEBUG
        handleToggleDebugCommand();
    }
    else if (command == "08") {  // CMD_VERSION
        handleVersionCommand();
    }
    else if (command == "09") {  // CMD_RESET
        handleResetCommand();
    }
    else if (command.startsWith("0A")) {  // CMD_SET_TOLERANCE
        handleSetToleranceCommand(command);
    }
    else if (command == "0B") {  // CMD_GET_TOLERANCE
        handleGetToleranceCommand();
    }
    else if (command == "0C") {  // CMD_SYSTEM_INFO
        handleSystemInfoCommand();
    }
    else if (command == "0D") {  // CMD_SET_PARK_SOFTWARE
        handleSoftwareSetParkCommand();
    }
    else if (command == "0E") {  // CMD_FACTORY_RESET
        handleFactoryResetCommand();
    }
    else if (command == "0F") {  // CMD_TOGGLE_FILTER
        handleToggleFilterCommand();
    }
    else if (command.startsWith("10")) {  // CMD_SET_FILTER_ALPHA
        handleSetFilterAlphaCommand(command);
    }
    else if (command == "11") {  // CMD_RAW_SENSOR_DATA
        handleRawSensorDataCommand();
    }
    else if (command == "12") {  // CMD_STORAGE_TEST
        handleStorageTestCommand();
    }
    else if (command == "13") {  // CMD_SENSOR_DIAGNOSTIC
        handleSensorDiagnosticCommand();
    }
    else {
        sendSerialError("Unknown command: " + command + ". Use <00> for help.");
    }
}

void printSerialHelp() {
    Serial.println("Available Commands (use <CODE> format):");
    Serial.println("---------------------------------------");
    Serial.println("<00> - Show this help message");
    Serial.println("<01> - Get device status and sensor info");
    Serial.println("<02> - Get current pitch and roll values");
    Serial.println("<03> - Check if telescope is in park position");
    Serial.println("<04> - Set current position as park position");
    Serial.println("<05> - Get saved park position values");
    Serial.println("<06> - Recalibrate the position sensor");
    Serial.println("<07> - Toggle debug messages on/off");
    Serial.println("<08> - Get firmware version");
    Serial.println("<09> - Reset the device");
    Serial.println("<0AXXX> - Set tolerance (XXX = hundredths of degrees)");
    Serial.println("<0B> - Get current tolerance setting");
    Serial.println("<0C> - Get system information (XIAO Sense specific)");
    Serial.println("<0D> - Software set park position (no button needed)");
    Serial.println("<0E> - Factory reset (clear all settings)");
    Serial.println("<0F> - Toggle sensor filtering on/off");
    Serial.println("<10XX> - Set filter alpha (XX = alpha*100, 00-99)");
    Serial.println("<11> - Get raw sensor data");
    Serial.println("<12> - Test persistent storage");
    Serial.println("<13> - Comprehensive sensor diagnostic");
    Serial.println();
    Serial.println("Command format: <XX> where XX is 2-digit hex code");
    Serial.println("Example: <02> to get current position");
    Serial.println("Example: <0A050> to set tolerance to 0.50 degrees");
    Serial.println("Example: <1020> to set filter alpha to 0.20");
    Serial.println("Tolerance range: <0A001> to <0A999> (0.01° to 9.99°)");
    Serial.println("Filter alpha range: <1000> to <1099> (0.00 to 0.99)");
    Serial.println();
    Serial.println("XIAO Sense Features:");
    Serial.println("- Built-in LSM6DS3TR-C IMU (no external wiring needed)");
    Serial.println("- Internal LED status indication");
    Serial.println("- Software-only interface (no physical buttons)");
    Serial.println("- Enhanced storage system (v2.0.1)");
    Serial.println("- Advanced sensor filtering and diagnostics");
    Serial.println();
    Serial.println("Response format: JSON");
    Serial.println("Success: {\"status\":\"ok\",\"data\":{...}}");
    Serial.println("Error:   {\"status\":\"error\",\"message\":\"...\"}");
    Serial.println("ACK:     {\"status\":\"ack\",\"command\":\"...\"}");
}

void handleStatusCommand() {
    bool hasCalibration = hasStoredCalibration();
    bool ledStatus = digitalRead(LED_BUILTIN);
    
    JSONBuilder json;
    json.add("deviceName", DEVICE_NAME);
    json.add("version", DEVICE_VERSION);
    json.add("manufacturer", DEVICE_MANUFACTURER);
    json.add("platform", "XIAO nRF52840 Sense");
    json.add("imu", "LSM6DS3TR-C");
    json.add("parked", isParked);
    json.add("calibrated", hasCalibration);
    json.add("ledStatus", ledStatus);
    json.add("uptime", (unsigned long)millis());
    
    sendSerialJSONResponse(json.build());
}

void handlePositionCommand() {
    float pitch, roll;
    
    if (readPosition(pitch, roll)) {
        JSONBuilder json;
        json.add("pitch", pitch);
        json.add("roll", roll);
        sendSerialJSONResponse(json.build());
    } else {
        sendSerialError("Failed to read position from sensor");
    }
}

void handleParkedCommand() {
    updatePositionAndParkStatus(); // Use helper function
    
    JSONBuilder json;
    json.add("parked", isParked);
    json.add("currentPitch", currentPitch);
    json.add("currentRoll", currentRoll);
    json.add("parkPitch", parkPitch);
    json.add("parkRoll", parkRoll);
    json.add("tolerance", positionTolerance, 1);
    json.add("pitchDiff", calculatePositionDifference(currentPitch, parkPitch));
    json.add("rollDiff", calculatePositionDifference(currentRoll, parkRoll));
    
    sendSerialJSONResponse(json.build());
}

void handleSetParkCommand() {
    updatePositionAndParkStatus(); // Use helper function
    
    if (isValidPosition(currentPitch, currentRoll)) {
        parkPitch = currentPitch;
        parkRoll = currentRoll;
        
        // Save using helper functions
        bool success = saveFloatPreference("parkPitch", parkPitch) && 
                      saveFloatPreference("parkRoll", parkRoll);
        
        JSONBuilder json;
        json.add("parkPitch", parkPitch);
        json.add("parkRoll", parkRoll);
        json.add("saved", success);
        sendSerialJSONResponse(json.build());
        
        Debug.println("Park position updated and saved");
    } else {
        sendSerialError("Failed to read current position from sensor");
    }
}

void handleGetParkCommand() {
    JSONBuilder json;
    json.add("parkPitch", parkPitch);
    json.add("parkRoll", parkRoll);
    json.add("tolerance", positionTolerance, 1);
    sendSerialJSONResponse(json.build());
}

void handleCalibrateCommand() {
    sendSerialJSONResponse(buildSimpleJSONResponse("message", "Starting sensor calibration - keep sensor still!"));
    
    calibrateSensor();
    saveCalibration();
    
    JSONBuilder json;
    json.add("message", "Sensor calibration complete and saved");
    json.add("calibrated", true);
    json.add("saved", true);
    sendSerialJSONResponse(json.build());
}

void handleToggleDebugCommand() {
    extern bool DEBUG_ENABLED;
    DEBUG_ENABLED = !DEBUG_ENABLED;
    
    String status = DEBUG_ENABLED ? "ENABLED" : "DISABLED";
    Debug.println("Debug messages " + status + " via serial command");
    
    JSONBuilder json;
    json.add("debugEnabled", DEBUG_ENABLED);
    json.add("message", "Debug messages " + status);
    sendSerialJSONResponse(json.build());
}

void handleVersionCommand() {
    JSONBuilder json;
    json.add("firmwareVersion", DEVICE_VERSION);
    json.add("deviceName", DEVICE_NAME);
    json.add("manufacturer", DEVICE_MANUFACTURER);
    json.add("platform", "XIAO nRF52840 Sense");
    json.add("imu", "LSM6DS3TR-C");
    json.add("bluetoothReady", true);
    sendSerialJSONResponse(json.build());
}

void handleResetCommand() {
    sendSerialJSONResponse(buildSimpleJSONResponse("message", "Resetting device in 3 seconds"));
    delay(3000);
    // nRF52840 reset method
    NVIC_SystemReset();
}

void handleSetToleranceCommand(String command) {
    if (command.length() != 5) {
        sendSerialError("Invalid tolerance command format. Use <0AXXX> where XXX is tolerance in hundredths of degrees");
        return;
    }
    
    String toleranceStr = command.substring(2);
    
    // Validate that it's all digits
    for (int i = 0; i < toleranceStr.length(); i++) {
        if (!isDigit(toleranceStr.charAt(i))) {
            sendSerialError("Invalid tolerance value. Must be 3 digits (001-999)");
            return;
        }
    }
    
    int toleranceHundredths = toleranceStr.toInt();
    
    if (toleranceHundredths < 1 || toleranceHundredths > 999) {
        sendSerialError("Tolerance out of range. Must be 001-999 (0.01° to 9.99°)");
        return;
    }
    
    float newTolerance = toleranceHundredths / 100.0;
    positionTolerance = newTolerance;
    
    bool success = saveFloatPreference("tolerance", positionTolerance);
    
    JSONBuilder json;
    json.add("tolerance", positionTolerance);
    json.add("toleranceHundredths", toleranceHundredths);
    json.add("saved", success);
    sendSerialJSONResponse(json.build());
    
    Debug.println("Tolerance updated to " + String(positionTolerance, 2) + "° and saved");
}

void handleGetToleranceCommand() {
    int toleranceHundredths = round(positionTolerance * 100);
    
    JSONBuilder json;
    json.add("tolerance", positionTolerance);
    json.add("toleranceHundredths", toleranceHundredths);
    json.add("toleranceString", String(positionTolerance, 2) + "°");
    sendSerialJSONResponse(json.build());
}

void handleSystemInfoCommand() {
    Debug.println("=== SYSTEM INFO COMMAND ===");
    
    JSONBuilder json;
    json.add("platform", "XIAO nRF52840 Sense");
    json.add("chipModel", "nRF52840");
    json.add("boardType", "XIAO Sense");
    json.add("imu", "LSM6DS3TR-C");
    json.add("flashSize", "1MB");
    json.add("ramSize", "256KB");
    json.add("bluetoothSupport", true);
    json.add("bleSupport", true);
    json.add("fileSystem", "RAM Storage");
    json.add("builtinIMU", true);
    json.add("externalButton", false);
    json.add("uptime", (unsigned long)millis());
    
    sendSerialJSONResponse(json.build());
    
    Debug.println("System info command complete");
}

void handleSoftwareSetParkCommand() {
    Debug.println("=== SOFTWARE SET PARK COMMAND ===");
    
    updatePositionAndParkStatus();
    
    if (isValidPosition(currentPitch, currentRoll)) {
        parkPitch = currentPitch;
        parkRoll = currentRoll;
        
        // Save using helper functions
        bool success = saveFloatPreference("parkPitch", parkPitch) && 
                      saveFloatPreference("parkRoll", parkRoll);
        
        JSONBuilder json;
        json.add("message", "Park position set via software command");
        json.add("parkPitch", parkPitch);
        json.add("parkRoll", parkRoll);
        json.add("saved", success);
        sendSerialJSONResponse(json.build());
        
        Debug.println("Park position set via software command and saved");
    } else {
        sendSerialError("Failed to read current position from sensor");
    }
}

void handleFactoryResetCommand() {
    Debug.println("=== FACTORY RESET COMMAND ===");
    
    sendSerialJSONResponse(buildSimpleJSONResponse("message", "Factory reset initiated - clearing all stored data"));
    
    clearAllPreferences();
    
    Debug.println("All stored data cleared via software command");
    
    JSONBuilder json;
    json.add("message", "Factory reset complete - device will restart in 3 seconds");
    json.add("resetMethod", "software");
    sendSerialJSONResponse(json.build());
    
    delay(3000);
    NVIC_SystemReset();
}

void handleToggleFilterCommand() {
    extern bool use_filtering;
    use_filtering = !use_filtering;
    setFiltering(use_filtering);
    
    JSONBuilder json;
    json.add("filterEnabled", use_filtering);
    json.add("message", "Sensor filtering " + String(use_filtering ? "ENABLED" : "DISABLED"));
    json.add("note", "Disabling filter improves responsiveness but increases noise");
    sendSerialJSONResponse(json.build());
}

void handleSetFilterAlphaCommand(String command) {
    if (command.length() != 4) {
        sendSerialError("Invalid filter alpha command format. Use <10XX> where XX is alpha*100 (00-99)");
        return;
    }
    
    String alphaStr = command.substring(2);
    
    // Validate that it's all digits
    for (int i = 0; i < alphaStr.length(); i++) {
        if (!isDigit(alphaStr.charAt(i))) {
            sendSerialError("Invalid alpha value. Must be 2 digits (00-99)");
            return;
        }
    }
    
    int alphaHundredths = alphaStr.toInt();
    
    if (alphaHundredths < 0 || alphaHundredths > 99) {
        sendSerialError("Alpha out of range. Must be 00-99 (0.00 to 0.99)");
        return;
    }
    
    float newAlpha = alphaHundredths / 100.0;
    setFilterAlpha(newAlpha);
    
    JSONBuilder json;
    json.add("filterAlpha", newAlpha);
    json.add("alphaHundredths", alphaHundredths);
    json.add("message", "Filter alpha set to " + String(newAlpha, 2));
    json.add("note", "Lower alpha = more responsive, higher alpha = more filtering");
    sendSerialJSONResponse(json.build());
}

void handleRawSensorDataCommand() {
    Debug.println("=== RAW SENSOR DATA COMMAND ===");
    
    // Read raw data directly from IMU
    extern LSM6DS3 imu;
    extern float ax_offset, ay_offset, az_offset;
    extern float gx_offset, gy_offset, gz_offset;
    
    float ax_raw = imu.readFloatAccelX();
    float ay_raw = imu.readFloatAccelY();
    float az_raw = imu.readFloatAccelZ();
    float gx_raw = imu.readFloatGyroX();
    float gy_raw = imu.readFloatGyroY();
    float gz_raw = imu.readFloatGyroZ();
    float temp = imu.readTempC();
    
    // Calculate calibrated values
    float ax_cal = ax_raw - ax_offset;
    float ay_cal = ay_raw - ay_offset;
    float az_cal = az_raw - az_offset;
    
    // Calculate pitch and roll from raw calibrated data
    float magnitude = sqrt(ax_cal * ax_cal + ay_cal * ay_cal + az_cal * az_cal);
    float pitch_raw = atan2(-ax_cal, sqrt(ay_cal * ay_cal + az_cal * az_cal)) * 180.0 / PI;
    float roll_raw = atan2(ay_cal, az_cal) * 180.0 / PI;
    
    JSONBuilder json;
    json.add("ax_raw", ax_raw, 4);
    json.add("ay_raw", ay_raw, 4);
    json.add("az_raw", az_raw, 4);
    json.add("gx_raw", gx_raw, 4);
    json.add("gy_raw", gy_raw, 4);
    json.add("gz_raw", gz_raw, 4);
    json.add("temperature", temp, 2);
    json.add("ax_calibrated", ax_cal, 4);
    json.add("ay_calibrated", ay_cal, 4);
    json.add("az_calibrated", az_cal, 4);
    json.add("magnitude", magnitude, 4);
    json.add("pitch_unfiltered", pitch_raw, 3);
    json.add("roll_unfiltered", roll_raw, 3);
    json.add("ax_offset", ax_offset, 4);
    json.add("ay_offset", ay_offset, 4);
    json.add("az_offset", az_offset, 4);
    sendSerialJSONResponse(json.build());
}

void handleStorageTestCommand() {
    Debug.println("=== STORAGE TEST COMMAND ===");
    
    // Use the helper functions instead of direct flash functions
    JSONBuilder json;
    json.add("persistentStorageAvailable", false);  // Always false for mbed core
    
    // Test enhanced RAM storage instead
    float testValue = millis() / 1000.0;
    bool saveSuccess = saveFloatPreference("test_key", testValue);
    float loadedValue = loadFloatPreference("test_key", -999.0);
    bool loadSuccess = (abs(loadedValue - testValue) < 0.001);
    
    json.add("testSaveSuccess", saveSuccess);
    json.add("testLoadSuccess", loadSuccess);
    json.add("testValueSent", testValue, 3);
    json.add("testValueReceived", loadedValue, 3);
    json.add("message", "Enhanced RAM storage " + String((saveSuccess && loadSuccess) ? "WORKING" : "FAILED"));
    json.add("note", "Settings will be lost on power cycle (Seeed mbed core limitation)");
    
    sendSerialJSONResponse(json.build());
}

void handleSensorDiagnosticCommand() {
    Debug.println("=== COMPREHENSIVE SENSOR DIAGNOSTIC ===");
    
    extern LSM6DS3 imu;
    extern bool use_filtering;
    extern float alpha;  // REMOVE const from this line
    
    // Take multiple readings for stability analysis
    const int numReadings = 10;
    float pitchReadings[numReadings];
    float rollReadings[numReadings];
    bool allReadingsValid = true;
    
    Debug.println("Taking " + String(numReadings) + " readings for stability analysis...");
    
    for (int i = 0; i < numReadings; i++) {
        float pitch, roll;
        if (readPosition(pitch, roll)) {
            pitchReadings[i] = pitch;
            rollReadings[i] = roll;
        } else {
            allReadingsValid = false;
            break;
        }
        delay(50);
    }
    
    JSONBuilder json;
    json.add("diagnosticTime", millis());
    json.add("readingsRequested", numReadings);
    json.add("allReadingsValid", allReadingsValid);
    
    if (allReadingsValid) {
        // Calculate statistics
        float pitchSum = 0, rollSum = 0;
        float pitchMin = pitchReadings[0], pitchMax = pitchReadings[0];
        float rollMin = rollReadings[0], rollMax = rollReadings[0];
        
        for (int i = 0; i < numReadings; i++) {
            pitchSum += pitchReadings[i];
            rollSum += rollReadings[i];
            
            if (pitchReadings[i] < pitchMin) pitchMin = pitchReadings[i];
            if (pitchReadings[i] > pitchMax) pitchMax = pitchReadings[i];
            if (rollReadings[i] < rollMin) rollMin = rollReadings[i];
            if (rollReadings[i] > rollMax) rollMax = rollReadings[i];
        }
        
        float pitchAvg = pitchSum / numReadings;
        float rollAvg = rollSum / numReadings;
        float pitchRange = pitchMax - pitchMin;
        float rollRange = rollMax - rollMin;
        
        json.add("pitchAverage", pitchAvg, 3);
        json.add("rollAverage", rollAvg, 3);
        json.add("pitchRange", pitchRange, 3);
        json.add("rollRange", rollRange, 3);
        json.add("pitchStability", (pitchRange < 0.5) ? "GOOD" : (pitchRange < 1.0) ? "FAIR" : "POOR");
        json.add("rollStability", (rollRange < 0.5) ? "GOOD" : (rollRange < 1.0) ? "FAIR" : "POOR");
    }
    
    // Hardware and configuration info
    json.add("imuModel", "LSM6DS3TR-C");
    json.add("filterEnabled", use_filtering);
    json.add("filterAlpha", alpha, 3);
    json.add("hasCalibration", hasStoredCalibration());
    json.add("storageAvailable", false);  // Always false for mbed core
    
    // Temperature reading
    float temperature = imu.readTempC();
    json.add("temperature", temperature, 1);
    json.add("temperatureStatus", (temperature > 15 && temperature < 50) ? "NORMAL" : "CHECK");
    
    sendSerialJSONResponse(json.build());
}