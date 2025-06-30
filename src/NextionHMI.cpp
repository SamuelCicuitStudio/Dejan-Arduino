#include "NextionHMI.h"
#include"Arduino.h"


/**
 * @brief Constructor for NextionHMI class.
 * @param commandReceiver Pointer to the CommandReceiver instance.
 * @param motor1 Reference to the A4988Manager for motor 1 control.
 * @param motor2 Reference to the A4988Manager for motor 2 control.
 */
NextionHMI::NextionHMI(CommandReceiver* commandReceiver, A4988Manager& motor1, A4988Manager& motor2,ConfigManager*Conf)
    : cmdReceiver(commandReceiver), _motor1(motor1), _motor2(motor2), commandReceived(false),Conf(Conf) {
        CaseSpeed = Conf->GetInt(CASE_RPM_KEY, CASE_RPM_DEFAULT);
        DiscSpeed = Conf->GetInt(DISC_RPM_KEY, DISC_RPM_DEFAULT);
        Delay     = Conf->GetInt(DELAY_MS_KEY, DELAY_MS_DEFAULT);
        offset    = Conf->GetInt(OFFSET_STEPS_KEY, OFFSET_STEPS_DEFAULT);
        CaseDir   = Conf->GetBool(CASE_DIR_KEY, CASE_DIR_DEFAULT);
        DiscDir   = Conf->GetBool(DISC_DIR_KEY, DISC_DIR_DEFAULT);

    _motor1.setFrequency(CaseSpeed);
    _motor2.setFrequency(DiscSpeed);
    cmdReceiver->setMotorParameters(2,DiscSpeed,DISC_MICROSTEP,DiscDir);// Disc Motor
    cmdReceiver->setMotorParameters(1,CaseSpeed,CASE_MICROSTEP,CaseDir);// Case Motor
    cmdReceiver->setSensorParameters(2,Delay, offset);
    _motor1.Stop();
    _motor2.Stop();
    _motor1.setFrequency(0.0);
    _motor2.setFrequency(0.0);

    SYSTEM_ON = false;  // Initialize system as OFF
}

/**
 * @brief Initializes the UART communication with the Nextion HMI display.
 */
void NextionHMI::begin() {
    Serial1.begin(NEXTION_BAUDRATE, SERIAL_8N1, SCREEN_RXD_PIN, SCREEN_TXD_PIN);
    while (!Serial1);  // Wait for Serial to be ready (only needed for some ESP32 boards)
}

/**
 * @brief Sends a command to the Nextion HMI display.
 * @param command The command string to send.
 */
void NextionHMI::sendCommand(const String& command) {
    Serial1.print(command);
    Serial1.write(0xFF);
    Serial1.write(0xFF);
    Serial1.write(0xFF);
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
        CaseSpeed+=13;
        if(CaseSpeed>1000)CaseSpeed =1000;
        Conf->PutInt(CASE_RPM_KEY, CaseSpeed); 
        cmdReceiver->setMotorParameters(1, CaseSpeed, CASE_MICROSTEP, CaseDir);
        sendSystemStatus();
    } 
    else if (response == "B") {
        Serial.println("Case direction button pressed");
        CaseDir = !CaseDir;
        Conf->PutBool(CASE_DIR_KEY, CaseDir); 
        cmdReceiver->setMotorParameters(1, CaseSpeed, CASE_MICROSTEP, CaseDir);
        sendSystemStatus();
    }
    else if (response == "W") {
        Serial.println("first time param update");
        delay(200);
        sendSystemStatus();
    }
    else if (response == "C") {
        Serial.println("Case down button pressed");
        CaseSpeed-=13;
        if(CaseSpeed<100)CaseSpeed =100;
        Conf->PutInt(CASE_RPM_KEY, CaseSpeed); 
        cmdReceiver->setMotorParameters(1, CaseSpeed, CASE_MICROSTEP, CaseDir);
        sendSystemStatus();
    }
    else if (response == "S") {
        Serial.println("Start button pressed");
        SYSTEM_ON = true;  // Set system status to ON
        _motor1.Start();
        _motor2.Start();
        CaseSpeed = Conf->GetInt(CASE_RPM_KEY, CASE_RPM_DEFAULT);;
        DiscSpeed = Conf->GetInt(DISC_RPM_KEY, DISC_RPM_DEFAULT);;
        _motor1.setFrequency(CaseSpeed);
        _motor2.setFrequency(DiscSpeed);
        cmdReceiver->setMotorParameters(2,DEFAULT_DISK_SPEED,DISC_MICROSTEP,DiscDir);// Disc Motor
        cmdReceiver->setMotorParameters(1,DEFAULT_CASE_SPEED,CASE_MICROSTEP,CaseDir);// Case Motor
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
        DiscSpeed+=26;
        if(DiscSpeed>1000)DiscSpeed =1000;
        Conf->PutInt(DISC_RPM_KEY, DiscSpeed);
        cmdReceiver->setMotorParameters(2, DiscSpeed, DISC_MICROSTEP, DiscDir);
        sendSystemStatus();
    }
    else if (response == "F") {
        Serial.println("Disk direction button pressed");
        DiscDir = !DiscDir;
        Conf->PutBool(DISC_DIR_KEY, DiscDir);
        cmdReceiver->setMotorParameters(2, DiscSpeed, DISC_MICROSTEP, DiscDir);
        sendSystemStatus();
    }
    else if (response == "E") {
        Serial.println("Disk down button pressed");
        DiscSpeed-=26;
        if(DiscSpeed<100)DiscSpeed =100;
        Conf->PutInt(DISC_RPM_KEY, DiscSpeed);
        cmdReceiver->setMotorParameters(2, DiscSpeed, DISC_MICROSTEP, DiscDir);
        sendSystemStatus();
    }
    else if (response == "H") {
        Serial.println("Delay up button pressed");
        Delay += 100;
        Conf->PutInt(DELAY_MS_KEY, Delay);
        cmdReceiver->setSensorParameters(2, Delay, offset);
        sendSystemStatus();
    }
    else if (response == "I") {
        Serial.println("Delay down button pressed");
        Delay -= 100;
         if(Delay<0)Delay =100;
        Conf->PutInt(DELAY_MS_KEY, Delay);
        cmdReceiver->setSensorParameters(2, Delay, offset);
        sendSystemStatus();
    }
    else if (response == "J") {
        Serial.println("Offset down button pressed");
        offset += 5;
         if(offset<0)offset = 0;
        Conf->PutInt(OFFSET_STEPS_KEY, offset);
        cmdReceiver->setSensorParameters(2, Delay, offset);
        sendSystemStatus();
    }
    else if (response == "K") {
        Serial.println("Offset down button pressed");
        offset -= 5;
        Conf->PutInt(OFFSET_STEPS_KEY, offset);
         if(offset<0)offset =0;
        cmdReceiver->setSensorParameters(2, Delay, offset);
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
 * @brief Calculate RPM from the given frequency
 */
int NextionHMI::calculateRPM(float pulseFrequency, int microsteps, int stepsPerRevolution) {
    // Calculate RPM
    float rpm = (pulseFrequency * 60) / (stepsPerRevolution * microsteps);

    // Return the RPM as an integer
    return static_cast<int>(rpm);
}
/**
 * @brief Sends the current system status to the Nextion HMI display.
 */
void NextionHMI::sendSystemStatus() {
    if (SYSTEM_ON) {

        // Calculate RPMs
        int caseRPM =calculateRPM(CaseSpeed,CASE_MICROSTEP,FULL_STEPS_PER_REV);
        int discRPM = calculateRPM(DiscSpeed,DISC_MICROSTEP,FULL_STEPS_PER_REV);

        // Update Nextion display with calculated RPM values
        sendCommand("n1.val=" + String(caseRPM));  // Update case RPM
        sendCommand("n2.val=" + String(discRPM));  // Update disc RPM
        sendCommand("n0.val=" + String(Delay));    // Update delay
        sendCommand("n3.val=" + String(offset));    // Update delay
    }
    else {
        sendCommand("n1.val=0");  // Case speed off
        sendCommand("n2.val=0");  // Disc speed off
        sendCommand("n0.val=0");  // Delay off
        sendCommand("n3.val=0"); // offset off
    }
}


void NextionHMI::InitMotorsParameters(){
    cmdReceiver->setMotorParameters(1, 0, CASE_MICROSTEP, CaseDir);
    cmdReceiver->setMotorParameters(2, 0, DISC_MICROSTEP, DiscDir);

}