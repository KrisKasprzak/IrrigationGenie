/*
  IrrigationGenie - Gfx.ino
  drawing helpers, icons, text formatting, splash and touch calibration
*/

// ---------------------------------------------------------------------------------
// text. strings may hold a UTF-8 degree sign (shared with the phone), the GFX
// fonts have no glyph for it so it is drawn as a small circle
// ---------------------------------------------------------------------------------

#define DEG "\xC2\xB0"

int DegRadius(const GFXfont *f) {
  return f == &FONT_BIG ? 3 : 2;
}

int TextWidth(const GFXfont *f, const char *s) {
  int w = 0;
  for (const uint8_t *p = (const uint8_t *)s; *p; p++) {
    if (p[0] == 0xC2 && p[1] == 0xB0) {
      w += DegRadius(f) * 2 + 3;
      p++;
      continue;
    }
    if (f == nullptr) {
      w += 6;
    } else if (*p >= f->first && *p <= f->last) {
      w += f->glyph[*p - f->first].xAdvance;
    }
  }
  return w;
}

// y is the baseline for GFX fonts, the top for the built-in font
void TextAt(Adafruit_GFX &g, const GFXfont *f, int x, int y, uint16_t color, const char *s) {
  g.setFont(f);
  g.setTextSize(1);
  g.setTextColor(color);
  g.setTextWrap(false);
  g.setCursor(x, y);
  for (const uint8_t *p = (const uint8_t *)s; *p; p++) {
    if (p[0] == 0xC2 && p[1] == 0xB0) {
      int r = DegRadius(f);
      int cx = g.getCursorX() + r + 1;
      int cy = f ? y - (f == &FONT_BIG ? 22 : f == &FONT_MED ? 14 : 10) : y + 1;
      g.drawCircle(cx, cy, r, color);
      if (f == &FONT_BIG) g.drawCircle(cx, cy, r - 1, color);
      g.setCursor(g.getCursorX() + r * 2 + 3, y);
      p++;
      continue;
    }
    g.write(*p);
  }
}

void TextAt(const GFXfont *f, int x, int y, uint16_t color, const char *s) {
  TextAt(Display, f, x, y, color, s);
}

void TextCenter(const GFXfont *f, int cx, int y, uint16_t color, const char *s) {
  TextAt(Display, f, cx - TextWidth(f, s) / 2, y, color, s);
}

void TextRight(const GFXfont *f, int rx, int y, uint16_t color, const char *s) {
  TextAt(Display, f, rx - TextWidth(f, s), y, color, s);
}

// copy s into buf, shortened with "..." to fit a pixel width
const char *Fit(const GFXfont *f, const char *s, int maxW, char *buf, size_t len) {
  strlcpy(buf, s, len);
  if (TextWidth(f, buf) <= maxW) return buf;
  int n = strlen(buf);
  while (n > 0) {
    buf[--n] = 0;
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%s..", buf);
    if (TextWidth(f, tmp) <= maxW) {
      strlcpy(buf, tmp, len);
      break;
    }
  }
  return buf;
}

// word wrap into up to maxLines lines, returns lines used
int TextWrap(const GFXfont *f, int x, int y, int maxW, int lineH, int maxLines, uint16_t color, const char *s) {
  char line[64];
  int lines = 0;
  const char *p = s;
  while (*p && lines < maxLines) {
    int n = 0, lastSpace = -1;
    line[0] = 0;
    while (p[n] && n < (int)sizeof(line) - 1) {
      line[n] = p[n];
      line[n + 1] = 0;
      if (TextWidth(f, line) > maxW) {
        line[n] = 0;
        break;
      }
      if (p[n] == ' ') lastSpace = n;
      n++;
    }
    if (p[n] && lastSpace > 0 && lines < maxLines - 1) {
      n = lastSpace;
      line[n] = 0;
    }
    TextAt(f, x, y + lines * lineH, color, line);
    p += n;
    while (*p == ' ') p++;
    lines++;
  }
  return lines;
}

// ---------------------------------------------------------------------------------
// units and formatting (all values are metric internally)
// ---------------------------------------------------------------------------------

float ToTemp(float c) {
  return cfg.metric ? c : c * 1.8f + 32.0f;
}

char *FmtTemp(float c, char *buf) {
  sprintf(buf, "%d" DEG, (int)roundf(ToTemp(c)));
  return buf;
}

char *FmtRain(float mm, char *buf) {
  if (cfg.metric) sprintf(buf, "%.1f mm", mm);
  else sprintf(buf, "%.2f in", mm / 25.4f);
  return buf;
}

char *FmtWind(float kmh, char *buf) {
  if (cfg.metric) sprintf(buf, "%d km/h", (int)roundf(kmh));
  else sprintf(buf, "%d mph", (int)roundf(kmh / 1.609f));
  return buf;
}

char *FmtVolume(float gal, char *buf) {
  float v = cfg.metric ? gal * 3.785f : gal;
  if (v >= 10000) sprintf(buf, "%.1fk", v / 1000.0f);
  else sprintf(buf, "%d", (int)roundf(v));
  return buf;
}

const char *VolUnit() {
  return cfg.metric ? "L" : "gal";
}

char *FmtMinOfDay(uint16_t m, char *buf) {
  int h = m / 60, mm = m % 60;
  if (cfg.clock24) sprintf(buf, "%02d:%02d", h, mm);
  else sprintf(buf, "%d:%02d %s", h % 12 == 0 ? 12 : h % 12, mm, h < 12 ? "AM" : "PM");
  return buf;
}

char *FmtClock(time_t t, char *buf) {
  struct tm lt;
  localtime_r(&t, &lt);
  return FmtMinOfDay(lt.tm_hour * 60 + lt.tm_min, buf);
}

char *FmtDur(uint32_t s, char *buf) {
  if (s >= 3600) sprintf(buf, "%d:%02d:%02d", (int)(s / 3600), (int)(s / 60 % 60), (int)(s % 60));
  else sprintf(buf, "%02d:%02d", (int)(s / 60), (int)(s % 60));
  return buf;
}

char *FmtNextRun(time_t t, char *buf) {
  static const char *DOW[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  time_t now = time(nullptr);
  struct tm a, b;
  localtime_r(&now, &a);
  localtime_r(&t, &b);
  char tm[12];
  FmtMinOfDay(b.tm_hour * 60 + b.tm_min, tm);
  int32_t dd = DaysFromCivil(b.tm_year + 1900, b.tm_mon + 1, b.tm_mday) - DaysFromCivil(a.tm_year + 1900, a.tm_mon + 1, a.tm_mday);
  if (dd == 0) sprintf(buf, "Today %s", tm);
  else if (dd == 1) sprintf(buf, "Tomorrow %s", tm);
  else sprintf(buf, "%s %s", DOW[b.tm_wday], tm);
  return buf;
}

// ---------------------------------------------------------------------------------
// shapes and icons
// ---------------------------------------------------------------------------------

uint16_t Blend(uint16_t a, uint16_t b, uint8_t t) {
  uint8_t r = (((a >> 11) & 31) * (255 - t) + ((b >> 11) & 31) * t) / 255;
  uint8_t g = (((a >> 5) & 63) * (255 - t) + ((b >> 5) & 63) * t) / 255;
  uint8_t bl = ((a & 31) * (255 - t) + (b & 31) * t) / 255;
  return (r << 11) | (g << 5) | bl;
}

// thick arc, angles in degrees, 0 = 3 o'clock, clockwise
void DrawArc(int cx, int cy, int r, int thick, int a0, int a1, uint16_t color) {
  int ri = r - thick;
  for (int a = a0; a < a1; a += 3) {
    int b = min(a + 3, a1);
    float s0 = a * DEG_TO_RAD, s1 = b * DEG_TO_RAD;
    int x0o = cx + cos(s0) * r, y0o = cy + sin(s0) * r;
    int x1o = cx + cos(s1) * r, y1o = cy + sin(s1) * r;
    int x0i = cx + cos(s0) * ri, y0i = cy + sin(s0) * ri;
    int x1i = cx + cos(s1) * ri, y1i = cy + sin(s1) * ri;
    Display.fillTriangle(x0o, y0o, x1o, y1o, x0i, y0i, color);
    Display.fillTriangle(x1o, y1o, x1i, y1i, x0i, y0i, color);
  }
}

void DrawDrop(Adafruit_GFX &g, int cx, int cy, int r, uint16_t color) {
  g.fillCircle(cx, cy, r, color);
  g.fillTriangle(cx - r + 1, cy - r / 3, cx + r - 1, cy - r / 3, cx, cy - r * 2, color);
}

void DrawCloud(int cx, int cy, int r, uint16_t color) {
  Display.fillCircle(cx - r * 35 / 100, cy + r * 10 / 100, r * 35 / 100, color);
  Display.fillCircle(cx + r * 5 / 100, cy - r * 15 / 100, r * 45 / 100, color);
  Display.fillCircle(cx + r * 45 / 100, cy + r * 12 / 100, r * 32 / 100, color);
  Display.fillRoundRect(cx - r * 70 / 100, cy + r * 5 / 100, r * 145 / 100, r * 42 / 100, r * 20 / 100, color);
}

void DrawSun(int cx, int cy, int r) {
  Display.fillCircle(cx, cy, r / 2, C_SUN);
  for (int a = 0; a < 360; a += 45) {
    float s = a * DEG_TO_RAD;
    Display.drawLine(cx + cos(s) * r * 0.7f, cy + sin(s) * r * 0.7f, cx + cos(s) * r, cy + sin(s) * r, C_SUN);
  }
}

// WMO weather code icon, r ~ half the icon size
void DrawWxIcon(int cx, int cy, int r, uint8_t code) {
  if (code == 0) {
    DrawSun(cx, cy, r);
  } else if (code <= 2) {
    DrawSun(cx - r * 3 / 10, cy - r * 3 / 10, r * 7 / 10);
    DrawCloud(cx + r / 10, cy + r * 2 / 10, r * 8 / 10, C_CLOUD);
  } else if (code == 3) {
    DrawCloud(cx, cy, r, C_CLOUD);
  } else if (code == 45 || code == 48) {
    for (int i = -1; i <= 1; i++) {
      Display.fillRoundRect(cx - r * 8 / 10, cy + i * r * 4 / 10 - 1, r * 16 / 10, 3, 1, C_CLOUD);
    }
  } else if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
    DrawCloud(cx, cy - r / 4, r, C_CLOUD);
    for (int i = -1; i <= 1; i++) {
      Display.fillCircle(cx + i * r * 4 / 10, cy + r * 7 / 10, max(1, r / 8), C_WHITE);
    }
  } else if (code >= 95) {
    DrawCloud(cx, cy - r / 4, r, C_MUTED);
    Display.fillTriangle(cx + r / 10, cy, cx - r * 3 / 10, cy + r * 5 / 10, cx, cy + r * 5 / 10, C_SUN);
    Display.fillTriangle(cx, cy + r * 4 / 10, cx + r * 3 / 10, cy + r * 4 / 10, cx - r / 10, cy + r, C_SUN);
  } else {
    // drizzle and rain
    DrawCloud(cx, cy - r / 4, r, C_CLOUD);
    for (int i = -1; i <= 1; i++) {
      int x = cx + i * r * 4 / 10;
      Display.drawLine(x, cy + r * 4 / 10, x - r / 6, cy + r * 9 / 10, C_ICE);
      Display.drawLine(x + 1, cy + r * 4 / 10, x + 1 - r / 6, cy + r * 9 / 10, C_ICE);
    }
  }
}

void DrawWifiIcon(Adafruit_GFX &g, int x, int y, int rssi, bool ok, uint16_t on, uint16_t off) {
  int bars = !ok ? 0 : rssi > -55 ? 4 : rssi > -65 ? 3 : rssi > -75 ? 2 : 1;
  for (int i = 0; i < 4; i++) {
    int h = 4 + i * 3;
    g.fillRect(x + i * 4, y + 13 - h, 3, h, i < bars ? on : off);
  }
  if (!ok) {
    g.drawLine(x, y, x + 14, y + 14, C_BAD);
    g.drawLine(x + 1, y, x + 15, y + 14, C_BAD);
  }
}

void DrawNavIcon(int i, int cx, int cy, uint16_t c) {
  switch (i) {
    case 0:  // home
      Display.fillTriangle(cx - 9, cy - 1, cx + 9, cy - 1, cx, cy - 9, c);
      Display.fillRect(cx - 6, cy - 1, 12, 9, c);
      Display.fillRect(cx - 2, cy + 3, 4, 5, C_BG);
      break;
    case 1:  // zones: map pin
      Display.fillCircle(cx, cy - 3, 6, c);
      Display.fillTriangle(cx - 5, cy, cx + 5, cy, cx, cy + 8, c);
      Display.fillCircle(cx, cy - 3, 2, C_BG);
      break;
    case 2:  // schedule: calendar
      Display.drawRoundRect(cx - 8, cy - 6, 16, 14, 2, c);
      Display.fillRect(cx - 8, cy - 6, 16, 4, c);
      Display.fillRect(cx - 5, cy - 9, 2, 4, c);
      Display.fillRect(cx + 3, cy - 9, 2, 4, c);
      Display.fillRect(cx - 4, cy + 1, 3, 3, c);
      Display.fillRect(cx + 1, cy + 1, 3, 3, c);
      break;
    case 3:  // usage: bars
      Display.fillRect(cx - 8, cy + 1, 4, 7, c);
      Display.fillRect(cx - 2, cy - 4, 4, 12, c);
      Display.fillRect(cx + 4, cy - 8, 4, 16, c);
      break;
    case 4:  // info
      Display.drawCircle(cx, cy, 8, c);
      Display.fillRect(cx - 1, cy - 1, 3, 6, c);
      Display.fillRect(cx - 1, cy - 5, 3, 2, c);
      break;
  }
}

void DrawButton(int x, int y, int w, int h, const char *label, uint16_t fill, uint16_t textColor, const GFXfont *f) {
  Display.fillRoundRect(x, y, w, h, 6, fill);
  int ty = y + h / 2 + (f == &FONT_MED ? 8 : f == &FONT_SMALL ? 6 : -3);
  TextCenter(f, x + w / 2, ty, textColor, label);
}

void DrawToggle(int x, int y, bool on) {
  Display.fillRoundRect(x, y, 34, 18, 9, on ? C_GOOD : C_LINE);
  Display.fillCircle(on ? x + 25 : x + 9, y + 9, 7, C_WHITE);
}

// ---------------------------------------------------------------------------------
// splash
// ---------------------------------------------------------------------------------

void DrawSplash() {
  for (int y = 0; y < SCREEN_H; y += 4) {
    Display.fillRect(0, y, SCREEN_W, 4, Blend(RGB(8, 47, 73), C_BG, y * 255 / SCREEN_H));
  }
  DrawDrop(Display, 160, 86, 22, C_ACCENT);
  Display.fillCircle(152, 90, 6, Blend(C_ACCENT, C_WHITE, 160));
  TextCenter(&FONT_BIG, 160, 150, C_TEXT, "IrrigationGenie");
  TextCenter(&FONT_SMALL, 160, 172, C_SUB, "Smart irrigation  v" CODE_VERSION);
}

void SplashProgress(int pct, const char *msg) {
  Display.fillRoundRect(40, 192, 240, 6, 3, C_CARD2);
  Display.fillRoundRect(40, 192, 240 * pct / 100, 6, 3, C_ACCENT);
  Display.fillRect(0, 204, SCREEN_W, 24, C_BG);
  TextCenter(&FONT_SMALL, 160, 220, C_SUB, msg);
}

// ---------------------------------------------------------------------------------
// touch calibration, 3 point affine (handles rotation, mirroring and scale)
// ---------------------------------------------------------------------------------

const int16_t CAL_X[3] = { 32, 160, 288 };
const int16_t CAL_Y[3] = { 30, 210, 120 };

bool MapTouch(TS_Point p, int16_t &x, int16_t &y) {
  float fx = cfg.cal[0] * p.x + cfg.cal[1] * p.y + cfg.cal[2];
  float fy = cfg.cal[3] * p.x + cfg.cal[4] * p.y + cfg.cal[5];
  x = constrain((int)fx, 0, SCREEN_W - 1);
  y = constrain((int)fy, 0, SCREEN_H - 1);
  return true;
}

void DrawCross(int x, int y, uint16_t c) {
  Display.drawFastHLine(x - 12, y, 25, c);
  Display.drawFastVLine(x, y - 12, 25, c);
  Display.drawCircle(x, y, 6, c);
}

// averaged raw reading of one firm press, waits for release
TS_Point ReadCalPoint() {
  // a touch that never releases (wiring fault) times out instead of hanging here
  uint32_t t = millis();
  while (Touch.touched() && millis() - t < 3000) {
    delay(10);
    ServiceSound();
  }
  delay(150);
  while (!Touch.touched()) {
    delay(10);
    ServiceSound();
  }
  delay(60);
  long sx = 0, sy = 0;
  int n = 0;
  while (n < 24 && Touch.touched()) {
    TS_Point p = Touch.getPoint();
    sx += p.x;
    sy += p.y;
    n++;
    delay(8);
  }
  Click();
  t = millis();
  while (Touch.touched() && millis() - t < 3000) {
    delay(10);
    ServiceSound();
  }
  delay(100);
  if (n == 0) n = 1;
  return TS_Point(sx / n, sy / n, 0);
}

void SolveAffine(const float rx[3], const float ry[3], const float s[3], float &a, float &b, float &c) {
  float det = rx[0] * (ry[1] - ry[2]) - ry[0] * (rx[1] - rx[2]) + (rx[1] * ry[2] - rx[2] * ry[1]);
  if (fabs(det) < 1) det = 1;
  a = (s[0] * (ry[1] - ry[2]) - ry[0] * (s[1] - s[2]) + (s[1] * ry[2] - s[2] * ry[1])) / det;
  b = (rx[0] * (s[1] - s[2]) - s[0] * (rx[1] - rx[2]) + (rx[1] * s[2] - rx[2] * s[1])) / det;
  c = (rx[0] * (ry[1] * s[2] - ry[2] * s[1]) - ry[0] * (rx[1] * s[2] - rx[2] * s[1]) + s[0] * (rx[1] * ry[2] - rx[2] * ry[1])) / det;
}

void RunCalibration() {
  StopSound();
  Screen was = CurScreen;
  CurScreen = SCR_CAL;

  for (;;) {
    float rx[3], ry[3], sx[3], sy[3];
    for (int i = 0; i < 3; i++) {
      Display.fillScreen(C_BG);
      TextCenter(&FONT_MED, 160, 100, C_TEXT, "Touch Calibration");
      TextCenter(&FONT_SMALL, 160, 128, C_SUB, "Press and hold the target");
      char b[16];
      sprintf(b, "%d of 3", i + 1);
      TextCenter(&FONT_SMALL, 160, 150, C_MUTED, b);
      DrawCross(CAL_X[i], CAL_Y[i], C_ACCENT);
      TS_Point p = ReadCalPoint();
      rx[i] = p.x;
      ry[i] = p.y;
      sx[i] = CAL_X[i];
      sy[i] = CAL_Y[i];
    }
    SolveAffine(rx, ry, sx, cfg.cal[0], cfg.cal[1], cfg.cal[2]);
    SolveAffine(rx, ry, sy, cfg.cal[3], cfg.cal[4], cfg.cal[5]);
    cfg.calOK = true;

    // test: dots follow the finger, then DONE or REDO
    Display.fillScreen(C_BG);
    TextCenter(&FONT_SMALL, 160, 100, C_SUB, "Draw to test, then press DONE");
    DrawButton(40, 180, 100, 40, "REDO", C_CARD2, C_TEXT, &FONT_MED);
    DrawButton(180, 180, 100, 40, "DONE", C_ACCENT_DK, C_TEXT, &FONT_MED);
    while (Touch.touched()) delay(10);
    int action = 0;
    while (action == 0) {
      ServiceSound();
      if (Touch.touched()) {
        int16_t x, y;
        MapTouch(Touch.getPoint(), x, y);
        if (y >= 180 && x >= 40 && x < 140) action = 1;
        else if (y >= 180 && x >= 180 && x < 280) action = 2;
        else Display.fillCircle(x, y, 2, C_ACCENT);
      }
      delay(5);
    }
    Click();
    while (Touch.touched()) delay(10);
    if (action == 2) break;
  }
  SaveConfig();
  CurScreen = was;
}
