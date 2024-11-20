#ifndef SENSOR_H
#define SENSOR_H

#include "A4988Manager.h"
#include "Config.h"

class Sensor {
public:
    // Constructor
    Sensor(int pin, A4988Manager &motor);
    
    void begin();


    // Function to start the FreeRTOS monitoring task
    void startMonitoringTask();

private:
    int _pin;
    A4988Manager *_motor; // Non-static pointer to the motor instance
    static Sensor *currentSensor; // To store the current sensor instance for interrupt
    // FreeRTOS task handle
    static TaskHandle_t monitoringTaskHandle;

    // Task function to monitor the pin state
    static void monitoringTask(void *pvParameters);
};

#endif // SENSOR_H
