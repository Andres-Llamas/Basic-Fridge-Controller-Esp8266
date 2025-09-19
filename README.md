# Basic-Fridge-Controller-Esp8266
My fridge controller card died... so I made one with mini NodeMCU Esp8266.
My fridge is an old General Electric Model TBS17YA from 2008
It replaces the stock board and adds a Wi-Fi dashboard, configurable setpoints, safe compressor logic, and OTA updates — all without extra storage hardware.

**⚠️ Warning**
This project is for my personal use only due to not finding an affordable board replacement.
This project is only meant to my specific use case, for my specific fridge.
I'll not keep support for this repository except for my own needs. 
In case anyone besides me is watching this repo and reading this, it's your responsibility if you plan to implement this project 
on your own.

## ✨ Features

- **Cooling & defrost control** with hysteresis and timer scheduling
- **Anti short-cycle protection** (5 min min-off, 90 s min-on, 10 s boot delay)
- **Non-blocking defrost arm** (UI stays responsive)
- **Web dashboard (LittleFS)**:
  - Live temperature, compressor & defrost state
  - Setpoint & threshold adjustment
  - Add/remove up to 10 daily defrost timers
  - Wi-Fi configuration panel (set/scan/reset)
  - **Firmware update button** (OTA via `/update`)
- **Wi-Fi config persistence** in LittleFS (`wifi.json`)
- **mDNS**: device accessible at `http://refri.local/`

---

## 📂 Project structure

```
/src
  main.cpp
  Behaviours.cpp
  WifiServerManager.cpp
  TimeManager.cpp
  sensors.cpp
/include
  Behaviours.h
  WifiServerManager.h
  TimeManager.h
  sensors.h
  CustomStructs.h
/data
  index.html
  style.css
  app.js
  (gzipped versions for faster serving)
```

---

## 🛠️ Build & Flash

This project uses **PlatformIO** with the Arduino framework.


1. Install PlatformIO (VSCode extension or CLI).
2. Clone this repo.
3. Ensure your `platformio.ini` has:
   ```ini
   board = your board
   framework = arduino
   board_build.filesystem = littlefs

ESP8266 Web UI (LittleFS) — Quick Steps (PlatformIO)

1) Enable LittleFS in `platformio.ini` (if not already):
   [env:d1_mini]
   platform = espressif8266
   board = d1_mini
   framework = arduino
   board_build.filesystem = littlefs

2) Put the three asset files into your project `data/` folder:
   - data/index.html
   - data/style.css
   - data/app.js   
   Tip: We also included `.gz` versions. ESP8266 will serve them transparently.

3) Replace WifiServerManager.h / .cpp with whatever modifications you want (add, remove, modify endpoints and behaviors)
   (or merge the LittleFS/serving bits into your versions).

4) Build & upload the sketch as usual, then upload the filesystem image:
   - VSCode PlatformIO:
     - "Upload Filesystem Image" (Project Tasks → env → Platform → Upload Filesystem Image)
   - Or CLI:
     pio run -t uploadfs

5) Open the UI:
   - http://refri.local  (if mDNS works)
   - or http://<device-ip>

Notes:
- The UI calls your existing endpoints: /sensors, /getConfig, /setFrTemp, /setTrhTemp, /checkFreeze, /checkDefrost, /timersDef
- Static files are served from LittleFS; if a .gz compressed version exists, it's served automatically.
- If you prefer not to use LittleFS, you can embed the files into PROGMEM and serve from flash — but LittleFS is simpler and more flexible.

---

## 🔌Hardware & wiring 
⚠️ **Warning**: Mains voltage is dangerous. Use relays/modules rated for your fridge’s compressor and defrost heater load. Always isolate and insulate connections.
Wiring based on the 200d5940g014 board

### Components
- NodeMCU ESP8266 board (mini or v2/v3)
- 2× relay modules (or solid-state relays):
   - **Freezing relay** → compressor
   - **Defrost relay** → defrost heater
- NTC thermistor probe (10k typical) for box temperature
- Voltage divider for thermistor → ESP8266 ADC (max 1.0 V on NodeMCU pin A0)
- 5V power supply for NodeMCU + relay boards

### Pinout (default in code)
| Function       | Pin (ESP8266) | Notes |
|----------------|---------------|-------|
| Thermistor ADC | A0            | Divider scaled for 0–1.0 V |
| Freezing relay | D5 (GPIO14)   | Active HIGH/LOW depending on relay module |
| Defrost relay  | D7 (GPIO13)   | Active HIGH/LOW depending on relay module |
| Status LED     | Onboard LED   | Optional |

### Schematic (simplified)

```
 [Thermistor]
      |
     [Divider R1]---3.3V
      |
     [Divider R2]---GND
      |
      +--- A0 (ESP8266)

 D5 (GPIO14) ----[Relay 1]---- Compressor mains
 D7 (GPIO13) ----[Relay 2]---- Defrost heater mains
```

---

## 🌐 Web UI

- Navigate to `http://refri.local/` (or the ESP8266 IP).
- Dashboard sections:
  - **Status**: temperature, compressor, defrost, time
  - **Controls**: adjust setpoint & threshold, toggle states
  - **Timers**: define up to 10 defrost schedules
  - **Wi-Fi**: set/scan/reset network credentials
  - **Maintenance**: firmware OTA upload

---

## 🔄 OTA Firmware Updates

1. Build firmware:
   ```bash
   pio run
   ```
   The binary will be in `.pio/build/nodemcuv2/firmware.bin`.
2. Open the web UI → **Maintenance → Firmware Update**.
3. Upload the binary, enter credentials (set in `WifiServerManager.cpp`), wait for reboot.

> ⚠️ OTA updates replace **only the firmware**. If you change files in `/data`, re-upload filesystem with `pio run -t uploadfs`.

---

## ⚡ Safety

- Compressor short-cycle protection (min-on/off times).
- Boot delay before compressor activation.
- OTA/update recommended only when compressor is **OFF**.
- Strong password required for OTA page (`/update`).
- Insulate all mains wiring and relay contacts.
- Keep the ESP8266 + low-voltage side **physically isolated** from mains side.

---

## 📝 Stuff that maybe I'll add to the future
- Historical charts (temperature/time series)
- Authenticated dashboard
- Configurable compressor protection values
- ESP32 port (more memory, BLE)

## 📜 License

MIT — use freely, modify responsibly.