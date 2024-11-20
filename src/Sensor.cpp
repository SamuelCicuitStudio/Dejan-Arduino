#include "Sensor.h"
#include "Config.h"

// Initialize static members
int Sensor::stepsToTake = 10;           ///< Number of steps the sensor will take
unsigned long Sensor::interruptStartTime = 0; ///< Time when the interrupt starts
bool Sensor::interruptActive = false;    ///< Flag indicating if the interrupt is active
int Sensor::currentStep = 0;             ///< Current step count during sensor operation
int Sensor::delayBetweenSteps = 2;       ///< Delay between each step (in milliseconds)
unsigned long Sensor::lastStepTime = 0;  ///< Last time a step was performed
int Sensor::originalSpeed = 0;           ///< Original speed of the motor before interruption
Sensor* Sensor::currentSensor = nullptr; ///< Pointer to the current instance of the sensor

/**
 * @brief Constructor to initialize the sensor.
 * 
 * @param pin The pin number where the sensor is connected.
 * @param motor The motor that will be controlled by this sensor.
 */
Sensor::Sensor(int pin, A4988Manager& motor)
    : _pin(pin), _motor(&motor) {
    currentSensor = this; // Set current instance of the sensor
}

/**
 * @brief Sets the stop time for the sensor's operation.
 * 
 * @param value The stop time in milliseconds.
 */
void Sensor::SetStopTime(int value) {
    StopTime = value; // Assign the stop time
}

/**
 * @brief Sets the number of steps the sensor should take.
 * 
 * @param value The number of steps to take.
 */
void Sensor::SetStepsToTake(int value) {
    stepsToTake = value; // Assign the number of steps
}

/**
 * @brief Initializes the sensor and sets up the interrupt.
 * Configures the pin mode and attaches the interrupt to the specified pin.
 */
void Sensor::begin() {
    pinMode(_pin, INPUT_PULLUP); // Configure the sensor pin as input with pull-up resistor
    attachInterrupt(digitalPinToInterrupt(_pin), handleInterrupt, FALLING); // Attach interrupt to pin
    SetStopTime(STOP_TIME);  // Set default stop time
    SetStepsToTake(STEPS_TO_TAKE);  // Set default steps to take
}

/**
 * @brief Interrupt handler to pause the motor and start stepping.
 * This function is called when the sensor's interrupt is triggered.
 */
void Sensor::handleInterrupt() {
    if (currentSensor && !interruptActive) {
        // Ensure there's a valid sensor instance and the interrupt isn't active
        originalSpeed = currentSensor->_motor->getSpeed(); // Save the motor's current speed
        currentSensor->_motor->setFrequency(0.0); // Stop the motor

        // Reset the step counter and timing variables
        currentSensor->currentStep = 0;
        currentSensor->lastStepTime = millis();

        // Start the non-blocking stop timer
        interruptStartTime = millis(); // Record the start time of the interrupt
        interruptActive = true; // Mark the interrupt as active
    }
}

/**
 * @brief Updates the sensor's state by stepping the motor at defined intervals.
 * It checks if the stop time has elapsed and restores the motor's speed.
 */
void Sensor::update() {
    if (interruptActive) {
        unsigned long currentMillis = millis(); // Get the current time

        // Perform stepping at the defined intervals
        if (currentStep < stepsToTake && currentMillis - lastStepTime >= delayBetweenSteps) {
            _motor->step(); // Call the step function to take a step
            lastStepTime = currentMillis; // Update the last step time
            currentStep++; // Increment the step count
        }

        // Check if the StopTime has elapsed
        if (millis() - interruptStartTime >= StopTime) {
            _motor->setFrequency(originalSpeed); // Restore the motor's original speed
            interruptActive = false; // Mark the interrupt as inactive
        }
    }
}

/**
 * @brief Gets the current stop time of the sensor.
 * 
 * @return The current stop time in milliseconds.
 */
uint32_t Sensor::GetStopTime() {
    return StopTime; // Return the stop time
}

/**
 * @brief Gets the current number of steps to be taken by the sensor.
 * 
 * @return The number of steps to take.
 */
uint32_t Sensor::GetStepsToTake() {
    return stepsToTake; // Return the number of steps
}
