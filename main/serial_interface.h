#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include "Debug.h"
#include "constants.h"
#include "Arduino.h"

// Command definitions (2-character hex codes) - Updated for XIAO Sense
#define CMD_HELP "00"
#define CMD_GET_STATUS "01"
#define CMD_GET_POSITION "02"
#define CMD_IS_PARKED "03"
#define CMD_SET_PARK "04"
#define CMD_GET_PARK "05"
#define CMD_CALIBRATE "06"
#define CMD_TOGGLE_DEBUG "07"
#define CMD_VERSION "08"
#define CMD_RESET "09"
#define CMD_SET_TOLERANCE "0A"
#define CMD_GET_TOLERANCE "0B"
#define CMD_SYSTEM_INFO "0C"
#define CMD_SET_PARK_SOFTWARE "0D"
#define CMD_FACTORY_RESET "0E"

// Response codes
#define RESP_OK "OK"
#define RESP_ERROR "ERROR"
#define RESP_INVALID "INVALID"

// Function prototypes
void initSerial();
void handleSerialCommands();
void processSerialCommand(String command);
void sendSerialResponse(String response);
void sendSerialError(String error);
void sendSerialAck(String command);
void sendSerialJSONResponse(String jsonData);
void printSerialHelp();

// Command handler prototypes
void handleStatusCommand();
void handlePositionCommand();
void handleParkedCommand();
void handleSetParkCommand();
void handleGetParkCommand();
void handleCalibrateCommand();
void handleToggleDebugCommand();
void handleVersionCommand();
void handleResetCommand();
void handleSetToleranceCommand(String command);
void handleGetToleranceCommand();
void handleSystemInfoCommand();
void handleSoftwareSetParkCommand();  // New for software-only interface
void handleFactoryResetCommand();     // New dedicated factory reset

#endif // SERIAL_INTERFACE_H