#include "Behaviours.h"

clockTime defrostTimersToActivate[10];
clockTime defrostTimersToDeactivate[10];

bool Behaviours::isDefrostActive = false;
bool Behaviours::isFreezingActive = false;

void Behaviours::Initialize()
{
    pinMode(freezingRelayPin, OUTPUT);
    pinMode(defrostRelayPin, OUTPUT);

    digitalWrite(freezingRelayPin, LOW);
    digitalWrite(defrostRelayPin, LOW);    
}

void Behaviours::Looping()
{
    Behaviours::CheckToStartFreezeBehaviour();
    Behaviours::CheckToStopFreezeBehaviour();
    Behaviours::CheckToStartOrStopDefrostBehaviour();    
}

// Freezing and defrost cycles
void Behaviours::SetFreezingState(bool state)
{
    if (state == true && Behaviours::isDefrostActive)
    {
        Serial.println("Can not activate freezing. Defrost is in process");
        state = false;
    }
    digitalWrite(freezingRelayPin, state);
    isFreezingActive = state;
}

void Behaviours::SetDefrostState(bool state)
{
    if (state == true)
    {
        Behaviours::SetFreezingState(false);
    }
    delay(60000); // 60 second delay before activating defrost proccess;
    digitalWrite(defrostRelayPin, state);
    isDefrostActive = state;
}

void Behaviours::CheckTimeForDefrostActivation()
{

    for (int i = 0; i < 5; i++)
    {
        clockTime startTime = defrostTimersToActivate[i];
        clockTime stopTime = defrostTimersToDeactivate[i];
        if (startTime.hours != 0) // if the hours is 0, that means that index haven't been set by the user
        {
            if (Behaviours::isDefrostActive == false) // if the valve is closed
            {
                // Check at what time to open it according to the established by the user
                if (startTime.hours == TimeManager::currentTime.hours && startTime.minutes == TimeManager::currentTime.minutes)
                {
                    Serial.print("Activation from timer with index: ");
                    Serial.println(i);
                    TimeManager::ShowDateAndTime();
                    Behaviours::SetDefrostState(true);
                }
            }
            else // if the valve is open
            {
                // Check at what time to close it according to the established by the user
                if (stopTime.hours == TimeManager::currentTime.hours && stopTime.minutes == TimeManager::currentTime.minutes)
                {
                    Serial.print("Deactivation from timer with index: ");
                    Serial.println(i);
                    TimeManager::ShowDateAndTime();
                    Behaviours::SetDefrostState(false);
                }
            }
        }
    }
}

void Behaviours::CheckToStartFreezeBehaviour()
{
    if (sensors::currentTemperature > sensors::tempToSetFridge + sensors::tempToSetFridgeThreshold)
    {
        Behaviours::SetFreezingState(true);
    }
}

void Behaviours::CheckToStopFreezeBehaviour()
{
    if (sensors::currentTemperature < sensors::tempToSetFridge - sensors::tempToSetFridgeThreshold)
    {
        Behaviours::SetFreezingState(false);
    }
}

void Behaviours::CheckToStartOrStopDefrostBehaviour()
{
    CheckTimeForDefrostActivation();
}

// Accesed trhough wifi manager

void Behaviours::AddDefrostTimer(clockTime timeToActivate, clockTime timeToStop, int indexToSet)
{
    // this method adds clockTime structures which contain day, hours, minutes and seconds to the list in order to make a
    // register of the hours the user wants the irrigator system to turn on
    defrostTimersToActivate[indexToSet] = timeToActivate;
    defrostTimersToDeactivate[indexToSet] = timeToStop;
    Serial.print("The following data has been stored in the clock system index ");
    Serial.println(indexToSet);
}

clockTime Behaviours::GetClockTimeFromList(int index)
{
    clockTime val;
    val = defrostTimersToActivate[index];
    Serial.print("Getting timer from index ");
    Serial.println(index);
    // TimeManager::ShowDateAndTime(val);
    // TimeManager::ShowDateAndTime();
    return val;
}