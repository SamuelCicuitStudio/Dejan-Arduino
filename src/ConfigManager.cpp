
#include "ConfigManager.h"


/************************************************************************************************/
/*                           Config Manager class definition                                    */
/************************************************************************************************/
/**
 * @brief Constructor for the ConfigManager class.
 * 
 * Initializes the ConfigManager with a reference to an external Preferences object,
 * which is used for storing and retrieving configuration settings.
 * 
 * @param prefs Reference to the Preferences object.
 */
ConfigManager::ConfigManager(Preferences* preferences) : preferences(preferences),namespaceName(CONFIG_PARTITION) {}

/**
 * @brief Destructor for the ConfigManager class.
 * 
 * Ensures that the preferences are properly closed when the ConfigManager 
 * object is destroyed, preventing memory leaks and ensuring data integrity.
 */
ConfigManager::~ConfigManager() {
    end();  // Ensure preferences are closed properly
}

/**
 * @brief Restarts the system after a specified delay.
 * 
 * This function initiates a countdown before restarting the device. 
 * During the countdown, it prints the remaining time and a series of '#' 
 * characters for visibility. It also resets the watchdog timer to prevent 
 * the system from resetting prematurely.
 * 
 * @param delayTime Time in milliseconds to wait before restarting the device.
 */
void ConfigManager::RestartSysDelayDown(unsigned long delayTime) {
    unsigned long startTime = millis();  // Record the start time

    #ifdef ENABLE_SERIAL_DEBUG 
        Serial.println("################################");
        Serial.print("Restarting the Device in: ");
        Serial.print(delayTime / 1000);  // Convert delayTime to seconds
        Serial.println(" Sec");
    #endif

    // Ensure 32 '#' are printed after the countdown
    unsigned long interval = delayTime / 32;  // Divide delayTime by 32 to get interval

    
        for (int i = 0; i < 32; i++) {  // Print 32 '#' characters
            #ifdef ENABLE_SERIAL_DEBUG
            Serial.print("#");
            #endif
            delay(interval);  // Delay for visibility of each '#' character
            esp_task_wdt_reset();  // Reset watchdog timer
        }
        Serial.println();  // Move to the next line after printing
    

    #ifdef ENABLE_SERIAL_DEBUG
        Serial.println("Restarting now...");
    #endif

    simulatePowerDown();  // Simulate power down before restart
}
/**
 * @brief Restarts the system after a specified delay.
 * 
 * This function initiates a countdown before restarting the device. 
 * During the countdown, it prints the remaining time and a series of '#' 
 * characters for visibility. It also resets the watchdog timer to prevent 
 * the system from resetting prematurely.
 * 
 * @param delayTime Time in milliseconds to wait before restarting the device.
 */
void ConfigManager::RestartSysDelay(unsigned long delayTime) {
    unsigned long startTime = millis();  // Record the start time

    #ifdef ENABLE_SERIAL_DEBUG
        Serial.println("################################");
        Serial.print("Restarting the Device in: ");
        Serial.print(delayTime / 1000);  // Convert delayTime to seconds
        Serial.println(" Sec");
    #endif

    // Ensure 32 '#' are printed after the countdown
    unsigned long interval = delayTime / 32;  // Divide delayTime by 32 to get interval

    if (DEBUGMODE) {
        for (int i = 0; i < 32; i++) {  // Print 32 '#' characters
            #ifdef ENABLE_SERIAL_DEBUG
            Serial.print("#");
            #endif
            delay(interval);  // Delay for visibility of each '#' character
            esp_task_wdt_reset();  // Reset watchdog timer
        }
        Serial.println();  // Move to the next line after printing
    }

    #ifdef ENABLE_SERIAL_DEBUG
        Serial.println("Restarting now...");
    #endif
    //simulatePowerDown();  // Simulate power down before restart
     ESP.restart();
}

/**
 * @brief Restarts the system after a specified delay.
 * 
 * This function initiates a countdown before restarting the device. 
 * During the countdown, it prints the remaining time and a series of '#' 
 * characters for visibility. It also resets the watchdog timer to prevent 
 * the system from resetting prematurely.
 * 
 * @param delayTime Time in milliseconds to wait before restarting the device.
 */
void ConfigManager::CountdownDelay(unsigned long delayTime) {
    unsigned long startTime = millis();  // Record the start time

    #ifdef ENABLE_SERIAL_DEBUG
        Serial.println("################################");
        Serial.print("Waiting User Action: ");
        Serial.print(delayTime / 1000);  // Convert delayTime to seconds
        Serial.println(" Sec");
    #endif

    // Ensure 32 '#' are printed after the countdown
    if (DEBUGMODE) {
        unsigned long interval = delayTime / 32;  // Divide delayTime by 32 to get interval

        for (int i = 0; i < 32; i++) {  // Print 32 '#' characters
            #ifdef ENABLE_SERIAL_DEBUG
            Serial.print("#");
            #endif
            delay(interval);  // Delay dynamically based on the given delayTime
            esp_task_wdt_reset();  // Reset watchdog timer
        }
        Serial.println();  // Move to the next line after printing
    }
}

/**
 * @brief Simulates a power-down by putting the ESP32 into deep sleep.
 * 
 * This function enables the ESP32 to enter deep sleep mode for 1 second. 
 * It is used to simulate the power-down state of the device.
 */
void ConfigManager::simulatePowerDown() {
    // Put the ESP32 into deep sleep for 1 second (simulate power-down)
    esp_sleep_enable_timer_wakeup(1000000); // 1 second (in microseconds)
    esp_deep_sleep_start();  // Enter deep sleep
}

/**
 * @brief Opens the preferences in read-write mode.
 * 
 * This function initializes the Preferences library to allow 
 * writing configuration settings. It opens the specified 
 * configuration partition in read-write mode.
 */
void ConfigManager::startPreferencesReadWrite() {
    preferences->begin(CONFIG_PARTITION, false);  // false = read-write mode
    #ifdef ENABLE_SERIAL_DEBUG
    Serial.println("Preferences opened in write mode.");
    #endif 
}

/**
 * @brief Opens the preferences in read-only mode.
 * 
 * This function initializes the Preferences library to allow 
 * reading configuration settings. It opens the specified 
 * configuration partition in read-only mode.
 */
void ConfigManager::startPreferencesRead() {
    preferences->begin(CONFIG_PARTITION, true);  // true = read-only mode
    #ifdef ENABLE_SERIAL_DEBUG
    Serial.println("Preferences opened in read mode.");
    #endif 
}

/**
 * @brief Initializes the configuration manager and preferences.
 * 
 * This function sets up the ConfigManager by checking the reset flag
 * and initializing various settings and configurations if necessary. 
 * It uses the Preferences library to store configuration data and
 * ensures that the system can either reset to default settings or
 * use existing configurations.
 */
void ConfigManager::begin() {
    #ifdef ENABLE_SERIAL_DEBUG
        Serial.println("###########################################################");
        Serial.println("#               Starting CONFIG Manager ⚙️                #");
        Serial.println("###########################################################");
    #endif 

    bool resetFlag = GetBool(RESET_FLAG, true);  // Default to true if not set

    if (resetFlag) {
        // Only print once, if necessary, then reset device
        #ifdef ENABLE_SERIAL_DEBUG
            Serial.println("ConfigManager: Initializing the device... 🔄");
        #endif
        initializeDefaults();  // Reset preferences if the flag is set
        RestartSysDelay(7000);  // Use a delay for restart after reset
    } else {
        // Use existing configuration, no need for unnecessary delay
        #ifdef ENABLE_SERIAL_DEBUG
            Serial.println("ConfigManager: Using existing configuration... ✅");
        #endif
    }
}


/**
 * @brief Retrieves the reset flag from preferences.
 * 
 * This function checks and returns the value of the "Reset" flag from 
 * the preferences. If the flag is not set, it defaults to true. The 
 * function also resets the watchdog timer to prevent the system from 
 * resetting unexpectedly.
 * 
 * @return bool The value of the reset flag.
 */
bool ConfigManager::getResetFlag() {
    esp_task_wdt_reset();
    bool value = preferences->getBool(RESET_FLAG, true); // Default to true if not set
    return value;
}

/**
 * @brief Closes the preferences object.
 * 
 * This function is responsible for closing the preferences object to 
 * ensure that any changes are saved and resources are freed. 
 * It should be called when no further preference operations are needed.
 */
void ConfigManager::end() {
    preferences->end();  // Close preferences
}


/**
 * @brief Initializes all needed default variables.
 * 
 * This function is called to initialize all necessary default variables 
 * for the ConfigManager, ensuring that all values are set to a known 
 * state before use.
 */
void ConfigManager::initializeDefaults() {
    initializeVariables();  // Initialize all default variables
}

/**
 * @brief Initializes all default variables.
 * 
 * This function sets the initial values for various boolean and string 
 * variables used by the ConfigManager. It includes settings for GPIO, 
 * Wi-Fi SSID, and password. Debug messages are printed if DEBUGMODE is enabled.
 */
void ConfigManager::initializeVariables() {
    // Assign default values to configuration variables

    PutInt(DELAY_MS_KEY, DEFAULT_STOP_TIME);           // Default delay in ms
    PutInt(OFFSET_STEPS_KEY, OFFSET_STEPS_DEFAULT);   // Default offset in steps
    PutInt(CASE_RPM_KEY, CASE_RPM_DEFAULT);           // Default case RPM
    PutInt(DISC_RPM_KEY, DISC_RPM_DEFAULT);           // Default disc RPM
    PutBool(CASE_DIR_KEY, CASE_DIR_DEFAULT);          // Default case direction
    PutBool(DISC_DIR_KEY, DISC_DIR_DEFAULT);          // Default disc direction
    PutBool(RESET_FLAG, false);  // Reset flag is set to false after initialization

}


/**
 * @brief Gets a boolean value from preferences.
 * 
 * This function retrieves a boolean value associated with the given key 
 * from the preferences. If the key does not exist, it returns the specified 
 * default value. The function also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key associated with the boolean value.
 * @param defaultValue The default value to return if the key does not exist.
 * @return bool The retrieved boolean value or the default value.
 */
bool ConfigManager::GetBool(const char* key, bool defaultValue) {
    esp_task_wdt_reset();
    bool value = preferences->getBool(key, defaultValue);
    return value;
}

/**
 * @brief Gets an integer value from preferences.
 * 
 * This function retrieves an integer value associated with the given key 
 * from the preferences. If the key does not exist, it returns the specified 
 * default value. The function also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key associated with the integer value.
 * @param defaultValue The default value to return if the key does not exist.
 * @return int The retrieved integer value or the default value.
 */
int ConfigManager::GetInt(const char* key, int defaultValue) {
    esp_task_wdt_reset();
    int value = preferences->getInt(key, defaultValue);
    return value;
}
/**
 * @brief Gets an integer value from preferences.
 * 
 * This function retrieves an integer value associated with the given key 
 * from the preferences. If the key does not exist, it returns the specified 
 * default value. The function also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key associated with the integer value.
 * @param defaultValue The default value to return if the key does not exist.
 * @return int The retrieved integer value or the default value.
 */
uint64_t ConfigManager::GetULong64(const char* key, int defaultValue) {
    esp_task_wdt_reset();
    uint64_t value = preferences->getULong64(key, defaultValue);
    return value;
}

/**
 * @brief Gets a float value from preferences.
 * 
 * This function retrieves a float value associated with the given key 
 * from the preferences. If the key does not exist, it returns the specified 
 * default value. The function also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key associated with the float value.
 * @param defaultValue The default value to return if the key does not exist.
 * @return float The retrieved float value or the default value.
 */
float ConfigManager::GetFloat(const char* key, float defaultValue) {
    esp_task_wdt_reset();
    float value = preferences->getFloat(key, defaultValue);
    return value;
}

/**
 * @brief Gets a string value from preferences.
 * 
 * This function retrieves a string value associated with the given key 
 * from the preferences. If the key does not exist, it returns the specified 
 * default value. The function also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key associated with the string value.
 * @param defaultValue The default value to return if the key does not exist.
 * @return String The retrieved string value or the default value.
 */
String ConfigManager::GetString(const char* key, const String& defaultValue) {
    esp_task_wdt_reset();
    String value = preferences->getString(key, defaultValue);
    return value;
}

/**
 * @brief Puts a boolean value into preferences.
 * 
 * This function stores a boolean value associated with the given key 
 * in the preferences. It also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key to associate with the boolean value.
 * @param value The boolean value to store.
 */
void ConfigManager::PutBool(const char* key, bool value) {
    esp_task_wdt_reset();
    RemoveKey(key);
    preferences->putBool(key, value);  // Store the new value
}

/**
 * @brief Puts an unsigned integer value into preferences.
 * 
 * This function stores an unsigned integer value associated with the given 
 * key in the preferences. It also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key to associate with the unsigned integer value.
 * @param value The unsigned integer value to store.
 */
void ConfigManager::PutUInt(const char* key, int value) {
    esp_task_wdt_reset();
    RemoveKey(key);
    preferences->putUInt(key, value);  // Store the new value
}

/**
 * @brief Puts an unsigned integer value into preferences.
 * 
 * This function stores an unsigned integer value associated with the given 
 * key in the preferences. It also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key to associate with the unsigned integer value.
 * @param value The unsigned integer value to store.
 */
void ConfigManager::PutULong64(const char* key, int value) {
    esp_task_wdt_reset();
    RemoveKey(key);
    preferences->putULong64(key, value);  // Store the new value
}

/**
 * @brief Puts an integer value into preferences.
 * 
 * This function stores an integer value associated with the given key 
 * in the preferences. It also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key to associate with the integer value.
 * @param value The integer value to store.
 */
void ConfigManager::PutInt(const char* key, int value) {
    esp_task_wdt_reset();
    RemoveKey(key);
    preferences->putInt(key, value);  // Store the new value
}

/**
 * @brief Puts a float value into preferences.
 * 
 * This function stores a float value associated with the given key 
 * in the preferences. It also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key to associate with the float value.
 * @param value The float value to store.
 */
void ConfigManager::PutFloat(const char* key, float value) {
    esp_task_wdt_reset();
    RemoveKey(key);
    preferences->putFloat(key, value);  // Store the new value
}

/**
 * @brief Puts a string value into preferences.
 * 
 * This function stores a string value associated with the given key 
 * in the preferences. It also resets the watchdog timer to prevent 
 * unexpected resets.
 * 
 * @param key The key to associate with the string value.
 * @param value The string value to store.
 */
void ConfigManager::PutString(const char* key, const String& value) {
    esp_task_wdt_reset();
    RemoveKey(key);
    preferences->putString(key, value);  // Store the new value
}

/**
 * @brief Clears all stored preferences.
 * 
 * This function removes all key-value pairs from the preferences 
 * storage.
 */
void ConfigManager::ClearKey() {
    preferences->clear();
}

/**
 * @brief Removes a specific key from the preferences.
 * 
 * This function checks if the specified key exists in the 
 * preferences and removes it if it does. If the key is not found, 
 * it logs a message if debugging is enabled.
 * 
 * @param key The key to remove from the preferences.
 */
void ConfigManager::RemoveKey(const char * key) {
    esp_task_wdt_reset();  // Reset the watchdog timer

    // Check if the key exists before removing it
    if (preferences->isKey(key)) {
        preferences->remove(key);  // Remove the key if it exists
        #ifdef ENABLE_SERIAL_DEBUG
            //Serial.print("Removed key: ");
            //Serial.println(key);
        #endif
    } else {
        #ifdef ENABLE_SERIAL_DEBUG
            Serial.print("Key not found, skipping: ");
            Serial.println(key);
        #endif
    }
}


