#include "sensors.h"

int sensors::samples = 5;
float sensors::currentTemperature = 0;
float sensors::tempToSetFridge = 5;
float sensors::tempToSetFridgeThreshold = 1;

void sensors::Initialize()
{
    pinMode(VD_POWER_PIN, OUTPUT);
    loadConfig();
}

void sensors::CalculateTemperature()
{
    if (!sensors::CheckIfCanCheckSensors())
        return;

    uint8_t i;
    float average = 0.0f;
    samples = 0;

    // Power the divider only for the sampling window
    digitalWrite(VD_POWER_PIN, HIGH);
    for (i = 0; i < SAMPLINGRATE; i++)
    {
        samples += analogRead(NTC_PIN);
        delay(10);
    }
    digitalWrite(VD_POWER_PIN, LOW);

    average = (float)samples / (float)SAMPLINGRATE;
    Serial.println("\n \n");
    Serial.print("ADC readings ");
    Serial.println(average);

    // Guard rails for ADC math
    average = constrain(average, 1.0f, 1022.0f);
    float ratio = 1023.0f / average - 1.0f;
    if (ratio <= 0.0f)
        return;
    float R = RrEF / ratio;
    Serial.print("Thermistor resistance ");
    Serial.println(R);

    float temperature;
    temperature = R / NOMINAL_RESISTANCE;                // (R/Ro)
    temperature = log(temperature);                      // ln(R/Ro)
    temperature /= BETA;                                 // 1/B * ln(R/Ro)
    temperature += 1.0 / (NOMINAL_TEMPERATURE + 273.15); // + (1/To)
    temperature = 1.0 / temperature;                     // Invert
    temperature -= 273.15;                               // convert absolute temp to C

    Serial.print("Temperature ");
    Serial.print(temperature);
    Serial.println(" *C");

    sensors::currentTemperature = temperature;
    delay(500);
}
/// @brief this function is to turn on the sensor for a short period of time instead of always
/// @brief this function decides when to sample; it no longer toggles power itself
bool sensors::CheckIfCanCheckSensors()
{
    int s = TimeManager::currentTime.seconds;
    if ((s > 5 && s < 10) || (s > 35 && s < 40))
    {
        return true;
    }
    return false;
}

void sensors::loadConfig()
{
    if (!LittleFS.exists(CONFIG_FILE)) 
    {
        Serial.println("Config file not found, using defaults.");
        return;
    }
    File f = LittleFS.open(CONFIG_FILE, "r");
    if (!f)
    {
        Serial.println("Failed to open config file.");
        return;
    }
    String s = f.readString();
    f.close();
    sensors::tempToSetFridgeThreshold = getJsonString(s, "tempThreshold").toFloat();
    sensors::tempToSetFridge = getJsonString(s, "fridgeTempToSet").toFloat();
}

void sensors::SaveNewFridgeThreshold(float newVal)
{
    sensors::tempToSetFridgeThreshold = newVal;
    if (SaveNewJSON())
        Serial.println("new threshold saved");
}

void sensors::SaveNewFridgeTempToSet(float newVal)
{
    sensors::tempToSetFridge = newVal;
    if (SaveNewJSON())
        Serial.println("new temp saved");
}

bool sensors::SaveNewJSON()
{
    String json = "{";
    json += "\"tempThreshold\":\"" + String(sensors::tempToSetFridgeThreshold) + "\",";
    json += "\"fridgeTempToSet\":\"" + String(sensors::tempToSetFridge) + "\"";
    json += "}";

    // persist the new JSON to the config file
    File f = LittleFS.open(CONFIG_FILE, "w");
    if (!f)
    {
        Serial.println("Failed to open config file for writing.");
        return false;
    }
    size_t written = f.print(json);
    f.close();
    Serial.println("Config saved.");
    return (written == json.length());
}

// Simple getters for external code.
float sensors::GetCurrentTemp()
{
    return sensors::currentTemperature;
}

float sensors::GetTempToSetFridge()
{
    return sensors::tempToSetFridge;
}

float sensors::GetTempThreshold()
{
    return sensors::tempToSetFridgeThreshold;
}