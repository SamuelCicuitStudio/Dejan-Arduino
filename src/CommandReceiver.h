#ifndef COMMAND_RECEIVER_H
#define COMMAND_RECEIVER_H

#include "A4988Manager.h" // Make sure to include the header for A4988Manager
#include <Arduino.h> // Include Arduino core for basic types and functions
#include "Sensor.h"

class CommandReceiver {
public:
    // Constructor
    CommandReceiver(Sensor* sensor,A4988Manager& motor1, A4988Manager& motor2);

    // Initialize the receiver
    void begin();

    // Check and process commands
    void checkCommand();

private:
    // Static command buffer and received command flag
    String commandBuffer;        // Regular String for the command buffer
    bool commandReceived;        // Flag for received command
    Sensor* sensor;
    // Motors managed by this receiver
    A4988Manager& _motor1;      // Declare _motor1 first
    A4988Manager& _motor2;      // Declare _motor2 second

    // Function to handle received command
    void receiveCommand(const String& command);

    // Set motor parameters based on received commands
    void setMotorParameters(int motor, float speed, int microsteps, int direction );
    void setSensorParameters(int stopTime, int stepsToTake);
    void sendSystemStatus();
};

#endif // COMMAND_RECEIVER_H
