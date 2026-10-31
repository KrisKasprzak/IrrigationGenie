// fake hardware for the screenshot harness: the display is an in-memory 320x240 canvas
#pragma once
#include "Arduino.h"
#include "Adafruit_GFX.h"

class Adafruit_ILI9341 : public GFXcanvas16 {
  int16_t wx = 0, wy = 0, ww = 1, wh = 1;
  int32_t wp = 0;
public:
  Adafruit_ILI9341(int8_t, int8_t, int8_t) : GFXcanvas16(320, 240) {}
  void begin(uint32_t = 0) {}
  void setRotation(uint8_t) override {}
  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    wx = x; wy = y; ww = w; wh = h; wp = 0;
  }
  void writePixels(uint16_t *c, uint32_t n, bool = true, bool = false) {
    for (uint32_t i = 0; i < n; i++, wp++) drawPixel(wx + wp % ww, wy + wp / ww, c[i]);
  }
};

class TS_Point {
public:
  TS_Point() : x(0), y(0), z(0) {}
  TS_Point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}
  int16_t x, y, z;
};

class XPT2046_Touchscreen {
public:
  XPT2046_Touchscreen(uint8_t, uint8_t = 255) {}
  bool begin() { return true; }
  bool touched() { return false; }
  TS_Point getPoint() { return TS_Point(); }
};

struct IPAddress {
  String toString() const { return String("192.168.1.50"); }
};

#define WIFI_AUTH_OPEN 0
struct WiFiClass {
  const char *names[6] = { "HomeNetwork", "HomeNetwork-5G", "Garage-Ext", "NETGEAR-42", "Guest", "Printer-Direct" };
  int rssi[6] = { -48, -55, -63, -71, -76, -84 };
  int scanNetworks() { return 6; }
  String SSID(int i) { return String(names[i]); }
  String SSID() { return String("HomeNetwork"); }
  int RSSI(int i = 0) { return rssi[i]; }
  int encryptionType(int i) { return i == 4 ? WIFI_AUTH_OPEN : 3; }
  void scanDelete() {}
  IPAddress localIP() { return IPAddress(); }
  IPAddress softAPIP() { return IPAddress(); }
};
extern WiFiClass WiFi;

struct File {
  explicit operator bool() const { return false; }
  size_t size() { return 0; }
  size_t read(uint8_t *, size_t) { return 0; }
  void close() {}
};
struct FSStub {
  bool exists(const char *) { return false; }
  File open(const char *, const char * = "r") { return File(); }
  bool remove(const char *) { return true; }
};
extern FSStub LittleFS;

struct WebServer {
  WebServer(int) {}
};
