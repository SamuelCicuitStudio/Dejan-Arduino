#include "A4988Manager.h"

// Initialize static instance pointers for two motors
volatile A4988Manager* A4988Manager::instance1 = nullptr;
volatile A4988Manager* A4988Manager::instance2 = nullptr;

/**
 * @brief Constructor for A4988Manager.
 * 
 * Initializes pin configuration and sets initial state for stepping.
 * 
 * @param stepPin Pin number connected to STEP input.
 * @param dirPin Pin number connected to DIR input.
 * @param enablePin Pin number connected to ENABLE input.
 * @param ms1Pin Pin number connected to MS1 for microstepping.
 * @param ms2Pin Pin number connected to MS2 for microstepping.
 * @param ms3Pin Pin number connected to MS3 for microstepping.
 * @param slpPin Pin number connected to SLEEP input.
 * @param resetPin Pin number connected to RESET input.
 * @param timerNumber The timer number to be used for stepping interrupts.
 */
A4988Manager::A4988Manager(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                           uint8_t ms1Pin, uint8_t ms2Pin, uint8_t ms3Pin,
                           uint8_t slpPin, uint8_t resetPin, int timerNumber)
    : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin),
      _ms1Pin(ms1Pin), _ms2Pin(ms2Pin), _ms3Pin(ms3Pin),
      _slpPin(slpPin), _resetPin(resetPin),
      _stepping(false), _frequency(0), _interval(0), _timerNumber(timerNumber) {}

/**
 * @brief Initializes the A4988 motor driver by configuring pins and setting the motor state.
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
    digitalWrite(_slpPin, HIGH);    // Wake up the driver
    Reset();   // Reset the driver
    configureTimer(); // Configure the timer for stepping
}

/**
 * @brief Timer interrupt service routine (ISR) callback.
 * 
 * This ISR is called when the timer triggers. It invokes the step method of the A4988Manager instance.
 * 
 * @param arg The pointer to the A4988Manager instance.
 */
void A4988Manager::timerISR(void* arg) {
    A4988Manager* instance = (A4988Manager*)arg;
    if (instance) {
        instance->step();
    }
}

/**
 * @brief Set the microstepping mode of the A4988 driver.
 * 
 * Configures the microstepping pins (MS1, MS2, MS3) to the specified values.
 * 
 * @param ms1 Microstepping mode MS1 pin value.
 * @param ms2 Microstepping mode MS2 pin value.
 * @param ms3 Microstepping mode MS3 pin value.
 */
void A4988Manager::setMicrostepping(int ms1, int ms2, int ms3) {
    digitalWrite(_ms1Pin, ms1);
    digitalWrite(_ms2Pin, ms2);
    digitalWrite(_ms3Pin, ms3);
}

/**
 * @brief Set the step resolution for the motor.
 * 
 * Configures the microstepping mode based on the desired resolution.
 * 
 * @param resolution The step resolution (1, 2, 4, 8, 16).
 */
void A4988Manager::setStepResolution(int resolution) {
    switch (resolution) {
        case 1:  // Full step
            setMicrostepping(LOW, LOW, LOW);
            break;
        case 2:  // Half step
            setMicrostepping(HIGH, LOW, LOW);
            break;
        case 4:  // Quarter step
            setMicrostepping(LOW, HIGH, LOW);
            break;
        case 8:  // Eighth step
            setMicrostepping(HIGH, HIGH, LOW);
            break;
        case 16: // Sixteenth step
            setMicrostepping(HIGH, HIGH, HIGH);
            break;
        default:
            Serial.println("Invalid resolution.");
            break;
    }
}

/**
 * @brief Set the speed of the motor in Hz.
 * 
 * This function sets the frequency of the motor's step pulses, which determines its speed.
 * 
 * @param speed The speed in Hz.
 */
void A4988Manager::setSpeed(float speed) {
    // Store the desired frequency (in Hz)
    _frequency = speed;

    // If speed is 0 Hz, set the interval to the highest possible value (e.g., a very large number)
    // Otherwise, calculate the interval between steps in microseconds
    if (speed > 0) {
        _interval = 1000000.0 / speed;  // Calculate the interval for positive speeds
    } else {
        _interval = UINT64_MAX;  // Set to the highest possible interval for 0 Hz (max 32-bit unsigned value)
    }

    // Reconfigure the timer alarm with the updated interval
    if (_timerGroup != -1 && _timerIdx != -1) {
        timer_set_alarm_value(_timerGroup, _timerIdx, _interval);  // Update the timer alarm value
    }
}



/**
 * @brief Start the stepping process.
 * 
 * This sets the stepping flag to true, allowing the motor to step.
 */
void A4988Manager::startStepping() {
    _stepping = true;
}

/**
 * @brief Stop the stepping process.
 * 
 * This sets the stepping flag to false, stopping the motor from stepping.
 */
void A4988Manager::stopStepping() {
    _stepping = false;
}

/**
 * @brief Perform one step.
 * 
 * If the stepping flag is true, this method generates a step pulse on the STEP pin.
 */
void A4988Manager::step() {
    if (_stepping) {
        digitalWrite(_stepPin, HIGH);
        delayMicroseconds(1); // Short pulse
        digitalWrite(_stepPin, LOW);
    }
}

/**
 * @brief Configure the timer for interrupts.
 * 
 * This method configures the timer to trigger an interrupt at the specified interval for stepping.
 */
void A4988Manager::configureTimer() {
    // Set the instance pointer based on the timer number
    if (_timerNumber == 1) {
        instance1 = this; // Assign this instance to the first motor
        _timerGroup = TIMER_GROUP_0;
        _timerIdx = TIMER_0;
    } else if (_timerNumber == 2) {
        instance2 = this; // Assign this instance to the second motor
        _timerGroup = TIMER_GROUP_0;
        _timerIdx = TIMER_1;
    } else {
        Serial.println("Invalid timer number.");
        return;
    }

    // Configure the timer
    timer_config_t config;
    config.divider = 80;  // Set prescaler to 80 (1 microsecond tick)
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_START;
    config.alarm_en = TIMER_ALARM_EN;
    config.auto_reload = TIMER_AUTORELOAD_EN;  // Auto-reload enabled
    timer_init(_timerGroup, _timerIdx, &config);

    // Set the timer alarm to trigger the ISR at the appropriate interval
    timer_set_alarm_value(_timerGroup, _timerIdx, _interval);
    timer_enable_intr(_timerGroup, _timerIdx);

    // Register the callback function for the timer interrupt
    timer_isr_register(_timerGroup, _timerIdx, timerISR, (void*)this, ESP_INTR_FLAG_IRAM, nullptr);
}

/**
 * @brief Get the current speed of the motor.
 * 
 * Returns the current speed in Hz.
 * 
 * @return The current speed in Hz.
 */
float A4988Manager::getSpeed() {
    return _frequency;
}

/**
 * @brief Get the direction pin.
 * 
 * Returns the direction pin number.
 * 
 * @return The direction pin number.
 */
int A4988Manager::getDirPin() const {
    return _dirPin;
}

/**
 * @brief Enable the motor driver.
 * 
 * This method enables the motor driver by setting the ENABLE pin low.
 */
void A4988Manager::Start() {
    digitalWrite(_enablePin, LOW); // Enable the driver
}

/**
 * @brief Disable the motor driver.
 * 
 * This method disables the motor driver by setting the ENABLE pin high.
 */
void A4988Manager::Stop() {
    digitalWrite(_enablePin, HIGH); // Disable the driver
}

/**
 * @brief Reset the motor driver.
 * 
 * This method performs a hardware reset of the A4988 motor driver.
 */
void A4988Manager::Reset() {
    digitalWrite(_resetPin, LOW); // Set reset pin low
    delayMicroseconds(100);        // Wait for a short duration (100 microseconds)
    digitalWrite(_resetPin, HIGH); // Set reset pin high to re-enable the driver
}

/**
 * @brief Set the direction pin value.
 * 
 * This method sets the direction of the motor by writing a value to the DIR pin.
 * 
 * @param value The value to set the DIR pin to (HIGH or LOW).
 */
void A4988Manager::SetDirPin(bool value) {
    digitalWrite(_dirPin, value); // Set direction of the driver
}
