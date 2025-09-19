#ifndef Behaviours_h
#define Behaviours_h

#include <Arduino.h>
#include "sensors.h"
#include "CustomStructs.h"
#include "TimeManager.h"

#define freezingRelayPin D5
#define defrostRelayPin D7

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
private:
    static void CheckTimeForDefrostActivation();
};

extern clockTime defrostTimersToActivate[10];
extern clockTime defrostTimersToDeactivate[10];

#endif
