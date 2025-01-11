#include "NextionHMI.h"



/**
 * @brief Constructor for NextionHMI class.
 * @param commandReceiver Pointer to the CommandReceiver instance.
 * @param motor1 Reference to the A4988Manager for motor 1 control.
 * @param motor2 Reference to the A4988Manager for motor 2 control.
 */
NextionHMI::NextionHMI(CommandReceiver* commandReceiver, A4988Manager& motor1, A4988Manager& motor2,HardwareSerial* hmiSerial)
    : cmdReceiver(commandReceiver), _motor1(motor1), _motor2(motor2), commandReceived(false), hmiSerial(hmiSerial) {
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
    //hmiSerial->begin(NEXTION_BAUDRATE, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN);
    //while (!hmiSerial01);  // Wait for Serial to be ready (only needed for some ESP32 boards)
}

/**
 * @brief Sends a command to the Nextion HMI display.
 * @param command The command string to send.
 */
void NextionHMI::sendCommand(const String& command) {
    hmiSerial->print(command);
    hmiSerial->write(0xFF);
    hmiSerial->write(0xFF);
    hmiSerial->write(0xFF);
}

/**
 * @brief Reads the response from the Nextion HMI display.
 * @return The response string from the display.
 */
String NextionHMI::readResponse() {
    String response = "";

    // Check if data is available from the Nextion HMI serial interface
    while (hmiSerial->available()) {
        char c = hmiSerial->read();
        response += c;
    }

    // Process the response if it contains enough data
    if (response.length() > 4) {
        // Extract the 5th character from the response
        char processedData = response.charAt(4);

        // Check if the processed data matches any of the specified characters
        if (processedData == 'A' || processedData == 'B' || processedData == 'C' ||
            processedData == 'E' || processedData == 'F' || processedData == 'G' ||
            processedData == 'S' || processedData == 'P' || processedData == 'I' ||
            processedData == 'H') {
            // Return the complete response string
            Serial.println("Received from Nextion:");
            Serial.println(processedData);
            return (String)processedData;
        } else {
            // Return an empty string for unsupported characters
            return "";
        }
    }

    // Return an empty string if the response is too short
    return "";
}



/**
 * @brief Converts input string to line-by-line format.
 * @param input The input string to convert.
 * @return The formatted output string with new lines after each printable character.
 */
String NextionHMI::exportToLineByLineString(String input) {
    String result = "";
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        if (c >= 32 && c <= 126) {  // Include printable ASCII characters
            result += c;      // Add the character to the result
            result += '\n';   // Add a newline after each character
        }
    }
    return result;
}

/**
 * @brief Handles button press events received from the Nextion HMI display.
 * @param response The response string identifying the button pressed.
 */
void NextionHMI::handleButtonPress(const String& response) {
    if (response == "A") {
        Serial.println("Case up button pressed");
        CaseSpeed++;
        cmdReceiver->setMotorParameters(1, CaseSpeed, CASE_MICROSTEP, CaseDir);
        sendSystemStatus();
    } 
    else if (response == "B") {
        Serial.println("Case direction button pressed");
        CaseDir = !CaseDir;
        cmdReceiver->setMotorParameters(1, CaseSpeed, CASE_MICROSTEP, CaseDir);
        sendSystemStatus();
    }
    else if (response == "C") {
        Serial.println("Case down button pressed");
        CaseSpeed--;
        cmdReceiver->setMotorParameters(1, CaseSpeed, CASE_MICROSTEP, CaseDir);
        sendSystemStatus();
    }
    else if (response == "S") {
        Serial.println("Start button pressed");
        SYSTEM_ON = true;  // Set system status to ON
        _motor1.Start();
        _motor2.Start();
        CaseSpeed = DEFAULT_CASE_SPEED;
        DiscSpeed = DEFAULT_DISK_SPEED;
        _motor1.setFrequency(CaseSpeed);
        _motor2.setFrequency(DiscSpeed);
        sendSystemStatus();
    }
    else if (response == "P") {
        Serial.println("Stop button pressed");
        SYSTEM_ON = false;  // Set system status to OFF
        _motor1.Stop();
        _motor2.Stop();
        _motor1.setFrequency(0.0);
        _motor2.setFrequency(0.0);
        sendSystemStatus();
    }
    else if (response == "G") {
        Serial.println("Disk up button pressed");
        DiscSpeed++;
        cmdReceiver->setMotorParameters(2, DiscSpeed, DISC_MICROSTEP, DiscDir);
        sendSystemStatus();
    }
    else if (response == "F") {
        Serial.println("Disk direction button pressed");
        DiscDir = !DiscDir;
        cmdReceiver->setMotorParameters(2, DiscSpeed, DISC_MICROSTEP, DiscDir);
        sendSystemStatus();
    }
    else if (response == "E") {
        Serial.println("Disk down button pressed");
        DiscSpeed--;
        cmdReceiver->setMotorParameters(2, DiscSpeed, DISC_MICROSTEP, DiscDir);
        sendSystemStatus();
    }
    else if (response == "H") {
        Serial.println("Delay up button pressed");
        Delay += 100;
        cmdReceiver->setSensorParameters(2, Delay, DEFAULT_STEPS_TO_TAKE);
        sendSystemStatus();
    }
    else if (response == "I") {
        Serial.println("Delay down button pressed");
        Delay -= 100;
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
        // Effective steps per revolution for case and disc
        int caseStepsPerRev = FULL_STEPS_PER_REV * CASE_MICROSTEP;
        int discStepsPerRev = FULL_STEPS_PER_REV * DISC_MICROSTEP;

        // Calculate RPMs
        int caseRPM = (CaseSpeed * 60) / caseStepsPerRev;
        int discRPM = (DiscSpeed * 60) / discStepsPerRev;

        // Update Nextion display with calculated RPM values
        sendCommand("n1.val=" + String(caseRPM));  // Update case RPM
        sendCommand("n2.val=" + String(discRPM));  // Update disc RPM
        sendCommand("n0.val=" + String(Delay / 1000));    // Update delay
    }
    else {
        sendCommand("n1.val=0");  // Case speed off
        sendCommand("n2.val=0");  // Disc speed off
        sendCommand("n0.val=0");  // Delay off
    }
}
