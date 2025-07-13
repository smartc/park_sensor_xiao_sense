# Telescope Park Sensor v2.0.2 - XIAO Sense Edition

**XIAO nRF52840 Sense Based Telescope Park Position Sensor with Built-in LSM6DS3TR-C IMU**

---

## Table of Contents
- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Software Installation](#software-installation)
- [Serial Command Interface](#serial-command-interface)
- [Operation Guide](#operation-guide)
- [Advanced Features](#advanced-features)
- [Troubleshooting](#troubleshooting)
- [Technical Specifications](#technical-specifications)
- [Development Notes](#development-notes)

---

## Overview

The Telescope Park Sensor v2.0.2 is a precision position monitoring device designed to detect when a telescope is in its "parked" position. Built on the XIAO nRF52840 Sense platform with a built-in LSM6DS3TR-C 6-axis IMU, it provides accurate pitch and roll measurements with no external components required.

### Key Features
- **Zero external wiring** - Built-in IMU and LED, software-only interface
- **High-precision positioning** - ±0.01° to ±9.99° configurable tolerance
- **Advanced filtering** - Configurable sensor filtering for optimal performance
- **Serial command interface** - 20+ commands for complete control and diagnostics
- **Visual status indication** - Built-in LED shows park status
- **Enhanced storage** - QSPI flash support with RAM fallback
- **Bluetooth ready** - nRF52840 platform supports BLE (future feature)
- **Debug capabilities** - Comprehensive diagnostics and runtime debug control

### Use Cases
- **Astronomy automation** - ASCOM-compatible telescope parking verification
- **Safety monitoring** - Ensure telescope is properly stowed
- **Remote operation** - Serial/Bluetooth interface for computer control
- **Portable operation** - USB-powered, compact form factor

---

## Hardware Requirements

### Main Components
| Component | Model/Type | Quantity | Notes |
|-----------|------------|----------|--------|
| **Microcontroller** | XIAO nRF52840 Sense | 1 | Built-in IMU and LED |
| **IMU** | LSM6DS3TR-C (built-in) | 1 | 6-axis accelerometer/gyroscope |
| **LED** | Built-in Blue LED | 1 | Park status indication |
| **USB Cable** | USB-C | 1 | For programming and power |

### Optional Components
| Component | Purpose | Notes |
|-----------|---------|--------|
| **External power** | Battery operation | 3.7V LiPo or USB power bank |
| **Enclosure** | Weather protection | Custom 3D printed or small project box |
| **Mounting hardware** | Telescope attachment | Depends on telescope design |

### Tools Required
- Computer with Arduino IDE or PlatformIO
- USB-C cable
- Mounting hardware (optional)

**Total Cost**: ~$15 USD (just the XIAO board!)

---

## Software Installation

### Prerequisites
1. **Arduino IDE** (v1.8.19 or newer) or **PlatformIO**
2. **Seeed nRF52 Board Package** 
3. **Required Libraries**:
   - `LSM6DS3` library by Seeed Studio
   - `Wire` library (included)

### Arduino IDE Setup
1. **Install Seeed nRF52 Board Package**:
   - File → Preferences
   - Add to Additional Board Manager URLs:
     ```
     https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "Seeed nRF52" and install

2. **Install Libraries**:
   - Tools → Manage Libraries
   - Search and install:
     - "LSM6DS3" by Seeed Studio

3. **Board Configuration**:
   - Board: "Seeed XIAO nRF52840 Sense"
   - Port: Select your device's COM port
   - Upload Speed: Default

### File Structure
```
main/
├── main.ino                    # Main program file
├── constants.h                 # Pin definitions and constants
├── helpers.h/cpp               # Helper functions and JSON handling
├── position_sensor.h/cpp       # LSM6DS3TR-C sensor interface
├── serial_interface.h/cpp      # Serial command handlers
├── led_control.h/cpp           # LED status control
├── flash_storage.h/cpp         # Enhanced storage system
└── Debug.h/cpp                 # Debug system
```

### Upload Process
1. Connect XIAO Sense to computer via USB-C
2. Select "Seeed XIAO nRF52840 Sense" board
3. Select correct COM port
4. Compile and upload
5. Open Serial Monitor at 115200 baud
6. Device should display startup message

---

## Serial Command Interface

### Communication Settings
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None

### Command Format
Commands use format: `<XX>` where XX is 2-digit hexadecimal code.

### Core Commands

| Command | Code | Description | Response |
|---------|------|-------------|----------|
| **Help** | `<00>` | Show command list | Text help |
| **Status** | `<01>` | Get device status | JSON with device info |
| **Position** | `<02>` | Get current pitch/roll | JSON with degrees |
| **Is Parked** | `<03>` | Check park status | JSON with detailed position |
| **Set Park** | `<04>` | Set current position as park | JSON confirmation |
| **Get Park** | `<05>` | Get saved park position | JSON with park settings |
| **Calibrate** | `<06>` | Recalibrate sensor | JSON progress/completion |
| **Debug Toggle** | `<07>` | Enable/disable debug | JSON with new state |
| **Version** | `<08>` | Get firmware version | JSON with version info |
| **Reset** | `<09>` | Restart device | JSON countdown |

### Configuration Commands

| Command | Code | Description | Example |
|---------|------|-------------|---------|
| **Set Tolerance** | `<0AXXX>` | Set tolerance (XXX=hundredths) | `<0A200>` = 2.00° |
| **Get Tolerance** | `<0B>` | Get current tolerance | JSON response |
| **System Info** | `<0C>` | Get XIAO Sense hardware info | JSON with specs |
| **Software Set Park** | `<0D>` | Set park without button | JSON confirmation |
| **Factory Reset** | `<0E>` | Clear all settings | JSON confirmation |

### Advanced Diagnostic Commands (v2.0.2)

| Command | Code | Description | Example |
|---------|------|-------------|---------|
| **Toggle Filter** | `<0F>` | Enable/disable sensor filtering | JSON response |
| **Set Filter Alpha** | `<10XX>` | Set filter strength | `<1020>` = α=0.20 |
| **Raw Sensor Data** | `<11>` | Get unprocessed sensor readings | JSON with raw data |
| **Storage Test** | `<12>` | Test persistent storage | JSON test results |
| **Sensor Diagnostic** | `<13>` | Comprehensive sensor analysis | JSON diagnostic report |

### Response Format
All responses are JSON:

```json
{
  "status": "ok",
  "data": {
    "pitch": 15.23,
    "roll": -2.67
  }
}
```

### Example Session
```
> <00>
{"status":"ack","command":"00"}
[Command help displayed]

> <02>
{"status":"ack","command":"02"}
{"status":"ok","data":{"pitch":15.23,"roll":-2.67}}

> <04>
{"status":"ack","command":"04"}
{"status":"ok","data":{"parkPitch":15.23,"parkRoll":-2.67,"saved":true}}
```

---

## Operation Guide

### Initial Setup
1. **Connect Device**: Plug XIAO Sense into computer via USB-C
2. **Verify Operation**: Open serial monitor, check for startup messages
3. **Mount Sensor**: Attach securely to telescope or mount
4. **Set Park Position**: Position telescope in park, send `<04>` command

### Daily Use Workflow

#### Before Observing
1. **Check Status**: Send `<01>` or observe built-in LED
2. **Verify Park**: Blue LED should be ON when parked
3. **Begin Session**: Move telescope to targets

#### After Observing  
1. **Return to Park**: Move telescope to park position
2. **Verify Parking**: Blue LED should turn ON
3. **Confirm**: Send `<03>` for detailed status

### LED Status Indicators

| LED State | Meaning |
|-----------|---------|
| **Blue LED ON** | Telescope IS in park position |
| **Blue LED OFF** | Telescope NOT in park position |
| **Blinking Pattern** | Sensor error (check connections) |

### Calibration Procedure
1. **Ensure Stability**: Keep sensor completely still
2. **Start Calibration**: Send `<06>` command
3. **Wait**: ~10 seconds for completion
4. **Verify**: Check position readings for stability

---

## Advanced Features

### Sensor Filtering (v2.0.2)
Control noise vs. responsiveness trade-off:

```bash
<0F>     # Toggle filtering on/off
<1020>   # Set filter alpha to 0.20 (more responsive)
<1080>   # Set filter alpha to 0.80 (more stable)
```

**Filter Alpha Guide**:
- **0.00-0.30**: High responsiveness, more noise
- **0.30-0.70**: Balanced performance (recommended)
- **0.70-0.99**: Maximum stability, slower response

### Storage System
- **Primary**: QSPI Flash (persistent across power cycles)
- **Fallback**: Enhanced RAM storage (lost on power cycle)
- **Auto-detection**: System automatically uses best available storage

### Diagnostic Commands
Get comprehensive sensor health information:

```bash
<11>     # Raw sensor data with calibration offsets
<12>     # Test storage read/write functionality  
<13>     # Full diagnostic with stability analysis
```

---

## Troubleshooting

### Common Issues

#### Device Not Responding
- Check USB-C connection
- Verify correct COM port selection
- Try different USB cable
- Press reset button on XIAO

#### Inaccurate Readings
- Recalibrate: `<06>` command
- Check mounting stability
- Adjust filter settings: `<0F>` and `<10XX>`
- Use raw sensor diagnostic: `<11>`

#### Park Position Not Detected
- Verify park position set: `<05>`
- Check tolerance: `<0B>`
- Increase tolerance if too strict: `<0A300>` (3.0°)
- Re-set park position: `<04>`

#### LED Not Working
- LED is built-in and should always work
- Check with status command: `<01>`
- Try device reset: `<09>`

### Diagnostic Commands
```bash
<01>    # System status
<02>    # Current position  
<03>    # Detailed park analysis
<13>    # Comprehensive diagnostic
<07>    # Enable debug messages
```

### Factory Reset
Clear all settings and start fresh:
```bash
<0E>    # Factory reset command
```

---

## Technical Specifications

### Hardware (XIAO nRF52840 Sense)
- **Processor**: ARM Cortex-M4F @ 64MHz
- **Memory**: 256KB RAM, 1MB Flash
- **IMU**: LSM6DS3TR-C 6-axis (built-in)
- **Connectivity**: USB-C, Bluetooth 5.0 ready
- **Dimensions**: 21mm × 17.5mm
- **Power**: USB or 3.7V LiPo

### Performance
- **Position Resolution**: 0.01°
- **Tolerance Range**: 0.01° to 9.99°
- **Update Rate**: 20Hz (50ms)
- **Accuracy**: ±0.1° typical
- **Boot Time**: ~3 seconds
- **Response Time**: <50ms

### Environmental
- **Operating Temperature**: -10°C to +50°C
- **Humidity**: 0-90% non-condensing
- **Power Consumption**: ~50mA @ 3.3V
- **Battery Life**: 20+ hours (1000mAh)

---

## Development Notes

### Version History
- **v1.0.x**: ESP32 with external MPU6050, physical buttons/LED
- **v2.0.1**: Initial XIAO port with enhanced storage
- **v2.0.2**: Current version with advanced diagnostics and filtering

### Architecture Highlights
- **Modular design** with separate files for each subsystem
- **Enhanced storage** with QSPI flash + RAM fallback
- **Advanced filtering** for noise reduction
- **Comprehensive diagnostics** for troubleshooting
- **JSON API** for easy integration

### Planned Features
- **v2.1.0**: Bluetooth Low Energy serial interface
- **v2.2.0**: Environmental sensors (temperature, humidity)
- **v2.3.0**: Web configuration interface
- **v3.0.0**: Wireless operation with mobile app

### Integration
Works with:
- **ASCOM drivers** via serial interface
- **Custom software** using JSON API
- **Rust bridge application** (in development)
- **Python scripts** for automation

### Contributing
- Code optimization suggestions welcome
- Feature requests via GitHub issues
- Hardware testing and validation
- Documentation improvements

---

## Quick Start Checklist

- [ ] Install Arduino IDE with Seeed nRF52 support
- [ ] Install LSM6DS3 library by Seeed Studio  
- [ ] Connect XIAO Sense via USB-C
- [ ] Upload firmware (select "Seeed XIAO nRF52840 Sense")
- [ ] Open Serial Monitor at 115200 baud
- [ ] Send `<00>` for help, `<01>` for status
- [ ] Mount sensor on telescope
- [ ] Position telescope in park, send `<04>` to set park
- [ ] Test by moving telescope and returning to park
- [ ] Blue LED should indicate park status

**Total setup time**: ~15 minutes

---

**Hardware**: XIAO nRF52840 Sense (~$15)  
**Software**: Arduino IDE (free)  
**External components**: None required  
**Wiring**: None required  

*The simplest telescope park sensor you can build!*