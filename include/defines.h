// Pins
#define freezingRelayPin D5
#define defrostRelayPin D7

// relays mode
#define RELAYS_MODE 1 // 0 for normally closed, 1 for normally open

// Sensors
#define NTC_PIN A0              // Pin,to which the voltage divider is connected
#define VD_POWER_PIN D2         // 5V for the voltage divider
#define NOMINAL_RESISTANCE 5000 // Nominal resistance at 25⁰C
#define NOMINAL_TEMPERATURE 25  // temperature for nominal resistance (almost always 25⁰ C)
#define SAMPLINGRATE 5          // Number of samples
#define BETA 3950               // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
#define RrEF 10000              // Value of  resistor used for the voltage divider

// File System
#define CONFIG_FILE "/config.json"
#define WIFI_FILE "/confidential.json"
#define TIMERS_FILE "/timers.txt"
#define TEMP_FILE "/temp_log.csv" // idx,dayIndex,hour,tempC, 0,0,8,3.42 | 1,0,9,3.10
#define FREEZE_FILE "/freeze_log.csv"

// Data logs
#define MAX_HOURLY_SAMPLES 336 // 24 * 14; 2 weeks
#define MAX_DAILY_SAMPLES 14

// Cycles
#define MAX_DEFROST_TIME 30 // in minutes
#define MIN_DEFROST_TIME 10 // in minutes