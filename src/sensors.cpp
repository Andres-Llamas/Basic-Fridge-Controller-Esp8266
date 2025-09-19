#include "sensors.h"

int sensors::samples = 5;
float sensors::currentTemperature = 0;
float sensors::tempToSetFridge = 5;
float sensors::tempToSetFridgeThreshold = 1;

void sensors::Initialize()
{
    pinMode(vd_power_pin, OUTPUT);
}

void sensors::CalculateTemperature()
{
    if (!sensors::CheckIfCanCheckSensors())
        return;

    uint8_t i;
    float average = 0.0f;
    samples = 0;

    // Power the divider only for the sampling window
    digitalWrite(vd_power_pin, HIGH);
    for (i = 0; i < samplingrate; i++)
    {
        samples += analogRead(ntc_pin);
        delay(10);
    }
    digitalWrite(vd_power_pin, LOW);

    average = (float)samples / (float)samplingrate;
    Serial.println("\n \n");
    Serial.print("ADC readings ");
    Serial.println(average);

    // Guard rails for ADC math
    average = constrain(average, 1.0f, 1022.0f);
    float ratio = 1023.0f / average - 1.0f;
    if (ratio <= 0.0f) return;
    float R = Rref / ratio;
    Serial.print("Thermistor resistance ");
    Serial.println(R);

    float temperature;
    temperature = R / nominal_resistance;          // (R/Ro)
    temperature = log(temperature);                 // ln(R/Ro)
    temperature /= beta;                            // 1/B * ln(R/Ro)
    temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
    temperature = 1.0 / temperature;                // Invert
    temperature -= 273.15;                          // convert absolute temp to C

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
