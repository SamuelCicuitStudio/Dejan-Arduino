#ifndef NEXTION_HMI_H
#define NEXTION_HMI_H

#include <HardwareSerial.h>
#include "Config.h"
#include "A4988Manager.h" // Make sure to include the header for A4988Manager
#include "CommandReceiver.h"
#include <HardwareSerial.h>

#define DEFAULT_CASE_SPEED 250
#define DEFAULT_CASE_DIR false
#define DEFAULT_DISK_SPEED 500
#define DEFAULT_DISK_DIR false
#define DEFAULT_DELAY 2000
#define NEXTION_BAUDRATE 9600





class NextionHMI {
public:
    // Constructor to initialize with sensor and motors
    NextionHMI(CommandReceiver* commandReceiver, A4988Manager& motor1, A4988Manager& motor2,HardwareSerial* hmiSerial);

    void begin();                             // Initialize UART for Nextion HMI
    void sendCommand(const String &command);  // Send a command to the Nextion HMI
    String readResponse();                    // Read the response from the Nextion HMI
    void handleButtonPress(const String &response);  // Handle button press responses
    void sendSystemStatus();
    String exportToLineByLineString(String input);
private:
    String commandBuffer;      // Regular String for the command buffer
    bool commandReceived;      // Flag for received command
    CommandReceiver* cmdReceiver;            // Pointer to the Sensor
    A4988Manager& _motor1;     // Reference to motor 1
    A4988Manager& _motor2;     // Reference to motor 2
    HardwareSerial* hmiSerial;

    uint16_t CaseSpeed;
    uint16_t DiscSpeed;
    uint16_t Delay;
    bool CaseDir;
    bool DiscDir;

    // Function to handle received command
    void receiveCommand(const String& command);
    
    bool SYSTEM_ON;
    // Microstepping factors
    const int CASE_MICROSTEP = 1;
    const int DISC_MICROSTEP = 8;
};

#endif // NEXTION_HMI_H
