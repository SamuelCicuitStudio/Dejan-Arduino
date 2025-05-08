#ifndef NEXTION_HMI_H
#define NEXTION_HMI_H

#include <HardwareSerial.h>
#include "Config.h"
#include "A4988Manager.h" // Make sure to include the header for A4988Manager
#include "CommandReceiver.h"
#include "ConfigManager.h"

class NextionHMI {
public:
    // Constructor to initialize with sensor and motors
    NextionHMI(CommandReceiver* commandReceiver, A4988Manager& motor1, A4988Manager& motor2,ConfigManager*Conf);

    void begin();                             // Initialize UART for Nextion HMI
    void sendCommand(const String &command);  // Send a command to the Nextion HMI

    void handleButtonPress(const String &response);  // Handle button press responses
    void sendSystemStatus();
    void InitMotorsParameters();
    int calculateRPM(float pulseFrequency, int microsteps, int stepsPerRevolution);
    String exportToLineByLineString(String input);
private:
    String commandBuffer;      // Regular String for the command buffer
    bool commandReceived;      // Flag for received command
    CommandReceiver* cmdReceiver;            // Pointer to the Sensor
    A4988Manager& _motor1;     // Reference to motor 1
    A4988Manager& _motor2;     // Reference to motor 2
    ConfigManager*Conf;

    uint16_t CaseSpeed;
    uint16_t DiscSpeed;
    uint16_t Delay;
    uint16_t offset;
    bool CaseDir;
    bool DiscDir;

    // Function to handle received command
    void receiveCommand(const String& command);
    
    bool SYSTEM_ON;

};

#endif // NEXTION_HMI_H
