#include <Arduino.h>
#include "WifiServerManager.h"

static ESP8266WebServer _server(80);
static ESP8266HTTPUpdateServer _updater;

// --- Basic Auth for critical settings (password comes from config.json) ---
static const char *ADMIN_USER = "admin";
static String g_uiPassword = "1234"; // will be overwritten by config.json

static bool requireAdmin()
{
    if (!_server.authenticate(ADMIN_USER, g_uiPassword.c_str()))
    {
        _server.requestAuthentication(); // browser popup
        return false;
    }
    return true;
}

// --------------------------------------------------------
// Constructor
// --------------------------------------------------------
WifiServerManager::WifiServerManager(const char *apName, const char *apPassword)
    : _apNameDefault(apName), _apPassDefault(apPassword)
{
    _cfg.wifiSsid = "";
    _cfg.wifiPassword = "";
    _cfg.apPassword = "";
    _cfg.uiPassword = "1234";
}

// --------------------------------------------------------
// Helpers: config load/save
// --------------------------------------------------------
bool WifiServerManager::_loadConfig()
{
    if (!LittleFS.exists(WIFI_FILE))
    {
        Serial.println("Config file not found, using defaults.");
        return false;
    }

    File f = LittleFS.open(WIFI_FILE, "r");
    if (!f)
    {
        Serial.println("Failed to open wifi file.");
        return false;
    }
    String s = f.readString();
    f.close();

    _cfg.wifiSsid = getJsonString(s, "wifi_ssid");
    _cfg.wifiPassword = getJsonString(s, "wifi_password");
    _cfg.apPassword = getJsonString(s, "ap_password");
    _cfg.uiPassword = getJsonString(s, "ui_password");

    if (_cfg.uiPassword.length() == 0)
        _cfg.uiPassword = "1234";

    Serial.println("Config loaded:");
    Serial.print("  wifi_ssid     = ");
    Serial.println(_cfg.wifiSsid);
    Serial.print("  wifi_password = ");
    Serial.println(_cfg.wifiPassword.length() ? "******" : "");
    Serial.print("  ap_password   = ");
    Serial.println(_cfg.apPassword);
    Serial.print("  ui_password   = ");
    Serial.println(_cfg.uiPassword.length() ? "******" : "");
    return true;
}

bool WifiServerManager::_saveConfig()
{
    String json = "{";
    json += "\"wifi_ssid\":\"" + _cfg.wifiSsid + "\",";
    json += "\"wifi_password\":\"" + _cfg.wifiPassword + "\",";
    json += "\"ap_password\":\"" + _cfg.apPassword + "\",";
    json += "\"ui_password\":\"" + _cfg.uiPassword + "\"";
    json += "}";

    File f = LittleFS.open(WIFI_FILE, "w");
    if (!f)
    {
        Serial.println("Failed to open wifi for writing.");
        return false;
    }
    size_t written = f.print(json);
    f.close();
    Serial.println("Config saved.");
    return (written == json.length());
}

// --------------------------------------------------------
// WiFi connection / AP
// --------------------------------------------------------
void WifiServerManager::_connectWithStored()
{
    if (_cfg.wifiSsid.length() == 0)
    {
        Serial.println("No stored STA credentials, starting AP.");
        _startAPPortal();
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(_cfg.wifiSsid.c_str(), _cfg.wifiPassword.c_str());

    Serial.print("Connecting to WiFi SSID '");
    Serial.print(_cfg.wifiSsid);
    Serial.println("' ...");

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000UL)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("WiFi connected, IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("WiFi connection failed, starting AP.");
        _startAPPortal();
    }
}

void WifiServerManager::_startAPPortal()
{
    String apSsid = _apNameDefault ? String(_apNameDefault) : String("FridgeController");
    String apPass;

    if (_cfg.apPassword.length())
    {
        apPass = _cfg.apPassword;
    }
    else if (_apPassDefault && strlen(_apPassDefault) > 0)
    {
        apPass = _apPassDefault;
    }
    else
    {
        apPass = ""; // open AP (not recommended)
    }

    WiFi.mode(WIFI_AP);
    bool ok = WiFi.softAP(apSsid.c_str(), apPass.length() ? apPass.c_str() : nullptr);

    Serial.print("AP '");
    Serial.print(apSsid);
    Serial.print("' ");
    Serial.println(ok ? "started." : "failed!");

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

// --------------------------------------------------------
// HTTP static file helpers
// --------------------------------------------------------
String WifiServerManager::_contentTypeFor(const String &filename)
{
    if (filename.endsWith(".htm") || filename.endsWith(".html"))
        return "text/html";
    if (filename.endsWith(".css"))
        return "text/css";
    if (filename.endsWith(".js"))
        return "application/javascript";
    if (filename.endsWith(".json"))
        return "application/json";
    if (filename.endsWith(".png"))
        return "image/png";
    if (filename.endsWith(".gif"))
        return "image/gif";
    if (filename.endsWith(".jpg"))
        return "image/jpeg";
    if (filename.endsWith(".ico"))
        return "image/x-icon";
    if (filename.endsWith(".svg"))
        return "image/svg+xml";
    return "text/plain";
}

bool WifiServerManager::_handleFileRead(const String &path)
{
    String p = path;
    if (p.endsWith("/"))
        p += "index.html";
    if (!LittleFS.exists(p))
        return false;
    File f = LittleFS.open(p, "r");
    if (!f)
        return false;
    String contentType = _contentTypeFor(p);
    _server.streamFile(f, contentType);
    f.close();
    return true;
}

// --------------------------------------------------------
// HTTP routes
// --------------------------------------------------------
void WifiServerManager::_setupHttpRoutes()
{
    // Root -> index.html
    _server.on("/", HTTP_GET, [this]()
               {
    if (!_handleFileRead("/index.html")) {
      _server.send(500, "text/plain", "index.html not found");
    } });

    // --- Sensors / status (read-only, no auth) ---
    _server.on("/sensors", HTTP_GET, []()
               {
    String json = "{";
    json += "\"currentTemp\":" + String(sensors::GetCurrentTemp(), 2) + ",";
    json += "\"setTemp\":"     + String(sensors::GetTempToSetFridge(), 2) + ",";
    json += "\"threshold\":"   + String(sensors::GetTempThreshold(), 2) + ",";
    json += "\"freezing\":"    + String(Behaviours::isFreezingActive ? 1 : 0) + ",";
    json += "\"defrost\":"     + String(Behaviours::isDefrostActive ? 1 : 0);
    json += "}";
    _server.send(200, "application/json", json); });

    // --- Critical settings: require admin login ---
    _server.on("/setFrTemp", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    if (!_server.hasArg("temp")) { _server.send(400, "text/plain", "missing 'temp'"); return; }
    float t = _server.arg("temp").toFloat();
    if (t < -5.0f || t > 15.0f) {
      _server.send(400, "text/plain", "temp out of range (-5..15 C)");
      return;
    }
    sensors::SaveNewFridgeTempToSet(t);
    _server.send(200, "text/plain", "ok"); });

    _server.on("/setTrhTemp", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    if (!_server.hasArg("temp")) { _server.send(400, "text/plain", "missing 'temp'"); return; }
    float th = _server.arg("temp").toFloat();
    if (th < 0.2f || th > 5.0f) {
      _server.send(400, "text/plain", "threshold out of range (0.2..5 C)");
      return;
    }
    sensors::SaveNewFridgeThreshold(th);
    _server.send(200, "text/plain", "ok"); });

    _server.on("/freezeOn", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    Behaviours::SetFreezingState(true);
    _server.send(200, "text/plain", "freeze on"); });

    _server.on("/freezeOff", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    Behaviours::SetFreezingState(false);
    _server.send(200, "text/plain", "freeze off"); });

    _server.on("/defrostOn", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    Behaviours::SetDefrostState(true);
    _server.send(200, "text/plain", "defrost on"); });

    _server.on("/defrostOff", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    Behaviours::SetDefrostState(false);
    _server.send(200, "text/plain", "defrost off"); });

    // Example: set a defrost timer
    _server.on("/timersDef", HTTP_GET, []()
               {
    if (!requireAdmin()) return;

    if (!_server.hasArg("index") ||
        !_server.hasArg("startHour") || !_server.hasArg("startMinute") ||
        !_server.hasArg("stopHour")  || !_server.hasArg("stopMinute")) {
      _server.send(400, "text/plain", "missing params");
      return;
    }

    int idx = _server.arg("index").toInt();
    if (idx < 0 || idx >= 10) {
      _server.send(400, "text/plain", "index out of range (0..9)");
      return;
    }

    clockTime startT { "ANY",
                       _server.arg("startHour").toInt(),
                       _server.arg("startMinute").toInt(),
                       0 };

    clockTime stopT  { "ANY",
                       _server.arg("stopHour").toInt(),
                       _server.arg("stopMinute").toInt(),
                       0 };

    Behaviours::AddDefrostTimer(startT, stopT, idx);
    _server.send(200, "text/plain", "ok"); });

    _server.on("/timersGet", HTTP_GET, []()
               {
      if (!requireAdmin()) return;
  if (!_server.hasArg("index")) { _server.send(400, "text/plain", "missing index"); return; }
  int idx = _server.arg("index").toInt();
  if (idx < 0 || idx >= 10) { _server.send(400, "text/plain", "index out of range (0..9)"); return; }

  clockTime start, stop;
  Behaviours::GetDefrostTimerPair(idx, start, stop);

  String json = "{";
  json += "\"startHours\":"   + String(start.hours)   + ",";
  json += "\"startMinutes\":" + String(start.minutes) + ",";
  json += "\"startSeconds\":" + String(start.seconds) + ",";
  json += "\"stopHours\":"    + String(stop.hours)    + ",";
  json += "\"stopMinutes\":"  + String(stop.minutes)  + ",";
  json += "\"stopSeconds\":"  + String(stop.seconds);
  json += "}";
  _server.send(200, "application/json", json); });

    // --- WiFi: get current config (masked password) ---
    _server.on("/wifiGet", HTTP_GET, [this]()
               {
    String json = "{";
    json += "\"ssid\":\"" + _cfg.wifiSsid + "\",";
    json += "\"pass\":\"********\"";
    json += "}";
    _server.send(200, "application/json", json); });

    // --- WiFi: scan ---
    _server.on("/wifiScan", HTTP_GET, []()
               {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"open\":" + String(WiFi.encryptionType(i) == ENC_TYPE_NONE ? 1 : 0);
      json += "}";
    }
    json += "]";
    _server.send(200, "application/json", json); });

    // --- WiFi: set new STA credentials (requires admin) ---
    _server.on("/wifiSet", HTTP_ANY, [this]()
               {
    if (!requireAdmin()) return;
    if (!_server.hasArg("ssid") || !_server.hasArg("pass")) {
      _server.send(400, "text/plain", "missing ssid/pass");
      return;
    }
    _cfg.wifiSsid     = _server.arg("ssid");
    _cfg.wifiPassword = _server.arg("pass");

    // Optionally allow updating AP/UI passwords via extra args
    if (_server.hasArg("apPass"))  _cfg.apPassword  = _server.arg("apPass");
    if (_server.hasArg("uiPass"))  _cfg.uiPassword  = _server.arg("uiPass");

    if (_cfg.uiPassword.length() == 0) _cfg.uiPassword = "1234";
    g_uiPassword = _cfg.uiPassword;

    _saveConfig();
    _server.send(200, "text/plain", "saved; rebooting...");

    delay(500);
    ESP.restart(); });

    // --- WiFi: clear creds and reboot (requires admin) ---
    _server.on("/wifiReset", HTTP_GET, []()
               {
    if (!requireAdmin()) return;
    if (LittleFS.exists(WIFI_FILE)) {
      LittleFS.remove(WIFI_FILE);
    }
    _server.send(200, "text/plain", "wifi config cleared; rebooting...");
    delay(500);
    ESP.restart(); });

    _server.on("/ip", HTTP_GET, []()
               {
  IPAddress ip = WiFi.localIP();
  String json = "{";
  json += "\"ip\":\"" + ip.toString() + "\"";
  json += "}";
  _server.send(200, "application/json", json); });

    _server.on("/logs/temp.csv", HTTP_GET, []()
               {
    String csv;
    if (!DataLogger::GetTempLogCSV(csv)) {
      _server.send(500, "text/plain", "cannot read temp log");
      return;
    }

    _server.on("/logs/freeze.csv", HTTP_GET, []() {
    String csv;
    if (!DataLogger::GetFreezeLogCSV(csv)) {
      _server.send(500, "text/plain", "cannot read freeze log");
      return;
    }
    _server.sendHeader("Content-Type", "text/csv");
    _server.sendHeader("Content-Disposition", "attachment; filename=\"freeze_log.csv\"");
    _server.send(200, "text/csv", csv);
  });
  
    _server.sendHeader("Content-Type", "text/csv");
    _server.sendHeader("Content-Disposition", "attachment; filename=\"temp_log.csv\"");
    _server.send(200, "text/csv", csv); });

    // Static files
    _server.onNotFound([this]()
                       {
    if (!_handleFileRead(_server.uri())) {
      _server.send(404, "text/plain", "Not Found");
    } });
}

// --------------------------------------------------------
// Public API
// --------------------------------------------------------
void WifiServerManager::Initialize()
{
    Serial.println("Initializing WiFi / HTTP / LittleFS...");

    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed! Running without filesystem.");
    }

    // Load config (if present)
    _loadConfig();

    // Fallbacks
    if (_cfg.apPassword.length() == 0 && _apPassDefault && strlen(_apPassDefault) > 0)
    {
        _cfg.apPassword = _apPassDefault;
    }
    if (_cfg.uiPassword.length() == 0)
    {
        _cfg.uiPassword = "1234";
    }
    g_uiPassword = _cfg.uiPassword;

    // Connect or start AP
    _connectWithStored();

    // mDNS
    if (MDNS.begin("fridge"))
    {
        Serial.println("mDNS responder started: http://fridge.local");
    }
    else
    {
        Serial.println("Error setting up mDNS responder!");
    }

    _setupHttpRoutes();

    // OTA protected with same credentials
    _updater.setup(&_server, "/update", ADMIN_USER, g_uiPassword.c_str());

    _server.begin();
    Serial.println("HTTP server ready.");
}

void WifiServerManager::UpdateServerCLient()
{
    _server.handleClient();
    MDNS.update();
}
