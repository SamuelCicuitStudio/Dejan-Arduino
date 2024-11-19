#ifndef A4988Manager_h
#define A4988Manager_h

#include <Arduino.h>
#include "driver/timer.h"

class A4988Manager {
public:
    A4988Manager(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                 uint8_t ms1Pin, uint8_t ms2Pin, uint8_t ms3Pin,
                 uint8_t slpPin, uint8_t resetPin, int timerNumber);

    void begin();
    static void timerISR(void* arg);
    void setMicrostepping(int ms1, int ms2, int ms3);
    void setStepResolution(int resolution);
    void setSpeed(float speed);
    void startStepping();
    void stopStepping();
    void step();
    void configureTimer();
    float getSpeed();
    int getDirPin() const;
    void Start();
    void Stop();
    void Reset();
    void SetDirPin(bool value);

private:
    uint8_t _stepPin;
    uint8_t _dirPin;
    uint8_t _enablePin;
    uint8_t _ms1Pin;
    uint8_t _ms2Pin;
    uint8_t _ms3Pin;
    uint8_t _slpPin;
    uint8_t _resetPin;
    volatile bool _stepping;
    float _frequency;
    uint32_t _interval;
    int _timerNumber;
    timer_group_t _timerGroup;
    timer_idx_t _timerIdx;
    
    static volatile A4988Manager* instance1;
    static volatile A4988Manager* instance2;
};

#endif