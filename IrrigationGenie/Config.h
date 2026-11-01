/*
  IrrigationGenie - Config.h
  pins, constants, colors, data structures shared by all tabs
*/

#pragma once
#include <Arduino.h>

#define CODE_VERSION "1.0"
#define DEVICE_NAME "IrrigationGenie"
#define MDNS_NAME "irrigationgenie"  // http://irrigationgenie.local
#define AP_SSID "IrrigationGenie-Setup"

//#define SHOW_DEBUG

// ---------------------------------------------------------------------------------
// pins  (standard VSPI pins for display and touch, see pinout in IrrigationGenie.ino)
// ---------------------------------------------------------------------------------
#define PIN_SCK 18
#define PIN_MOSI 23
#define PIN_MISO 19
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 15
#define T_CS 14
#define T_IRQ 36
#define PIN_LED 21     // TFT backlight (PWM) via 2N3906 PNP high side switch
// true:  transistor pulls the TFT LED pin to GND (LED pin fed from 3.3V by a resistor),
//        or a PNP high side switch -> pin HIGH = darker
// false: pin HIGH = brighter (LED pin driven directly or through a non-inverting stage)
#define BL_ACTIVE_LOW true
#define PIN_BUZZER 22  // passive piezo (PWM)

// touch click sound: pitch in Hz and length in ms (a few ms reads as a click, longer as a beep)
#define CLICK_HZ 500
#define CLICK_MS 1
#define CLICK_PWM 50  // duty 0..255, like analogWrite

#define NUM_ZONES 7
#define NUM_OUTPUTS 8  // master + 7 zones

// index 0 = master valve, 1..7 = zones. these pins do not glitch at boot
// zone 6 = GPIO 12, zone 7 = GPIO 13 (moved from 16 / 17, which are now free; 16 / 17 are PSRAM on WROVER)
// GPIO 12 is a boot strapping pin: it must not be pulled HIGH at power up. The 2N2222A driver
// (1k to the base) keeps it low; do not add a pull-up on it
const uint8_t OUTPUT_PINS[NUM_OUTPUTS] = {
  32,  // master valve
  33,  // zone 1
  25,  // zone 2
  26,  // zone 3
  27,  // zone 4
  4,   // zone 5
  12,  // zone 6 (moved from 16, strapping pin, keep low at boot)
  13   // zone 7 (moved from 17)
};

// false: each relay is driven by a 2N2222A (see docs/screenshots/Wiring.png), pin HIGH = relay on
// true:  relay modules with active low inputs, pin LOW = relay on
#define RELAY_ACTIVE_LOW false

// ---------------------------------------------------------------------------------
// screen
// ---------------------------------------------------------------------------------
#define SCREEN_W 320
#define SCREEN_H 240
#define STATUS_H 24
#define NAV_H 40
#define CONTENT_Y STATUS_H
#define CONTENT_H (SCREEN_H - STATUS_H - NAV_H)  // 176
#define NAV_Y (SCREEN_H - NAV_H)

// aerial map image stored in LittleFS (raw RGB565, little endian)
#define MAP_W 320
#define MAP_H 176
#define MAP_FILE "/map.rgb"
#define MAP_JPG "/map.jpg"
#define CONFIG_FILE "/config.json"
#define USAGE_FILE "/usage.bin"

#define MAX_PTS 12      // max polygon points per zone
#define USAGE_DAYS 30
#define WX_DAYS 7       // 3 past days + today + 3 forecast days
#define WX_TODAY 3      // index of today in the weather arrays

#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"

#define WEATHER_UPDATE_MIN 30
#define WIFI_LOST_WARN_SEC 30

// rgb888 -> rgb565 at compile time
#define RGB(r, g, b) ((uint16_t)((((r)&0xF8) << 8) | (((g)&0xFC) << 3) | ((b) >> 3)))

// theme
#define C_BG RGB(11, 20, 38)
#define C_CARD RGB(22, 35, 61)
#define C_CARD2 RGB(33, 51, 82)
#define C_LINE RGB(48, 70, 105)
#define C_ACCENT RGB(34, 211, 238)
#define C_ACCENT_DK RGB(14, 116, 144)
#define C_TEXT RGB(255, 255, 255)
#define C_SUB RGB(159, 179, 200)
#define C_MUTED RGB(100, 120, 145)
#define C_GOOD RGB(34, 197, 94)
#define C_WARN RGB(245, 158, 11)
#define C_BAD RGB(239, 68, 68)
#define C_ICE RGB(125, 211, 252)
#define C_SUN RGB(250, 204, 21)
#define C_CLOUD RGB(203, 213, 225)
#define C_BLACK 0x0000
#define C_WHITE 0xFFFF

// zone colors, same as the phone app (validated colorblind-safe order on dark)
const uint16_t ZONE_COLORS[NUM_ZONES] = {
  RGB(0x39, 0x87, 0xe5), RGB(0xd9, 0x59, 0x26), RGB(0x19, 0x9e, 0x70), RGB(0xc9, 0x85, 0x00),
  RGB(0xd5, 0x51, 0x81), RGB(0x00, 0x83, 0x00), RGB(0x90, 0x85, 0xe9)
};

// ---------------------------------------------------------------------------------
// enums
// ---------------------------------------------------------------------------------
enum WaterMode : uint8_t { MODE_MANUAL = 0,
                           MODE_SCHEDULE = 1,
                           MODE_SMART = 2 };

enum RunSource : uint8_t { SRC_NONE = 0,
                           SRC_MANUAL,
                           SRC_PROGRAM,
                           SRC_DRAIN };

enum Screen : uint8_t {
  SCR_HOME = 0,
  SCR_ZONES,
  SCR_SCHED,
  SCR_USAGE,
  SCR_INFO,
  SCR_SMART,
  SCR_SETTINGS,
  SCR_WIFI,
  SCR_KEYBOARD,
  SCR_CAL,
  SCR_SAVER
};

// irrigation status of a forecast day
enum DayPlan : uint8_t { PLAN_OFF = 0,
                         PLAN_RUN,
                         PLAN_SKIP };

enum Popup : uint8_t {
  POP_NONE = 0,
  POP_ZONE,
  POP_TIME,
  POP_MINUTES,
  POP_CONFIRM_WINTER,
  POP_MESSAGE
};

// ---------------------------------------------------------------------------------
// structures
// ---------------------------------------------------------------------------------
struct ZoneCfg {
  char name[24];
  bool enabled;
  uint8_t minutes;  // baseline run minutes (schedule / smart 100%)
  float gpm;        // gallons per minute, used for usage
  int16_t bx, by;   // badge location on the map 0..1000
  uint8_t npts;     // polygon points
  int16_t px[MAX_PTS], py[MAX_PTS];
};

struct Settings {
  uint8_t mode;
  bool winter;
  uint8_t days;  // bit0 = Sunday
  bool startEn[2];
  uint16_t startMin[2];  // minutes after midnight

  // smart settings (all metric internally)
  float rainSkipMM;   // rain credit that cancels watering
  uint8_t popSkip;    // skip if the day's chance of rain is above this %
  uint8_t rainDays;   // skip after this many rain days in a row, 0 = off
  float freezeC;      // skip if low temp at or below
  float windSkipKmh;  // skip if max wind above, 0 = off
  uint8_t maxBoost;   // % max scaling
  uint8_t minPct;     // below this % the run is skipped
  float et0Ref;       // mm/day where the baseline minutes are correct

  // general
  uint8_t manualMin;  // default manual run
  uint8_t maxRunMin;  // safety cutoff
  uint8_t drainMin;   // winter drain time, 0 = keep open
  uint8_t brightness;
  bool sound;
  bool metric;
  bool clock24;
  bool allowMulti;  // more than one zone at a time
  uint8_t saverSec;
  float costPer1000;
  uint32_t rainDelayUntil;  // epoch

  // location
  bool hasLocation;
  bool locFromIP;
  float lat, lon;
  char place[48];
  char tzName[40];
  char posix[56];

  // wifi
  char ssid[33];
  char pass[65];

  // touch calibration, affine: sx = a*rx + b*ry + c, sy = d*rx + e*ry + f
  bool calOK;
  float cal[6];

  ZoneCfg zone[NUM_ZONES];
};

struct ZoneRun {
  bool on;
  uint8_t source;
  uint32_t startMs;
  uint32_t durMs;  // 0 = no limit (drain)
};

struct Program {
  bool active;
  uint8_t pct;
  uint8_t current;  // zone index running, 255 = gap
  uint16_t secs[NUM_ZONES];
  uint32_t gapUntil;
};

struct WxDay {
  uint32_t date;  // local midnight epoch
  float tmax, tmin, rain, et0, wind;
  uint8_t pop, rh, code;
  uint32_t sunrise, sunset;
};

struct Weather {
  bool valid;
  uint32_t fetched;  // epoch
  float curTemp, curRH, curWind, curPrecip;
  uint8_t curCode;
  int32_t utcOffset;
  char tzName[40];
  WxDay day[WX_DAYS];
};

struct SmartResult {
  bool skip;
  uint8_t pct;
  char reason[64];
  float et, rainPast, rainFcst;
};

struct UsageDay {
  int32_t dayNum;  // days since 1970 local
  uint32_t sec[NUM_ZONES];
  float baseGal;  // what the baseline schedule would have used
  float progGal;  // what the program actually used
  uint8_t pct;    // smart %, 255 = no program
};

struct Note {
  uint16_t freq;
  uint16_t ms;
};

// defined in later tabs
extern const Note SND_STARTUP[], SND_ZONE_ON[], SND_ZONE_OFF[], SND_WARNING[], SND_OK[], SND_ERROR[], SND_WINTER[];
extern Screen CurScreen;
