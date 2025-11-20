#include <Arduino.h>
#include "sensors.h"
#include "Behaviours.h"
#include "WifiServerManager.h"
#include "TimeManager.h"
#include "DataLogger.h"

// These are only DEFAULTS for AP mode when there is no config.json yet
// or when STA connection fails. Actual STA credentials are in /config.json.
WifiServerManager server("Fridge_AP", "12345678");
int resistorPin = D1; // TODO make an option to regulate temperature with the knob resistor

void setup(void)
{
    Serial.begin(9600);
    delay(500);

    Behaviours::Initialize();
    sensors::Initialize();
    TimeManager::Initialization();

    server.Initialize(); // will mount LittleFS, load config.json, connect WiFi / start AP

    DataLogger::Initialize();

    delay(1000);
}

void loop(void)
{
    server.UpdateServerCLient();
    TimeManager::UpdateTime();

    sensors::CalculateTemperature();
    Behaviours::Looping();

    DataLogger::Update();
    delay(500);
}