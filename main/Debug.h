// Debug.h - nRF52840 Port
#ifndef DEBUG_H
#define DEBUG_H

#include "Arduino.h"

#define DEBUG 1

// Runtime debug control
extern bool DEBUG_ENABLED;

class DebugClass {
public:
  void print(const String& msg);
  void print(int msg);
  void println(const String& msg);
  void println(int msg);
  void printf(const char *format, ...);
};

extern DebugClass Debug;

#endif