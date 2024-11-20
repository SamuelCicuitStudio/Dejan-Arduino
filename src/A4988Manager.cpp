#include "A4988Manager.h"
#include <Arduino.h>

/**
 * @brief Constructor for the A4988Manager class
 * 
 * Initializes the motor driver with the provided pins for step, direction, enable,
 * microstepping pins, sleep, and reset pins.
 * 
 * @param stepPin Pin for the step signal.
 * @param dirPin Pin for the direction signal.
 * @param enablePin Pin to enable/disable the driver.
 * @param ms1Pin Microstepping mode pin 1.
 * @param ms2Pin Microstepping mode pin 2.
 * @param ms3Pin Microstepping mode pin 3.
 * @param slpPin Sleep pin to wake the driver.
 * @param resetPin Reset pin to reset the driver.
 */
A4988Manager::A4988Manager(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                           uint8_t ms1Pin, uint8_t ms2Pin, uint8_t ms3Pin,
                           uint8_t slpPin, uint8_t resetPin,bool _Number)
    : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin),
      _ms1Pin(ms1Pin), _ms2Pin(ms2Pin), _ms3Pin(ms3Pin),
      _slpPin(slpPin), _resetPin(resetPin),_Number(_Number),
      _stepping(false), _frequency(0), _interval(0), _lastStepTime(0),
      _microSteps(1), _stepTaskHandle(nullptr) {}

/**
 * @brief Initializes the motor driver and sets pin modes.
 * 
 * This function sets all relevant pins to OUTPUT mode, disables the driver,
 * wakes up the driver, and resets it.
 */
void A4988Manager::begin() {
    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
    pinMode(_enablePin, OUTPUT);
    pinMode(_ms1Pin, OUTPUT);
    pinMode(_ms2Pin, OUTPUT);
    pinMode(_ms3Pin, OUTPUT);
    pinMode(_slpPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);

    Stop(); // Disable the driver
    digitalWrite(_slpPin, HIGH); // Wake up the driver
    Reset(); // Reset the driver
    ResetStopFlag(); // Reset the stop flag to resume normal stepping
    stepsToTake = DEFAULT_STEPS_TO_TAKE;
    StopTime = DEFAULT_STOP_TIME;
}

/**
 * @brief Sets the microstepping mode using the MS1, MS2, and MS3 pins.
 * 
 * @param ms1 State of the MS1 pin (HIGH/LOW).
 * @param ms2 State of the MS2 pin (HIGH/LOW).
 * @param ms3 State of the MS3 pin (HIGH/LOW).
 */
void A4988Manager::setMicrostepping(int ms1, int ms2, int ms3) {
    digitalWrite(_ms1Pin, ms1);
    digitalWrite(_ms2Pin, ms2);
    digitalWrite(_ms3Pin, ms3);
}

/**
 * @brief Sets the step resolution for the motor driver.
 * 
 * @param resolution Step resolution (1 = full step, 2 = half step, 4 = quarter step, etc.).
 */
void A4988Manager::setStepResolution(int resolution) {
    switch (resolution) {
        case 1:  // Full step
            setMicrostepping(LOW, LOW, LOW);
            _microSteps = resolution;
            break;
        case 2:  // Half step
            setMicrostepping(HIGH, LOW, LOW);
            _microSteps = resolution;
            break;
        case 4:  // Quarter step
            setMicrostepping(LOW, HIGH, LOW);
            _microSteps = resolution;
            break;
        case 8:  // Eighth step
            setMicrostepping(HIGH, HIGH, LOW);
            _microSteps = resolution;
            break;
        case 16: // Sixteenth step
            setMicrostepping(HIGH, HIGH, HIGH);
            _microSteps = resolution;
            break;
        default:
            Serial.println("Invalid resolution.");
            break;
    }
}

/**
 * @brief Sets the direction of the motor (clockwise or counter-clockwise).
 * 
 * @param value Direction value (HIGH = clockwise, LOW = counter-clockwise).
 */
void A4988Manager::setDirPin(bool value) {
    digitalWrite(_dirPin, value); // Set direction of the driver
}

/**
 * @brief Enables the motor driver.
 * 
 * This function sets the enable pin LOW to activate the motor driver.
 */
void A4988Manager::Start() {
    digitalWrite(_enablePin, LOW); // Enable the driver
}

/**
 * @brief Disables the motor driver.
 * 
 * This function sets the enable pin HIGH to disable the motor driver.
 */
void A4988Manager::Stop() {
    digitalWrite(_enablePin, HIGH); // Disable the driver
}

/**
 * @brief Resets the motor driver.
 * 
 * This function sets the reset pin LOW, waits for a short duration, then sets the
 * reset pin HIGH to re-enable the driver.
 */
void A4988Manager::Reset() {
    digitalWrite(_resetPin, LOW); // Set reset pin low
    delayMicroseconds(100);        // Wait for a short duration
    digitalWrite(_resetPin, HIGH); // Set reset pin high to re-enable the driver
}

/**
 * @brief Sets the frequency of the stepping signal (in Hz).
 * 
 * @param frequency Frequency in Hz.
 */
void A4988Manager::setFrequency(float frequency) {
    _frequency = frequency;
    if (_frequency == 0.0) {
        stopMotorTask(); // Stop motor task if frequency is zero
    } else {
        // Calculate step interval based on frequency and microsteps
        _interval = 1000.0 / (_frequency); // Interval in milliseconds
        Start();// enable the driver
        startMotorTask(); // Start motor task if frequency is non-zero
    }
}

/**
 * @brief Gets the current speed of the motor.
 * 
 * @return The current frequency (speed) of the motor.
 */
float A4988Manager::getSpeed() {
    return _frequency;
}

/**
 * @brief Gets the direction pin.
 * 
 * @return The direction pin.
 */
int A4988Manager::getDir() {
    return digitalRead(_dirPin);
}

/**
 * @brief Gets the current step resolution.
 * 
 * @return The current microstep resolution.
 */
uint8_t A4988Manager::getStepResolution() {
    return _microSteps;
}

/**
 * @brief Starts the motor step task using FreeRTOS.
 * 
 * This function creates a FreeRTOS task to step the motor.
 */
void A4988Manager::startMotorTask() {
    if (_stepTaskHandle == nullptr) {
        xTaskCreatePinnedToCore(motorStepTask, "Motor Step Task", 1024, this, 1, &_stepTaskHandle, STEP_CORE);
    }
}

/**
 * @brief Stops the motor step task.
 * 
 * This function deletes the FreeRTOS task responsible for stepping the motor.
 */
void A4988Manager::stopMotorTask() {
    if (_stepTaskHandle != nullptr) {
        vTaskDelete(_stepTaskHandle);
        _stepTaskHandle = nullptr;
    }
}

/**
 * @brief Task for controlling the stepping of the motor.
 *
 * This function runs in a FreeRTOS task loop and controls the stepping of the motor connected
 * to the specified step pin. It monitors a sensor pin for a rising edge signal to trigger
 * the motor to pause for a specified stop time. The motor continues stepping normally unless
 * a rising edge is detected, in which case the stop flag is set, and the motor steps for a specified
 * number of pulses before pausing. After completing the pause, the motor resumes normal stepping.
 *
 * The behavior of the motor differs depending on the motor's number:
 * - For motor 0 (`motor->_Number == false`), the motor steps continuously with no additional logic.
 * - For motor 1 (`motor->_Number == true`), the motor checks for a rising edge on the sensor pin.
 *   - When a rising edge is detected, the motor pauses after completing a set number of steps.
 *   - After the pause, the motor resumes normal stepping, unless the stop flag is set again.
 *
 * @param pvParameters A pointer to the A4988Manager instance that controls the motor.
 *                     This parameter is passed to the task and provides access to motor-specific settings.
 *
 * @note The function runs continuously in a task, and the motor stepping is controlled by the
 *       interval (`motor->_interval`) and stop time (`motor->StopTime`). The motor behavior can
 *       be dynamically modified by setting the stop flag (`motor->_StopFlag`).
 *
 * @warning Ensure that the sensor pin used for rising edge detection (`SENSOR_PIN`) is connected
 *          to the appropriate GPIO pin and that the sensor provides a clean rising edge signal.
 */
void A4988Manager::motorStepTask(void *pvParameters) {
    A4988Manager* motor = static_cast<A4988Manager*>(pvParameters);
    bool previousState = digitalRead(SENSOR_PIN);  // Initial pin state

    // Task loop for motor stepping
    while (true) {

        if (!motor->_Number) {
            // Motor false behavior: Step signal LOW -> HIGH at specified interval
            digitalWrite(motor->_stepPin, LOW);
            vTaskDelay(motor->_interval / portTICK_PERIOD_MS);  // Wait for the next interval
            digitalWrite(motor->_stepPin, HIGH);
            vTaskDelay(motor->_interval / portTICK_PERIOD_MS);  // Wait for the next interval

        } else if (motor->_Number) {
            // Monitor the sensor pin for rising edge detection
            bool currentState = digitalRead(SENSOR_PIN);

            // Detect rising edge (LOW -> HIGH)
            if (previousState == LOW && currentState == HIGH) {
                motor->SetStopFlag();  // Set the stop flag for motor
                vTaskDelay(10 * portTICK_PERIOD_MS);  // Wait 10ms debounce
            }

            // Update previous state to current state for next loop
            previousState = currentState;

            // If stop flag is not set, keep stepping continuously
            if (!motor->_StopFlag) {
                digitalWrite(motor->_stepPin, LOW);
                vTaskDelay(motor->_interval / portTICK_PERIOD_MS);  // Wait for the next interval
                digitalWrite(motor->_stepPin, HIGH);
                vTaskDelay(motor->_interval / portTICK_PERIOD_MS);  // Wait for the next interval
            }
            // If stop flag is set, step for the specified number of pulses
            else {
                int pulseCount = 0;

                while (pulseCount < motor->stepsToTake) {
                    digitalWrite(motor->_stepPin, LOW);
                    vTaskDelay(motor->_interval / portTICK_PERIOD_MS);  // Wait for the next interval
                    digitalWrite(motor->_stepPin, HIGH);
                    vTaskDelay(motor->_interval / portTICK_PERIOD_MS);  // Wait for the next interval
                    pulseCount++;  // Increment pulse count
                }

                // Wait for the stop time before resuming normal stepping
                vTaskDelay(motor->StopTime * portTICK_PERIOD_MS);  // Wait for the stop time

                motor->ResetStopFlag();  // Reset the stop flag to resume normal stepping
            }
        }
    }
}



/**
 * @brief Generates a single step pulse for the A4988 stepper driver.
 * 
 * Sets the step pin HIGH for a short duration and then LOW to register a step.
 * The delay between the HIGH and LOW states ensures the pulse is properly recognized by the driver.
 */
void A4988Manager::step() {
    digitalWrite(_stepPin, HIGH); // Set the step pin HIGH
    delayMicroseconds(10);        // Short delay to ensure the pulse is registered
    digitalWrite(_stepPin, LOW);  // Set the step pin LOW
}

/**
 * @brief Sets the stop flag to indicate the motor should stop.
 * 
 * This function updates the `_StopFlag` variable to `true`, signaling that the motor should stop operating.
 */
void A4988Manager::SetStopFlag() {
    _StopFlag = true;
}

/**
 * @brief Resets the stop flag to allow motor operation.
 * 
 * This function updates the `_StopFlag` variable to `false`, signaling that the motor can resume operation.
 */
void A4988Manager::ResetStopFlag() {
    _StopFlag = false;
}

/**
 * @brief Gets the current stop time of the sensor.
 * 
 * @return The current stop time in milliseconds.
 */
uint32_t A4988Manager::GetStopTime() {
    return StopTime; // Return the stop time
}

/**
 * @brief Gets the current number of steps to be taken by the sensor.
 * 
 * @return The number of steps to take.
 */
uint32_t A4988Manager::GetStepsToTake() {
    return stepsToTake; // Return the number of steps
}

/**
 * @brief Sets the stop time for the sensor's operation.
 * 
 * @param value The stop time in milliseconds.
 */
void A4988Manager::SetStopTime(int value) {
    StopTime = value; // Assign the stop time
}

/**
 * @brief Sets the number of steps the sensor should take.
 * 
 * @param value The number of steps to take.
 */
void A4988Manager::SetStepsToTake(int value) {
    stepsToTake = value; // Assign the number of steps
}
