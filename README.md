# Telescope Park Sensor v1.0.2

**ESP32 Based Telescope Park Position Sensor with GY-521 Accelerometer/Gyroscope**

---

## Table of Contents
- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Software Installation](#software-installation)
- [Serial Command Interface](#serial-command-interface)
- [Button Controls](#button-controls)
- [LED Status Indicators](#led-status-indicators)
- [Operation Guide](#operation-guide)
- [Troubleshooting](#troubleshooting)
- [Technical Specifications](#technical-specifications)
- [Development Notes](#development-notes)

---

## Overview

The Telescope Park Sensor is a precision position monitoring device designed to detect when a telescope is in its "parked" position. Using a 6-axis MPU6050 sensor (3-axis accelerometer + 3-axis gyroscope), it provides accurate pitch and roll measurements to determine telescope orientation.

### Key Features
- **High-precision positioning** - ±0.01° to ±9.99° configurable tolerance
- **Serial command interface** - 13 commands for complete control
- **Physical button controls** - Multi-press sequences for field operation
- **Visual status indication** - LED shows park status at a glance
- **Persistent storage** - Settings retained between power cycles
- **Sensor calibration** - Automatic offset compensation
- **Debug capabilities** - Runtime toggleable debug messages

### Use Cases
- **Astronomy automation** - ASCOM-compatible telescope parking verification
- **Safety monitoring** - Ensure telescope is properly stowed
- **Remote operation** - Serial interface for computer control
- **Portable operation** - Battery-powered field use

---

## Hardware Requirements

### Main Components
| Component | Model/Type | Quantity | Notes |
|-----------|------------|----------|--------|
| **Microcontroller** | ESP32 Development Board | 1 | Any ESP32 dev board with USB |
| **Sensor Module** | WayinTop GY-521 (MPU6050) | 1 | 6-axis accelerometer/gyroscope |
| **Push Button** | Momentary switch | 1 | NO (Normally Open) type |
| **LED** | 5mm LED (any color) | 1 | Red recommended for visibility |
| **Resistor** | 220Ω - 470Ω | 1 | For LED current limiting |

### Optional Components
| Component | Purpose | Notes |
|-----------|---------|--------|
| **External power** | Battery operation | 3.7V LiPo or 5V power bank |
| **Enclosure** | Weather protection | IP65+ rating for outdoor use |
| **Mounting hardware** | Telescope attachment | Depends on telescope design |

### Tools Required
- Soldering iron and solder
- Breadboard or perfboard
- Jumper wires
- Wire strippers
- Multimeter (for testing)

---

## Wiring Diagram

### Pin Connections

#### MPU6050 (GY-521) to ESP32
```
MPU6050     ESP32
-------     -----
VCC    →    3.3V
GND    →    GND
SDA    →    GPIO 21 (D21)
SCL    →    GPIO 22 (D22)
```

#### Push Button to ESP32
```
Button      ESP32
------      -----
Pin 1  →    3.3V via 10kΩ pull-up resistor
Pin 1  →    GPIO 5 (D5)
Pin 2  →    GND
```
*Note: 10kΩ external pull-up resistor and 330µF capacitor for hardware debouncing*

#### Button Debouncing Circuit
```
3.3V ──[10kΩ]──┬─── GPIO 5 (D5)
               │
               ├─── [Button] ─── GND
               │
               └─── [330µF Cap] ─── GND
```

#### LED to ESP32
```
LED         ESP32
---         -----
Anode  →    GPIO 18 (D18) → 220Ω Resistor
Cathode →   GND
```

### Complete Wiring Schematic
```
                    ESP32
                 ┌─────────┐
    MPU6050      │         │
   ┌─────────┐   │         │
   │ VCC─────┼───┤ 3.3V    │
   │ GND─────┼───┤ GND     │
   │ SDA─────┼───┤ GPIO21  │
   │ SCL─────┼───┤ GPIO22  │
   └─────────┘   │         │
                 │         │    10kΩ
    3.3V ────────┼─────────┼──[Resistor]──┬─── GPIO5
                 │         │              │
                 │         │       [Button]──── GND
                 │         │              │
                 │         │        [330µF Cap]─ GND
                 │         │
                 │ GPIO18  ├─── [220Ω] ─── [LED+] ─── GND
                 │         │
                 │ USB     ├─── Computer (for programming/serial)
                 └─────────┘
```

### Power Considerations
- **USB Power**: 5V from computer (most common)
- **External Power**: 3.3V-5V via VIN pin
- **Current Draw**: ~100mA typical, ~150mA with WiFi (removed in v1.0.2)
- **Battery Life**: 10-20 hours with 2000mAh power bank

---

## Software Installation

### Prerequisites
1. **Arduino IDE** (v1.8.19 or newer) or **PlatformIO**
2. **ESP32 Board Package** installed in Arduino IDE
3. **Required Libraries**:
   - `MPU6050` library by Electronic Cats
   - `Wire` library (included with Arduino)
   - `Preferences` library (included with ESP32)

### Arduino IDE Setup
1. **Install ESP32 Board Package**:
   - File → Preferences
   - Add to Additional Board Manager URLs:
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "ESP32" and install

2. **Install Libraries**:
   - Tools → Manage Libraries
   - Search and install:
     - "MPU6050" by Electronic Cats

3. **Board Configuration**:
   - Board: "ESP32 Dev Module"
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Size: 4MB
   - Partition Scheme: Default

### File Structure
```
Telescope_Park_Sensor_v1.0.2/
├── main.ino                    # Main program file
├── constants.h                 # Pin definitions and constants
├── helpers.h                   # Helper function declarations
├── helpers.cpp                 # Helper function implementations
├── position_sensor.h           # MPU6050 sensor interface
├── position_sensor.cpp         # Sensor implementation
├── serial_interface.h          # Serial command definitions
├── serial_interface.cpp        # Serial command handlers
├── button_control.h            # Button handling declarations
├── button_control.cpp          # Button control implementation
├── led_control.h               # LED control declarations
├── led_control.cpp             # LED control implementation
├── Debug.h                     # Debug system declarations
└── Debug.cpp                   # Debug system implementation
```

### Upload Process
1. Connect ESP32 to computer via USB
2. Select correct COM port in Arduino IDE
3. Compile and upload the sketch
4. Open Serial Monitor at 115200 baud
5. Device should display startup message

---

## Serial Command Interface

### Communication Settings
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None

### Command Format
Commands use the format: `<XX>` where XX is a 2-digit hexadecimal code.

**Example**: `<02>` to get current position

### Available Commands

| Command | Hex Code | Description | Response Format |
|---------|----------|-------------|-----------------|
| **Help** | `<00>` | Show command list | Text help |
| **Status** | `<01>` | Get device status | JSON with device info |
| **Position** | `<02>` | Get current pitch/roll | JSON with degrees |
| **Is Parked** | `<03>` | Check park status | JSON with detailed position |
| **Set Park** | `<04>` | Set current position as park | JSON confirmation |
| **Get Park** | `<05>` | Get saved park position | JSON with park settings |
| **Calibrate** | `<06>` | Recalibrate sensor | JSON progress/completion |
| **Debug Toggle** | `<07>` | Enable/disable debug messages | JSON with new state |
| **Version** | `<08>` | Get firmware version | JSON with version info |
| **Reset** | `<09>` | Restart device | JSON countdown |
| **Set Tolerance** | `<0AXXX>` | Set tolerance (XXX=hundredths) | JSON confirmation |
| **Get Tolerance** | `<0B>` | Get current tolerance | JSON with tolerance |
| **Button Test** | `<0C>` | Test button functionality | JSON with button state |

### Response Format
All responses are in JSON format:

**Success Response**:
```json
{
  "status": "ok",
  "data": {
    "pitch": 15.23,
    "roll": -2.67
  }
}
```

**Error Response**:
```json
{
  "status": "error",
  "message": "Failed to read position from sensor"
}
```

**Acknowledgment**:
```json
{
  "status": "ack",
  "command": "02"
}
```

### Example Command Session
```
> <00>
{"status":"ack","command":"00"}
Available Commands (use <CODE> format):
---------------------------------------
<00> - Show this help message
<01> - Get device status and sensor info
<02> - Get current pitch and roll values
...

> <02>
{"status":"ack","command":"02"}
{"status":"ok","data":{"pitch":15.23,"roll":-2.67}}

> <04>
{"status":"ack","command":"04"}
{"status":"ok","data":{"parkPitch":15.23,"parkRoll":-2.67,"saved":true}}
```

### Setting Tolerance Examples
- `<0A050>` - Set tolerance to 0.50 degrees
- `<0A010>` - Set tolerance to 0.10 degrees (high precision)
- `<0A200>` - Set tolerance to 2.00 degrees (relaxed)

---

## Button Controls

### Button Functions
The device includes a single push button with multiple functions based on press duration and patterns:

### Single Press Actions
| Press Type | Duration | Function |
|------------|----------|----------|
| **Short Press** | 100ms - 3s | Part of multi-press sequence |
| **Long Press** | 3s - 8s | Start sensor calibration |
| **Very Long Press** | > 8s | Factory reset (clears all data) |

### Multi-Press Sequences
| Pattern | Function | Safety Feature |
|---------|----------|----------------|
| **3 Quick Presses** | Set park position | 500ms confirmation delay |
| **5 Quick Presses** | Toggle debug messages | 500ms confirmation delay |

### Button Operation Guide
1. **Setting Park Position**:
   - Position telescope in desired park position
   - Press button 3 times quickly (within 1.5 seconds)
   - Wait for confirmation (LED may blink)
   - Position is automatically saved

2. **Sensor Calibration**:
   - Ensure sensor is completely still
   - Hold button for 3-8 seconds
   - Release when instructed
   - Keep still during ~10 second calibration
   - Calibration data is automatically saved

3. **Factory Reset**:
   - Hold button for more than 8 seconds
   - Release when prompted
   - All settings will be cleared
   - Device will restart automatically

4. **Debug Toggle**:
   - Press button 5 times quickly
   - Debug messages will be enabled/disabled
   - Useful for troubleshooting

### Button Feedback
- **Visual**: LED may blink during operations
- **Serial**: JSON notifications sent via serial
- **Audio**: None (silent operation)

---

## LED Status Indicators

### LED States
| LED State | Meaning | Duration |
|-----------|---------|----------|
| **OFF** | Telescope NOT in park position | Continuous |
| **ON (Solid)** | Telescope IS in park position | Continuous |
| **Blinking Pattern** | Sensor initialization error | Continuous until reset |

### LED Behavior Details

#### Normal Operation
- **LED OFF**: Current position differs from park position by more than tolerance
- **LED ON**: Current position is within tolerance of park position
- **Updates**: LED state updates every 50ms for responsive indication

#### Error Patterns
- **3 Blinks, Pause, 3 Blinks**: Sensor initialization failed
  - Check sensor connections
  - Verify power supply
  - Try restarting device

#### Startup Sequence
1. **Brief Flash**: LED test during initialization
2. **OFF**: Normal startup, waiting for position data
3. **Normal Operation**: LED reflects actual park status

### LED Troubleshooting
- **LED Never Lights**: Check wiring, resistor value, LED polarity
- **LED Always On**: Check park position settings, tolerance values
- **Erratic Blinking**: Possible sensor connection issues

---

## Operation Guide

### Initial Setup
1. **Mount Sensor**: Attach firmly to telescope or mount
2. **Connect Power**: USB or external power source
3. **Verify Operation**: Check serial output and LED function
4. **Set Park Position**: Use button or serial command when parked

### Daily Use Workflow

#### Before Observing Session
1. **Check Status**: Send `<01>` command or observe LED
2. **Verify Park**: Telescope should be in park position (LED ON)
3. **Move to Target**: Begin normal telescope operations

#### After Observing Session
1. **Return to Park**: Move telescope to park position
2. **Verify Parking**: LED should turn ON when properly parked
3. **Double-Check**: Send `<03>` command for detailed status

#### Periodic Maintenance
- **Weekly**: Check connections and mounting
- **Monthly**: Verify calibration accuracy
- **Seasonally**: Recalibrate if needed (`<06>` command)

### Calibration Procedure
1. **Preparation**:
   - Ensure sensor is completely still
   - Remove any vibrations (wind, touching)
   - Stable temperature (avoid direct sunlight)

2. **Start Calibration**:
   - Send `<06>` command OR hold button 3-8 seconds
   - Keep sensor motionless during entire process
   - Wait for completion message (~10 seconds)

3. **Verification**:
   - Check position readings for stability
   - Verify park position detection accuracy
   - Re-set park position if needed

### Setting Park Position
1. **Move to Park**: Position telescope in desired park position
2. **Set Position**: Use `<04>` command OR 3 quick button presses
3. **Verify**: Check with `<05>` command to confirm settings
4. **Test**: Move telescope away and back to verify detection

### Tolerance Adjustment
- **Tight Tolerance** (0.1°-0.5°): High precision, may be sensitive to vibration
- **Medium Tolerance** (0.5°-1.0°): Good balance of precision and stability
- **Loose Tolerance** (1.0°-3.0°): Robust against vibration, less precise

**Recommended Starting Value**: 2.0° (factory default)

---

## Troubleshooting

### Common Issues

#### Sensor Not Responding
**Symptoms**: No position data, error messages
**Solutions**:
1. Check I2C connections (SDA/SCL)
2. Verify 3.3V power supply
3. Try different I2C pins
4. Replace sensor module

#### Inaccurate Position Readings
**Symptoms**: Position readings drift, inconsistent values
**Solutions**:
1. Recalibrate sensor (`<06>` command)
2. Check for mechanical vibration
3. Verify stable mounting
4. Allow sensor to reach stable temperature

#### Park Position Not Detected
**Symptoms**: LED doesn't turn on when telescope is parked
**Solutions**:
1. Verify park position is set (`<05>` command)
2. Check tolerance setting (`<0B>` command)
3. Increase tolerance if too strict
4. Re-set park position (`<04>` command)

#### Button Not Working
**Symptoms**: Button presses not recognized
**Solutions**:
1. Check button wiring and connections
2. Test with `<0C>` command
3. Verify pull-up resistor (internal should be enabled)
4. Replace button if defective

#### Serial Communication Issues
**Symptoms**: No response to commands, garbled text
**Solutions**:
1. Check baud rate (115200)
2. Verify USB cable and connection
3. Try different serial terminal
4. Check COM port selection

#### LED Problems
**Symptoms**: LED not working, wrong behavior
**Solutions**:
1. Check LED polarity (anode to GPIO18)
2. Verify resistor value (220Ω-470Ω)
3. Test with multimeter
4. Check GPIO18 output

### Diagnostic Commands
Use these commands to diagnose issues:

```bash
<01>    # Get full system status
<02>    # Check sensor readings
<03>    # Detailed park position analysis
<0C>    # Test button functionality
<07>    # Enable debug messages
```

### Error Messages

| Error Message | Cause | Solution |
|---------------|-------|----------|
| "Failed to read position from sensor" | Sensor connection issue | Check wiring, recalibrate |
| "Sensor initialization failed" | I2C communication failure | Verify connections, power |
| "Invalid tolerance value" | Tolerance out of range | Use 001-999 (0.01°-9.99°) |
| "Failed to save preference" | Storage issue | Try factory reset |

### Factory Reset Procedure
If all else fails, perform a factory reset:
1. **Button Method**: Hold button > 8 seconds
2. **Serial Method**: Send `<09>` command
3. **Manual**: Re-upload firmware

After reset:
- Recalibrate sensor
- Set new park position
- Adjust tolerance as needed

---

## Technical Specifications

### Hardware Specifications

#### ESP32 Microcontroller
- **Processor**: Dual-core Tensilica LX6, 240MHz
- **Memory**: 520KB SRAM, 4MB Flash
- **GPIO**: 30 pins available
- **Communication**: UART, I2C, SPI
- **Power**: 3.3V logic, 5V tolerant inputs

#### MPU6050 Sensor (GY-521)
- **Accelerometer**: ±2g, ±4g, ±8g, ±16g (±2g used)
- **Gyroscope**: ±250°/s, ±500°/s, ±1000°/s, ±2000°/s
- **Resolution**: 16-bit ADC
- **Interface**: I2C (400kHz max, 100kHz used)
- **Power**: 3.3V-5V supply

#### Position Accuracy
- **Resolution**: 0.01° minimum
- **Tolerance Range**: 0.01° to 9.99°
- **Update Rate**: 20Hz (50ms intervals)
- **Stability**: ±0.1° typical drift

### Software Specifications

#### Firmware
- **Version**: 1.0.2
- **Language**: C++ (Arduino framework)
- **Compiler**: ESP32 GCC toolchain
- **Libraries**: MPU6050, Wire, Preferences

#### Memory Usage
- **Program Storage**: ~200KB of 4MB flash
- **Dynamic Memory**: ~50KB of 520KB SRAM
- **EEPROM**: Preferences stored in flash

#### Communication
- **Serial**: 115200 baud, 8N1
- **Protocol**: JSON-based command/response
- **Commands**: 13 total commands
- **Response Time**: <100ms typical

#### Performance
- **Boot Time**: ~3 seconds
- **Command Response**: <50ms
- **Position Update**: 50ms (20Hz)
- **Calibration Time**: ~10 seconds

### Environmental Specifications

#### Operating Conditions
- **Temperature**: -10°C to +50°C
- **Humidity**: 0-90% non-condensing
- **Vibration**: Minimal for best accuracy
- **Mounting**: Rigid attachment required

#### Power Requirements
- **Supply Voltage**: 3.3V-5V
- **Current Draw**: 80-120mA typical
- **Power Consumption**: 0.4-0.6W
- **Battery Life**: 10-20 hours (2000mAh)

---

## Development Notes

### Version History
- **v1.0.0**: Initial development version
- **v1.0.1**: Added button controls, LED status, serial interface
- **v1.0.2**: Removed WiFi/ALPACA, refactored code, 2-digit commands

### Code Architecture

#### Modular Design
- **constants.h**: All pin definitions and timing constants
- **helpers.h/cpp**: Shared utility functions and JSON handling
- **position_sensor.h/cpp**: MPU6050 interface and calibration
- **serial_interface.h/cpp**: Command parsing and responses
- **button_control.h/cpp**: Multi-press button handling
- **led_control.h/cpp**: Status LED management
- **Debug.h/cpp**: Runtime debug message system

#### Design Patterns
- **Global state management**: Centralized variable access
- **JSON responses**: Consistent API format
- **Helper functions**: Code reuse and maintainability
- **Preferences abstraction**: Simplified settings storage
- **Error handling**: Graceful failure and recovery

### Future Enhancements (Planned)
- **v1.0.3**: Bluetooth serial interface
- **v1.1.0**: Python ALPACA bridge application
- **v1.2.0**: Mobile app for configuration
- **v2.0.0**: Wireless operation with web interface

### Known Limitations
- **I2C dependency**: Single point of failure
- **Temperature sensitivity**: May require recalibration
- **Vibration sensitivity**: Mounting critical for accuracy
- **Single sensor**: No redundancy or cross-checking

### Contributing
This is a personal project, but suggestions and improvements are welcome:
- Code optimization
- Feature requests
- Bug reports
- Documentation improvements

### License
This project is open source. Feel free to modify and adapt for your needs.

---

## Appendix

### Pin Assignment Reference
```cpp
// GPIO Pin Definitions
#define BUTTON_PIN 5      // GPIO 5 (D5)
#define LED_PIN 18        // GPIO 18 (D18)
#define SDA_PIN 21        // Default SDA pin for ESP32
#define SCL_PIN 22        // Default SCL pin for ESP32
```

### Command Quick Reference
```
<00> Help           <06> Calibrate      <0AXXX> Set Tolerance
<01> Status         <07> Debug Toggle   <0B> Get Tolerance  
<02> Position       <08> Version        <0C> Button Test
<03> Is Parked      <09> Reset
<04> Set Park       
<05> Get Park       
```

### JSON Response Examples
```json
// Status Response
{
  "status": "ok",
  "data": {
    "deviceName": "Telescope Park Sensor",
    "version": "1.0.2",
    "manufacturer": "Corey Smart",
    "parked": true,
    "calibrated": true,
    "buttonPressed": false,
    "ledStatus": true,
    "freeHeap": 330620,
    "uptime": 66186
  }
}

// Position Response
{
  "status": "ok",
  "data": {
    "pitch": 15.23,
    "roll": -2.67
  }
}

// Park Status Response
{
  "status": "ok",
  "data": {
    "parked": true,
    "currentPitch": 15.25,
    "currentRoll": -2.65,
    "parkPitch": 15.23,
    "parkRoll": -2.67,
    "tolerance": 2.0,
    "pitchDiff": 0.02,
    "rollDiff": 0.02
  }
}
```

---

**End of Documentation**

*Last Updated: Version 1.0.2*  
*Author: Corey Smart*  
*Project: Telescope Park Sensor*