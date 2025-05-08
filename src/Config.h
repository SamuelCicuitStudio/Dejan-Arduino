#ifndef CONFIG_H
#define CONFIG_H

// ==================================================
// DEVICE Config Flags Name (Keys for ESP32 Preferences)
// ==================================================

// Motor Control keys
#define DELAY_MS_KEY        "DLMSV"
#define OFFSET_STEPS_KEY    "OFSTP"
#define CASE_RPM_KEY        "CASRP"
#define DISC_RPM_KEY        "DISRP"
#define CASE_DIR_KEY        "CASDR"
#define DISC_DIR_KEY        "DISDR"
#define RESET_FLAG "RSTFL"


// Default values
#define DELAY_MS_DEFAULT      DEFAULT_DELAY
#define OFFSET_STEPS_DEFAULT  DEFAULT_STEPS_TO_TAKE
#define CASE_RPM_DEFAULT      DEFAULT_CASE_SPEED
#define DISC_RPM_DEFAULT      DEFAULT_DISK_SPEED
#define CASE_DIR_DEFAULT      DEFAULT_CASE_DIR
#define DISC_DIR_DEFAULT      DEFAULT_DISK_DIR


#define DEFAULT_CASE_SPEED 250
#define DEFAULT_CASE_DIR false
#define DEFAULT_DISK_SPEED 750
#define DEFAULT_DISK_DIR false
#define DEFAULT_DELAY 2000
#define DEFAULT_OFFSET 2000
#define NEXTION_BAUDRATE 9600
#define CASE_MICROSTEP 4
#define DISC_MICROSTEP 8

// ==================================================
// Debug print toggle
// ==================================================
#define ENABLE_SERIAL_DEBUG
#define DEBUGMODE true
#define CONFIG_PARTITION "config"
// =========================================================================
// Pin Definitions for Case Configuration
// =========================================================================

#define DIR_PIN_CASE        42     // Direction Pin for Stepper Motor (Case)
#define STEP_PIN_CASE       41     // Step Pin for Stepper Motor (Case)
#define ENABLE_PIN_CASE     35     // Enable Pin for Stepper Motor (Case)

// =========================================================================
// Microstepping Pins for Case Configuration
// =========================================================================
#define MS03_PIN_CASE       38     // Microstepping Pin MS3 (Case)
#define MS02_PIN_CASE       37     // Microstepping Pin MS2 (Case)
#define MS01_PIN_CASE       36     // Microstepping Pin MS1 (Case)

// =========================================================================
// Control Pins for Case Configuration
// =========================================================================
#define SLP_PIN_CASE        40     // Sleep Pin for Stepper Motor (Case)
#define RESET_PIN_CASE      39     // Reset Pin for Stepper Motor (Case)

// =========================================================================
// Pin Definitions for Disc Configuration
// =========================================================================
#define DIR_PIN_DISC        3      // Direction Pin for Stepper Motor (Disc)
#define STEP_PIN_DISC       46     // Step Pin for Stepper Motor (Disc)
#define ENABLE_PIN_DISC     14     // Enable Pin for Stepper Motor (Disc)

// =========================================================================
// Microstepping Pins for Disc Configuration
// =========================================================================
#define MS03_PIN_DISC       20     // Microstepping Pin MS3 (Disc)
#define MS02_PIN_DISC       12     // Microstepping Pin MS2 (Disc)
#define MS01_PIN_DISC       13     // Microstepping Pin MS1 (Disc)

// =========================================================================
// Control Pins for Disc Configuration
// =========================================================================
#define SLP_PIN_DISC        9      // Sleep Pin for Stepper Motor (Disc)
#define RESET_PIN_DISC      47     // Reset Pin for Stepper Motor (Disc)
#define DEFAULT_FREQ  50
#define STEP_CORE 0

#define FULL_STEPS_PER_REV 200     // Constants for steps per revolution for NEMA 17 stepper motor
// =========================================================================
// Sensor and Communication Pin Definitions
// =========================================================================
#define SENSOR_PIN          18     // Sensor Pin
#define  DEFAULT_STEPS_TO_TAKE  100   // Number of steps to take when the sensor is triggered
#define  DEFAULT_STOP_TIME  1000      // Time to stop in milliseconds
// =========================================================================
// Display Communication Pins
// =========================================================================
#define SCREEN_RXD_PIN      4      // RX Pin for Display Communication
#define SCREEN_TXD_PIN      5      // TX Pin for Display Communication

// =========================================================================
// SD Card Pin Definitions
// =========================================================================
#define SD_CLK_PIN          10     // Clock Pin for SD Card
#define SD_MISO_PIN         11     // MISO Pin for SD Card
#define SD_MOSI_PIN         21     // MOSI Pin for SD Card
#define SD_NCS_PIN          8      // Chip Select Pin for SD Card

// =========================================================================
// General Configuration Settings
// =========================================================================
#define FLAG_LED_PIN        16     // Pin for Status LED
#define BAUDE_RATE          115200 // Baud Rate for Serial Communication

#endif
