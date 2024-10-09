#include <ArduinoJson.h>
#include "CommandReceiver.h"

// Constructor implementation
CommandReceiver::CommandReceiver(Sensor* sensor, A4988Manager& motor1, A4988Manager& motor2)
    : commandReceived(false), // Initialize commandReceived first
      sensor(sensor),
      _motor1(motor1),        // Initialize _motor1
      _motor2(motor2) {}      // Initialize _motor2

// Initialize the receiver
void CommandReceiver::begin() {
    Serial.begin(9600); // Start serial communication
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
    StaticJsonDocument<200> doc; // Adjust size as needed

    // Deserialize the JSON command
    DeserializationError error = deserializeJson(doc, command);

    if (error) {
        //Serial.print("JSON deserialization failed: ");
        //Serial.println(error.c_str());
        return; // Exit if there is an error
    }

    // Process the commands based on the JSON structure
    if (!doc.containsKey("command")) {
        //Serial.println("Invalid command: 'command' field missing");
        return; // Ignore if no "command" field exists
    }

    const char* cmdType = doc["command"];
    bool commandRecognized = false; // Track if the command is recognized

    // Handle system commands
    if (strcmp(cmdType, "STOPSYSTEM") == 0) {
        _motor1.Stop();
        _motor2.Stop();
        _motor1.stopStepping();
        _motor2.stopStepping();
        commandRecognized = true;
        Serial.println("Command received: STOPSYSTEM");

    } else if (strcmp(cmdType, "STARTSYSTEM") == 0) {
        _motor1.Start();
        _motor2.Start();
        _motor1.startStepping();
        _motor2.startStepping();
        commandRecognized = true;
        Serial.println("Command received: STARTSYSTEM");

    } else if (strcmp(cmdType, "motor") == 0) {
        // Ensure necessary motor parameters are present
        if (doc.containsKey("motorType") && doc.containsKey("speed") && doc.containsKey("microsteps") && doc.containsKey("direction")) {
            String motor = doc["motorType"];
            int speed = doc["speed"];
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
        if (doc.containsKey("stop") && doc.containsKey("stepstotake")) {
            int stopTime = doc["stop"];
            int stepsToTake = doc["stepstotake"];

            // Set sensor parameters
            setSensorParameters(stopTime, stepsToTake);
            commandRecognized = true;
            Serial.println("Command received: SENSOR");
        } else {
            Serial.println("Invalid sensor command: missing parameters");
        }

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
    selectedMotor.setSpeed(speed);
    selectedMotor.setStepResolution(microsteps);

    // Set direction (assuming 1 for forward and 0 for backward)
    selectedMotor.SetDirPin(direction);
    selectedMotor.Reset();// reset the driver of that motor
    // Start stepping
    selectedMotor.Start(); // Start that motor Driver
    selectedMotor.startStepping(); // Start stepping the motor
}

// Set sensor parameters based on received commands
void CommandReceiver::setSensorParameters(int stopTime, int stepsToTake) {
    sensor->SetStopTime(stopTime);
    sensor->SetStepsToTake(stepsToTake); // Ensure the method name matches the one in your Sensor class
}
