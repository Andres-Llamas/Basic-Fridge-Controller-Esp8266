#include "WifiServerManager.h"

char WifiServerManager::*_wifiName;
char WifiServerManager::*_wifiPassword;
ESP8266WebServer _server(80);

void handle_root();
void SetTimersForDefrostHandler();
void StablishMDNSDirectionsAndBeginServer();
void CheckDefrostRelayStateHandler();
void CheckFreezerRelayStateHandler();
void CheckSensorsHandler();
void SetFridgeTemperatureHandler();
void SetTemperatureThresholdHandler();
void GetDefrostTimers();

void WifiServerManager::Initialize()
{
    Serial.println("ESP starts");
    Serial.println(_wifiName);

    // Connect to your wi-fi modem
    WiFi.begin(_wifiName, _wifiPassword);

    // Check wi-fi is connected to wi-fi network
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected successfully");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP()); // Show ESP32 IP on serial

    StablishMDNSDirectionsAndBeginServer();
    Serial.println("HTTP server started");
    delay(100);

    Serial.print("Connected! IP-Address: ");
    Serial.println(WiFi.localIP()); // Displaying the IP Address

    if (MDNS.begin("refri"))
    {
        Serial.println("DNS started, available with: ");
        Serial.println("http://refri.local/");
        Serial.print("Or\nhttp://");
        Serial.print(WiFi.localIP());
        Serial.print(".local/");
    }
    else
        Serial.println("Error starting MDNS");
}

void StablishMDNSDirectionsAndBeginServer()
{
    _server.onNotFound([]()
                       { _server.send(404, "text/plain", "Link was not found!"); });

    _server.on("/", handle_root);
    _server.on("/checkDef", CheckDefrostRelayStateHandler);
    _server.on("/checkFreeze", CheckFreezerRelayStateHandler);
    _server.on("/timersDef", SetTimersForDefrostHandler);
    _server.on("/sensors", CheckSensorsHandler);
    _server.on("/setFrTemp",SetFridgeTemperatureHandler);
    _server.on("/setTrhTemp",SetTemperatureThresholdHandler);
    _server.on("/getDef",GetDefrostTimers);

    _server.begin();
}

void CheckDefrostRelayStateHandler()
{

    String message = "";
    if (Behaviours::isDefrostActive)
        message = "Defrost process is currently active";
    else
        message = "Defrost proccess is deactivated";
    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <body>";
    HTML += "       <h1><center>" + message + "</center></h1>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/\"><center>Return</center></a>";
    HTML += "   </body>";
    HTML += "</html>";
    _server.send(200, "text/html", HTML);
}

void CheckFreezerRelayStateHandler()
{
    String message = "";
    if (Behaviours::isFreezingActive)
        message = "Freezing process is currently active";
    else
        message = "Freezing proccess is deactivated";
    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <body>";
    HTML += "       <h1><center>" + message + "</center></h1>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/\"><center>Return</center></a>";
    HTML += "   </body>";
    HTML += "</html>";
    _server.send(200, "text/html", HTML);
}

void SetTimersForDefrostHandler()
{
    /*When the user enters "http://sisriego.local/timers", the user must write the parameters as.-
    http://sisriego.local/timers?startHour=1&startMinute=2&stopHour=3&stopMinute=4&indexToSet=5. or whit IP address
    http://192.168.100.23/timers?startHour=1&startMinute=2&stopHour=3&stopMinute=4&indexToSet=5
    then this method will count how many parameters there are, then will iterate for each one, if the name matches with the
    name of the variable from this method, then gets the value written from the user and stores it in a string, then after the
    iteration is done, this method will convert each string to int and store then uin the "clockTime" structure in order to pass that
    structure to the method "AddFreezerTimer" from "Behaviors.h"
    */
    String startHour = "";
    String startMinute = "";
    String stopHour = "";
    String stopMinute = "";
    String indexToSet = "";
    clockTime startTime;
    clockTime stopTime;
    String message = "Number of args received :";
    message += _server.args(); // Get number of parameters
    message += "\n";           // Add a new line

    for (int i = 0; i < _server.args(); i++)
    {

        message += "Arg nº" + (String)i + " -> "; // Include the current iteration value
        message += _server.argName(i) + ":";      // Get the name of the parameter
        message += _server.arg(i) + "\n";         // Get the value of the parameter
        if (_server.argName(i).equals("startHour"))
            startHour = _server.arg(i);
        else if (_server.argName(i).equals("startMinute"))
            startMinute = _server.arg(i);
        else if (_server.argName(i).equals("stopHour"))
            stopHour = _server.arg(i);
        else if (_server.argName(i).equals("stopMinute"))
            stopMinute = _server.arg(i);
        else if (_server.argName(i).equals("indexToSet"))
            indexToSet = _server.arg(i);
        else
        {
            message = "Error at ";
            message += i;
            message += " parameter index. The parameter is incorrect";
        }
    }
    startTime.hours = startHour.toInt();
    startTime.minutes = startMinute.toInt();
    startTime.seconds = 0; // his is because I will use only hours and minutes to control the system
    stopTime.hours = stopHour.toInt();
    stopTime.minutes = stopMinute.toInt();
    stopTime.seconds = 0;
    Behaviours::AddDefrostTimer(startTime, stopTime, indexToSet.toInt());
    _server.send(200, "text/plain", message); // Response to the HTTP request
}

void CheckSensorsHandler()
{

    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <body>";
    HTML += "       <h1><center>Sensors values</center></h1>";
    HTML += "       <p><center> Temperature = ";
    HTML += sensors::currentTemperature;
    HTML += "c*</center></p>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/\"><center>Return</center></a>";
    HTML += "   </body>";
    HTML += "</html>";
    _server.send(200, "text/html", HTML);
}

void SetFridgeTemperatureHandler()
{
    String temp = "";
    if (_server.arg("temp") == "")
        temp = "Parameter not found";
    else
        temp = _server.arg("temp");

    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <body>";
    HTML += "       <h1><center> changed Fridge temperature to " + temp + "</center></h1>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/\"><center>Return</center></a>";
    HTML += "   </body>";
    HTML += "</html>";
    sensors::tempToSetFridge = temp.toFloat();
    Serial.print("Changed temperature for fridge to: ");
    Serial.println(sensors::tempToSetFridge);
    _server.send(200, "text/html", HTML);
}

void SetTemperatureThresholdHandler()
{
    String temp = "";
    if (_server.arg("temp") == "")
        temp = "Parameter not found";
    else
        temp = _server.arg("temp");

    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <body>";
    HTML += "       <h1><center> changed Fridge temperature threshold to " + temp + "</center></h1>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/\"><center>Return</center></a>";
    HTML += "   </body>";
    HTML += "</html>";
    sensors::tempToSetFridgeThreshold = temp.toFloat();
    Serial.print("Changed temperature threshold for fridge to: ");
    Serial.println(sensors::tempToSetFridgeThreshold);
    _server.send(200, "text/html", HTML);
}

void GetDefrostTimers()
{
    String index = "";
    if (_server.arg("index") == "")
        index = "Parameter not found";
    else
        index = _server.arg("index");
    int val = index.toInt();
    clockTime time = Behaviours::GetClockTimeFromList(val);
    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <body>";
    HTML += "       <h1><center>Valve activation time from timer number " + index + "</center></h1>";
    HTML += "       <h2><center>Hour: ";
    HTML += time.hours;
    HTML += "</center></h2>";
    HTML += "       <h2><center>Minutes: ";
    HTML += time.minutes;
    HTML += "</center></h2>";
    HTML += "       <h2><center>Seconds: ";
    HTML += time.seconds;
    HTML += "</center></h2>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/\"><center>Return</center></a>";
    HTML += "   </body>";
    HTML += "</html>";
    _server.send(200, "text/html", HTML);
}

void handle_root()
{
    String HTML = "";
    HTML += "<!DOCTYPE html>";
    HTML += "<html>";
    HTML += "   <head>";
    HTML += "       <title>ESP Input Form</title>";
    HTML += "       <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    HTML += "   </head>";
    HTML += "   <body>";
    HTML += "       <h1><center> Fridge system </center></h1>";
    HTML += "<a href=\"http://" + WiFi.localIP().toString() + "/checkDef\"><center>Check defrost active state</center></a>";
    HTML += "<a href=\"http://" + WiFi.localIP().toString() + "/checkFreeze\"><center>Check Freezing active state</center></a>";
    HTML += "       <a href=\"http://";
    HTML += WiFi.localIP().toString();
    HTML += "/sensors\"><center>Check Sensors values</center></a>";

    HTML += "       <p><center> To set a defrost timer is with: http://";
    HTML += WiFi.localIP().toString();
    HTML += "/timersDef?startHour=1&startMinute=2&stopHour=3&stopMinute=4&indexToSet=5</center></p>";

    HTML += "       <p><center> To check a defrost timer is with: http://";
    HTML += WiFi.localIP().toString();
    HTML += "/getDef?index=0</center></p>";

    HTML += "       <p><center> To set the fridge temperature is with: http://";
    HTML += WiFi.localIP().toString();
    HTML += "/setFrTemp?temp=0</center></p>";

    HTML += "       <p><center> To set the fridge temperature threshold is with: http://";
    HTML += WiFi.localIP().toString();
    HTML += "/setTrhTemp?temp=1</center></p>";

    HTML += "   </body>";
    HTML += "</html>";

    Serial.println("entered the login page");
    _server.send(200, "text/html", HTML);
}

WifiServerManager::WifiServerManager(char *wifiName, char *wifiPassword)
{
    _wifiName = wifiName;
    _wifiPassword = wifiPassword;
}

void WifiServerManager::UpdateServerCLient()
{
    _server.handleClient();
}