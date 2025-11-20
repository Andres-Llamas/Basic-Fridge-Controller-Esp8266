#ifndef sensors_h
#define sensors_h

#include <Arduino.h>
#include "TimeManager.h"
#include "defines.h"
#include <LittleFS.h>
#include "utilities.h"

class sensors
{
private:
    static int samples; // array to store the samples
    static bool CheckIfCanCheckSensors();
    static void loadConfig();
    static float tempToSetFridgeThreshold;
    static float tempToSetFridge;
    static float currentTemperature;
    static bool SaveNewJSON();

public:
    static float GetCurrentTemp();
    static float GetTempThreshold();
    static float GetTempToSetFridge();
    static void Initialize();
    static void CalculateTemperature();
    static void SaveNewFridgeThreshold(float newVal);
    static void SaveNewFridgeTempToSet(float newVal);
};
#endif