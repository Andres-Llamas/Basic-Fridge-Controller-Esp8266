#include <Arduino.h>
#include "WifiServerManager.h"

static ESP8266WebServer _server(80);
static ESP8266HTTPUpdateServer _updater; 
static const char* WIFI_FILE = "/wifi.json";

WifiServerManager::WifiServerManager(const char *wifiName, const char *wifiPassword)
: _wifiName(wifiName), _wifiPassword(wifiPassword) {}

bool WifiServerManager::_loadCreds(String &ssid, String &pass) {
  if (!LittleFS.exists(WIFI_FILE)) return false;
  File f = LittleFS.open(WIFI_FILE, "r");
  if (!f) return false;
  String s = f.readString();
  f.close();
  int a = s.indexOf("\"ssid\":\""); if (a < 0) return false;
  a += 8; int b = s.indexOf("\"", a); if (b < 0) return false;
  ssid = s.substring(a, b);
  a = s.indexOf("\"pass\":\""); if (a < 0) return false;
  a += 8; b = s.indexOf("\"", a); if (b < 0) return false;
  pass = s.substring(a, b);
  return ssid.length() > 0;
}

bool WifiServerManager::_saveCreds(const String &ssid, const String &pass) {
  File f = LittleFS.open(WIFI_FILE, "w");
  if (!f) return false;
  String json = String("{\"ssid\":\"") + ssid + "\",\"pass\":\"" + pass + "\"}";
  f.print(json);
  f.close();
  return true;
}

void WifiServerManager::_startAPPortal() {
  Serial.println("Starting SoftAP portal...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Refri-Setup", "12345678"); // change password if desired
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("AP IP: http://%s/\n", ip.toString().c_str());
}

void WifiServerManager::_connectWithStored() {
  String ssid, pass;
  if (_loadCreds(ssid, pass)) {
    Serial.printf("Stored SSID: %s\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
      delay(300); Serial.print(".");
    }
    Serial.println();
  } else if (_wifiName && *_wifiName) { // optional fallback to compile-time creds
    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifiName, _wifiPassword);
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
      delay(300); Serial.print(".");
    }
    Serial.println();
  } else {
    Serial.println("No stored WiFi creds.");
  }
}

String WifiServerManager::_contentTypeFor(const String& filename) {
  if (filename.endsWith(".html") || filename.endsWith(".htm")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".json")) return "application/json";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  if (filename.endsWith(".ico")) return "image/x-icon";
  if (filename.endsWith(".gz")) return "application/octet-stream";
  return "text/plain";
}

bool WifiServerManager::_handleFileRead(const String& reqPath) {
  String path = reqPath;
  if (path.endsWith("/")) path += "index.html";
  String pathGz = path + ".gz";
  if (LittleFS.exists(pathGz)) {
    File file = LittleFS.open(pathGz, "r");
    _server.streamFile(file, _contentTypeFor(path));
    file.close(); return true;
  }
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    _server.streamFile(file, _contentTypeFor(path));
    file.close(); return true;
  }
  return false;
}

void WifiServerManager::Initialize() {
  if (!LittleFS.begin()) Serial.println("LittleFS mount FAILED"); else Serial.println("LittleFS OK");

  _connectWithStored();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi STA connect failed; falling back to AP portal.");
    _startAPPortal();
  } else {
    Serial.printf("WiFi connected: %s\n", WiFi.localIP().toString().c_str());
  }

  if (MDNS.begin("refri")) Serial.println("MDNS: http://refri.local"); else Serial.println("MDNS failed");

  // -------- Core endpoints --------
  _server.on("/sensors", HTTP_GET, [](){
    String json = "{";
    json += "\"temperature\":" + String(sensors::currentTemperature, 2) + ",";
    json += "\"freezeActive\":" + String(Behaviours::isFreezingActive ? "true":"false") + ",";
    json += "\"defrostActive\":" + String(Behaviours::isDefrostActive ? "true":"false") + ",";
    json += "\"time\":\"" + String(TimeManager::currentTime.day) + " " + String(TimeManager::currentTime.hours) + ":" + String(TimeManager::currentTime.minutes) + ":" + String(TimeManager::currentTime.seconds) + "\"";
    json += "}";
    _server.send(200, "application/json", json);
  });

  _server.on("/getConfig", HTTP_GET, [](){
    String json = "{";
    json += "\"setpoint\":" + String(sensors::tempToSetFridge, 2) + ",";
    json += "\"threshold\":" + String(sensors::tempToSetFridgeThreshold, 2) + ",";
    json += "\"timers\":[";
    for (int i=0;i<10;i++) {
      clockTime a = defrostTimersToActivate[i];
      clockTime b = defrostTimersToDeactivate[i];
      json += "{\"start\":\"" + String(a.hours) + ":" + String(a.minutes) + "\",\"stop\":\"" + String(b.hours) + ":" + String(b.minutes) + "\"}";
      if (i<9) json += ",";
    }
    json += "]}";
    _server.send(200, "application/json", json);
  });

  _server.on("/setFrTemp", HTTP_GET, [](){
    if (!_server.hasArg("temp")) { _server.send(400, "text/plain", "missing 'temp'"); return; }
    float t = _server.arg("temp").toFloat();
    if (t < -5.0f || t > 10.0f) { _server.send(400, "text/plain", "temp out of range (-5..10 C)"); return; }
    sensors::tempToSetFridge = t;
    _server.send(200, "text/plain", "ok");
  });

  _server.on("/setTrhTemp", HTTP_GET, [](){
    if (!_server.hasArg("temp")) { _server.send(400, "text/plain", "missing 'temp'"); return; }
    float th = _server.arg("temp").toFloat();
    if (th < 0.2f || th > 5.0f) { _server.send(400, "text/plain", "threshold out of range (0.2..5 C)"); return; }
    sensors::tempToSetFridgeThreshold = th;
    _server.send(200, "text/plain", "ok");
  });

  _server.on("/checkFreeze", HTTP_GET, [](){
    Behaviours::SetFreezingState(!Behaviours::isFreezingActive);
    _server.send(200, "text/plain", Behaviours::isFreezingActive ? "ON":"OFF");
  });

  _server.on("/checkDefrost", HTTP_GET, [](){
    Behaviours::SetDefrostState(!Behaviours::isDefrostActive);
    _server.send(200, "text/plain", Behaviours::isDefrostActive ? "ON":"OFF");
  });

  _server.on("/timersDef", HTTP_GET, [](){
    if (!_server.hasArg("indexToSet") || !_server.hasArg("startHour") || !_server.hasArg("startMinute") ||
        !_server.hasArg("stopHour") || !_server.hasArg("stopMinute")) {
      _server.send(400, "text/plain", "missing params");
      return;
    }
    int idx = _server.arg("indexToSet").toInt();
    if (idx < 0 || idx > 9) { _server.send(400, "text/plain", "indexToSet must be 0..9"); return; }
    int sh = _server.arg("startHour").toInt();
    int sm = _server.arg("startMinute").toInt();
    int eh = _server.arg("stopHour").toInt();
    int em = _server.arg("stopMinute").toInt();
    if (sh<0||sh>23||eh<0||eh>23||sm<0||sm>59||em<0||em>59) {
      _server.send(400, "text/plain", "invalid hour/minute"); return;
    }
    clockTime a{TimeManager::currentTime.day, sh, sm, 0};
    clockTime b{TimeManager::currentTime.day, eh, em, 0};
    Behaviours::AddDefrostTimer(a,b,idx);
    _server.send(200, "text/plain", "ok");
  });

  // DIY Wi-Fi
  _server.on("/wifiGet", HTTP_GET, [](){
    String ssid="", pass="";
    if (LittleFS.exists(WIFI_FILE)) {
      File f = LittleFS.open(WIFI_FILE, "r");
      String s = f.readString(); f.close();
      int a = s.indexOf("\"ssid\":\""); if (a >= 0) { a += 8; int b = s.indexOf("\"", a); if (b > a) ssid = s.substring(a,b); }
    }
    String json = String("{\"ssid\":\"") + ssid + "\",\"pass\":\"********\"}";
    _server.send(200, "application/json", json);
  });

  _server.on("/wifiSet", HTTP_ANY, [](){
    if (!_server.hasArg("ssid") || !_server.hasArg("pass")) { _server.send(400, "text/plain", "missing ssid/pass"); return; }
    String ssid = _server.arg("ssid");
    String pass = _server.arg("pass");
    if (ssid.length() < 1 || ssid.length() > 32 || pass.length() > 63) { _server.send(400, "text/plain", "invalid lengths"); return; }
    File f = LittleFS.open(WIFI_FILE, "w");
    if (!f) { _server.send(500, "text/plain", "save failed"); return; }
    String json = String("{\"ssid\":\"") + ssid + "\",\"pass\":\"" + pass + "\"}";
    f.print(json); f.close();
    _server.send(200, "text/plain", "saved, rebooting...");
    delay(500);
    ESP.restart();
  });

  _server.on("/wifiScan", HTTP_GET, [](){
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i=0;i<n;i++) {
      json += String("{\"ssid\":\"") + WiFi.SSID(i) + "\",\"rssi\":" + WiFi.RSSI(i) + ",\"enc\":" + WiFi.encryptionType(i) + "}";
      if (i<n-1) json += ",";
    }
    json += "]";
    _server.send(200, "application/json", json);
  });

  _server.on("/wifiReset", HTTP_GET, [](){
    LittleFS.remove(WIFI_FILE);
    _server.send(200, "text/plain", "wifi creds cleared; rebooting...");
    delay(500);
    ESP.restart();
  });

  // Static files
  _server.onNotFound([this](){
    if (!_handleFileRead(_server.uri())) _server.send(404, "text/plain", "Not Found");
  });

  _updater.setup(&_server, "/update", "andreps", "MeLaPelasPrro"); //OTA
  _server.begin();
  Serial.println("HTTP server ready.");
}

void WifiServerManager::UpdateServerCLient() {
  _server.handleClient();
  MDNS.update();
}
