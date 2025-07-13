#ifndef HELPERS_H
#define HELPERS_H

#include "Arduino.h"
// Remove InternalFileSystem.h - not available with mbed core
// We'll use a simple in-memory storage for now

// Position and park status management
void updatePositionAndParkStatus();
bool isCurrentlyParked();

// Simple storage system (in-memory for mbed core)
// Note: These will be lost on power cycle - for persistent storage,
// we would need to implement EEPROM or Flash storage differently
bool saveFloatPreference(const char* key, float value);
bool saveIntPreference(const char* key, int value);
float loadFloatPreference(const char* key, float defaultValue);
int loadIntPreference(const char* key, int defaultValue);
bool clearAllPreferences();

// JSON response builders
String buildSimpleJSONResponse(const String& key, const String& value);
String buildSimpleJSONResponse(const String& key, float value, int decimals = 2);
String buildSimpleJSONResponse(const String& key, bool value);
String buildJSONError(const String& message);
String buildJSONNotification(const String& message);

// JSON object builder class for complex responses
class JSONBuilder {
private:
    String json;
    bool hasContent;
    
public:
    JSONBuilder();
    void add(const String& key, const String& value);
    void add(const String& key, const char* value);
    void add(const String& key, float value, int decimals = 2);
    void add(const String& key, bool value);
    void add(const String& key, int value);
    void add(const String& key, unsigned long value);
    String build();
    void reset();
};

// Position validation helpers
bool isValidPosition(float pitch, float roll);
float calculatePositionDifference(float current, float target);

// Debug helpers
void debugPositionInfo(float pitch, float roll, bool parked);
void debugButtonAction(const String& action);
void debugSensorCalibration(int samples, int successful);

#endif // HELPERS_H