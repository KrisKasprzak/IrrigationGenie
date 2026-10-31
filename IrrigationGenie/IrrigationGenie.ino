/*

  IrrigationGenie - intelligent home irrigation controller
  version 1.0

  ESP32 + 2.8" ILI9341 TFT with XPT2046 touch, 7 zones + master valve,
  manual / day-of-week schedule / weather intelligent watering, phone web app

  Board settings (Arduino IDE, esp32 core 3.x)
    Board:            ESP32 Dev Module
    Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)   <- LittleFS holds the map image
    PSRAM:            Disabled

    NOTE: the sketch is about 1.35 MB. With the default partition scheme (1.25 MB app)
    the build fails with "Sketch too big / text section exceeds available space in board".
    Fix: Tools > Partition Scheme > Huge APP (3MB No OTA/1MB SPIFFS).
    A partitions.csv in the sketch folder does not help, the IDE still checks the menu size.

  pinouts

  ESP32     display
  3v3         VCC
  GND         GND
  5           TFT_CS
  15          TFT_RESET
  2           TFT_DC
  23          MOSI (SDI)
  18          SCK
  19          MISO (SDO)   (if touch is flaky, leave TFT SDO disconnected, the display does not need it)
  21          LED          backlight PWM through a 2N3906 PNP high side switch:
                           GPIO 21 -> 1k -> base, emitter -> 3.3V, collector -> TFT LED pin
                           pin LOW = backlight on (BL_ACTIVE_LOW true in Config.h)
  18          T_CLK
  14          T_CS
  23          T_DIN
  19          T_DO
  36          T_IRQ

  ESP32     outputs (1k to a 2N2222A per relay, pin HIGH = relay on, relays switch 24VAC to the valves)
  32          master valve
  33          zone 1
  25          zone 2
  26          zone 3
  27          zone 4
  4           zone 5
  16          zone 6       (WROVER modules use 16/17 for PSRAM, move zones 6/7 to 13 and 12)
  17          zone 7
  22          passive piezo buzzer (+ via 100 ohm, - to GND)

  spare: 13 (output), 34, 35, 39 (input only, e.g. future flow meter / rain sensor)

  phone: browse to http://irrigationgenie.local or the IP address shown on the info screen

*/

#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <time.h>
#include <ArduinoJson.h>
#include <XPT2046_Touchscreen.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include "fonts\FreeSans18pt7b.h"
#include "fonts\FreeSans12pt7b.h"
#include "fonts\FreeSans9pt7b.h"

#include "Config.h"
#include "Plan.h"
#include "WebPage.h"

#define FONT_BIG FreeSans18pt7b
#define FONT_MED FreeSans12pt7b
#define FONT_SMALL FreeSans9pt7b

Adafruit_ILI9341 Display = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen Touch(T_CS, T_IRQ);
GFXcanvas16 StatusBar(SCREEN_W, STATUS_H);
WebServer server(80);

Settings cfg;
Weather wx;
SmartResult smart;
ZoneRun run[NUM_ZONES];
Program prog;
UsageDay usage[USAGE_DAYS];

bool MasterOn = false;       // actual master output
bool MasterManual = false;   // user opened master by hand
uint32_t MasterManualMs = 0;
uint32_t DrainStartMs = 0;
bool Draining = false;
bool OutputState[NUM_OUTPUTS];

bool TimeValid = false;
bool WifiOK = false;
bool InternetOK = true;
bool APMode = false;
bool UsageDirty = false;
bool ConfigDirty = false;
uint32_t ConfigDirtyMs = 0;
uint32_t ConfigStamp = 1;  // bumps on every config change, phone re-reads config
uint32_t MapStamp = 1;
uint32_t WxStamp = 0;
uint32_t LastTriggerKey = 0;
int LastDay = -1;
int LastMinute = -1;
uint32_t WifiLostMs = 0;
uint32_t BootMs = 0;

// net task handshake
volatile bool NetWantWeather = false;
volatile bool NetWantIPLocate = false;
volatile bool NetWeatherReady = false;
volatile bool NetLocateReady = false;
volatile bool NetBusy = false;
Weather NetWx;
float NetLat, NetLon;
int NetFailCount = 0;
struct IPLocation {
  bool ok;
  float lat, lon;
  char place[48];
  char tzName[40];
  int32_t offset;
} NetLoc;

uint32_t SecondTimer = 0;
uint32_t WeatherTimer = 0;
uint32_t LastTouchMs = 0;

void setup() {

  // valves off before anything else
  SetupOutputs();

  Serial.begin(115200);
  BootMs = millis();

  SetupSound();
  SetupBacklight();

  Display.begin(40000000);
  Display.setRotation(3);
  Display.fillScreen(C_BG);
  Touch.begin();
  Touch.setRotation(3);

  if (!LittleFS.begin(true)) {
    Serial.println(F("LittleFS mount failed"));
  }

  LoadConfig();
  LoadUsage();
  SetBacklight(cfg.brightness);

  DrawSplash();
  SplashProgress(10, "Starting...");
  PlaySound(SND_STARTUP);

  // touch calibration at start: always on first boot, or hold screen during splash
  SplashProgress(25, "Hold screen to calibrate");
  // must be held steadily for 1 s, a glitchy touch reading will not trigger it
  uint32_t t = millis();
  int heldCount = 0;
  while (millis() - t < 2000) {
    ServiceSound();
    heldCount = Touch.touched() ? heldCount + 1 : 0;
    if (heldCount >= 100) break;
    delay(10);
  }
  bool held = heldCount >= 100;
  StopSound();
  if (!cfg.calOK || held) {
    RunCalibration();
    DrawSplash();
  }

  if (cfg.hasLocation && strlen(cfg.posix) > 0) {
    ApplyTimeZone();
  }

  SplashProgress(45, "Connecting to WiFi");
  bool connected = false;
  if (strlen(cfg.ssid) > 0) {
    connected = ConnectWiFi(15000);
  } else {
    WiFi.mode(WIFI_STA);  // brings up the network stack, the web server needs it
  }
  if (!connected) {
    StartAP();  // before the web server so the phone can do the setup
  }

  StartWebServer();
  StartNetTask();

  if (connected) {
    SplashProgress(70, "Getting internet time");
    StartTime();
    if (!cfg.hasLocation) {
      // bootstrap with an approximate location, the phone can set the exact one
      SplashProgress(80, "Finding location");
      NetWantIPLocate = true;
    }
    SplashProgress(90, "Getting weather");
    RequestWeather();
    SplashProgress(100, WiFi.localIP().toString().c_str());
    delay(600);
    GoScreen(SCR_HOME);
  } else {
    GoScreen(SCR_WIFI);
  }

  LastTouchMs = millis();
}

void loop() {

  server.handleClient();
  ServiceSound();
  ServiceTouch();
  ServiceValves();
  ServiceNetResults();

  if (millis() - SecondTimer >= 1000) {
    SecondTimer = millis();
    EverySecond();
  }

  ServiceScreenSaver();

  // save config a little after the last change to limit flash writes
  if (ConfigDirty && millis() - ConfigDirtyMs > 3000) {
    ConfigDirty = false;
    SaveConfig();
  }
}

void EverySecond() {

  CheckWiFi();
  TimeValid = time(nullptr) > 1700000000;

  AccumulateUsage();

  if (TimeValid) {
    struct tm lt;
    time_t now = time(nullptr);
    localtime_r(&now, &lt);

    if (lt.tm_min != LastMinute) {
      LastMinute = lt.tm_min;
      CheckSchedule(lt);

      // new day at midnight: resync time, roll usage, refresh weather
      if (LastDay != -1 && lt.tm_mday != LastDay) {
        StartTime();
        RequestWeather();
        SaveUsage();
      }
      LastDay = lt.tm_mday;

      // hourly usage save if anything changed
      if (lt.tm_min == 0 && UsageDirty) {
        SaveUsage();
      }
    }
  }

  if (millis() - WeatherTimer > (uint32_t)WEATHER_UPDATE_MIN * 60000UL) {
    RequestWeather();
  }

  UpdateStatusBar();
  UpdateScreen();
}

void MarkConfigDirty() {
  ConfigDirty = true;
  ConfigDirtyMs = millis();
  ConfigStamp++;
}

// end of code
