#include "TimeManager.h"
#include <NTPClient.h>
#include "WiFiUdp.h"

clockTime TimeManager::currentTime;
long _utcOffsetInSeconds = -21600; // The offset difference between the Time zones calculated by UTC*60*60
char *daysOfTheWeek[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", _utcOffsetInSeconds);


void TimeManager::Initialization()
{
    timeClient.begin();
}

void TimeManager::UpdateTime()
{
    timeClient.update();
    TimeManager::currentTime.day = daysOfTheWeek[timeClient.getDay()];
    TimeManager::currentTime.hours = timeClient.getHours();
    TimeManager::currentTime.minutes = timeClient.getMinutes();
    TimeManager::currentTime.seconds = timeClient.getSeconds();
}

void TimeManager::ShowDateAndTime(clockTime timeToShow)
{
    Serial.print(" ");
    Serial.print(timeToShow.day);
    Serial.print(",");
    Serial.print(":");
    Serial.print(timeToShow.hours);    
    Serial.print(":");
    Serial.print(timeToShow.minutes);    
    Serial.print(":");
    Serial.println(timeToShow.seconds);    
}

void TimeManager::ShowDateAndTime()
{
    Serial.print("Current real time: ");
    Serial.print(TimeManager::currentTime.day);
    Serial.print(", ");
    Serial.print(TimeManager::currentTime.hours);
    Serial.print(":");
    Serial.print(TimeManager::currentTime.minutes);
    Serial.print(":");
    Serial.println(TimeManager::currentTime.seconds);
}