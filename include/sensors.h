#ifndef sensors_h
#define sensors_h

#include <Arduino.h>
#include "TimeManager.h"

#define ntc_pin A0           // Pin,to which the voltage divider is connected
#define vd_power_pin D2           // 5V for the voltage divider
#define nominal_resistance 5000 // Nominal resistance at 25⁰C
#define nominal_temeprature 25   // temperature for nominal resistance (almost always 25⁰ C)
#define samplingrate 5           // Number of samples
#define beta 3950                // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
#define Rref 10000               // Value of  resistor used for the voltage divider

class sensors
{
private:
    static int samples;                 // array to store the samples            
    static bool CheckIfCanCheckSensors();
public:
    static float tempToSetFridge;
    static float tempToSetFridgeThreshold;
    static float currentTemperature;
    static void Initialize();     
    static void CalculateTemperature();  
};
#endif