#include "Sensor.h"
#include "Config.h"

// Initialize static members
Sensor* Sensor::currentSensor = nullptr; ///< Pointer to the current instance of the sensor

/**
 * @brief Constructor to initialize the sensor.
 * 
 * @param pin The pin number where the sensor is connected.
 * @param motor The motor that will be controlled by this sensor.
 */
Sensor::Sensor(int pin)
    : _pin(pin) {
    currentSensor = this; // Set current instance of the sensor
}


/**
 * @brief Initializes the sensor and sets up the interrupt.
 * Configures the pin mode and attaches the interrupt to the specified pin.
 */
void Sensor::begin() {
    pinMode(_pin, INPUT); // Configure the sensor pin as input 
}
