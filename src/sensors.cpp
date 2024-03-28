#include "sensors.h"

int sensors::samples = 5;
float sensors::currentTemperature = 0;
float sensors::tempToSetFridge = -3;
float sensors::tempToSetFridgeThreshold = 1;

void sensors::Initialize()
{
    pinMode(vd_power_pin, OUTPUT);
}

void sensors::CalculateTemperature()
{
    if (sensors::CheckIfCanCheckSensors())
    {
        uint8_t i;
        float average;
        samples = 0;
        // take voltage readings from the voltage divider
        digitalWrite(vd_power_pin, HIGH);
        for (i = 0; i < samplingrate; i++)
        {
            samples += analogRead(ntc_pin);
            delay(10);
        }
        digitalWrite(vd_power_pin, LOW);
        average = 0;
        average = samples / samplingrate;
        Serial.println("\n \n");
        Serial.print("ADC readings ");
        Serial.println(average);

        // Calculate NTC resistance
        average = 1023 / average - 1;
        average = Rref / average;
        Serial.print("Thermistor resistance ");
        Serial.println(average);

        float temperature;
        temperature = average / nominal_resistance;          // (R/Ro)
        temperature = log(temperature);                      // ln(R/Ro)
        temperature /= beta;                                 // 1/B * ln(R/Ro)
        temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
        temperature = 1.0 / temperature;                     // Invert
        temperature -= 273.15;                               // convert absolute temp to C

        Serial.print("Temperature ");
        Serial.print(temperature);
        Serial.println(" *C");

        sensors::currentTemperature = temperature;
        delay(500);
    }
}

/// @brief this function is to turn on the sensor for a short period of time instead of always
bool sensors::CheckIfCanCheckSensors()
{
    if ((TimeManager::currentTime.seconds > 5 && TimeManager::currentTime.seconds < 10) || (TimeManager::currentTime.seconds > 35 && TimeManager::currentTime.seconds < 40))
    {
        digitalWrite(vd_power_pin, HIGH);
        return true;
    }
    digitalWrite(vd_power_pin, LOW);
    return false;
}