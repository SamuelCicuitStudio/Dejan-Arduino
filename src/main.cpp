#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"
#include "SDCardManager.h"
#include "NextionHMI.h"

// ==================================================
// Motor Instances
// ==================================================
// Initialize A4988 motor drivers for disc and case motors with their respective pins
A4988Manager discMotor(
    STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC,
    MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
    SLP_PIN_DISC, RESET_PIN_DISC, true // Motor type 2 stepping linked to sensor
);

A4988Manager caseMotor(
    STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE,
    MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
    SLP_PIN_CASE, RESET_PIN_CASE, false // Motor type 1 stepping linked to sensor
);



// ==================================================
// Object Pointers
// ==================================================
Sensor* sensor = nullptr;                // Pointer to sensor instance
SDCardManager* SDcard = nullptr;         // Pointer to SD card manager
CommandReceiver* commandReceiver = nullptr; // Pointer to command receiver instance
NextionHMI* nextionHMI = nullptr;        // Pointer to Nextion HMI instance

String GetResponse() ;

void setup() {
    // ==================================================
    // Serial Communication Setup
    // ==================================================
    Serial.begin(BAUDE_RATE); // Initialize serial communication with the defined baud rate
    while (!Serial) {
    ; // Wait for serial to connect
    }
    // Initialize Nextion serial communication
    Serial1.begin(9600, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN);
    Serial.println("ESP32 Ready. Listening to Nextion Display...");
    pinMode(FLAG_LED_PIN, OUTPUT);
    digitalWrite(FLAG_LED_PIN, HIGH);

    // ==================================================
    // Motor Initialization
    // ==================================================
    discMotor.begin(); // Initialize disc motor
    caseMotor.begin(); // Initialize case motor

    // ==================================================
    // SD Card Initialization
    // ==================================================
    SDcard = new SDCardManager();
    if (SDcard->begin()) {
        Serial.println("SD card initialized successfully.");
    } else {
        Serial.println("Failed to initialize SD card.");
    }

    // ==================================================
    // Sensor and Command Receiver Initialization
    // ==================================================
    sensor = new Sensor(SENSOR_PIN);      // Create sensor instance
    sensor->begin();                     // Initialize sensor

    commandReceiver = new CommandReceiver(sensor, caseMotor, discMotor); // Create command receiver
    commandReceiver->begin();            // Initialize command receiver

    // ==================================================
    // Nextion HMI Initialization
    // ==================================================
    nextionHMI = new NextionHMI(commandReceiver, caseMotor, discMotor); // Create Nextion HMI instance
    nextionHMI->begin();
}

void loop() {
    // ==================================================
    // Main Loop: Command and Status Handling
    // ==================================================
    
    // Read response from HMI
    String hmiResponse = GetResponse();
    

    // Handle button press from HMI
    nextionHMI->handleButtonPress(hmiResponse);

    // Process the first character of the HMI response
    if (hmiResponse.length() > 0) { // Ensure response is not empty
        char processedData = hmiResponse.charAt(0);
        Serial.println(hmiResponse);
        // Check if the processed character matches any valid status commands
        if (processedData == 'A' || processedData == 'B' || processedData == 'C' ||
            processedData == 'E' || processedData == 'F' || processedData == 'G' ||
            processedData == 'S' || processedData == 'P' || processedData == 'I' ||
            processedData == 'H') {
            
            // Send system status to the HMI
            nextionHMI->sendSystemStatus();
        }
    } else {
        //Serial.println("No valid response received from HMI.");
    }

    delay(1); // Small delay to avoid overwhelming the loop
}

String GetResponse() {
  // Check if data is available from Nextion display
  if (Serial1.available()) {
    // Read the data from the Nextion display
    String receivedData = "";
    while (Serial1.available()) {
      char c = Serial1.read();
      receivedData += c;
    }

    // Ensure the received data has enough characters before processing
    if (receivedData.length() > 4) {
      // Extract the 5th character from the received data
      char processedData = receivedData.charAt(4);

      // Check if the processed data matches any of the specified characters
      if (processedData == 'A' || processedData == 'B' || processedData == 'C' ||
          processedData == 'D' || processedData == 'H' || processedData == 'I' ||
          processedData == 'S' || processedData == 'P') {
        // Print the processed data to the Serial Monitor
        Serial.println("Received from Nextion:");
        Serial.println(processedData);
        return (String) processedData;
      } else {
        Serial.println("Received an unsupported character.");
        return "";
      }
    } else {
      Serial.println("Received data is too short to process.");
    }
  }
  return "";
}