#include <ArduinoJson.h>
#include "CommandReceiver.h"
#include "Config.h"

// Constructor implementation
CommandReceiver::CommandReceiver(Sensor* sensor, A4988Manager& motor1, A4988Manager& motor2)
    : commandReceived(false), // Initialize commandReceived first
      sensor(sensor),
      _motor1(motor1),        // Initialize _motor1
      _motor2(motor2) {}      // Initialize _motor2

// Initialize the receiver
void CommandReceiver::begin() {
    Serial.begin(BAUDE_RATE); // Start serial communication
}

// Check and process commands if data is available
void CommandReceiver::checkCommand() {
    // Check if data is available in the serial buffer
    while (Serial.available()) {
        commandBuffer = Serial.readStringUntil('\n'); // Read command until newline
        commandReceived = true; // Set the flag for received command
    }

    // Process command if received
    if (commandReceived) {
        commandReceived = false; // Reset the flag
        receiveCommand(commandBuffer); // Call the receiveCommand function
    }
}

// Function to receive and handle command
void CommandReceiver::receiveCommand(const String& command) {
    // Allocate a JSON document
    JsonDocument doc; // Adjust size as needed

    // Deserialize the JSON command
    DeserializationError error = deserializeJson(doc, command);

    if (error) {
        // Deserialization failed, return
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
        return; // Exit if there is an error
    }

    // Process the commands based on the JSON structure
    if (!doc["command"].is<const char*>()) {
        // Command is invalid if 'command' field is missing
        Serial.println("Invalid command: 'command' field missing");
        return; // Ignore if no "command" field exists
    }

    const char* cmdType = doc["command"];
    bool commandRecognized = false; // Track if the command is recognized

    // Handle system commands
    if (strcmp(cmdType, "STOPSYSTEM") == 0) {
        _motor1.Stop();
        _motor2.Stop();
        _motor1.setFrequency(0.0);
        _motor2.setFrequency(0.0);
        commandRecognized = true;
        Serial.println("Command received: STOPSYSTEM");

    } else if (strcmp(cmdType, "STARTSYSTEM") == 0) {
        _motor1.Start();
        _motor2.Start();
        _motor1.setFrequency(_motor1.getSpeed());
        _motor2.setFrequency(_motor2.getSpeed());
        commandRecognized = true;
        Serial.println("Command received: STARTSYSTEM");

    } else if (strcmp(cmdType, "motor") == 0) {
        // Ensure necessary motor parameters are present
        if (doc["motorType"].is<String>() && doc["speed"].is<float>() && doc["microsteps"].is<int>() && doc["direction"].is<int>()) {
            String motor = doc["motorType"];
            float speed = doc["speed"];
            int microsteps = doc["microsteps"];
            int direction = doc["direction"];

            setMotorParameters(motor == "motorCase" ? 1 : 2, speed, microsteps, direction);
            commandRecognized = true;
            Serial.println("Command received: MOTOR");
        } else {
            Serial.println("Invalid motor command: missing parameters");
        }

    } else if (strcmp(cmdType, "sensor") == 0) {
        // Ensure necessary sensor parameters are present
        if (doc["stop"].is<int>() && doc["stepstotake"].is<int>()) {
            int stopTime = doc["stoptime"];
            int stepsToTake = doc["stepstotake"];

            // Set sensor parameters
            setSensorParameters(2,stopTime, stepsToTake);
            commandRecognized = true;
            Serial.println("Command received: SENSOR");
        } else {
            Serial.println("Invalid sensor command: missing parameters");
        }

    } else if (strcmp(cmdType, "GETSTATUS") == 0) {
        // Handle GETSTATUS command
        sendSystemStatus();
        commandRecognized = true;
        Serial.println("Command received: GETSTATUS");

    } else {
        Serial.println("Unknown command received");
    }

    // Send acknowledgment if the command was recognized and processed
    if (commandRecognized) {
        Serial.println("Command understood and executed");
    }
}

// Set motor parameters based on received commands
void CommandReceiver::setMotorParameters(int motor, float speed, int microsteps, int direction) {
    A4988Manager& selectedMotor = (motor == 1) ? _motor1 : _motor2;

    selectedMotor.Stop(); // Stop that motor
    selectedMotor.setStepResolution(microsteps);
    // Set direction (assuming 1 for forward and 0 for backward)
    selectedMotor.setDirPin(direction);
    selectedMotor.Reset(); // Reset the driver of that motor
    selectedMotor.Start(); // Start that motor Driver
    selectedMotor.setFrequency(speed);//start the stepping task
}

// Set sensor parameters based on received commands
void CommandReceiver::setSensorParameters(int motor,int stopTime, int stepsToTake) {
    A4988Manager& selectedMotor = (motor == 1) ? _motor1 : _motor2;
    selectedMotor.SetStopTime(stopTime);
    selectedMotor.SetStepsToTake(stepsToTake); // Ensure the method name matches the one in your Sensor class

}

// Send the current status of the system
void CommandReceiver::sendSystemStatus() {
    // Create a JSON document
    JsonDocument doc;

    // Populate system status
    doc["status"] = "ok";

    // Motor case parameters
    JsonObject motorCase = doc["motorCase"].to<JsonObject>();
    motorCase["speed"] = _motor1.getSpeed(); // Assuming you have a getter method in A4988Manager to get the speed
    motorCase["microsteps"] = _motor1.getStepResolution(); // Assuming getter for microsteps
    motorCase["direction"] = _motor1.getDir(); // Assuming getter for direction

    // Motor disc parameters
    JsonObject motorDisc = doc["motorDisc"].to<JsonObject>();
    motorDisc["speed"] = _motor2.getSpeed(); // Same assumptions as above
    motorDisc["microsteps"] = _motor2.getStepResolution();
    motorDisc["direction"] = _motor2.getDir();

    // Sensor parameters
    JsonObject Sensor = doc["sensor"].to<JsonObject>();
    Sensor["stop"] = _motor2.GetStopTime(); // Assuming GetStopTime is a method in Sensor class
    Sensor["stepsToTake"] = _motor2.GetStepsToTake(); // Assuming GetStepsToTake is a method in Sensor class

    // System status
    JsonObject system = doc["system"].to<JsonObject>();
    system["status"] = "active"; // Or whatever the current system status is
    system["lastCommand"] = "STARTSYSTEM"; // The last command that was executed, could be dynamically updated

    // Serialize the JSON to a string
    String output;
    serializeJson(doc, output);

    // Send the JSON string over Serial
    Serial.println(output);
}