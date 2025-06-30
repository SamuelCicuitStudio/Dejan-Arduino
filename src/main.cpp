#include <Arduino.h>               // Include Arduino core library for basic functionality
#include "A4988Manager.h"           // Include motor driver manager library for controlling A4988 stepper motors
#include "Sensor.h"                 // Include the sensor library for sensor interaction
#include "CommandReceiver.h"        // Include the command receiver library for interpreting commands
#include "config.h"                 // Include configuration header for pin definitions and settings
#include "SDCardManager.h"          // Include SD card manager for handling SD card operations
#include "NextionHMI.h"             // Include Nextion HMI library for display interactions
#include <Preferences.h>            // ESP32 Preferences library for non-volatile storage (settings persistence)

void readResponse();               // Declare function to handle serial responses from Nextion HMI

// ==================================================
// Motor Instances
// ==================================================
// Initialize A4988 motor drivers for disc and case motors, setting their respective pins
A4988Manager discMotor(
  STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC,
  MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
  SLP_PIN_DISC, RESET_PIN_DISC, true  // Motor type 2 (linked to sensor)
);

A4988Manager caseMotor(
  STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE,
  MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
  SLP_PIN_CASE, RESET_PIN_CASE, false // Motor type 1
);

HardwareSerial nextionSerial(1);  // Declare HardwareSerial object for Nextion display (UART1)
Preferences prefs;               // Declare Preferences object for non-volatile storage

// ==================================================
// Object Pointers
// ==================================================
// Declare pointers to objects for sensor, SD card manager, command receiver, Nextion HMI, and configuration manager
Sensor* sensor = nullptr;             // Pointer to Sensor instance
SDCardManager* SDcard = nullptr;      // Pointer to SD card manager instance
CommandReceiver* commandReceiver = nullptr; // Pointer to command receiver instance
NextionHMI* nextionHMI = nullptr;     // Pointer to Nextion HMI instance
ConfigManager* Config = nullptr;      // Pointer to configuration manager instance

void setup() {
  // ==================================================
  // Serial Communication Setup
  // ==================================================
  Serial.begin(BAUDE_RATE);         // Initialize serial communication with the specified baud rate
  while (!Serial) { ; }             // Wait for serial connection to establish
  Serial.println("Serial console initialized 🖥️");  // Print message to indicate successful serial connection

  // ==================================================
  // Preferences & Config Manager Initialization
  // ==================================================
  Serial.println("Opening Preferences storage ⚙️"); // Print message for Preferences setup
  prefs.begin(CONFIG_PARTITION, false); // Start Preferences with the specified partition (non-read-only)
  Serial.println("Initializing Config Manager 🛠️");  // Print message for Config manager setup
  Config = new ConfigManager(&prefs); // Create instance of ConfigManager with Preferences object
  Config->begin();                   // Begin configuration process
  Serial.println("Config Manager ready ✅");  // Print confirmation message after config initialization

  // ==================================================
  // Nextion HMI Serial Communication Setup
  // ==================================================
  Serial1.begin(NEXTION_BAUDRATE, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN); // Start Nextion serial communication
  Serial.println("Nextion HMI serial initialized 📺"); // Print message to indicate Nextion communication setup

  // ==================================================
  // Flag LED Indicator Setup
  // ==================================================
  pinMode(FLAG_LED_PIN, OUTPUT);        // Set FLAG_LED_PIN as an output pin for LED control
  digitalWrite(FLAG_LED_PIN, HIGH);     // Set the FLAG LED to HIGH (turn on LED)
  Serial.println("LED Flag ON 🔴");      // Print message indicating that the LED flag is on

  // ==================================================
  // Motor Initialization
  // ==================================================
  Serial.println("Initializing motors ⚙️");   // Print message to indicate motor initialization
  discMotor.begin();                        // Initialize disc motor
  caseMotor.begin();                        // Initialize case motor
  Serial.println("Motors ready ✅");         // Print message indicating motors are ready

  // ==================================================
  // SD Card Initialization
  // ==================================================
  Serial.println("Initializing SD card 💾");  // Print message for SD card setup
  SDcard = new SDCardManager();             // Create instance of SDCardManager
  if (SDcard->begin()) {                    // Try to initialize SD card
    Serial.println("SD card initialized successfully 📂"); // Print message for successful SD card initialization
  } else {
    Serial.println("❌ SD card initialization failed");  // Print error message if SD card initialization fails
  }

  // ==================================================
  // Sensor & Command Receiver Initialization
  // ==================================================
  Serial.println("Initializing sensor 📡"); // Print message for sensor initialization
  sensor = new Sensor(SENSOR_PIN);         // Create instance of Sensor with the defined pin
  sensor->begin();                         // Initialize the sensor
  Serial.println("Sensor initialized ✅");  // Print message confirming the sensor is initialized
  Serial.println("Setting up Command Receiver 🎛️");  // Print message for command receiver setup
  commandReceiver = new CommandReceiver(sensor, caseMotor, discMotor); // Create instance of CommandReceiver
  commandReceiver->begin();                        // Initialize the command receiver
  Serial.println("Command Receiver initialized ✅"); // Print message confirming command receiver is initialized

  // ==================================================
  // Nextion HMI Setup
  // ==================================================
  Serial.println("Starting Nextion HMI Manager 🖥️"); // Print message for HMI manager setup
  nextionHMI = new NextionHMI(commandReceiver, caseMotor, discMotor, Config); // Create instance of Nextion HMI manager
  nextionHMI->begin();                             // Initialize the HMI manager
  Serial.println("System initialization complete ✅");  // Print message confirming all system components initialized
  nextionHMI->sendSystemStatus();
}

void loop() {
  readResponse();  // Call function to read response from Nextion display
}

// ==================================================
// Read Response: Handle Incoming Data from Nextion HMI
// ==================================================
void readResponse() {
  while (Serial1.available()) {      // While data is available from Nextion via UART1
    String receivedData = "";         // Initialize empty string to hold incoming data

    // Read all available data from serial
    while (Serial1.available()) {
      char c = Serial1.read();        // Read each character
      receivedData += c;              // Append character to the receivedData string
    }

    // Process the data if its length is greater than 4 characters
    if (receivedData.length() > 4) {
      char processedData = receivedData.charAt(4);  // Extract the 5th character from the received data string

      // Check if the extracted character is a valid command (A-K)
      if (processedData == 'A' || processedData == 'B' || processedData == 'C' ||
          processedData == 'D' || processedData == 'H' || processedData == 'I' ||
          processedData == 'E' || processedData == 'F' || processedData == 'G' ||
          processedData == 'S' || processedData == 'P' || processedData == 'J' ||
          processedData == 'K'|| processedData == 'W') {

        Serial.print("📨 Received from Nextion: ");   // Print received command for debugging
        Serial.println(processedData);

        // Forward the valid command to the Nextion HMI handler
        nextionHMI->handleButtonPress(String(processedData));
      } else {
        // If the command is invalid, print a warning and return
        Serial.print("⚠️ Unsupported character received: ");
        Serial.println(processedData);
        return;  // Exit the function since the data is not supported
      }
    }
  }
  delay(100);  // Add a small delay to prevent overwhelming the loop
}
