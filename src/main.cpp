#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"
#include "SDCardManager.h"

volatile bool risingEdgeDetected = false;  // Flag to indicate a rising edge has been detected
volatile bool semaphore = false;  // Flag to indicate a rising edge has been detected

// ==================================================
// Motor Instances
// ==================================================
// Initialize A4988 motor drivers for the disc and case motors with their respective pins
A4988Manager discMotor(STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC, 
                       MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
                       SLP_PIN_DISC, RESET_PIN_DISC, true, &risingEdgeDetected,&semaphore); // true motor type 2 stepping linked to sensor

A4988Manager caseMotor(STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE, 
                       MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
                       SLP_PIN_CASE, RESET_PIN_CASE, false, &risingEdgeDetected,&semaphore); // false motor type 1 stepping linked to sensor

// ==================================================
// Object Pointers
// ==================================================
// Initialize pointers for sensor, SD card manager, and command receiver
Sensor* sensor = nullptr;           // Pointer for sensor instance
SDCardManager* SDcard = nullptr;    // Pointer for SD card manager
CommandReceiver* commandReceiver = nullptr; // Pointer for command receiver instance


// Interrupt Service Routine (ISR) to detect rising edge
void IRAM_ATTR risingEdgeISR() {
    semaphore = true;
    // Check if the sensor is high and set the flag
    if(digitalRead(SENSOR_PIN))
    risingEdgeDetected = true;
    if(!digitalRead(SENSOR_PIN))
    risingEdgeDetected = false;
    semaphore = false;
}

// Task to initialize the interrupt
void interruptTask(void* parameter) {
    // Attach the interrupt to the sensor pin
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), risingEdgeISR, CHANGE);
    vTaskDelete(NULL);  // Delete the task after attaching the interrupt
}

void setup() {
    // ==================================================
    // Serial Communication Setup
    // ==================================================
    Serial.begin(BAUDE_RATE); // Initialize serial communication with the defined baud rate
    pinMode(FLAG_LED_PIN, OUTPUT);
    digitalWrite(FLAG_LED_PIN, HIGH);

    // Initialize the sensor pin
    pinMode(SENSOR_PIN, INPUT_PULLUP); // Configure the sensor pin as input with pull-up resistor

    // Create the interrupt task pinned to Core 1
    xTaskCreatePinnedToCore(
        interruptTask,   // Task function to run
        "InterruptTask", // Task name
        1024,            // Stack size for the task
        NULL,            // Parameters for the task
        1,               // Priority of the task
        NULL,            // Task handle
        1                 // Pin the task to Core 1
    );

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
    sensor = new Sensor(SENSOR_PIN); // Create sensor instance with pin and motor reference
    sensor->begin(); // Initialize sensor
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
