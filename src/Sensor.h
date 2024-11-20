#ifndef SENSOR_H
#define SENSOR_H

#include "A4988Manager.h"
#include "Config.h"

class Sensor {
public:
    // Constructor
    Sensor(int pin);
    
    void begin();

private:
    int _pin;
    static Sensor *currentSensor; // To store the current sensor instance for interrupt

};

#endif // SENSOR_H
