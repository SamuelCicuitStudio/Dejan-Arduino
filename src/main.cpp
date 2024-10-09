#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"

// Motor settings
constexpr int STEPS_TO_TAKE = 100; // Number of steps to take when the sensor is triggered
constexpr int STOP_TIME = 5000;     // Time to stop in milliseconds

// Motor instances
A4988Manager discMotor(STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC, 
                       MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
                       SLP_PIN_DISC, RESET_PIN_DISC, 1); // Timer number 1

A4988Manager caseMotor(STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE, 
                       MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
                       SLP_PIN_CASE, RESET_PIN_CASE, 2); // Timer number 2

Sensor* sensor = nullptr; // Initialize sensor pointer
CommandReceiver* commandReceiver = nullptr; // Command receiver instance

void setup() {
    Serial.begin(9600); // Initialize serial communication
    discMotor.begin(); // Initialize disc motor
    caseMotor.begin(); // Initialize case motor
    
    sensor = new Sensor(SENSOR_PIN, discMotor); // Create sensor instance
    commandReceiver = new CommandReceiver(sensor, caseMotor, discMotor); // Create command receiver instance
    sensor->begin(); // Initialize sensor
    commandReceiver->begin(); // Initialize command receiver
    //sensor->SetReadStatus();//set the ready status
}

void loop() {
    commandReceiver->checkCommand(); // Check for received commands
    sensor->update(); // Update sensor and motor state
    delay(2); 
}

