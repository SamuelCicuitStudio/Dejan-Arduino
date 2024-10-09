#ifndef A4988Manager_h
#define A4988Manager_h

#include <Arduino.h>

class A4988Manager {
public:
    A4988Manager(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin,
                 uint8_t ms1Pin, uint8_t ms2Pin, uint8_t ms3Pin,
                 uint8_t slpPin, uint8_t resetPin, int timerNumber);

    void begin();
    void setMicrostepping(int ms1, int ms2, int ms3);
    void setStepResolution(int resolution);
    void setSpeed(float speed);
    void startStepping();
    float getSpeed();
    void stopStepping();
    int getDirPin() const;
    void Stop();
    void Start();
    void Reset();
    void SetDirPin(bool value);
    void SetStopTime(uint16_t value);
    
    static volatile A4988Manager* instance1;
    static volatile A4988Manager* instance2;
    void step() volatile; // Add volatile qualifier

private:
    void configureTimer();

    uint8_t _stepPin, _dirPin, _enablePin;
    uint8_t _ms1Pin, _ms2Pin, _ms3Pin;
    uint8_t _slpPin, _resetPin;
    bool _stepping;
    float _frequency;
    unsigned long _interval;
    int _timerNumber;
    
};

#endif
