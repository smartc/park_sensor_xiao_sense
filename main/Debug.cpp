// Debug.cpp - nRF52840 Port
#include "Debug.h"
#include <stdarg.h> // Include the header for variable argument functions

// Runtime debug control - starts disabled
bool DEBUG_ENABLED = false;

void DebugClass::print(const String& msg) {
  if (DEBUG && DEBUG_ENABLED) {
    Serial.print(msg);
  }
}

void DebugClass::print(int msg) {
  if (DEBUG && DEBUG_ENABLED) {
    Serial.print(msg);
  }
}

void DebugClass::println(const String& msg) {
  if (DEBUG && DEBUG_ENABLED) {
    Serial.println(msg);
  }
}

void DebugClass::println(int msg) {
  if (DEBUG && DEBUG_ENABLED) {
    Serial.println(msg);
  }
}

void DebugClass::printf(const char *format, ...) {
  if (DEBUG && DEBUG_ENABLED) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
  }
}

DebugClass Debug;