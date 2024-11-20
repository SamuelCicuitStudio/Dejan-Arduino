#ifndef A4988Manager_H
#define A4988Manager_H

#include <Arduino.h>
#include <FreeRTOS.h>

class A4988Manager {
public:
    // Constructor for the A4988Manager
    A4988Manager(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                 uint8_t ms1Pin, uint8_t ms2Pin, uint8_t ms3Pin,
                 uint8_t slpPin, uint8_t resetPin);

    void begin();
    void setMicrostepping(int ms1, int ms2, int ms3);
    void setStepResolution(int resolution);
    void setDirPin(bool value);
    void Start();
    void Stop();
    void Reset();
    void setFrequency(float frequency);
    float getSpeed();
    int getDir();
    uint8_t getStepResolution();
    void startMotorTask();
    void stopMotorTask();
    void step();

private:
    uint8_t _stepPin, _dirPin, _enablePin, _ms1Pin, _ms2Pin, _ms3Pin;
    uint8_t _slpPin, _resetPin;
    bool _stepping;
    float _frequency;
    unsigned long _interval;
    unsigned long _lastStepTime;
    uint8_t _microSteps;
    TaskHandle_t _stepTaskHandle;
    static void motorStepTask(void *pvParameters);
};

#endif // A4988Manager_H
