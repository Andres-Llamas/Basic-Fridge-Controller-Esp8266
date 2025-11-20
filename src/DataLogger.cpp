#include "DataLogger.h"
namespace
{    
    int lastLoggedHour = -1;
    int lastHourSeen = -1;
    int currentDayIdx = 0; // 0..13 circular
    int freezeStartsToday = 0;

    bool fsReady = false;

    void ensureFS()
    {
        if (!fsReady)
        {
            if (LittleFS.begin())
            {
                fsReady = true;
            }
            else
            {
                Serial.println("DataLogger: LittleFS mount failed");
            }
        }
    }

    // Utilidad: guardar líneas limitando el tamaño del archivo
    void appendLineWithLimit(const char *path, const String &line, int maxLines)
    {
        ensureFS();
        if (!fsReady)
            return;

        // Leer archivo completo (no más de ~400 líneas → OK)
        String content;
        if (LittleFS.exists(path))
        {
            File f = LittleFS.open(path, "r");
            if (f)
            {
                content = f.readString();
                f.close();
            }
        }

        // Partir en líneas
        int count = 0;
        std::vector<String> lines;
        lines.reserve(maxLines + 5);

        int start = 0;
        while (start < (int)content.length())
        {
            int nl = content.indexOf('\n', start);
            if (nl < 0)
                nl = content.length();
            String l = content.substring(start, nl);
            l.trim();
            if (l.length() > 0)
            {
                lines.push_back(l);
                count++;
            }
            start = nl + 1;
        }

        // Añadir nueva línea
        lines.push_back(line);
        count++;

        // Si excede, recortar desde el inicio
        while (count > maxLines)
        {
            lines.erase(lines.begin());
            count--;
        }

        // Reescribir archivo
        File f = LittleFS.open(path, "w");
        if (!f)
        {
            Serial.print("DataLogger: cannot open for write: ");
            Serial.println(path);
            return;
        }
        for (auto &l : lines)
        {
            f.println(l);
        }
        f.close();
    }

    void logHourlyTemperature()
    {
        // Usamos hour + dayIndex + tempC
        int h = TimeManager::currentTime.hours;
        float t = sensors::GetCurrentTemp();
        String line = String(currentDayIdx) + "," + String(h) + "," + String(t, 2);
        appendLineWithLimit(TEMP_FILE, line, MAX_HOURLY_SAMPLES);
        Serial.print("DataLogger: temp logged ");
        Serial.println(line);
    }

    void logDailyFreezing()
    {
        // dayIndex, starts
        String line = String(currentDayIdx) + "," + String(freezeStartsToday);
        appendLineWithLimit(FREEZE_FILE, line, MAX_DAILY_SAMPLES);
        Serial.print("DataLogger: freeze starts logged ");
        Serial.println(line);
    }

} // namespace anónimo

namespace DataLogger
{

    void Initialize()
    {
        ensureFS();
        // Cargar archivo de freeze para recuperar último dayIdx si quisieras;
        // por simplicidad lo dejamos en 0, no afecta a que tengas 14 días de ventana.
        lastLoggedHour = -1;
        lastHourSeen = TimeManager::currentTime.hours;
        currentDayIdx = 0;
        freezeStartsToday = 0;
    }

    void Update()
    {
        int h = TimeManager::currentTime.hours;

        // Detección de cambio de día: hora "baja" (ej. 23 → 0)
        if (lastHourSeen >= 0 && h < lastHourSeen)
        {
            // Nuevo día
            logDailyFreezing(); // guardar conteo de ayer
            currentDayIdx = (currentDayIdx + 1) % 14;
            freezeStartsToday = 0; // reset para el nuevo día
        }
        lastHourSeen = h;

        // Log cada hora (una vez cuando cambia la hora)
        if (lastLoggedHour != h)
        {
            logHourlyTemperature();
            lastLoggedHour = h;
        }
    }

    void OnFreezingStart()
    {
        freezeStartsToday++;
    }

    bool GetTempLogCSV(String &outCsv)
    {
        ensureFS();
        if (!fsReady)
            return false;
        if (!LittleFS.exists(TEMP_FILE))
        {
            outCsv = "dayIndex,hour,tempC\n";
            return true;
        }
        File f = LittleFS.open(TEMP_FILE, "r");
        if (!f)
            return false;
        outCsv = "dayIndex,hour,tempC\n";
        outCsv += f.readString();
        f.close();
        return true;
    }

    bool GetFreezeLogCSV(String &outCsv)
    {
        ensureFS();
        if (!fsReady)
            return false;
        if (!LittleFS.exists(FREEZE_FILE))
        {
            outCsv = "dayIndex,starts\n";
            return true;
        }
        File f = LittleFS.open(FREEZE_FILE, "r");
        if (!f)
            return false;
        outCsv = "dayIndex,starts\n";
        outCsv += f.readString();
        f.close();
        return true;
    }

}