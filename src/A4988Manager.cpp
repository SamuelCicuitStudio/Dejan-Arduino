#include "A4988Manager.h"

// Initialize static instance pointers for two motors
volatile A4988Manager* A4988Manager::instance1 = nullptr;
volatile A4988Manager* A4988Manager::instance2 = nullptr;


// Constructor
A4988Manager::A4988Manager(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                           uint8_t ms1Pin, uint8_t ms2Pin, uint8_t ms3Pin,
                           uint8_t slpPin, uint8_t resetPin, int timerNumber)
    : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin),
      _ms1Pin(ms1Pin), _ms2Pin(ms2Pin), _ms3Pin(ms3Pin),
      _slpPin(slpPin), _resetPin(resetPin),
      _stepping(false), _frequency(0), _interval(0), _timerNumber(timerNumber) {}

// Initialize the motor
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

// Timer ISR for Timer1
ISR(TIMER1_COMPA_vect) {
    if (A4988Manager::instance1) {
        A4988Manager::instance1->step();
    }
}

// Timer ISR for Timer2
ISR(TIMER2_COMPA_vect) {
    if (A4988Manager::instance2) {
        A4988Manager::instance2->step();
    }
}
// Set microstepping mode
void A4988Manager::setMicrostepping(int ms1, int ms2, int ms3) {
    digitalWrite(_ms1Pin, ms1);
    digitalWrite(_ms2Pin, ms2);
    digitalWrite(_ms3Pin, ms3);
}

// Set step resolution
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

// Set motor speed (in Hz)
void A4988Manager::setSpeed(float speed) {
    _frequency = speed;
    _interval = (speed > 0) ? (1000000.0 / speed) : 0; // Convert frequency to microseconds
}

// Start stepping
void A4988Manager::startStepping() {
    _stepping = true;
}

// Stop stepping
void A4988Manager::stopStepping() {
    _stepping = false;
}

// Make a step
void A4988Manager::step() volatile {
    if (_stepping) {
        digitalWrite(_stepPin, HIGH);
        delayMicroseconds(1); // Short pulse
        digitalWrite(_stepPin, LOW);
    }
}
// Configure timer for interrupts
void A4988Manager::configureTimer() {
    // Set the instance pointer based on the timer number
    if (_timerNumber == 1) {
        instance1 = this; // Assign this instance to the first motor
        // Configure Timer1
        cli(); // Disable global interrupts
        TCCR1A = 0; // Clear control register A
        TCCR1B = 0; // Clear control register B
        TCCR1B |= (1 << WGM12); // CTC mode
        TCCR1B |= (1 << CS11); // Prescaler of 8
        OCR1A = (F_CPU / 8 / (1000000 / _interval)) - 1; // Set compare match register
        TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt
        sei(); // Enable global interrupts
    } else if (_timerNumber == 2) {
        instance2 = this; // Assign this instance to the second motor
        // Configure Timer2 (similarly to Timer1)
        cli(); // Disable global interrupts
        TCCR2A = 0; // Clear control register A
        TCCR2B = 0; // Clear control register B
        TCCR2A |= (1 << WGM21); // CTC mode
        TCCR2B |= (1 << CS21); // Prescaler of 8
        OCR2A = (F_CPU / 8 / (1000000 / _interval)) - 1; // Set compare match register
        TIMSK2 |= (1 << OCIE2A); // Enable timer compare interrupt
        sei(); // Enable global interrupts
    } else {
        Serial.println("Invalid timer number."); // Error handling for invalid timer number
    }
}

// Get current speed
float A4988Manager::getSpeed() {
    return _frequency; // Return the current speed
}

// Method to get the direction pin
int A4988Manager::getDirPin() const {
    return _dirPin; // Return the direction pin
}

// Function to enable the motor
void A4988Manager::Start() {
    digitalWrite(_enablePin, LOW); // Enable the driver
}

// Function to disable the motor
void A4988Manager::Stop() {
    digitalWrite(_enablePin, HIGH); // Disable the driver
}

void A4988Manager::Reset() {
    digitalWrite(_resetPin, LOW); // Set reset pin low
    delayMicroseconds(100);        // Wait for a short duration (100 microseconds)
    digitalWrite(_resetPin, HIGH); // Set reset pin high to re-enable the driver
}

void A4988Manager:: SetDirPin(bool value){
digitalWrite(_dirPin, value); // Set dir of the  driver
}