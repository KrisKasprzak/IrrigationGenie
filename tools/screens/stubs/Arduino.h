// IrrigationGenie screenshot harness: minimal Arduino API so the real UI code
// (Gfx.ino, UI.ino, Valves.ino, Sound.ino) and Adafruit GFX compile on a PC
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string>
#include <algorithm>

#ifndef ARDUINO
#define ARDUINO 100
#endif
#define PROGMEM
#define PGM_P const char *
#define F(x) x
#define DEG_TO_RAD 0.017453292519943295
#define radians(d) ((d) * DEG_TO_RAD)
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0

class __FlashStringHelper;
typedef bool boolean;
typedef uint8_t byte;

using std::max;
using std::min;

template <class T, class L, class H>
T constrain(T x, L lo, H hi) {
  return x < (T)lo ? (T)lo : (x > (T)hi ? (T)hi : x);
}

class String {
  std::string s;
public:
  String() {}
  String(const char *c) : s(c ? c : "") {}
  String(const std::string &x) : s(x) {}
  String(int v) : s(std::to_string(v)) {}
  String(float v, int d) {
    char b[32];
    snprintf(b, sizeof(b), "%.*f", d, v);
    s = b;
  }
  const char *c_str() const { return s.c_str(); }
  unsigned length() const { return s.size(); }
  String operator+(const String &o) const { return String(s + o.s); }
  friend String operator+(const char *a, const String &b) { return String(std::string(a) + b.s); }
  bool operator==(const String &o) const { return s == o.s; }
  bool operator==(const char *o) const { return s == o; }
  String substring(int a, int b) const { return String(s.substr(a, b - a)); }
};

// time is driven by the harness
extern uint32_t g_millis;
inline uint32_t millis() { return g_millis; }
inline void delay(uint32_t ms) { g_millis += ms; }

struct SerialStub {
  template <class... A> void print(A...) {}
  template <class... A> void println(A...) {}
  template <class... A> void printf(A...) {}
  void begin(int) {}
};
extern SerialStub Serial;

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline bool ledcAttach(int, int, int) { return true; }
inline bool ledcWrite(int, uint32_t) { return true; }
inline uint32_t ledcWriteTone(int, uint32_t) { return 0; }
inline uint32_t ledcChangeFrequency(int, uint32_t, int) { return 0; }
