#include "flash_storage.h"
#include "Debug.h"
#include "nrfx_qspi.h"

#define SETTINGS_MAGIC 0x54454C45
#define SETTINGS_ADDRESS 0x1000

// QSPI Commands (from Seeed example)
#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99
#define QSPI_DPM_ENTER      0x0003
#define QSPI_DPM_EXIT       0x0003

// From Seeed example
static uint32_t *QSPI_Status_Ptr = (uint32_t*) 0x40029604;
static nrfx_qspi_config_t QSPIConfig;
static nrf_qspi_cinstr_conf_t QSPICinstr_cfg;
static bool QSPIWait = false;

static TelescopeSettings currentSettings;
static bool flashStorageInitialized = false;
static bool persistentStorageAvailable = false;

// Verbatim from Seeed example
static nrfx_err_t QSPI_IsReady() {
  if (((*QSPI_Status_Ptr & 8) == 8) && (*QSPI_Status_Ptr & 0x01000000) == 0) {
    return NRFX_SUCCESS;  
  } else {
   return NRFX_ERROR_BUSY; 
  }
}

// Verbatim from Seeed example
static nrfx_err_t QSPI_WaitForReady() {
  while (QSPI_IsReady() == NRFX_ERROR_BUSY) {
    delay(1);
  }
  return NRFX_SUCCESS;
}

// Verbatim from Seeed example
static void QSIP_Configure_Memory() {
  uint8_t  temporary[] = {0x00, 0x02};
  
  QSPICinstr_cfg.opcode = QSPI_STD_CMD_RSTEN;
  QSPICinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
  QSPICinstr_cfg.io2_level = true;
  QSPICinstr_cfg.io3_level = true;
  QSPICinstr_cfg.wipwait = QSPIWait;
  QSPICinstr_cfg.wren = true;
  
  QSPI_WaitForReady();
  if (nrfx_qspi_cinstr_xfer(&QSPICinstr_cfg, NULL, NULL) != NRFX_SUCCESS) {
    Debug.println("QSPI reset enable failed");
  } else {
    QSPICinstr_cfg.opcode = QSPI_STD_CMD_RST;
    QSPI_WaitForReady();
    if (nrfx_qspi_cinstr_xfer(&QSPICinstr_cfg, NULL, NULL) != NRFX_SUCCESS) {
      Debug.println("QSPI reset failed");
    } else {
      QSPICinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
      QSPICinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
      QSPI_WaitForReady();
      if (nrfx_qspi_cinstr_xfer(&QSPICinstr_cfg, &temporary, NULL) != NRFX_SUCCESS) {
        Debug.println("QSPI mode switch failed");
      }
    }
  }
}

bool initFlashStorage() {
    Debug.println("Initializing QSPI (Seeed example method)...");
    
    // Initialize defaults
    currentSettings.magic = SETTINGS_MAGIC;
    currentSettings.parkPitch = 0.0;
    currentSettings.parkRoll = 0.0;
    currentSettings.tolerance = 2.0;
    currentSettings.cal_ax_offset = 0.0;
    currentSettings.cal_ay_offset = 0.0;
    currentSettings.cal_az_offset = 0.0;
    currentSettings.cal_gx_offset = 0.0;
    currentSettings.cal_gy_offset = 0.0;
    currentSettings.cal_gz_offset = 0.0;
    currentSettings.cal_timestamp = 0;
    currentSettings.checksum = 0;
    
    // QSPI Config - Verbatim from Seeed example
    QSPIConfig.xip_offset = NRFX_QSPI_CONFIG_XIP_OFFSET;
    QSPIConfig.pins.sck_pin = 21;
    QSPIConfig.pins.csn_pin = 25;
    QSPIConfig.pins.io0_pin = 20;
    QSPIConfig.pins.io1_pin = 24;
    QSPIConfig.pins.io2_pin = 22;
    QSPIConfig.pins.io3_pin = 23;
    QSPIConfig.irq_priority = (uint8_t)NRFX_QSPI_CONFIG_IRQ_PRIORITY;
    QSPIConfig.prot_if.readoc = (nrf_qspi_readoc_t)NRF_QSPI_READOC_READ4O;
    QSPIConfig.prot_if.writeoc = (nrf_qspi_writeoc_t)NRF_QSPI_WRITEOC_PP4O;
    QSPIConfig.prot_if.addrmode = (nrf_qspi_addrmode_t)NRFX_QSPI_CONFIG_ADDRMODE;
    QSPIConfig.prot_if.dpmconfig = true;
    QSPIConfig.phy_if.sck_freq = (nrf_qspi_frequency_t)NRF_QSPI_FREQ_32MDIV1;
    QSPIConfig.phy_if.spi_mode = (nrf_qspi_spi_mode_t)NRFX_QSPI_CONFIG_MODE;
    QSPIConfig.phy_if.dpmen = false;
    
    // DPM setup from example
    NRF_QSPI->DPMDUR = (QSPI_DPM_ENTER << 16) | QSPI_DPM_EXIT;
    
    // Initialize QSPI (retry loop from example)
    uint32_t Error_Code = 1;
    while (Error_Code != 0) {
        Error_Code = nrfx_qspi_init(&QSPIConfig, NULL, NULL);
        if (Error_Code != NRFX_SUCCESS) {
            Debug.println("QSPI init failed: " + String(Error_Code));
        } else {
            Debug.println("QSPI init successful");
        }
    }
    
    QSIP_Configure_Memory();
    NRF_QSPI->TASKS_ACTIVATE = 1;
    QSPI_WaitForReady();
    
    if (QSPI_IsReady() == NRFX_SUCCESS) {
        persistentStorageAvailable = true;
        Debug.println("✓ QSPI ready");
        
        // Try loading settings
        if (loadSettingsFromFlash(currentSettings)) {
            Debug.println("✓ Settings loaded from QSPI");
        }
    } else {
        Debug.println("⚠ QSPI not ready");
    }
    
    flashStorageInitialized = true;
    return persistentStorageAvailable;
}

bool isFlashStorageAvailable() {
    return persistentStorageAvailable;
}

uint32_t calculateChecksum(const TelescopeSettings& settings) {
    uint32_t checksum = 0;
    const uint8_t* data = (const uint8_t*)&settings;
    size_t dataSize = sizeof(TelescopeSettings) - sizeof(uint32_t);
    
    for (size_t i = 0; i < dataSize; i++) {
        checksum += data[i];
    }
    return checksum;
}

bool saveSettingsToFlash(const TelescopeSettings& settings) {
    if (!persistentStorageAvailable) return false;
    
    TelescopeSettings settingsToSave = settings;
    settingsToSave.checksum = calculateChecksum(settingsToSave);
    
    QSPI_WaitForReady();
    if (nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, SETTINGS_ADDRESS) != NRFX_SUCCESS) {
        Debug.println("QSPI erase failed");
        return false;
    }
    
    QSPI_WaitForReady();
    if (nrfx_qspi_write(&settingsToSave, sizeof(TelescopeSettings), SETTINGS_ADDRESS) != NRFX_SUCCESS) {
        Debug.println("QSPI write failed");
        return false;
    }
    
    QSPI_WaitForReady();
    Debug.println("✓ Settings saved to QSPI");
    return true;
}

bool loadSettingsFromFlash(TelescopeSettings& settings) {
    if (!persistentStorageAvailable) return false;
    
    QSPI_WaitForReady();
    if (nrfx_qspi_read(&settings, sizeof(TelescopeSettings), SETTINGS_ADDRESS) != NRFX_SUCCESS) {
        return false;
    }
    
    QSPI_WaitForReady();
    
    if (settings.magic != SETTINGS_MAGIC) {
        return false;
    }
    
    if (settings.checksum != calculateChecksum(settings)) {
        return false;
    }
    
    currentSettings = settings;
    return true;
}

bool eraseSettingsFlash() {
    if (!persistentStorageAvailable) return false;
    QSPI_WaitForReady();
    return nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, SETTINGS_ADDRESS) == NRFX_SUCCESS;
}

// Helper functions
bool saveFloatToFlash(const char* key, float value) {
    if (!flashStorageInitialized) return false;
    
    String keyStr = String(key);
    
    if (keyStr == "parkPitch") currentSettings.parkPitch = value;
    else if (keyStr == "parkRoll") currentSettings.parkRoll = value;
    else if (keyStr == "tolerance") currentSettings.tolerance = value;
    else if (keyStr == "cal_ax_offset") currentSettings.cal_ax_offset = value;
    else if (keyStr == "cal_ay_offset") currentSettings.cal_ay_offset = value;
    else if (keyStr == "cal_az_offset") currentSettings.cal_az_offset = value;
    else if (keyStr == "cal_gx_offset") currentSettings.cal_gx_offset = value;
    else if (keyStr == "cal_gy_offset") currentSettings.cal_gy_offset = value;
    else if (keyStr == "cal_gz_offset") currentSettings.cal_gz_offset = value;
    else if (keyStr == "cal_timestamp") currentSettings.cal_timestamp = (uint32_t)value;
    else return false;
    
    if (persistentStorageAvailable) {
        bool success = saveSettingsToFlash(currentSettings);
        Debug.println("Saved " + String(key) + " = " + String(value, 4) + (success ? " (QSPI)" : " (failed)"));
        return success;
    } else {
        Debug.println("Saved " + String(key) + " = " + String(value, 4) + " (RAM)");
        return true;
    }
}

float loadFloatFromFlash(const char* key, float defaultValue) {
    if (!flashStorageInitialized) return defaultValue;
    
    String keyStr = String(key);
    
    if (keyStr == "parkPitch") return currentSettings.parkPitch;
    else if (keyStr == "parkRoll") return currentSettings.parkRoll;
    else if (keyStr == "tolerance") return currentSettings.tolerance;
    else if (keyStr == "cal_ax_offset") return currentSettings.cal_ax_offset;
    else if (keyStr == "cal_ay_offset") return currentSettings.cal_ay_offset;
    else if (keyStr == "cal_az_offset") return currentSettings.cal_az_offset;
    else if (keyStr == "cal_gx_offset") return currentSettings.cal_gx_offset;
    else if (keyStr == "cal_gy_offset") return currentSettings.cal_gy_offset;
    else if (keyStr == "cal_gz_offset") return currentSettings.cal_gz_offset;
    else if (keyStr == "cal_timestamp") return (float)currentSettings.cal_timestamp;
    
    return defaultValue;
}

bool clearAllFlashSettings() {
    currentSettings.magic = SETTINGS_MAGIC;
    currentSettings.parkPitch = 0.0;
    currentSettings.parkRoll = 0.0;
    currentSettings.tolerance = 2.0;
    currentSettings.cal_ax_offset = 0.0;
    currentSettings.cal_ay_offset = 0.0;
    currentSettings.cal_az_offset = 0.0;
    currentSettings.cal_gx_offset = 0.0;
    currentSettings.cal_gy_offset = 0.0;
    currentSettings.cal_gz_offset = 0.0;
    currentSettings.cal_timestamp = 0;
    
    if (persistentStorageAvailable) {
        return eraseSettingsFlash();
    }
    return true;
}