#include "NextionHMI.h"

/**
 * @brief Constructor for NextionHMI class.
 * @param mySerial Pointer to the HardwareSerial instance for UART communication.
 * @param commandReceiver Pointer to the CommandReceiver instance.
 * @param motor1 Reference to the A4988Manager for motor 1 control.
 * @param motor2 Reference to the A4988Manager for motor 2 control.
 */
NextionHMI::NextionHMI(HardwareSerial* mySerial, CommandReceiver* commandReceiver, A4988Manager& motor1, A4988Manager& motor2)
    : mySerial(mySerial), cmdReceiver(commandReceiver), _motor1(motor1), _motor2(motor2), commandReceived(false) {
    CaseSpeed = DEFAULT_CASE_SPEED;
    DiscSpeed = DEFAULT_DISK_SPEED;
    Delay = DEFAULT_DELAY;
    CaseDir = DEFAULT_CASE_DIR;
    DiscDir = DEFAULT_DISK_DIR;
    SYSTEM_ON = false;  // Initialize system as OFF
}

/**
 * @brief Initializes the UART communication with the Nextion HMI display.
 */
void NextionHMI::begin() {
    mySerial->begin(NEXTION_BAUDRATE, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN);
}

/**
 * @brief Sends a command to the Nextion HMI display.
 * @param command The command string to send.
 */
void NextionHMI::sendCommand(const String& command) {
    mySerial->print(command);
    mySerial->write(0xFF);
    mySerial->write(0xFF);
    mySerial->write(0xFF);
}

/**
 * @brief Reads the response from the Nextion HMI display.
 * @return The response string from the display.
 */
String NextionHMI::readResponse() {
    String response = "";
    while (mySerial->available()) {
        response += (char)mySerial->read();
    }
    return response;
}

/**
 * @brief Handles button press events received from the Nextion HMI display.
 * @param response The response string identifying the button pressed.
 */
void NextionHMI::handleButtonPress(const String& response) {
    if (response == "CASE UP") {
        Serial.println("Case up button pressed");
        CaseSpeed++;
        cmdReceiver->setMotorParameters(1, CaseSpeed, 2, CaseDir);
        sendSystemStatus();
    } else if (response == "CASE DIR") {
        Serial.println("Case direction button pressed");
        CaseDir = !CaseDir;
        cmdReceiver->setMotorParameters(1, CaseSpeed, 2, CaseDir);
        sendSystemStatus();
    } else if (response == "CASE DOWN") {
        Serial.println("Case down button pressed");
        CaseSpeed--;
        cmdReceiver->setMotorParameters(1, CaseSpeed, 2, CaseDir);
        sendSystemStatus();
    } else if (response == "START") {
        Serial.println("Start button pressed");
        SYSTEM_ON = true;  // Set system status to ON
        _motor1.Start();
        _motor2.Start();
        CaseSpeed = DEFAULT_CASE_SPEED;
        DiscSpeed = DEFAULT_DISK_SPEED;
        _motor1.setFrequency(CaseSpeed);
        _motor2.setFrequency(DiscSpeed);
        sendSystemStatus();
    } else if (response == "STOP") {
        Serial.println("Stop button pressed");
        SYSTEM_ON = false;  // Set system status to OFF
        _motor1.Stop();
        _motor2.Stop();
        _motor1.setFrequency(0.0);
        _motor2.setFrequency(0.0);
        sendSystemStatus();
    } else if (response == "DISK UP") {
        Serial.println("Disk up button pressed");
        DiscSpeed++;
        cmdReceiver->setMotorParameters(2, DiscSpeed, 8, DiscDir);
        sendSystemStatus();
    } else if (response == "DISK DIR") {
        Serial.println("Disk direction button pressed");
        DiscDir = !DiscDir;
        cmdReceiver->setMotorParameters(2, DiscSpeed, 8, DiscDir);
        sendSystemStatus();
    } else if (response == "DISK DOWN") {
        Serial.println("Disk down button pressed");
        DiscSpeed--;
        cmdReceiver->setMotorParameters(2, DiscSpeed, 8, DiscDir);
        sendSystemStatus();
    } else if (response == "DELAY UP") {
        Serial.println("Delay up button pressed");
        Delay++;
        cmdReceiver->setSensorParameters(2, Delay, DEFAULT_STEPS_TO_TAKE);
        sendSystemStatus();
    } else if (response == "DELAY DOWN") {
        Serial.println("Delay down button pressed");
        Delay--;
        cmdReceiver->setSensorParameters(2, Delay, DEFAULT_STEPS_TO_TAKE);
        sendSystemStatus();
    }
}

/**
 * @brief Receives a command from the Nextion HMI display.
 * @param command The command string received from the display.
 */
void NextionHMI::receiveCommand(const String& command) {
    commandBuffer = command;
    commandReceived = true;
}

/**
 * @brief Sends the current system status to the Nextion HMI display.
 */
void NextionHMI::sendSystemStatus() {
    if (SYSTEM_ON) {
        sendCommand("n1.val=" + String(CaseSpeed));  // Update case speed
        sendCommand("n2.val=" + String(DiscSpeed));  // Update disc speed
        sendCommand("n0.val=" + String(Delay));      // Update delay
    } else {
        sendCommand("n1.val=0");  // Case speed off
        sendCommand("n2.val=0");  // Disc speed off
        sendCommand("n0.val=0");  // Delay off
    }
}
