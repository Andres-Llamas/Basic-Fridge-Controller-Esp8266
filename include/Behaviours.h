#ifndef Behaviours_h
#define Behaviours_h

#include <Arduino.h>
#include "sensors.h"
#include "CustomStructs.h"
#include "TimeManager.h"
#include "defines.h"
#include <LittleFS.h>
#include <stdio.h>
#include "DataLogger.h"

class Behaviours
{
public:
    static void SetFreezingState(bool state);
    static void SetDefrostState(bool state);
    static bool isFreezingActive;
    static bool isDefrostActive;
    static void Looping();
    static void Initialize();
    static void CheckToStartFreezeBehaviour();
    static void CheckToStopFreezeBehaviour();
    static void CheckToStartOrStopDefrostBehaviour();
    static void AddDefrostTimer(clockTime timeToActivate, clockTime timeToStop, int indexToSet);
    static clockTime GetClockTimeFromList(int index);
    static void GetDefrostTimerPair(int index, clockTime &start, clockTime &stop);

private:
    static void CheckTimeForDefrostActivation();
    static void WriteFreezingRelayPin(bool state);
    static void WriteDefrostRelayPinState(bool state);
};

extern clockTime defrostTimersToActivate[10];
extern clockTime defrostTimersToDeactivate[10];

#endif
