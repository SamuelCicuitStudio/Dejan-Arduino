#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"
#include "SDCardManager.h"
#include "NextionHMI.h"
void readResponse();
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

HardwareSerial nextionSerial(1); // Use UART1 for communication with Nextion

// ==================================================
// Object Pointers
// ==================================================
Sensor* sensor = nullptr;                // Pointer to sensor instance
SDCardManager* SDcard = nullptr;         // Pointer to SD card manager
CommandReceiver* commandReceiver = nullptr; // Pointer to command receiver instance
NextionHMI* nextionHMI = nullptr;        // Pointer to Nextion HMI instance


void setup() {
    // ==================================================
    // Serial Communication Setup
    // ==================================================
    Serial.begin(BAUDE_RATE); // Initialize serial communication with the defined baud rate
    while (!Serial) {
    ; // Wait for serial to connect
    }
    // Initialize Nextion serial communication
    Serial1.begin(NEXTION_BAUDRATE, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN);
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
    nextionHMI->InitMotorsParameters();//Init motor
}

void loop() {
    // ==================================================
    // Main Loop: Command and Status Handling
    // ==================================================
    readResponse();// get command from serial
    //commandReceiver->checkCommand();
}

   
void readResponse() {
    // ==================================================
    // readResponse: Command and Status Handling
    // ================================================== 
    while(Serial1.available()) {   
            String receivedData = "";
            // Check if data is available from the Nextion HMI serial interface
            while (Serial1.available()) {
                char c = Serial1.read();
                receivedData += c;
            };
            // Process the response if it contains enough data
            if (receivedData.length() > 4) {
                // Extract the 5th character from the response
                char processedData = receivedData.charAt(4);
                // Check if the processed data matches any of the specified characters
            if (processedData == 'A' || processedData == 'B' || processedData == 'C' ||
                processedData == 'D' || processedData == 'H' || processedData == 'I' ||
                processedData == 'S' || processedData == 'P') {
                    //Return the complete response string
                    Serial.println("Received from Nextion:");
                    Serial.print(processedData);
                    Serial.println();
                    // Handle button press from HMI
                    nextionHMI->handleButtonPress(String(processedData));
                } else {
                    // Return an empty string for unsupported characters
                    return ;
                }
            }
        };
        delay(100); // Small delay to avoid overwhelming the loop
        // Return an empty string if the response is too short
        return ;
}