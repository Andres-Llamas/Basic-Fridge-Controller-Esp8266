#ifndef WifiServerManager_h
#define WifiServerManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include "sensors.h"
#include "CustomStructs.h"
#include "Behaviours.h"
#include <ESP8266HTTPUpdateServer.h>

class WifiServerManager
{
private:
    const char *_wifiName;     // optional default/fallback
    const char *_wifiPassword; // optional default/fallback

    String _contentTypeFor(const String& path);
    bool _handleFileRead(const String& path);

    bool _loadCreds(String &ssid, String &pass);
    bool _saveCreds(const String &ssid, const String &pass);
    void _startAPPortal();
    void _connectWithStored();
public:
    void Initialize();
    void UpdateServerCLient();
    WifiServerManager(const char *wifiName, const char *wifiPassword);
};
#endif
