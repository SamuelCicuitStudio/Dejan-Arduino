#ifndef SDCardManager_h
#define SDCardManager_h

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "Config.h" // Include Config.h to use defined pins

class SDCardManager {
  public:
    SDCardManager(); // Constructor
    bool begin(); // Method to initialize the SD card
    bool isInitialized(); // Check if SD card is initialized
    String getError(); // Get error message if initialization fails
    void listFiles(); // List files in the root directory (for testing)

  private:
    bool _initialized;
    String _error;
};

#endif
