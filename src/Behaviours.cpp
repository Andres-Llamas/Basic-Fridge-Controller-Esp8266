#include "Behaviours.h"

clockTime defrostTimersToActivate[10];
clockTime defrostTimersToDeactivate[10];

bool Behaviours::isDefrostActive = false;
bool Behaviours::isFreezingActive = false;
bool defrostArmDelayActive = false;
unsigned long defrostArmStartMs = 0;

static unsigned long lastCompOffMs = 0;
static unsigned long lastCompOnMs  = 0;
static const unsigned long MIN_COMP_OFF_MS = 5UL * 60UL * 1000UL; // anti-short-cycle
static const unsigned long MIN_COMP_ON_MS  = 90UL * 1000UL;       // avoid chatter
static unsigned long bootMs = 0;
static const unsigned long BOOT_SAFE_MS = 10UL * 1000UL;

static int lastMinuteChecked = -1;
static bool defrostStartFired[10] = {false};
static bool defrostStopFired[10]  = {false};


void Behaviours::Initialize()
{
    bootMs = millis();
    pinMode(freezingRelayPin, OUTPUT);
    pinMode(defrostRelayPin, OUTPUT);

    digitalWrite(freezingRelayPin, LOW);
    digitalWrite(defrostRelayPin, LOW);    
}

void Behaviours::Looping()
{
// service defrost arming delay
if (defrostArmDelayActive && (millis() - defrostArmStartMs >= 60000UL)) {
    digitalWrite(defrostRelayPin, HIGH);
    isDefrostActive = true;
    defrostArmDelayActive = false;
    Serial.println("Defrost ON after 60s arm delay.");
}

    Behaviours::CheckToStartFreezeBehaviour();
    Behaviours::CheckToStopFreezeBehaviour();
    Behaviours::CheckToStartOrStopDefrostBehaviour();    
}

// Freezing and defrost cycles
void Behaviours::SetFreezingState(bool state)
{
    if (state && Behaviours::isDefrostActive) {
        Serial.println("Cannot activate freezing: defrost is active.");
        state = false;
    }
    unsigned long now = millis();
    if (state) {
        if (now - bootMs < BOOT_SAFE_MS) {
            Serial.println("Boot safe window: delaying compressor start.");
            return;
        }
        if ((now - lastCompOffMs) < MIN_COMP_OFF_MS) {
            Serial.println("Anti-short-cycle: too soon to restart compressor.");
            return;
        }
        digitalWrite(freezingRelayPin, HIGH);
        isFreezingActive = true;
        lastCompOnMs = now;
    } else {
        if (isFreezingActive && (now - lastCompOnMs) < MIN_COMP_ON_MS) {
            Serial.println("Min-on guard: keeping compressor on briefly to avoid chatter.");
            return;
        }
        digitalWrite(freezingRelayPin, LOW);
        isFreezingActive = false;
        lastCompOffMs = now;
    }
}
void Behaviours::SetDefrostState(bool state)
{
    if (state) {
        Behaviours::SetFreezingState(false);
        defrostArmDelayActive = true;
        defrostArmStartMs = millis();
        isDefrostActive = false; // will flip true after delay
        Serial.println("Defrost arming: 60s delay started.");
    } else {
        digitalWrite(defrostRelayPin, LOW);
        isDefrostActive = false;
        defrostArmDelayActive = false;
        Serial.println("Defrost OFF.");
    }
}
void Behaviours::CheckTimeForDefrostActivation()
{
    if (TimeManager::currentTime.minutes != lastMinuteChecked) {
        memset(defrostStartFired, 0, sizeof(defrostStartFired));
        memset(defrostStopFired, 0, sizeof(defrostStopFired));
        lastMinuteChecked = TimeManager::currentTime.minutes;
    }

    for (int i = 0; i < 10; i++)
    {
        clockTime startTime = defrostTimersToActivate[i];
        clockTime stopTime = defrostTimersToDeactivate[i];
        if (startTime.hours != 0) // simple validity check
        {
            if (!Behaviours::isDefrostActive && !defrostStartFired[i])
            {
                if (startTime.hours == TimeManager::currentTime.hours && startTime.minutes == TimeManager::currentTime.minutes)
                {
                    defrostStartFired[i] = true;
                    Serial.print("Activation from timer with index: ");
                    Serial.println(i);
                    TimeManager::ShowDateAndTime();
                    Behaviours::SetDefrostState(true);
                }
            }
            else if (Behaviours::isDefrostActive && !defrostStopFired[i])
            {
                if (stopTime.hours == TimeManager::currentTime.hours && stopTime.minutes == TimeManager::currentTime.minutes)
                {
                    defrostStopFired[i] = true;
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