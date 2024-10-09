#ifndef CONFIG_H
#define CONFIG_H


#define DIR_PIN_CASE        5     // Direction pin
#define STEP_PIN_CASE       6     // Step pin
#define ENABLE_PIN_CASE     12    // Enable pin

#define MS03_PIN_CASE       9     // MS3 pin (Microstepping)
#define MS02_PIN_CASE       10    // MS2 pin (Microstepping)
#define MS01_PIN_CASE       11    // MS1 pin (Microstepping)
#define SLP_PIN_CASE        7     // Sleep pin
#define RESET_PIN_CASE      8     // Reset pin

#define DIR_PIN_DISC        5     // Direction pin
#define STEP_PIN_DISC       A0    // Step pin
#define ENABLE_PIN_DISC     A5    // Enable pin

#define MS03_PIN_DISC       9     // MS3 pin (Microstepping)
#define MS02_PIN_DISC       A4    // MS2 pin (Microstepping)
#define MS01_PIN_DISC       A3    // MS1 pin (Microstepping)
#define SLP_PIN_DISC        A1    // Sleep pin
#define RESET_PIN_DISC      A2    // Reset pin

#define  SENSOR_PIN 2
#define  ESPITR_PIN 3
#define  DEFAULT_STOP_TIME 3000
#define  BAUDE_RATE 115200
#endif
