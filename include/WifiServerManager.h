#ifndef WifiServerManager_h
#define WifiServerManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <ESP8266HTTPUpdateServer.h>

#include "sensors.h"
#include "CustomStructs.h"
#include "Behaviours.h"
#include "utilities.h"

class WifiServerManager
{
private:
    struct WifiConfig
    {
        String wifiSsid;
        String wifiPassword;
        String apPassword;
        String uiPassword;
    };

    const char *_apNameDefault; // default AP SSID (fallback)
    const char *_apPassDefault; // default AP password (fallback)
    WifiConfig _cfg;

    String _contentTypeFor(const String &path);
    bool _handleFileRead(const String &path);

    bool _loadConfig();        // read /config.json into _cfg
    bool _saveConfig();        // write _cfg back to /config.json
    void _connectWithStored(); // try STA using stored wifi_ssid / wifi_password
    void _startAPPortal();     // fallback AP mode

    void _setupHttpRoutes(); // all HTTP endpoints & static files

public:
    WifiServerManager(const char *apName, const char *apPassword);

    void Initialize();
    void UpdateServerCLient();
};

#endif
