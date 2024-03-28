#ifndef WifiServerManager_h
#define WifiServerManager_h

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include "WifiServerManager.h"
#include "Sensors.h"
#include "CustomStructs.h"
#include "Behaviours.h"

class WifiServerManager
{
private:
    char *_wifiName;
    char *_wifiPassword;
public:
    void Initialize();
    void UpdateServerCLient();
    WifiServerManager(char *wifiName, char *wifiPassword);    
};
#endif