#ifndef DataLogger_h
#define DataLogger_h
#include <Arduino.h>
#include <LittleFS.h>
#include "TimeManager.h"
#include "sensors.h"
#include "defines.h"
namespace DataLogger
{
    void Initialize();
    void Update();          // Llamar desde loop
    void OnFreezingStart(); // Llamar cuando el compresor arranca

    // Para HTTP: obtener contenido CSV de los logs
    bool GetTempLogCSV(String &outCsv);
    bool GetFreezeLogCSV(String &outCsv);
}
#endif