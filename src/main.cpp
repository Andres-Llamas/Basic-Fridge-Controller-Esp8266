#include <Arduino.h>
#include "sensors.h"
#include "Behaviours.h"
#include "WifiServerManager.h"
#include "TimeManager.h"

WifiServerManager server("TP-Link_AP_72F2", "41168004");
int resistorPin = D1;//TODO make an option to regulate temperature with the knob resistor

void setup(void)
{    
    Serial.begin(9600); // initialize serial communication at a baud rate of 9600    
    Behaviours::Initialize();
    sensors::Initialize();
    server.Initialize();    
    TimeManager::Initialization();
    delay(1000);
}

void loop(void)
{
    server.UpdateServerCLient();
    TimeManager::UpdateTime();

    sensors::CalculateTemperature();
    Behaviours::Looping();
    delay(500);
}