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

HardwareSerial* hmiSerial = &Serial2;    // Using Serial2 for HMI communication

void setup() {
    // ==================================================
    // Serial Communication Setup
    // ==================================================
    Serial.begin(BAUDE_RATE); // Initialize serial communication with the defined baud rate
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
    nextionHMI = new NextionHMI(hmiSerial, commandReceiver, caseMotor, discMotor); // Create Nextion HMI instance
    nextionHMI->begin();
}

void loop() {
    // ==================================================
    // Main Loop: Command and Status Handling
    // ==================================================
    commandReceiver->checkCommand(); // Check and process received commands

    String hmiResponse = nextionHMI->readResponse(); // Read response from HMI
    if (!hmiResponse.isEmpty()) {
        nextionHMI->handleButtonPress(hmiResponse); // Handle button press from HMI
    }

    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate >= 2000) { // Update system status every 2 seconds
        nextionHMI->sendSystemStatus();
        lastStatusUpdate = millis();
    }

    delay(1); // Small delay to avoid overwhelming the loop
}
