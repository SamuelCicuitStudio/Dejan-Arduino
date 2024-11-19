#include "SDCardManager.h"

// Constructor: No parameters needed since pins are defined in Config.h
SDCardManager::SDCardManager() {
  _initialized = false;
  _error = "";
}

// Method to initialize the SD card
bool SDCardManager::begin() {
  // Set up the SPI interface with custom pins
  SPI.begin(SD_CLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_NCS_PIN);
  
  // Initialize the SD card
  if (!SD.begin(SD_NCS_PIN)) {
    _initialized = false;
    _error = "SD Card initialization failed!";
    return false;
  } else {
    _initialized = true;
    _error = "";
    return true;
  }
}

// Check if the SD card is initialized
bool SDCardManager::isInitialized() {
  return _initialized;
}

// Return the error message if initialization fails
String SDCardManager::getError() {
  return _error;
}

// List files in the root directory (for debugging purposes)
void SDCardManager::listFiles() {
  if (!_initialized) {
    Serial.println("SD Card not initialized.");
    return;
  }

  File root = SD.open("/");
  if (root) {
    Serial.println("Listing files on SD Card:");
    while (true) {
      File entry = root.openNextFile();
      if (!entry) {
        break;
      }
      if (entry.isDirectory()) {
        Serial.print("[DIR] ");
        Serial.println(entry.name());
      } else {
        Serial.print("[FILE] ");
        Serial.print(entry.name());
        Serial.print(" - ");
        Serial.println(entry.size());
      }
      entry.close();
    }
    root.close();
  } else {
    Serial.println("Failed to open root directory.");
  }
}
