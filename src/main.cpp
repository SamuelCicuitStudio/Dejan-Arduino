#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"
#include "SDCardManager.h"

// ==================================================
// Motor Settings
// ==================================================
// Define constants for motor behavior
constexpr int STEPS_TO_TAKE = 100;   // Number of steps to take when the sensor is triggered
constexpr int STOP_TIME = 5000;      // Time to stop in milliseconds

// ==================================================
// Motor Instances
// ==================================================
// Initialize A4988 motor drivers for the disc and case motors with their respective pins
A4988Manager discMotor(STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC, 
                       MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
                       SLP_PIN_DISC, RESET_PIN_DISC, 1); // Timer number 1 for disc motor

A4988Manager caseMotor(STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE, 
                       MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
                       SLP_PIN_CASE, RESET_PIN_CASE, 2); // Timer number 2 for case motor

// ==================================================
// Object Pointers
// ==================================================
// Initialize pointers for sensor, SD card manager, and command receiver
Sensor* sensor = nullptr;           // Pointer for sensor instance
SDCardManager* SDcard = nullptr;    // Pointer for SD card manager
CommandReceiver* commandReceiver = nullptr; // Pointer for command receiver instance

void setup() {
    // ==================================================
    // Serial Communication Setup
    // ==================================================
    Serial.begin(BAUDE_RATE); // Initialize serial communication with the defined baud rate
    pinMode(FLAG_LED_PIN,OUTPUT);
    digitalWrite(FLAG_LED_PIN,HIGH);
    // ==================================================
    // Motor Initialization
    // ==================================================
    discMotor.begin(); // Initialize disc motor with provided settings
    caseMotor.begin(); // Initialize case motor with provided settings
    
    // ==================================================
    // SD Card Initialization
    // ==================================================
    SDcard = new SDCardManager(); // Create SD card manager instance
    SDcard->begin(); // Initialize SD card manager

    // ==================================================
    // Sensor and Command Receiver Initialization
    // ==================================================
    sensor = new Sensor(SENSOR_PIN, discMotor); // Create sensor instance with pin and motor reference
    commandReceiver = new CommandReceiver(sensor, caseMotor, discMotor); // Create command receiver instance with sensor and motor references
    
    sensor->begin(); // Initialize sensor
    commandReceiver->begin(); // Initialize command receiver
}

void loop() {
    // ==================================================
    // Main Loop: Check for Commands and Update Sensor
    // ==================================================
    commandReceiver->checkCommand(); // Check for and process received commands
    sensor->update(); // Update sensor and motor state based on sensor input
    delay(2); // Small delay to avoid overwhelming the loop
}
