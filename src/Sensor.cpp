#include "Sensor.h"
#include "Config.h"
// Initialize static members
int Sensor::stepsToTake = 10; // Initialize stepsToTake to 0
unsigned long Sensor::interruptStartTime = 0; // Track when the interrupt starts
bool Sensor::interruptActive = false; // Track if the interrupt is active
int Sensor::currentStep = 0; // Track the current step
int Sensor::delayBetweenSteps = 2; // Delay between steps
unsigned long Sensor::lastStepTime = 0; // Last time the step was made
int Sensor::originalSpeed = 0; // Store the original speed of the motor
Sensor *Sensor::currentSensor = nullptr; // Initialize the static pointer

// Constructor
Sensor::Sensor(int pin, A4988Manager &motor)
    : _pin(pin), _motor(&motor) {
    currentSensor = this; // Set current instance
}

void Sensor:: SetStopTime(int value){
    StopTime = value;
}

void Sensor:: SetStepsToTake(int value){
stepsToTake = value;
}

void Sensor::begin() {
    pinMode(_pin, INPUT_PULLUP); // Configure pin as input with pull-up resistor
    pinMode(ESPITR_PIN,OUTPUT);
    digitalWrite(ESPITR_PIN,HIGH);//set not ready status
    attachInterrupt(digitalPinToInterrupt(_pin), handleInterrupt, FALLING); // Attach interrupt
}

void Sensor::handleInterrupt() {
    if (currentSensor && !interruptActive) {
        // Use the current instance
        originalSpeed = currentSensor->_motor->getSpeed(); // Get the motor speed
        currentSensor->_motor->setSpeed(0); // Stop the motor

        // Reset step counter and timing variables
        currentSensor->currentStep = 0;
        currentSensor->lastStepTime = millis();
        
        // Start the non-blocking stop timer
        interruptStartTime = millis(); // Record the start time
        interruptActive = true; // Set the interrupt as active
    }
}
void SetReadStatus(){
    digitalWrite(ESPITR_PIN,LOW);
};

void Sensor::update() {
    if (interruptActive) {
        // Perform stepping at defined intervals
        unsigned long currentMillis = millis();
        if (currentStep < stepsToTake && currentMillis - lastStepTime >= delayBetweenSteps) {
            _motor->step(); // Call the step function to take a step
            lastStepTime = currentMillis; // Update the last step time
            currentStep++; // Increment the step counter
        }

        // Check if StopTime has elapsed
        if (millis() - interruptStartTime >= StopTime) {
            _motor->setSpeed(originalSpeed); // Restore original speed
            interruptActive = false; // Reset the interrupt state
        }
    }
}

