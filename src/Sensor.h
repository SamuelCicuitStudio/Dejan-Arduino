#ifndef SENSOR_H
#define SENSOR_H

#include "A4988Manager.h"

class Sensor {
public:
    // Constructor
    Sensor(int pin, A4988Manager &motor);
    
    void begin();
    void update();
    void SetStopTime(int value);
    void SetStepsToTake(int value);
    void SetReadStatus();

    
    static void handleInterrupt(); // Interrupt handler should be static

private:
    int _pin;
    A4988Manager *_motor; // Non-static pointer to the motor instance
    static int stepsToTake;
    static unsigned long interruptStartTime;
    static bool interruptActive;
    static int currentStep;
    static int delayBetweenSteps;
    static unsigned long lastStepTime;
    static int originalSpeed;
    int StopTime; // Non-static, specific to the instance
    static Sensor *currentSensor; // To store the current sensor instance for interrupt
};

#endif // SENSOR_H
