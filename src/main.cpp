#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"
#include "SDCardManager.h"


// ==================================================
// Motor Instances
// ==================================================
// Initialize A4988 motor drivers for the disc and case motors with their respective pins
A4988Manager discMotor(STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC, 
                       MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
                       SLP_PIN_DISC, RESET_PIN_DISC,true); // true motor type 2 stepping inked to sensor

A4988Manager caseMotor(STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE, 
                       MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
                       SLP_PIN_CASE, RESET_PIN_CASE,false); // false motor type 1 stepping inked to sensor

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
    sensor->begin(); // Initialize sensor
    sensor->startMonitoringTask();// Start monitoring class
    commandReceiver = new CommandReceiver(sensor, caseMotor, discMotor); // Create command receiver instance with sensor and motor references  
    commandReceiver->begin(); // Initialize command receiver
}

void loop() {
    // ==================================================
    // Main Loop: Check for Commands and Update Sensor
    // ==================================================
    commandReceiver->checkCommand(); // Check for and process received commands
    delay(1); // Small delay to avoid overwhelming the loop
}
