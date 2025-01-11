#include <Arduino.h>
#include "A4988Manager.h"
#include "Sensor.h"
#include "CommandReceiver.h"
#include "config.h"
#include "SDCardManager.h"
#include "NextionHMI.h"

// ==================================================
// Motor Instances
// ==================================================
A4988Manager discMotor(
    STEP_PIN_DISC, DIR_PIN_DISC, ENABLE_PIN_DISC,
    MS01_PIN_DISC, MS02_PIN_DISC, MS03_PIN_DISC,
    SLP_PIN_DISC, RESET_PIN_DISC, true
);

A4988Manager caseMotor(
    STEP_PIN_CASE, DIR_PIN_CASE, ENABLE_PIN_CASE,
    MS01_PIN_CASE, MS02_PIN_CASE, MS03_PIN_CASE,
    SLP_PIN_CASE, RESET_PIN_CASE, false
);

HardwareSerial nextionSerial(1); // Use UART1 for communication with Nextion

// ==================================================
// Object Pointers
// ==================================================
Sensor* sensor = nullptr;
SDCardManager* SDcard = nullptr;
CommandReceiver* commandReceiver = nullptr;
NextionHMI* nextionHMI = nullptr;
// ==================================================
// Motor loop
// ==================================================
void taskLoop(void *pvParameters) {
    // Main loop code that will run in a FreeRTOS task on core 0
    while (true) {
        // Read response from HMI
        String hmiResponse = nextionHMI->readResponse();

        // Handle button press from HMI
        nextionHMI->handleButtonPress(hmiResponse);

        // Process the first character of the HMI response
        if (hmiResponse.length() > 0) { // Ensure response is not empty
            char processedData = hmiResponse.charAt(0);
            Serial.println(hmiResponse);

            // Check if the processed character matches any valid status commands
            if (processedData == 'A' || processedData == 'B' || processedData == 'C' ||
                processedData == 'E' || processedData == 'F' || processedData == 'G' ||
                processedData == 'S' || processedData == 'P' || processedData == 'I' ||
                processedData == 'H') {
                
                // Send system status to the HMI
                nextionHMI->sendSystemStatus();
            }
        }

        // Delay for 50 milliseconds (in FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(50)); // Converts milliseconds to ticks
    }
}


void setup() {
    // Serial Communication Setup
    Serial.begin(BAUDE_RATE);
    while (!Serial) { ; } // Wait for serial to connect
    Serial1.begin(NEXTION_BAUDRATE, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN);
    Serial.println("ESP32 Ready. Listening to Nextion Display...");

    pinMode(FLAG_LED_PIN, OUTPUT);
    digitalWrite(FLAG_LED_PIN, HIGH);

    // Motor Initialization
    discMotor.begin();
    caseMotor.begin();

    // SD Card Initialization
    SDcard = new SDCardManager();
    if (SDcard->begin()) {
        Serial.println("SD card initialized successfully.");
    } else {
        Serial.println("Failed to initialize SD card.");
    }

    // Sensor and Command Receiver Initialization
    sensor = new Sensor(SENSOR_PIN);
    sensor->begin();

    commandReceiver = new CommandReceiver(sensor, caseMotor, discMotor);
    commandReceiver->begin();

    // Nextion HMI Initialization
    nextionHMI = new NextionHMI(commandReceiver, caseMotor, discMotor);
    nextionHMI->begin();

    // Create a FreeRTOS task that will run on core 0
    xTaskCreatePinnedToCore(
        taskLoop,            // Task function
        "MainLoopTask",      // Task name
        10000,               // Stack size (adjust as needed)
        NULL,                // Parameters to pass to the task
        1,                   // Task priority
        NULL,                // Task handle
        1                    // Core 0
    );
}

void loop() {
    // Empty loop, as the task loop runs in a separate task on core 0
}
