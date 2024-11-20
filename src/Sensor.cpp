#include "Sensor.h"
#include "Config.h"

// Initialize static members
Sensor* Sensor::currentSensor = nullptr; ///< Pointer to the current instance of the sensor
TaskHandle_t Sensor::monitoringTaskHandle = nullptr;
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
 * @brief Initializes the sensor and sets up the interrupt.
 * Configures the pin mode and attaches the interrupt to the specified pin.
 */
void Sensor::begin() {
    pinMode(_pin, INPUT); // Configure the sensor pin as input 
}




/**
 * @brief Starts the FreeRTOS task for monitoring the sensor pin.
 *
 * This function checks if the monitoring task (`monitoringTask`) is already running.
 * If the task has not been created yet, it creates the task and assigns it to a core using 
 * `xTaskCreatePinnedToCore`. The task is given a stack size of 2048 bytes and a priority of 1. 
 * The current instance of the `Sensor` object is passed to the task as a parameter, allowing 
 * the task to access the sensor and motor instance for pin monitoring and motor control.
 * 
 * The task is pinned to the core specified by `STEP_CORE`, which should be defined elsewhere 
 * in the code.
 *
 * @note This function should be called to initiate the monitoring of the sensor pin for rising edges.
 * 
 * @see Sensor::monitoringTask For the task that performs the actual monitoring.
 */
void Sensor::startMonitoringTask() {
    if (monitoringTaskHandle == nullptr) {
        xTaskCreatePinnedToCore(
            monitoringTask,      // Task function
            "PinMonitorTask",    // Name of the task
            2048,                // Stack size (in bytes)
            this,                // Parameter to pass (current instance)
            0,                   // Task priority
            &monitoringTaskHandle, // Task handle
            STEP_CORE
        );
    }
}


/**
 * @brief FreeRTOS task that continuously monitors a sensor pin for a rising edge (LOW to HIGH transition).
 *
 * This task runs in an infinite loop, constantly checking the state of the sensor pin.
 * When a rising edge is detected (transition from LOW to HIGH), it prints a message
 * to the serial monitor and triggers a flag in the motor controller by calling the 
 * `SetStopFlag()` function on the associated motor. The task then updates the previous 
 * state of the pin and waits for a specified period before polling the pin state again.
 *
 * The task runs on FreeRTOS and uses `vTaskDelay` to prevent excessive polling of the sensor.
 * The delay between polling is adjustable (currently set to 30 milliseconds).
 *
 * @param pvParameters Pointer to the sensor object instance which holds the pin information
 *                     and references to the motor object.
 * 
 * @note This task should be started using `startMonitoringTask()` to initiate pin monitoring.
 *       The sensor pin should be configured as an input with a pull-up resistor.
 */
void Sensor::monitoringTask(void *pvParameters) {
    Sensor *sensor = static_cast<Sensor*>(pvParameters); // Get the instance
    bool previousState = digitalRead(sensor->_pin);      // Initial pin state

    while (true) {
        bool currentState = digitalRead(sensor->_pin);

        // Detect rising edge (LOW -> HIGH)
        if (previousState == LOW && currentState == HIGH) {
            //Serial.println("Rising edge detected!");
            sensor->_motor->SetStopFlag(); // Call SetStopFlag() on the motor;
        }

        // Update previous state
        previousState = currentState;

        // Delay to prevent rapid polling (adjust as needed)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
