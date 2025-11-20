#include "Behaviours.h"

clockTime defrostTimersToActivate[10];
clockTime defrostTimersToDeactivate[10];

bool Behaviours::isDefrostActive = false;
bool Behaviours::isFreezingActive = false;
bool defrostArmDelayActive = false;
unsigned long defrostArmStartMs = 0;

static unsigned long lastCompOffMs = 0;
static unsigned long lastCompOnMs = 0;
static const unsigned long MIN_COMP_OFF_MS = 5UL * 60UL * 1000UL; // anti-short-cycle
static const unsigned long MIN_COMP_ON_MS = 90UL * 1000UL;        // avoid chatter
static unsigned long bootMs = 0;
static const unsigned long BOOT_SAFE_MS = 10UL * 1000UL;

static int lastMinuteChecked = -1;
static bool defrostStartFired[10] = {false};
static bool defrostStopFired[10] = {false};

static void loadDefrostTimersFromFS();
static void saveDefrostTimersToFS();

void Behaviours::Initialize()
{
    bootMs = millis();
    pinMode(freezingRelayPin, OUTPUT);
    pinMode(defrostRelayPin, OUTPUT);

    WriteFreezingRelayPin(false);
    WriteDefrostRelayPinState(false);

    // Inicializa timers con valores por defecto
    for (int i = 0; i < 10; ++i)
    {
        defrostTimersToActivate[i] = {"Domingo", 0, 0, 0};
        defrostTimersToDeactivate[i] = {"Domingo", 0, 0, 0};
    }

    // Intenta cargarlos desde LittleFS
    loadDefrostTimersFromFS();
}

void Behaviours::WriteFreezingRelayPin(bool state)
{
    if (RELAYS_MODE == 0) // 0 for normally closed
    {
        digitalWrite(freezingRelayPin, state);
    }
    else
    {
        digitalWrite(freezingRelayPin, !state);
    }
}

void Behaviours::WriteDefrostRelayPinState(bool state)
{
    if (RELAYS_MODE == 0) // 0 for normally closed
    {
        digitalWrite(defrostRelayPin, state);
    }
    else
    {
        digitalWrite(defrostRelayPin, !state);
    }
}

void Behaviours::Looping()
{
    // service defrost arming delay
    if (defrostArmDelayActive && (millis() - defrostArmStartMs >= 60000UL))
    {
        WriteDefrostRelayPinState(true);
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
    if (state && Behaviours::isDefrostActive)
    {
        Serial.println("Cannot activate freezing: defrost is active.");
        state = false;
    }
    unsigned long now = millis();
    if (state)
    {
        if (now - bootMs < BOOT_SAFE_MS)
        {
            Serial.println("Boot safe window: delaying compressor start.");
            return;
        }
        if ((now - lastCompOffMs) < MIN_COMP_OFF_MS)
        {
            Serial.println("Anti-short-cycle: too soon to restart compressor.");
            return;
        }
        WriteFreezingRelayPin(true);
        if (!isFreezingActive)
        {
            // Transición OFF -> ON → arranque
            DataLogger::OnFreezingStart(); // <--- To log the amount of times the fridge starts per day
        }
        isFreezingActive = true;
        lastCompOnMs = now;
    }
    else
    {
        if (isFreezingActive && (now - lastCompOnMs) < MIN_COMP_ON_MS)
        {
            Serial.println("Min-on guard: keeping compressor on briefly to avoid chatter.");
            return;
        }
        WriteFreezingRelayPin(false);
        isFreezingActive = false;
        lastCompOffMs = now;
    }
}
void Behaviours::SetDefrostState(bool state)
{
    if (state)
    {
        Behaviours::SetFreezingState(false);
        defrostArmDelayActive = true;
        defrostArmStartMs = millis();
        isDefrostActive = false; // will flip true after delay
        Serial.println("Defrost arming: 60s delay started.");
    }
    else
    {
        WriteDefrostRelayPinState(false);
        isDefrostActive = false;
        defrostArmDelayActive = false;
        Serial.println("Defrost OFF.");
    }
}
void Behaviours::CheckTimeForDefrostActivation()
{
    if (TimeManager::currentTime.minutes != lastMinuteChecked)
    {
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
    if (sensors::GetCurrentTemp() > sensors::GetTempToSetFridge() + sensors::GetTempThreshold())
    {
        Behaviours::SetFreezingState(true);
    }
}

void Behaviours::CheckToStopFreezeBehaviour()
{
    if (sensors::GetCurrentTemp() < sensors::GetTempToSetFridge() - sensors::GetTempThreshold())
    {
        Behaviours::SetFreezingState(false);
    }
}

void Behaviours::CheckToStartOrStopDefrostBehaviour()
{
    CheckTimeForDefrostActivation();
}

bool CheckTimersDifference(clockTime start, clockTime stop)
{
    int s1 = start.hours * 3600 + start.minutes * 60 + start.seconds;
    int s2 = stop.hours * 3600 + stop.minutes * 60 + stop.seconds;

    int max = MAX_DEFROST_TIME * 60; // allowed max duration in seconds
    int min = MIN_DEFROST_TIME * 60;
    int diff = s2 - s1;

    // If negative, this means the stop time is next day.
    if (diff < 0)
    {
        diff += 24 * 3600; // add 24h
    }

    if (diff == 0)
    {
        Serial.println("The timers difference is 0");
        return false;
    }

    if (diff < min)
    {
        Serial.println("Defrost duration too short than " + String(MIN_DEFROST_TIME) + " minutes");
        return false;
    }

    if (diff > max)
    {
        Serial.println("The timers difference is greater than " + String(MAX_DEFROST_TIME) + " minutes");
        return false;
    }

    return true;
}

// Accesed trhough wifi manager
void Behaviours::AddDefrostTimer(clockTime timeToActivate, clockTime timeToStop, int indexToSet)
{
    // this method adds clockTime structures which contain day, hours, minutes and seconds to the list in order to make a
    // register of the hours the user wants the irrigator system to turn on

    if (indexToSet < 0 || indexToSet >= 10)
    {
        Serial.println("AddDefrostTimer: index out of range");
        return;
    }
    if (!CheckTimersDifference(timeToActivate, timeToStop))
    {
        Serial.println("ERROR SAVING TIMER");
        return;
    }

    defrostTimersToActivate[indexToSet] = timeToActivate;
    defrostTimersToDeactivate[indexToSet] = timeToStop;

    Serial.print("Defrost timer stored in index ");
    Serial.println(indexToSet);

    // Guardar todos los timers en LittleFS
    saveDefrostTimersToFS();
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
void Behaviours::GetDefrostTimerPair(int index, clockTime &start, clockTime &stop)
{
    if (index < 0 || index >= 10)
    {
        Serial.println("GetDefrostTimerPair: index out of range");
        start = {"ANY", 0, 0, 0};
        stop = {"ANY", 0, 0, 0};
        return;
    }
    start = defrostTimersToActivate[index];
    stop = defrostTimersToDeactivate[index];

    Serial.print("Getting timer pair from index ");
    Serial.println(index);
}

static void loadDefrostTimersFromFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed in Behaviours; defrost timers not loaded.");
        return;
    }

    if (!LittleFS.exists(TIMERS_FILE))
    {
        Serial.println("No defrost timers file; using defaults.");
        return;
    }

    File f = LittleFS.open(TIMERS_FILE, "r");
    if (!f)
    {
        Serial.println("Failed to open defrost timers file.");
        return;
    }

    Serial.println("Loading defrost timers from FS...");
    for (int i = 0; i < 10 && f.available(); ++i)
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        int sh = 0, sm = 0, ss = 0;
        int eh = 0, em = 0, es = 0;

        // Formato: sh sm ss eh em es
        int parsed = sscanf(line.c_str(), "%d %d %d %d %d %d",
                            &sh, &sm, &ss,
                            &eh, &em, &es);
        if (parsed == 6)
        {
            defrostTimersToActivate[i] = {"ANY", sh, sm, ss};
            defrostTimersToDeactivate[i] = {"ANY", eh, em, es};
            Serial.printf("Timer %d: %02d:%02d:%02d -> %02d:%02d:%02d\n",
                          i, sh, sm, ss, eh, em, es);
        }
    }

    f.close();
}

static void saveDefrostTimersToFS()
{
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed in Behaviours; defrost timers not saved.");
        return;
    }

    File f = LittleFS.open(TIMERS_FILE, "w");
    if (!f)
    {
        Serial.println("Failed to open defrost timers file for writing.");
        return;
    }

    for (int i = 0; i < 10; ++i)
    {
        clockTime &start = defrostTimersToActivate[i];
        clockTime &stop = defrostTimersToDeactivate[i];
        // sh sm ss eh em es
        f.printf("%d %d %d %d %d %d\n",
                 start.hours, start.minutes, start.seconds,
                 stop.hours, stop.minutes, stop.seconds);
    }

    f.close();
    Serial.println("Defrost timers saved to FS.");
}
