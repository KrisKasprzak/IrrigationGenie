/*
  IrrigationGenie - UI.ino
  screens, buttons, touch handling, popups, WiFi setup keyboard and screen saver
*/

// button ids
#define B_NAV 1        // 1..5
#define B_MODE 10      // 10..12
#define B_WXCARD 13
#define B_STOPALL 14
#define B_RUNNOW 15
#define B_WINTERBAR 16
#define B_ZONE 20      // 20..26 map badges
#define B_MASTER 27
#define B_TILE 30      // 30..36 schedule tiles
#define B_START 37     // 37..38
#define B_DAY 40       // 40..46
#define B_CHART 47
#define B_SETTINGS 50
#define B_WINTER 51
#define B_RAINDELAY 52
#define B_SOUND 53
#define B_DIM 54
#define B_BRIGHT 55
#define B_UNITS 56
#define B_WIFISET 57
#define B_CAL 58
#define B_LOCIP 59
#define B_BACK 60
#define B_CLOCK 61
#define B_POP_CLOSE 70
#define B_POP_MINUS 71
#define B_POP_PLUS 72
#define B_POP_START 73
#define B_POP_STOP 74
#define B_POP_OK 75
#define B_POP_CANCEL 76
#define B_POP_HUP 77
#define B_POP_HDN 78
#define B_POP_MUP 79
#define B_POP_MDN 80
#define B_POP_AMPM 81
#define B_POP_EN 82
#define B_NET 90       // 90..94
#define B_RESCAN 96
#define B_SKIP 97
#define B_PGUP 98
#define B_PGDN 99
#define B_KEY 100      // 100..139
#define B_SHIFT 140
#define B_SYM 141
#define B_SPACE 142
#define B_DEL 143
#define B_KBOK 144
#define B_KBCANCEL 145
#define B_KBSHOW 146

struct Btn {
  int16_t x, y, w, h;
  uint8_t id;
  bool rep;
};

#define MAX_BTNS 56
Btn Btns[MAX_BTNS];
uint8_t NumBtns = 0;

Screen CurScreen = SCR_HOME;
Popup CurPopup = POP_NONE;
bool NeedRedraw = false;
bool SaverOn = false;
bool WaitRelease = false;
bool MapOnScreen = false;
uint32_t MapDrawnStamp = 0;
bool Pulse = false;

// popup edit values
int PopZone = 0;
int PopMinutes = 15;
bool PopEnabled = true;
int PopStart = 0;
int PopTimeMin = 360;
char MsgTitle[32], MsgText[96];
int SelBar = -1;

// wifi list + keyboard
#define MAX_NETS 20
String NetSSID[MAX_NETS];
int8_t NetRSSI[MAX_NETS];
bool NetLock[MAX_NETS];
int NetCount = 0, NetPage = 0;
String KbTarget;
char KbText[65];
uint8_t KbLayout = 0;  // 0 lower, 1 upper, 2 symbols
bool KbShow = false;

const char *KB_ROWS[3][4] = {
  { "1234567890", "qwertyuiop", "asdfghjkl-", "zxcvbnm_.@" },
  { "1234567890", "QWERTYUIOP", "ASDFGHJKL-", "ZXCVBNM_.@" },
  { "!@#$%^&*()", "`~-_=+[]{}", "\\|;:'\",.<>", "/?        " }
};

const char *TAB_NAMES[5] = { "Home", "Zones", "Schedule", "Usage", "Info" };
const char *DAY_SHORT[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };

void AddBtn(int x, int y, int w, int h, uint8_t id, bool rep) {
  if (NumBtns >= MAX_BTNS) return;
  Btns[NumBtns++] = { (int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h, id, rep };
}

void AddBtn(int x, int y, int w, int h, uint8_t id) {
  AddBtn(x, y, w, h, id, false);
}

int HitTest(int x, int y) {
  // last added wins (popups are added on top)
  for (int i = NumBtns - 1; i >= 0; i--) {
    Btn &b = Btns[i];
    if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) return i;
  }
  return -1;
}

void ScreenChanged() {
  NeedRedraw = true;
}

bool HasNav(Screen s) {
  return s <= SCR_SETTINGS;
}

void GoScreen(Screen s) {
  CurScreen = s;
  CurPopup = POP_NONE;
  MapOnScreen = false;
  SelBar = -1;
  if (s == SCR_WIFI) {
    NetCount = 0;
    NetPage = 0;
  }
  DrawScreen();
}

void OpenPopup(Popup p) {
  CurPopup = p;
  MapOnScreen = false;
  DrawScreen();
}

// redraw only the popup (no flicker of the screen under it)
void RedrawPopup() {
  NumBtns = 0;
  if (CurPopup == POP_TIME) DrawTimePopup();
}

void ClosePopup() {
  CurPopup = POP_NONE;
  DrawScreen();
}

void ShowMessage(const char *title, const char *text) {
  strlcpy(MsgTitle, title, sizeof(MsgTitle));
  strlcpy(MsgText, text, sizeof(MsgText));
  OpenPopup(POP_MESSAGE);
}

// ---------------------------------------------------------------------------------
// touch
// ---------------------------------------------------------------------------------

void ServiceTouch() {
  static bool down = false;
  static uint32_t lastPoll = 0, lastSeen = 0, downMs = 0, repMs = 0;
  static int pressed = -1;

  if (CurScreen == SCR_CAL) return;
  if (millis() - lastPoll < 15) {
    if (NeedRedraw && !SaverOn) DrawScreen();
    return;
  }
  lastPoll = millis();

  bool t = Touch.touched();
  if (t) {
    lastSeen = millis();
    LastTouchMs = millis();
    if (SaverOn) {
      WakeUp();
      return;
    }
    if (WaitRelease) return;

    if (!down) {
      down = true;
      downMs = repMs = millis();
      int16_t x, y;
      MapTouch(Touch.getPoint(), x, y);
      int i = HitTest(x, y);
      pressed = -1;
      if (i >= 0) {
        pressed = Btns[i].id;
        bool rep = Btns[i].rep;
        Click();
        OnButton(pressed, x, y);
        if (!rep) pressed = -1;
      }
    } else if (pressed >= 0 && millis() - downMs > 500 && millis() - repMs > 110) {
      repMs = millis();
      OnButton(pressed, 0, 0);
    }
  } else if (millis() - lastSeen > 60) {
    // XPT2046 drops out for a few ms during a press, wait before calling it a release
    down = false;
    pressed = -1;
    WaitRelease = false;
  }

  if (NeedRedraw && !SaverOn) DrawScreen();
}

// ---------------------------------------------------------------------------------
// drawing a whole screen
// ---------------------------------------------------------------------------------

void DrawScreen() {
  if (SaverOn || CurScreen == SCR_CAL) return;
  NeedRedraw = false;
  NumBtns = 0;

  if (HasNav(CurScreen)) {
    bool mapOnly = CurScreen == SCR_ZONES && CurPopup == POP_NONE && MapOnScreen && MapDrawnStamp == MapStamp;
    if (!mapOnly) {
      Display.fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, C_BG);
    }
    DrawNav();
    UpdateStatusBar();
  }

  switch (CurScreen) {
    case SCR_HOME: DrawHome(); break;
    case SCR_ZONES: DrawZones(); break;
    case SCR_SCHED: DrawSchedule(); break;
    case SCR_USAGE: DrawUsage(); break;
    case SCR_INFO: DrawInfo(); break;
    case SCR_SMART: DrawSmart(); break;
    case SCR_SETTINGS: DrawSettings(); break;
    case SCR_WIFI: DrawWifi(); break;
    case SCR_KEYBOARD: DrawKeyboard(); break;
    default: break;
  }

  if (CurPopup != POP_NONE) {
    // popups are modal: only their buttons respond
    NumBtns = 0;
    MapOnScreen = false;
    switch (CurPopup) {
      case POP_ZONE: DrawZonePopup(); break;
      case POP_TIME: DrawTimePopup(); break;
      case POP_MINUTES: DrawMinutesPopup(); break;
      case POP_CONFIRM_WINTER: DrawWinterPopup(); break;
      case POP_MESSAGE: DrawMessagePopup(); break;
      default: break;
    }
  }
}

// once a second: clock in the status bar and live counters
void UpdateScreen() {
  if (SaverOn) return;
  Pulse = !Pulse;
  if (CurPopup == POP_ZONE) {
    UpdateZonePopup();
  } else if (CurPopup == POP_NONE) {
    if (CurScreen == SCR_HOME) UpdateHome();
    else if (CurScreen == SCR_ZONES) DrawBadges();
  }
}

void UpdateStatusBar() {
  if (SaverOn || !HasNav(CurScreen)) return;
  GFXcanvas16 &c = StatusBar;
  char buf[24];
  c.fillScreen(RGB(15, 27, 50));
  c.drawFastHLine(0, STATUS_H - 1, SCREEN_W, C_LINE);

  if (TimeValid) FmtClock(time(nullptr), buf);
  else strcpy(buf, "--:--");
  TextAt(c, &FONT_SMALL, 6, 17, C_TEXT, buf);

  // mode chip
  const char *mode;
  uint16_t mc;
  if (cfg.winter) {
    mode = Draining ? "WINTER DRAIN" : "WINTER";
    mc = C_ICE;
  } else if (AnyZoneOn()) {
    int z = RunningZone();
    sprintf(buf, "ZONE %d ON", z + 1);
    mode = buf;
    mc = C_ACCENT;
  } else if (cfg.mode == MODE_SMART) {
    if (smart.skip) sprintf(buf, "SMART SKIP");
    else sprintf(buf, "SMART %d%%", smart.pct);
    mode = buf;
    mc = C_GOOD;
  } else if (cfg.mode == MODE_SCHEDULE) {
    mode = "SCHEDULE";
    mc = C_ACCENT;
  } else {
    mode = "MANUAL";
    mc = C_SUB;
  }
  int w = strlen(mode) * 6 + 12;
  c.drawRoundRect(88, 4, w, 16, 8, mc);
  TextAt(c, nullptr, 94, 8, mc, mode);

  int x = 88 + w + 6;
  if (!InternetOK) {
    c.fillRoundRect(x, 4, 58, 16, 8, C_BAD);
    TextAt(c, nullptr, x + 8, 8, C_WHITE, "OFFLINE");
  } else if (RainDelayActive()) {
    c.fillRoundRect(x, 4, 46, 16, 8, C_ACCENT_DK);
    TextAt(c, nullptr, x + 6, 8, C_WHITE, "DELAY");
  }

  if (wx.valid) {
    FmtTemp(wx.curTemp, buf);
    int tw = TextWidth(&FONT_SMALL, buf);
    TextAt(c, &FONT_SMALL, 290 - tw, 17, C_TEXT, buf);
  }
  DrawWifiIcon(c, 298, 4, WiFi.RSSI(), WifiOK, C_ACCENT, C_LINE);
  Display.drawRGBBitmap(0, 0, c.getBuffer(), SCREEN_W, STATUS_H);
}

void DrawNav() {
  int active = CurScreen == SCR_SMART ? 0 : CurScreen == SCR_SETTINGS ? 4 : (int)CurScreen;
  Display.fillRect(0, NAV_Y, SCREEN_W, NAV_H, RGB(15, 27, 50));
  Display.drawFastHLine(0, NAV_Y, SCREEN_W, C_LINE);
  for (int i = 0; i < 5; i++) {
    int cx = 32 + i * 64;
    uint16_t c = i == active ? C_ACCENT : C_MUTED;
    if (i == active) Display.fillRect(cx - 16, NAV_Y, 32, 3, C_ACCENT);
    DrawNavIcon(i, cx, NAV_Y + 15, c);
    TextAt(nullptr, cx - strlen(TAB_NAMES[i]) * 3, NAV_Y + 29, c, TAB_NAMES[i]);
    AddBtn(i * 64, NAV_Y, 64, NAV_H, B_NAV + i);
  }
}

// false while the zone still has its default "Zone N" name (no need to show it twice)
bool CustomName(int z) {
  char def[12];
  sprintf(def, "Zone %d", z + 1);
  return strcmp(cfg.zone[z].name, def) != 0;
}

void Card(int x, int y, int w, int h) {
  Display.fillRoundRect(x, y, w, h, 8, C_CARD);
}

// ---------------------------------------------------------------------------------
// home
// ---------------------------------------------------------------------------------

void DrawHome() {
  char buf[48];

  if (cfg.winter) {
    Display.fillRoundRect(8, 30, 304, 28, 8, Blend(C_ICE, C_BG, 170));
    TextCenter(&FONT_SMALL, 160, 49, C_ICE, Draining ? "Winter mode - draining lines" : "Winter mode - watering off");
    AddBtn(8, 30, 304, 28, B_WINTERBAR);
  } else {
    const char *names[3] = { "Manual", "Schedule", "Smart" };
    Display.fillRoundRect(8, 30, 304, 28, 8, C_CARD);
    for (int i = 0; i < 3; i++) {
      int x = 8 + i * 101;
      bool sel = cfg.mode == i;
      if (sel) Display.fillRoundRect(x + 2, 32, 99, 24, 7, C_ACCENT_DK);
      TextCenter(&FONT_SMALL, x + 51, 49, sel ? C_TEXT : C_SUB, names[i]);
      AddBtn(x, 30, 101, 28, B_MODE + i);
    }
  }

  // weather card
  Card(8, 64, 150, 132);
  AddBtn(8, 64, 150, 132, B_WXCARD);
  if (wx.valid) {
    WxDay &d = wx.day[WX_TODAY];
    DrawWxIcon(36, 92, 20, wx.curCode);
    TextAt(&FONT_BIG, 66, 106, C_TEXT, FmtTemp(wx.curTemp, buf));
    char hi[8], lo[8];
    sprintf(buf, "H %s  L %s", FmtTemp(d.tmax, hi), FmtTemp(d.tmin, lo));
    TextAt(&FONT_SMALL, 16, 136, C_SUB, buf);
    char r[16];
    sprintf(buf, "Rain %d%%  %s", d.pop, FmtRain(d.rain, r));
    TextAt(&FONT_SMALL, 16, 156, C_ICE, buf);
    sprintf(buf, "Humidity %d%%", (int)wx.curRH);
    TextAt(&FONT_SMALL, 16, 176, C_SUB, buf);
    TextAt(nullptr, 16, 184, C_MUTED, "Smart details >");
  } else {
    DrawWxIcon(36, 94, 20, 3);
    TextAt(&FONT_SMALL, 66, 100, C_SUB, "No weather");
    TextWrap(&FONT_SMALL, 16, 140, 134, 18, 3, C_MUTED, cfg.hasLocation ? "Waiting for internet..." : "Set location from the phone app");
  }

  DrawHomeStatus();
}

// right card on home, redrawn every second while watering
void DrawHomeStatus() {
  char buf[48];
  int x = 164, y = 64, w = 148, h = 132;
  Card(x, y, w, h);

  int z = RunningZone();
  if (Draining) {
    TextAt(nullptr, x + 10, y + 8, C_ICE, "WINTER DRAIN");
    TextAt(&FONT_SMALL, x + 10, y + 40, C_TEXT, "All zones open");
    TextAt(&FONT_SMALL, x + 10, y + 60, C_SUB, "Master closed");
    uint32_t left = DrainSecondsLeft();
    TextAt(&FONT_BIG, x + 10, y + 98, C_TEXT, cfg.drainMin ? FmtDur(left, buf) : "--:--");
    DrawButton(x + 10, y + 108, w - 20, 20, "STOP", C_BAD, C_WHITE, nullptr);
    AddBtn(x, y + 104, w, 28, B_STOPALL);
  } else if (z >= 0) {
    TextAt(nullptr, x + 10, y + 8, C_ACCENT, prog.active ? "WATERING - PROGRAM" : "WATERING");
    Display.fillCircle(x + 16, y + 32, 6, ZONE_COLORS[z]);
    sprintf(buf, "Zone %d", z + 1);
    TextAt(&FONT_MED, x + 28, y + 40, C_TEXT, buf);
    if (CustomName(z)) TextAt(&FONT_SMALL, x + 10, y + 60, C_SUB, Fit(&FONT_SMALL, cfg.zone[z].name, w - 20, buf, sizeof(buf)));
    UpdateHome();
    DrawButton(x + 10, y + 108, w - 20, 20, "STOP", C_BAD, C_WHITE, nullptr);
    AddBtn(x, y + 104, w, 28, B_STOPALL);
  } else {
    time_t nr = NextRun();
    if (cfg.mode == MODE_SMART && !cfg.winter) {
      // which zones the next run waters, e.g. "NEXT 1-2-6-7"
      // every zone that is on (enabled with minutes), skipped or not
      char zs[16];
      NextZonesStr(100, zs);
      if (zs[0]) sprintf(buf, "NEXT %s", zs);
      else strcpy(buf, "NEXT - NO ZONES");
      TextAt(nullptr, x + 10, y + 8, C_SUB, buf);
    } else {
      TextAt(nullptr, x + 10, y + 8, C_SUB, "NEXT RUN");
    }
    if (cfg.winter) {
      TextAt(&FONT_SMALL, x + 10, y + 36, C_TEXT, "None (winter)");
    } else if (cfg.mode == MODE_MANUAL) {
      TextAt(&FONT_SMALL, x + 10, y + 36, C_TEXT, "Manual mode");
      TextWrap(&FONT_SMALL, x + 10, y + 58, w - 20, 18, 2, C_SUB, "Tap Zones to water");
    } else {
      TextAt(&FONT_SMALL, x + 10, y + 36, C_TEXT, nr ? FmtNextRun(nr, buf) : "Not scheduled");
      if (RainDelayActive()) {
        sprintf(buf, "Rain delay %dh", (int)((cfg.rainDelayUntil - time(nullptr) + 3599) / 3600));
        TextAt(&FONT_SMALL, x + 10, y + 58, C_ICE, buf);
      } else if (cfg.mode == MODE_SMART) {
        uint16_t c = smart.skip ? C_WARN : C_GOOD;
        if (smart.skip) strcpy(buf, "SKIP");
        else sprintf(buf, "%d%%", smart.pct);
        TextAt(&FONT_BIG, x + 10, y + 70, c, buf);
        TextWrap(nullptr, x + 10, y + 78, w - 20, 10, 2, C_SUB, smart.reason);
      } else {
        int tot = 0;
        for (int i = 0; i < NUM_ZONES; i++) {
          if (cfg.zone[i].enabled) tot += cfg.zone[i].minutes;
        }
        sprintf(buf, "%d min total", tot);
        TextAt(&FONT_SMALL, x + 10, y + 58, C_SUB, buf);
      }
    }
    if (!cfg.winter) {
      DrawButton(x + 10, y + 108, w - 20, 20, "RUN NOW", C_ACCENT_DK, C_WHITE, nullptr);
      AddBtn(x, y + 104, w, 28, B_RUNNOW);
    }
  }
}

void UpdateHome() {
  int z = RunningZone();
  if (Draining) {
    if (!cfg.drainMin) return;
    char buf[16];
    Display.fillRect(172, 136, 136, 30, C_CARD);
    TextAt(&FONT_BIG, 174, 162, C_TEXT, FmtDur(DrainSecondsLeft(), buf));
    return;
  }
  if (z < 0) return;
  char buf[16];
  uint32_t left = ZoneSecondsLeft(z);
  Display.fillRect(172, 132, 136, 32, C_CARD);
  TextAt(&FONT_BIG, 174, 158, C_TEXT, FmtDur(left, buf));
  uint32_t tot = run[z].durMs / 1000;
  int pw = tot ? 128 * (tot - left) / tot : 0;
  Display.fillRoundRect(174, 164, 128, 4, 2, C_CARD2);
  Display.fillRoundRect(174, 164, max(pw, 4), 4, 2, ZONE_COLORS[z]);
}

// ---------------------------------------------------------------------------------
// intelligent watering details
// ---------------------------------------------------------------------------------

void DrawSmart() {
  char buf[48], a[16], ref[16];

  TextAt(&FONT_SMALL, 8, 44, C_ACCENT, "Intelligent Watering");
  if (cfg.mode != MODE_SMART) TextAt(nullptr, 190, 34, C_WARN, "(smart mode is off)");

  // gauge: 0 .. maxBoost over 270 degrees
  int cx = 58, cy = 100, r = 40;
  DrawArc(cx, cy, r, 8, 135, 405, C_CARD2);
  uint16_t gc = smart.skip ? C_MUTED : smart.pct < 80 ? C_ACCENT : smart.pct <= 120 ? C_GOOD : C_WARN;
  int sweep = smart.skip ? 0 : 270 * min((int)smart.pct, (int)cfg.maxBoost) / cfg.maxBoost;
  if (sweep > 0) DrawArc(cx, cy, r, 8, 135, 135 + sweep, gc);
  if (smart.skip) strcpy(buf, "SKIP");
  else sprintf(buf, "%d%%", smart.pct);
  TextCenter(&FONT_MED, cx, cy + 8, smart.skip ? C_WARN : C_TEXT, buf);
  TextCenter(nullptr, cx, cy + 34, C_SUB, "next run");

  int x = 112;
  TextWrap(&FONT_SMALL, x, 66, 202, 17, 2, C_TEXT, smart.reason);
  sprintf(buf, "ET %s (ref %s)", FmtRain(smart.et, a), FmtRain(cfg.et0Ref, ref));
  TextAt(nullptr, x, 92, C_SUB, buf);
  sprintf(buf, "Rain last 2 days: %s", FmtRain(smart.rainPast, a));
  TextAt(nullptr, x, 104, C_SUB, buf);
  sprintf(buf, "Rain expected:    %s", FmtRain(smart.rainFcst, a));
  TextAt(nullptr, x, 116, C_SUB, buf);
  if (wx.valid) {
    sprintf(buf, "Humidity %d%%  Wind %s", (int)wx.curRH, FmtWind(wx.curWind, a));
    TextAt(nullptr, x, 128, C_SUB, buf);
  }

  // forecast: today + 3 days
  static const char *DOW[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  for (int i = 0; i < 4; i++) {
    int bx = 4 + i * 79;
    Card(bx, 142, 75, 56);
    if (!wx.valid) continue;
    WxDay &d = wx.day[WX_TODAY + i];
    time_t t = d.date + 43200;  // noon, safe from DST edges
    struct tm lt;
    localtime_r(&t, &lt);
    TextAt(nullptr, bx + 6, 147, C_SUB, i == 0 ? "Today" : DOW[lt.tm_wday]);
    // irrigation status for the day, right aligned on the same line
    DayPlan dp = DayStatus(WX_TODAY + i);
    const char *st = dp == PLAN_RUN ? "Run" : dp == PLAN_SKIP ? "Skip" : "Off";
    TextRight(nullptr, bx + 70, 147, dp == PLAN_RUN ? C_GOOD : dp == PLAN_SKIP ? C_WARN : C_MUTED, st);
    DrawWxIcon(bx + 18, 176, 12, d.code);
    char hi[8], lo[8];
    TextAt(&FONT_SMALL, bx + 36, 172, C_TEXT, FmtTemp(d.tmax, hi));
    TextAt(nullptr, bx + 38, 178, C_SUB, FmtTemp(d.tmin, lo));
    sprintf(buf, "%d%%", d.pop);
    TextAt(nullptr, bx + 6, 189, C_ICE, buf);
  }
}

// ---------------------------------------------------------------------------------
// zones: map of the yard
// ---------------------------------------------------------------------------------

int16_t MapX(int16_t v) {
  return (int32_t)v * MAP_W / 1000;
}

int16_t MapY(int16_t v) {
  return CONTENT_Y + (int32_t)v * MAP_H / 1000;
}

// even-odd scanline fill, handles the L-shaped zones that wrap the house
void FillPoly(const int16_t *xs, const int16_t *ys, int n, uint16_t color) {
  if (n < 3) return;
  int16_t ymin = ys[0], ymax = ys[0];
  for (int i = 1; i < n; i++) {
    ymin = min(ymin, ys[i]);
    ymax = max(ymax, ys[i]);
  }
  int16_t nodes[MAX_PTS + 6];
  Display.startWrite();
  for (int y = ymin; y < ymax; y++) {
    int cnt = 0;
    for (int i = 0, j = n - 1; i < n; j = i++) {
      if ((ys[j] <= y && ys[i] > y) || (ys[i] <= y && ys[j] > y)) {
        nodes[cnt++] = xs[j] + (int32_t)(y - ys[j]) * (xs[i] - xs[j]) / (ys[i] - ys[j]);
      }
    }
    for (int a = 1; a < cnt; a++) {  // insertion sort, cnt is tiny
      int16_t v = nodes[a];
      int b = a - 1;
      while (b >= 0 && nodes[b] > v) {
        nodes[b + 1] = nodes[b];
        b--;
      }
      nodes[b + 1] = v;
    }
    for (int k = 0; k + 1 < cnt; k += 2) {
      Display.writeFastHLine(nodes[k], y, nodes[k + 1] - nodes[k] + 1, color);
    }
  }
  Display.endWrite();
}

void OutlinePoly(const int16_t *xs, const int16_t *ys, int n, uint16_t color) {
  for (int i = 0, j = n - 1; i < n; j = i++) {
    Display.drawLine(xs[j], ys[j], xs[i], ys[i], color);
  }
}

// plan polygons (0..1000) -> screen
void DrawPlanPoly(const PlanPoly &p, uint16_t fill, uint16_t line) {
  int16_t xs[16], ys[16];
  for (int i = 0; i < p.n; i++) {
    xs[i] = MapX(p.pts[i][0]);
    ys[i] = MapY(p.pts[i][1]);
  }
  FillPoly(xs, ys, p.n, fill);
  OutlinePoly(xs, ys, p.n, line);
}

void ZonePoly(int z, bool fill) {
  ZoneCfg &zc = cfg.zone[z];
  if (zc.npts < 3) return;
  int16_t xs[MAX_PTS], ys[MAX_PTS];
  for (int i = 0; i < zc.npts; i++) {
    xs[i] = MapX(zc.px[i]);
    ys[i] = MapY(zc.py[i]);
  }
  uint16_t c = zc.enabled ? ZONE_COLORS[z] : C_MUTED;
  if (fill) FillPoly(xs, ys, zc.npts, Blend(c, RGB(19, 38, 29), run[z].on ? 60 : 150));
  OutlinePoly(xs, ys, zc.npts, c);
}

// photo from the phone, streamed from flash row by row
bool DrawMapImage() {
  File f = LittleFS.open(MAP_FILE, "r");
  if (!f || f.size() != (size_t)MAP_W * MAP_H * 2) {
    if (f) f.close();
    return false;
  }
  static uint16_t row[MAP_W * 4];
  Display.startWrite();
  Display.setAddrWindow(0, CONTENT_Y, MAP_W, MAP_H);
  for (int y = 0; y < MAP_H; y += 4) {
    f.read((uint8_t *)row, sizeof(row));
    Display.writePixels(row, MAP_W * 4);
  }
  Display.endWrite();
  f.close();
  return true;
}

// the cleaned up yard plan from Plan.h, zones from the (editable) config
void DrawPlan() {
  Display.fillRect(0, CONTENT_Y, MAP_W, MAP_H, C_BG);
  DrawPlanPoly(PLAN_PROPERTY, RGB(19, 38, 29), RGB(59, 91, 74));
  for (int z = 0; z < NUM_ZONES; z++) ZonePoly(z, true);
  DrawPlanPoly(PLAN_DRIVE, RGB(43, 52, 66), RGB(100, 116, 139));
  DrawPlanPoly(PLAN_HOUSE, RGB(51, 65, 92), C_SUB);
  TextCenter(&FONT_SMALL, MapX(PLAN_HOUSE_LABEL[0]), MapY(PLAN_HOUSE_LABEL[1]) + 5, C_CLOUD, "House");
  TextCenter(nullptr, MapX(PLAN_DRIVE_LABEL[0]), MapY(PLAN_DRIVE_LABEL[1]) - 3, C_SUB, "Driveway");

  // north arrow
  int nx = 302, ny = CONTENT_Y + 98;
  Display.fillTriangle(nx, ny - 22, nx + 7, ny - 6, nx - 7, ny - 6, C_ACCENT);
  TextAt(&FONT_SMALL, nx - 6, ny + 10, C_ACCENT, "N");
}

bool LastChip = false;

void DrawZones() {
  // the ALL OFF chip sits on the map, redraw everything when it goes away
  bool chip = AnyZoneOn() || MasterManual;
  if (chip != LastChip) MapOnScreen = false;
  LastChip = chip;
  bool mapOnly = MapOnScreen && MapDrawnStamp == MapStamp;
  if (!mapOnly) {
    if (DrawMapImage()) {
      for (int z = 0; z < NUM_ZONES; z++) ZonePoly(z, false);
    } else {
      DrawPlan();
    }
    MapOnScreen = true;
    MapDrawnStamp = MapStamp;
  } else if (!LittleFS.exists(MAP_FILE)) {
    // zone state changed: refill zone areas so the running one lights up
    for (int z = 0; z < NUM_ZONES; z++) ZonePoly(z, true);
    DrawPlanPoly(PLAN_DRIVE, RGB(43, 52, 66), RGB(100, 116, 139));
    DrawPlanPoly(PLAN_HOUSE, RGB(51, 65, 92), C_SUB);
    TextCenter(&FONT_SMALL, MapX(PLAN_HOUSE_LABEL[0]), MapY(PLAN_HOUSE_LABEL[1]) + 5, C_CLOUD, "House");
    TextCenter(nullptr, MapX(PLAN_DRIVE_LABEL[0]), MapY(PLAN_DRIVE_LABEL[1]) - 3, C_SUB, "Driveway");
  }

  // master chip
  Display.fillRoundRect(4, CONTENT_Y + 4, 78, 18, 9, MasterOn ? C_GOOD : C_CARD);
  Display.drawRoundRect(4, CONTENT_Y + 4, 78, 18, 9, MasterOn ? C_GOOD : C_LINE);
  TextAt(nullptr, 12, CONTENT_Y + 9, C_WHITE, MasterOn ? "MASTER OPEN" : "MASTER OFF");
  AddBtn(0, CONTENT_Y, 88, 28, B_MASTER);

  if (chip) {
    Display.fillRoundRect(250, CONTENT_Y + 4, 66, 18, 9, C_BAD);
    TextAt(nullptr, 260, CONTENT_Y + 9, C_WHITE, "ALL OFF");
    AddBtn(244, CONTENT_Y, 76, 28, B_STOPALL);
  }

  DrawBadges();
  for (int z = 0; z < NUM_ZONES; z++) {
    AddBtn(MapX(cfg.zone[z].bx) - 16, MapY(cfg.zone[z].by) - 16, 32, 32, B_ZONE + z);
  }
}

// numbered zone badges, running zones pulse
void DrawBadges() {
  for (int z = 0; z < NUM_ZONES; z++) {
    int x = MapX(cfg.zone[z].bx), y = MapY(cfg.zone[z].by);
    uint16_t c = cfg.zone[z].enabled ? ZONE_COLORS[z] : C_MUTED;
    if (run[z].on) {
      Display.drawCircle(x, y, 14, Pulse ? C_WHITE : c);
      Display.drawCircle(x, y, 13, Pulse ? c : C_WHITE);
    }
    Display.fillCircle(x, y, 11, run[z].on ? C_ACCENT : c);
    Display.drawCircle(x, y, 11, C_WHITE);
    char b[2] = { (char)('1' + z), 0 };
    TextCenter(&FONT_SMALL, x, y + 6, run[z].on ? C_BLACK : C_WHITE, b);
  }
}

// ---------------------------------------------------------------------------------
// schedule
// ---------------------------------------------------------------------------------

void DrawSchedule() {
  char buf[32], t[12];

  for (int k = 0; k < 2; k++) {
    int x = 6 + k * 158;
    Card(x, 28, 150, 28);
    sprintf(buf, "Start %d", k + 1);
    TextAt(nullptr, x + 8, 38, C_SUB, buf);
    if (cfg.startEn[k]) TextRight(&FONT_SMALL, x + 142, 48, C_TEXT, FmtMinOfDay(cfg.startMin[k], t));
    else TextRight(&FONT_SMALL, x + 142, 48, C_MUTED, "Off");
    AddBtn(x, 28, 150, 28, B_START + k);
  }

  for (int d = 0; d < 7; d++) {
    int x = 6 + d * 44;
    bool on = cfg.days & (1 << d);
    Display.fillRoundRect(x, 62, 41, 26, 8, on ? C_ACCENT_DK : C_CARD);
    TextCenter(&FONT_SMALL, x + 20, 80, on ? C_TEXT : C_MUTED, DAY_SHORT[d]);
    AddBtn(x, 62, 41, 26, B_DAY + d);
  }

  int total = 0;
  for (int i = 0; i < 8; i++) {
    int x = 6 + (i % 4) * 78, y = 94 + (i / 4) * 53;
    Card(x, y, 74, 49);
    if (i == NUM_ZONES) {
      // summary tile
      TextAt(nullptr, x + 6, y + 6, C_SUB, "TOTAL");
      sprintf(buf, "%dm", total);
      TextAt(&FONT_MED, x + 6, y + 34, C_TEXT, buf);
      const char *note = cfg.mode == MODE_SMART ? "smart scales" : cfg.mode == MODE_MANUAL ? "manual: off" : "fixed";
      TextAt(nullptr, x + 6, y + 39, cfg.mode == MODE_MANUAL ? C_WARN : C_MUTED, note);
      break;
    }
    ZoneCfg &z = cfg.zone[i];
    Display.fillRoundRect(x, y, 5, 49, 2, z.enabled ? ZONE_COLORS[i] : C_MUTED);
    sprintf(buf, "%d", i + 1);
    TextAt(nullptr, x + 10, y + 6, C_TEXT, buf);
    TextAt(nullptr, x + 20, y + 6, C_SUB, Fit(nullptr, z.name, 50, t, sizeof(t)));
    if (z.enabled) {
      sprintf(buf, "%dm", z.minutes);
      total += z.minutes;
      TextAt(&FONT_MED, x + 10, y + 40, C_TEXT, buf);
    } else {
      TextAt(&FONT_SMALL, x + 10, y + 38, C_MUTED, "off");
    }
    AddBtn(x, y, 74, 49, B_TILE + i);
  }
}

// ---------------------------------------------------------------------------------
// usage: 30 days stacked by zone
// ---------------------------------------------------------------------------------

#define CH_X 34
#define CH_Y 88
#define CH_H 92
#define CH_SLOT 9

UsageDay *UsageFor(int32_t dn) {
  UsageDay &u = usage[((dn % USAGE_DAYS) + USAGE_DAYS) % USAGE_DAYS];
  return u.dayNum == dn ? &u : nullptr;
}

void DrawUsage() {
  char buf[32], v[16];
  float gal, saved;
  uint32_t sec;
  UsageTotals(gal, sec, saved);

  const char *labels[3] = { "30-DAY USE", "RUN TIME", "SMART SAVED" };
  for (int i = 0; i < 3; i++) {
    int x = 6 + i * 104;
    Card(x, 28, 100, 44);
    TextAt(nullptr, x + 8, 33, C_SUB, labels[i]);
    if (i == 0) {
      TextAt(&FONT_MED, x + 8, 64, C_TEXT, FmtVolume(gal, v));
      TextAt(nullptr, x + 12 + TextWidth(&FONT_MED, v), 57, C_SUB, VolUnit());
    } else if (i == 1) {
      sprintf(v, "%.1f", sec / 3600.0f);
      TextAt(&FONT_MED, x + 8, 64, C_TEXT, v);
      TextAt(nullptr, x + 12 + TextWidth(&FONT_MED, v), 57, C_SUB, "hrs");
    } else {
      FmtVolume(max(saved, 0.0f), v);
      TextAt(&FONT_MED, x + 8, 64, C_GOOD, v);
      TextAt(nullptr, x + 12 + TextWidth(&FONT_MED, v), 57, C_SUB, VolUnit());
    }
  }

  if (!TimeValid) {
    TextCenter(&FONT_SMALL, 160, 140, C_MUTED, "Waiting for time...");
    return;
  }

  int32_t today = TodayNum();
  float maxGal = 1;
  for (int i = 0; i < USAGE_DAYS; i++) {
    UsageDay *u = UsageFor(today - i);
    if (u) maxGal = max(maxGal, DayGallons(*u));
  }
  // round the axis up to a friendly number
  float steps[] = { 10, 20, 50, 100, 200, 250, 500, 1000, 2000, 5000, 10000, 50000 };
  for (float s : steps) {
    if (maxGal <= s) {
      maxGal = s;
      break;
    }
  }

  // recessive grid
  for (int g = 0; g <= 2; g++) {
    int y = CH_Y + CH_H - g * CH_H / 2;
    for (int x = CH_X; x < CH_X + USAGE_DAYS * CH_SLOT; x += 4) Display.drawPixel(x, y, C_LINE);
  }
  TextRight(nullptr, CH_X - 3, CH_Y - 3, C_MUTED, FmtVolume(maxGal, v));
  TextRight(nullptr, CH_X - 3, CH_Y + CH_H / 2 - 3, C_MUTED, FmtVolume(maxGal / 2, v));
  TextRight(nullptr, CH_X - 3, CH_Y + CH_H - 3, C_MUTED, "0");

  bool any = false;
  for (int i = 0; i < USAGE_DAYS; i++) {
    UsageDay *u = UsageFor(today - (USAGE_DAYS - 1 - i));
    int x = CH_X + i * CH_SLOT;
    if (!u) continue;
    int base = CH_Y + CH_H;
    for (int z = 0; z < NUM_ZONES; z++) {
      float g = u->sec[z] / 60.0f * cfg.zone[z].gpm;
      int h = g / maxGal * CH_H + 0.5f;
      if (h <= 0) continue;
      any = true;
      Display.fillRect(x, base - h, CH_SLOT - 2, h, ZONE_COLORS[z]);
      base -= h;
      if (base < CH_Y + CH_H) Display.drawFastHLine(x, base, CH_SLOT - 2, C_BG);  // gap between segments
    }
    if (u->pct == 0) Display.fillCircle(x + 3, CH_Y + CH_H - 3, 2, C_ICE);  // skipped day
    if (i == SelBar) Display.drawRect(x - 1, CH_Y - 1, CH_SLOT, CH_H + 2, C_WHITE);
  }
  AddBtn(CH_X, CH_Y - 10, USAGE_DAYS * CH_SLOT, CH_H + 20, B_CHART);

  // x labels
  time_t first = time(nullptr) - (USAGE_DAYS - 1) * 86400L;
  struct tm lt;
  localtime_r(&first, &lt);
  strftime(buf, sizeof(buf), "%b %d", &lt);
  TextAt(nullptr, CH_X, CH_Y + CH_H + 6, C_MUTED, buf);
  TextRight(nullptr, CH_X + USAGE_DAYS * CH_SLOT, CH_Y + CH_H + 6, C_MUTED, "Today");

  if (SelBar >= 0) {
    UsageDay *u = UsageFor(today - (USAGE_DAYS - 1 - SelBar));
    time_t d = time(nullptr) - (USAGE_DAYS - 1 - SelBar) * 86400L;
    localtime_r(&d, &lt);
    strftime(v, sizeof(v), "%a %b %d", &lt);
    if (u) {
      uint32_t s = 0;
      for (int z = 0; z < NUM_ZONES; z++) s += u->sec[z];
      char g[12];
      sprintf(buf, "%s: %s %s, %d min", v, FmtVolume(DayGallons(*u), g), VolUnit(), (int)(s / 60));
      if (u->pct == 0) strcat(buf, ", skipped");
      else if (u->pct != 255) sprintf(buf + strlen(buf), ", %d%%", u->pct);
    } else {
      sprintf(buf, "%s: no watering", v);
    }
    TextAt(nullptr, CH_X, 77, C_TEXT, buf);
  } else if (!any) {
    TextCenter(&FONT_SMALL, 175, 140, C_MUTED, "No watering in 30 days");
  } else {
    TextAt(nullptr, CH_X, 77, C_MUTED, "Tap a bar for details");
  }
}

// ---------------------------------------------------------------------------------
// info and settings
// ---------------------------------------------------------------------------------

void InfoRow(int i, const char *label, const char *value) {
  int y = 42 + i * 18;
  TextAt(&FONT_SMALL, 8, y, C_SUB, label);
  char b[48];
  TextAt(&FONT_SMALL, 106, y, C_TEXT, Fit(&FONT_SMALL, value, 208, b, sizeof(b)));
}

void DrawInfo() {
  char buf[64], a[12], b[12];

  if (cfg.hasLocation) {
    snprintf(buf, sizeof(buf), "%s%s", cfg.place[0] ? cfg.place : "Set", cfg.locFromIP ? " (approx)" : "");
    InfoRow(0, "Location", buf);
    snprintf(buf, sizeof(buf), "%.4f, %.4f", cfg.lat, cfg.lon);
    InfoRow(1, "Lat / Lon", buf);
  } else {
    InfoRow(0, "Location", "Not set - use phone app");
    InfoRow(1, "Lat / Lon", "-");
  }
  InfoRow(2, "Time zone", cfg.tzName[0] ? cfg.tzName : "UTC");
  if (TimeValid) {
    time_t now = time(nullptr);
    struct tm lt;
    localtime_r(&now, &lt);
    char z[8], o[8];
    strftime(z, sizeof(z), "%Z", &lt);
    strftime(o, sizeof(o), "%z", &lt);
    snprintf(buf, sizeof(buf), "%s  UTC%c%d  DST %s", z, o[0], atoi(o + 1) / 100, lt.tm_isdst > 0 ? "on" : "off");
  } else {
    strcpy(buf, "Time not set");
  }
  InfoRow(3, "Offset", buf);
  if (wx.valid) {
    snprintf(buf, sizeof(buf), "Rise %s  Set %s", FmtClock(wx.day[WX_TODAY].sunrise, a), FmtClock(wx.day[WX_TODAY].sunset, b));
  } else {
    strcpy(buf, "-");
  }
  InfoRow(4, "Sun", buf);
  if (WifiOK) snprintf(buf, sizeof(buf), "%s  %d dBm", WiFi.SSID().c_str(), WiFi.RSSI());
  else strcpy(buf, APMode ? "Setup AP " AP_SSID : "Not connected");
  InfoRow(5, "WiFi", buf);
  InfoRow(6, "Phone", WifiOK ? (String("http://") + WiFi.localIP().toString()).c_str() : "http://192.168.4.1");
  if (wx.valid) {
    snprintf(buf, sizeof(buf), "Updated %s", FmtClock(wx.fetched, a));
  } else {
    strcpy(buf, "No data");
  }
  InfoRow(7, "Weather", buf);

  uint32_t up = (millis() - BootMs) / 1000;
  snprintf(buf, sizeof(buf), "v" CODE_VERSION "  up %dd %dh %dm", (int)(up / 86400), (int)(up / 3600 % 24), (int)(up / 60 % 60));
  TextAt(nullptr, 8, 186, C_MUTED, buf);
  DrawButton(214, 174, 100, 24, "SETTINGS", C_ACCENT_DK, C_WHITE, &FONT_SMALL);
  AddBtn(210, 170, 110, 30, B_SETTINGS);
}

void SettingBtn(int col, int row, uint8_t id, const char *label, uint16_t fill) {
  int x = 6 + col * 158, y = 30 + row * 34;
  DrawButton(x, y, 150, 30, label, fill, C_TEXT, &FONT_SMALL);
  AddBtn(x, y, 150, 30, id);
}

void DrawSettings() {
  char buf[32];
  SettingBtn(0, 0, B_WINTER, cfg.winter ? "Winter: ON" : "Winter: off", cfg.winter ? Blend(C_ICE, C_BG, 120) : C_CARD2);
  if (RainDelayActive()) sprintf(buf, "Rain delay: %dh", (int)((cfg.rainDelayUntil - time(nullptr) + 3599) / 3600));
  else strcpy(buf, "Rain delay: off");
  SettingBtn(1, 0, B_RAINDELAY, buf, RainDelayActive() ? C_ACCENT_DK : C_CARD2);
  SettingBtn(0, 1, B_SOUND, cfg.sound ? "Sound: on" : "Sound: off", C_CARD2);
  SettingBtn(1, 1, B_UNITS, cfg.metric ? "Units: metric" : "Units: US", C_CARD2);

  // brightness - / +
  DrawButton(6, 98, 46, 30, "-", C_CARD2, C_TEXT, &FONT_MED);
  DrawButton(110, 98, 46, 30, "+", C_CARD2, C_TEXT, &FONT_MED);
  sprintf(buf, "%d%%", cfg.brightness * 100 / 255);
  TextCenter(&FONT_SMALL, 81, 118, C_TEXT, buf);
  AddBtn(6, 98, 46, 30, B_DIM, true);
  AddBtn(110, 98, 46, 30, B_BRIGHT, true);
  SettingBtn(1, 2, B_CLOCK, cfg.clock24 ? "Clock: 24h" : "Clock: 12h", C_CARD2);

  SettingBtn(0, 3, B_WIFISET, "WiFi setup", C_CARD2);
  SettingBtn(1, 3, B_CAL, "Calibrate touch", C_CARD2);
  SettingBtn(0, 4, B_LOCIP, "Locate by IP", C_CARD2);
  SettingBtn(1, 4, B_BACK, "Back", C_ACCENT_DK);
}

// ---------------------------------------------------------------------------------
// popups
// ---------------------------------------------------------------------------------

void PopCard(int x, int y, int w, int h) {
  Display.fillRoundRect(x + 3, y + 3, w, h, 10, C_BLACK);
  Display.fillRoundRect(x, y, w, h, 10, C_CARD);
  Display.drawRoundRect(x, y, w, h, 10, C_LINE);
}

void DrawZonePopup() {
  char buf[32];
  int z = PopZone;
  PopCard(20, 34, 280, 160);
  Display.fillCircle(40, 56, 8, ZONE_COLORS[z]);
  sprintf(buf, "Zone %d", z + 1);
  TextAt(&FONT_MED, 56, 64, C_TEXT, buf);
  if (CustomName(z)) TextAt(&FONT_SMALL, 40, 88, C_SUB, Fit(&FONT_SMALL, cfg.zone[z].name, 240, buf, sizeof(buf)));
  DrawButton(262, 38, 32, 28, "X", C_CARD2, C_TEXT, &FONT_SMALL);
  AddBtn(256, 34, 44, 36, B_POP_CLOSE);

  UpdateZonePopup();

  DrawButton(40, 120, 44, 30, "-", C_CARD2, C_TEXT, &FONT_MED);
  DrawButton(236, 120, 44, 30, "+", C_CARD2, C_TEXT, &FONT_MED);
  AddBtn(36, 116, 52, 38, B_POP_MINUS, true);
  AddBtn(232, 116, 52, 38, B_POP_PLUS, true);
  DrawPopMinutes(160, 144);

  bool can = !cfg.winter;
  DrawButton(40, 158, 110, 30, run[z].on ? "RESTART" : "START", can ? C_GOOD : C_LINE, C_WHITE, &FONT_SMALL);
  DrawButton(170, 158, 110, 30, "STOP", run[z].on ? C_BAD : C_LINE, C_WHITE, &FONT_SMALL);
  AddBtn(40, 158, 110, 30, B_POP_START);
  AddBtn(170, 158, 110, 30, B_POP_STOP);
}

void DrawPopMinutes(int cx, int y) {
  char buf[16];
  Display.fillRect(cx - 60, y - 22, 120, 28, C_CARD);
  sprintf(buf, "%d min", PopMinutes);
  TextCenter(&FONT_MED, cx, y, C_TEXT, buf);
}

void UpdateZonePopup() {
  char buf[32], t[12];
  int z = PopZone;
  Display.fillRect(36, 94, 250, 20, C_CARD);
  if (cfg.winter) {
    TextAt(&FONT_SMALL, 40, 110, C_ICE, "Winter mode - disabled");
  } else if (run[z].on) {
    sprintf(buf, "Running - %s left", FmtDur(ZoneSecondsLeft(z), t));
    TextAt(&FONT_SMALL, 40, 110, C_GOOD, buf);
  } else {
    TextAt(&FONT_SMALL, 40, 110, C_MUTED, "Off");
  }
}

void DrawMinutesPopup() {
  char buf[32];
  int z = PopZone;
  PopCard(30, 40, 260, 150);
  Display.fillCircle(50, 62, 8, ZONE_COLORS[z]);
  sprintf(buf, "Zone %d run time", z + 1);
  TextAt(&FONT_MED, 66, 70, C_TEXT, buf);

  DrawButton(46, 86, 44, 32, "-", C_CARD2, C_TEXT, &FONT_MED);
  DrawButton(230, 86, 44, 32, "+", C_CARD2, C_TEXT, &FONT_MED);
  AddBtn(42, 82, 52, 40, B_POP_MINUS, true);
  AddBtn(226, 82, 52, 40, B_POP_PLUS, true);
  DrawPopMinutes(160, 112);

  TextAt(&FONT_SMALL, 46, 146, C_SUB, "Enabled");
  DrawToggle(120, 132, PopEnabled);
  AddBtn(40, 126, 120, 30, B_POP_EN);

  DrawButton(46, 156, 100, 28, "Cancel", C_CARD2, C_TEXT, &FONT_SMALL);
  DrawButton(174, 156, 100, 28, "Save", C_ACCENT_DK, C_TEXT, &FONT_SMALL);
  AddBtn(46, 156, 100, 30, B_POP_CANCEL);
  AddBtn(174, 156, 100, 30, B_POP_OK);
}

void DrawTimePopup() {
  char buf[16];
  PopCard(20, 30, 280, 166);
  sprintf(buf, "Start time %d", PopStart + 1);
  TextAt(&FONT_SMALL, 34, 50, C_SUB, buf);

  int h = PopTimeMin / 60, m = PopTimeMin % 60;
  // arrows
  for (int i = 0; i < 2; i++) {
    int x = 62 + i * 82;
    Display.fillRoundRect(x, 56, 60, 22, 6, C_CARD2);
    Display.fillTriangle(x + 30, 60, x + 22, 72, x + 38, 72, C_TEXT);
    Display.fillRoundRect(x, 122, 60, 22, 6, C_CARD2);
    Display.fillTriangle(x + 30, 140, x + 22, 128, x + 38, 128, C_TEXT);
  }
  AddBtn(62, 52, 60, 28, B_POP_HUP, true);
  AddBtn(144, 52, 60, 28, B_POP_MUP, true);
  AddBtn(62, 120, 60, 28, B_POP_HDN, true);
  AddBtn(144, 120, 60, 28, B_POP_MDN, true);

  int dh = cfg.clock24 ? h : (h % 12 == 0 ? 12 : h % 12);
  sprintf(buf, "%02d", dh);
  TextCenter(&FONT_BIG, 92, 112, C_TEXT, buf);
  TextCenter(&FONT_BIG, 133, 110, C_TEXT, ":");
  sprintf(buf, "%02d", m);
  TextCenter(&FONT_BIG, 174, 112, C_TEXT, buf);
  if (!cfg.clock24) {
    DrawButton(220, 84, 64, 32, h < 12 ? "AM" : "PM", C_ACCENT_DK, C_TEXT, &FONT_MED);
    AddBtn(216, 80, 72, 40, B_POP_AMPM);
  }

  TextAt(&FONT_SMALL, 34, 172, C_SUB, "On");
  DrawToggle(62, 158, PopEnabled);
  AddBtn(28, 152, 74, 32, B_POP_EN);
  DrawButton(110, 156, 84, 30, "Cancel", C_CARD2, C_TEXT, &FONT_SMALL);
  DrawButton(204, 156, 84, 30, "Save", C_ACCENT_DK, C_TEXT, &FONT_SMALL);
  AddBtn(110, 156, 84, 30, B_POP_CANCEL);
  AddBtn(204, 156, 84, 30, B_POP_OK);
}

void DrawWinterPopup() {
  char buf[48];
  PopCard(20, 40, 280, 150);
  if (!cfg.winter) {
    TextAt(&FONT_MED, 36, 70, C_ICE, "Enter winter mode?");
    if (cfg.drainMin) sprintf(buf, "all zones open %d min to drain.", cfg.drainMin);
    else strcpy(buf, "all zones open to drain lines.");
    TextAt(&FONT_SMALL, 36, 96, C_SUB, "Master valve closes and");
    TextAt(&FONT_SMALL, 36, 116, C_SUB, buf);
    TextAt(&FONT_SMALL, 36, 136, C_SUB, "No watering until turned off.");
  } else {
    TextAt(&FONT_MED, 36, 70, C_ICE, "Exit winter mode?");
    TextAt(&FONT_SMALL, 36, 100, C_SUB, "Zones close and normal");
    TextAt(&FONT_SMALL, 36, 120, C_SUB, "watering resumes.");
  }
  DrawButton(36, 150, 110, 30, "Cancel", C_CARD2, C_TEXT, &FONT_SMALL);
  DrawButton(174, 150, 110, 30, cfg.winter ? "Exit" : "Enter", Blend(C_ICE, C_BG, 100), C_TEXT, &FONT_SMALL);
  AddBtn(36, 150, 110, 30, B_POP_CANCEL);
  AddBtn(174, 150, 110, 30, B_POP_OK);
}

void DrawMessagePopup() {
  PopCard(30, 50, 260, 130);
  TextAt(&FONT_MED, 46, 80, C_TEXT, MsgTitle);
  TextWrap(&FONT_SMALL, 46, 106, 228, 18, 2, C_SUB, MsgText);
  DrawButton(170, 140, 100, 30, "OK", C_ACCENT_DK, C_TEXT, &FONT_SMALL);
  AddBtn(30, 50, 260, 130, B_POP_CLOSE);
}

// ---------------------------------------------------------------------------------
// WiFi setup from the base unit
// ---------------------------------------------------------------------------------

void ScanNetworks() {
  Display.fillRect(0, 34, SCREEN_W, 136, C_BG);
  TextCenter(&FONT_SMALL, 160, 100, C_SUB, "Scanning...");
  int n = WiFi.scanNetworks();
  NetCount = 0;
  for (int i = 0; i < n && NetCount < MAX_NETS; i++) {
    String s = WiFi.SSID(i);
    if (s.length() == 0) continue;
    bool dup = false;
    for (int k = 0; k < NetCount; k++) {
      if (NetSSID[k] == s) dup = true;
    }
    if (dup) continue;
    NetSSID[NetCount] = s;
    NetRSSI[NetCount] = WiFi.RSSI(i);
    NetLock[NetCount] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    NetCount++;
  }
  WiFi.scanDelete();
  NetPage = 0;
}

void DrawWifi() {
  Display.fillScreen(C_BG);
  TextAt(&FONT_MED, 8, 24, C_TEXT, "WiFi Setup");
  DrawButton(228, 4, 86, 26, "Rescan", C_CARD2, C_TEXT, &FONT_SMALL);
  AddBtn(224, 0, 96, 34, B_RESCAN);

  if (NetCount == 0) ScanNetworks();
  Display.fillRect(0, 34, SCREEN_W, 136, C_BG);

  char b[40];
  for (int r = 0; r < 5; r++) {
    int i = NetPage * 5 + r;
    if (i >= NetCount) break;
    int y = 36 + r * 27;
    Card(6, y, 308, 25);
    TextAt(&FONT_SMALL, 14, y + 18, C_TEXT, Fit(&FONT_SMALL, NetSSID[i].c_str(), 220, b, sizeof(b)));
    if (NetLock[i]) {
      Display.drawRoundRect(256, y + 6, 8, 8, 3, C_SUB);
      Display.fillRect(254, y + 11, 12, 9, C_SUB);
    }
    DrawWifiIcon(Display, 286, y + 5, NetRSSI[i], true, C_ACCENT, C_LINE);
    AddBtn(6, y, 308, 25, B_NET + r);
  }
  if (NetCount == 0) TextCenter(&FONT_SMALL, 160, 100, C_MUTED, "No networks found");

  TextAt(nullptr, 8, 176, C_SUB, "Or join WiFi '" AP_SSID "' with your");
  TextAt(nullptr, 8, 188, C_SUB, "phone and open http://192.168.4.1");
  if (NetCount > 5) {
    DrawButton(6, 204, 50, 30, "<", C_CARD2, C_TEXT, &FONT_MED);
    DrawButton(62, 204, 50, 30, ">", C_CARD2, C_TEXT, &FONT_MED);
    AddBtn(6, 204, 50, 30, B_PGUP);
    AddBtn(62, 204, 50, 30, B_PGDN);
  }
  DrawButton(214, 204, 100, 30, WifiOK ? "Back" : "Skip", C_CARD2, C_TEXT, &FONT_SMALL);
  AddBtn(214, 204, 100, 30, B_SKIP);
}

void TryConnect(const char *ssid, const char *pass) {
  Display.fillScreen(C_BG);
  DrawDrop(Display, 160, 90, 16, C_ACCENT);
  TextCenter(&FONT_SMALL, 160, 140, C_SUB, "Connecting to");
  TextCenter(&FONT_MED, 160, 166, C_TEXT, ssid);
  if (JoinNetwork(ssid, pass)) {
    GoScreen(SCR_HOME);
    ShowMessage("Connected", (String("Phone app: http://") + WiFi.localIP().toString()).c_str());
  } else {
    GoScreen(SCR_WIFI);
    ShowMessage("Not connected", "Check the password and try again.");
  }
}

void DrawKbField() {
  Display.fillRoundRect(6, 4, 208, 32, 6, C_CARD);
  Display.drawRoundRect(6, 4, 208, 32, 6, C_ACCENT_DK);
  char shown[66], b[40];
  int n = strlen(KbText);
  if (n == 0) {
    snprintf(shown, sizeof(shown), "Password for %s", KbTarget.c_str());
    TextAt(nullptr, 12, 16, C_MUTED, Fit(nullptr, shown, 196, b, sizeof(b)));
    return;
  }
  for (int i = 0; i < n; i++) shown[i] = KbShow ? KbText[i] : '*';
  shown[n] = 0;
  // show the tail if it is longer than the field
  const char *p = shown;
  while (TextWidth(&FONT_SMALL, p) > 192) p++;
  TextAt(&FONT_SMALL, 12, 26, C_TEXT, p);
}

void DrawKeyboard() {
  Display.fillScreen(C_BG);
  DrawKbField();
  DrawButton(218, 4, 46, 32, KbShow ? "Hide" : "Show", C_CARD2, C_TEXT, nullptr);
  DrawButton(268, 4, 46, 32, "X", C_CARD2, C_TEXT, &FONT_SMALL);
  AddBtn(218, 4, 46, 32, B_KBSHOW);
  AddBtn(268, 4, 46, 32, B_KBCANCEL);

  for (int r = 0; r < 4; r++) {
    const char *row = KB_ROWS[KbLayout][r];
    for (int k = 0; k < 10; k++) {
      if (row[k] == ' ' || row[k] == 0) continue;
      int x = k * 32, y = 42 + r * 35;
      Display.fillRoundRect(x + 1, y, 30, 32, 5, C_CARD2);
      char s[2] = { row[k], 0 };
      TextCenter(&FONT_SMALL, x + 16, y + 22, C_TEXT, s);
      AddBtn(x, y, 32, 34, B_KEY + r * 10 + k);
    }
  }
  int y = 184;
  DrawButton(2, y, 58, 40, KbLayout == 1 ? "abc" : "Shift", KbLayout == 1 ? C_ACCENT_DK : C_CARD, C_TEXT, &FONT_SMALL);
  DrawButton(64, y, 58, 40, KbLayout == 2 ? "abc" : "#+=", C_CARD, C_TEXT, &FONT_SMALL);
  DrawButton(126, y, 86, 40, "space", C_CARD, C_SUB, &FONT_SMALL);
  DrawButton(216, y, 46, 40, "Del", C_CARD, C_TEXT, &FONT_SMALL);
  DrawButton(266, y, 52, 40, "OK", C_GOOD, C_WHITE, &FONT_SMALL);
  AddBtn(2, y, 58, 40, B_SHIFT);
  AddBtn(64, y, 58, 40, B_SYM);
  AddBtn(126, y, 86, 40, B_SPACE);
  AddBtn(216, y, 46, 40, B_DEL, true);
  AddBtn(266, y, 52, 40, B_KBOK);
}

void KbType(char c) {
  int n = strlen(KbText);
  if (n < 64) {
    KbText[n] = c;
    KbText[n + 1] = 0;
  }
  DrawKbField();
}

// ---------------------------------------------------------------------------------
// button actions
// ---------------------------------------------------------------------------------

void OnButton(int id, int16_t tx, int16_t ty) {

  if (id >= B_NAV && id < B_NAV + 5) {
    Screen s[5] = { SCR_HOME, SCR_ZONES, SCR_SCHED, SCR_USAGE, SCR_INFO };
    GoScreen(s[id - B_NAV]);
    return;
  }
  if (id >= B_MODE && id <= B_MODE + 2) {
    cfg.mode = id - B_MODE;
    MarkConfigDirty();
    UpdateSmartPreview();
    DrawScreen();
    return;
  }
  if (id >= B_ZONE && id < B_ZONE + NUM_ZONES) {
    PopZone = id - B_ZONE;
    PopMinutes = cfg.manualMin;
    OpenPopup(POP_ZONE);
    return;
  }
  if (id >= B_TILE && id < B_TILE + NUM_ZONES) {
    PopZone = id - B_TILE;
    PopMinutes = cfg.zone[PopZone].minutes;
    PopEnabled = cfg.zone[PopZone].enabled;
    OpenPopup(POP_MINUTES);
    return;
  }
  if (id >= B_START && id <= B_START + 1) {
    PopStart = id - B_START;
    PopTimeMin = cfg.startMin[PopStart];
    PopEnabled = cfg.startEn[PopStart];
    OpenPopup(POP_TIME);
    return;
  }
  if (id >= B_DAY && id < B_DAY + 7) {
    cfg.days ^= 1 << (id - B_DAY);
    MarkConfigDirty();
    UpdateSmartPreview();
    DrawScreen();
    return;
  }
  if (id >= B_NET && id < B_NET + 5) {
    int i = NetPage * 5 + id - B_NET;
    if (i >= NetCount) return;
    KbTarget = NetSSID[i];
    if (!NetLock[i]) {
      TryConnect(KbTarget.c_str(), "");
      return;
    }
    KbText[0] = 0;
    KbLayout = 0;
    KbShow = false;
    GoScreen(SCR_KEYBOARD);
    return;
  }
  if (id >= B_KEY && id < B_KEY + 40) {
    int r = (id - B_KEY) / 10, k = (id - B_KEY) % 10;
    KbType(KB_ROWS[KbLayout][r][k]);
    return;
  }

  switch (id) {
    case B_WXCARD: GoScreen(SCR_SMART); break;
    case B_WINTERBAR: GoScreen(SCR_SETTINGS); break;
    case B_STOPALL: StopAll(); break;
    case B_RUNNOW:
      if (cfg.mode == MODE_SMART) {
        ComputeSmart(WX_TODAY, smart);
        StartProgram(smart.skip ? 100 : smart.pct);
      } else {
        StartProgram(100);
      }
      DrawScreen();
      break;
    case B_MASTER:
      if (cfg.winter) ShowMessage("Winter mode", "Master valve stays closed in winter mode.");
      else SetMasterManual(!MasterManual);
      break;
    case B_CHART:
      SelBar = constrain((tx - CH_X) / CH_SLOT, 0, USAGE_DAYS - 1);
      DrawScreen();
      break;
    case B_SETTINGS: GoScreen(SCR_SETTINGS); break;
    case B_BACK: GoScreen(SCR_INFO); break;
    case B_WINTER: OpenPopup(POP_CONFIRM_WINTER); break;
    case B_RAINDELAY:
      {
        // cycle off -> 24 -> 48 -> 72 -> off
        int h = RainDelayActive() ? (cfg.rainDelayUntil - time(nullptr) + 3599) / 3600 : 0;
        int nh = h == 0 ? 24 : h <= 24 ? 48 : h <= 48 ? 72 : 0;
        cfg.rainDelayUntil = nh ? time(nullptr) + nh * 3600UL : 0;
        MarkConfigDirty();
        DrawScreen();
      }
      break;
    case B_SOUND:
      cfg.sound = !cfg.sound;
      MarkConfigDirty();
      DrawScreen();
      break;
    case B_UNITS:
      cfg.metric = !cfg.metric;
      MarkConfigDirty();
      UpdateSmartPreview();
      DrawScreen();
      break;
    case B_CLOCK:
      cfg.clock24 = !cfg.clock24;
      MarkConfigDirty();
      DrawScreen();
      break;
    case B_DIM:
    case B_BRIGHT:
      cfg.brightness = constrain(cfg.brightness + (id == B_DIM ? -15 : 15), 15, 255);
      SetBacklight(cfg.brightness);
      MarkConfigDirty();
      DrawScreen();
      break;
    case B_WIFISET:
      StartAP();
      GoScreen(SCR_WIFI);
      break;
    case B_CAL:
      RunCalibration();
      GoScreen(SCR_SETTINGS);
      break;
    case B_LOCIP:
      if (!WifiOK) {
        ShowMessage("No WiFi", "Connect to WiFi first.");
      } else {
        NetWantIPLocate = true;
        ShowMessage("Locating", "Approximate location from your internet address. Use the phone app for exact.");
      }
      break;

    // popups
    case B_POP_CLOSE:
    case B_POP_CANCEL:
      ClosePopup();
      break;
    case B_POP_MINUS:
    case B_POP_PLUS:
      PopMinutes = constrain(PopMinutes + (id == B_POP_PLUS ? 1 : -1), CurPopup == POP_MINUTES ? 0 : 1, CurPopup == POP_MINUTES ? 120 : cfg.maxRunMin);
      DrawPopMinutes(160, CurPopup == POP_MINUTES ? 112 : 144);
      break;
    case B_POP_START:
      if (cfg.winter) break;
      StartZone(PopZone, (uint32_t)PopMinutes * 60, SRC_MANUAL);
      DrawScreen();
      break;
    case B_POP_STOP:
      StopZone(PopZone);
      DrawScreen();
      break;
    case B_POP_EN:
      PopEnabled = !PopEnabled;
      DrawToggle(CurPopup == POP_TIME ? 62 : 120, CurPopup == POP_TIME ? 158 : 132, PopEnabled);
      break;
    case B_POP_HUP: PopTimeMin = (PopTimeMin + 60) % 1440; RedrawPopup(); break;
    case B_POP_HDN: PopTimeMin = (PopTimeMin + 1380) % 1440; RedrawPopup(); break;
    case B_POP_MUP: PopTimeMin = (PopTimeMin / 5 * 5 + 5) % 1440; RedrawPopup(); break;
    case B_POP_MDN: PopTimeMin = (PopTimeMin / 5 * 5 + 1435) % 1440; RedrawPopup(); break;
    case B_POP_AMPM: PopTimeMin = (PopTimeMin + 720) % 1440; RedrawPopup(); break;
    case B_POP_OK:
      if (CurPopup == POP_MINUTES) {
        cfg.zone[PopZone].minutes = PopMinutes;
        cfg.zone[PopZone].enabled = PopEnabled;
      } else if (CurPopup == POP_TIME) {
        cfg.startMin[PopStart] = PopTimeMin;
        cfg.startEn[PopStart] = PopEnabled;
      } else if (CurPopup == POP_CONFIRM_WINTER) {
        CurPopup = POP_NONE;
        if (cfg.winter) ExitWinter();
        else EnterWinter();
      }
      MarkConfigDirty();
      UpdateSmartPreview();
      ClosePopup();
      break;

    // wifi + keyboard
    case B_RESCAN:
      NetCount = 0;
      DrawScreen();
      break;
    case B_PGUP:
      if (NetPage > 0) NetPage--;
      DrawScreen();
      break;
    case B_PGDN:
      if ((NetPage + 1) * 5 < NetCount) NetPage++;
      DrawScreen();
      break;
    case B_SKIP:
      if (WifiOK) StopAP();
      GoScreen(SCR_HOME);
      break;
    case B_SHIFT:
      KbLayout = KbLayout == 1 ? 0 : 1;
      DrawScreen();
      break;
    case B_SYM:
      KbLayout = KbLayout == 2 ? 0 : 2;
      DrawScreen();
      break;
    case B_SPACE: KbType(' '); break;
    case B_DEL:
      if (strlen(KbText)) KbText[strlen(KbText) - 1] = 0;
      DrawKbField();
      break;
    case B_KBSHOW:
      KbShow = !KbShow;
      DrawScreen();
      break;
    case B_KBCANCEL: GoScreen(SCR_WIFI); break;
    case B_KBOK: TryConnect(KbTarget.c_str(), KbText); break;
  }
}

// ---------------------------------------------------------------------------------
// screen saver: clock (or IP / running zone) bouncing around, touch to wake
// ---------------------------------------------------------------------------------

#define SAVER_W 184
#define SAVER_H 60
#define SAVER_PAD 3  // black border, larger than the step so the trail erases itself

GFXcanvas16 *SaverCanvas = nullptr;
float SaverX = 40, SaverY = 60, SaverDX = 1.6, SaverDY = 1.1;
uint8_t SaverHue = 0;
uint32_t SaverFrameMs = 0;

const uint16_t SAVER_COLORS[] = { C_ACCENT, C_GOOD, C_SUN, C_ICE, RGB(0xd5, 0x51, 0x81), RGB(0x90, 0x85, 0xe9), C_WHITE };

void ServiceScreenSaver() {
  if (CurScreen == SCR_CAL || CurScreen == SCR_KEYBOARD || CurScreen == SCR_WIFI) return;
  if (!SaverOn) {
    if (cfg.saverSec > 0 && millis() - LastTouchMs > (uint32_t)cfg.saverSec * 1000UL) {
      if (!SaverCanvas) SaverCanvas = new GFXcanvas16(SAVER_W, SAVER_H);
      SaverOn = true;
      CurPopup = POP_NONE;
      Display.fillScreen(C_BLACK);
      SetBacklight(cfg.brightness / 4);
    }
    return;
  }
  if (millis() - SaverFrameMs < 33 || !SaverCanvas || !SaverCanvas->getBuffer()) return;
  SaverFrameMs = millis();

  SaverX += SaverDX;
  SaverY += SaverDY;
  bool bounce = false;
  if (SaverX <= 0 || SaverX >= SCREEN_W - SAVER_W) {
    SaverDX = -SaverDX;
    SaverX = constrain(SaverX, 0, SCREEN_W - SAVER_W);
    bounce = true;
  }
  if (SaverY <= 0 || SaverY >= SCREEN_H - SAVER_H) {
    SaverDY = -SaverDY;
    SaverY = constrain(SaverY, 0, SCREEN_H - SAVER_H);
    bounce = true;
  }
  if (bounce) SaverHue = (SaverHue + 1) % (sizeof(SAVER_COLORS) / 2);

  GFXcanvas16 &c = *SaverCanvas;
  char buf[32], t[12];
  c.fillScreen(C_BLACK);
  uint16_t col = SAVER_COLORS[SaverHue];
  if (TimeValid) FmtClock(time(nullptr), buf);
  else strcpy(buf, "--:--");
  TextAt(c, &FONT_BIG, (SAVER_W - TextWidth(&FONT_BIG, buf)) / 2, 32, col, buf);

  // second line alternates every 5 s
  int z = RunningZone();
  bool alt = (millis() / 5000) % 2;
  if (z >= 0) sprintf(buf, "Zone %d  %s", z + 1, FmtDur(ZoneSecondsLeft(z), t));
  else if (cfg.winter) strcpy(buf, "Winter mode");
  else if (!InternetOK) strcpy(buf, "No internet");
  else if (alt && WifiOK) strlcpy(buf, WiFi.localIP().toString().c_str(), sizeof(buf));
  else if (alt) strcpy(buf, "192.168.4.1");
  else if (TimeValid) {
    time_t now = time(nullptr);
    struct tm lt;
    localtime_r(&now, &lt);
    strftime(buf, sizeof(buf), "%a %b %d", &lt);
  } else buf[0] = 0;
  TextAt(c, &FONT_SMALL, (SAVER_W - TextWidth(&FONT_SMALL, buf)) / 2, 52, Blend(col, C_BLACK, 90), buf);
  for (int i = 0; i < SAVER_PAD; i++) c.drawRect(i, i, SAVER_W - 2 * i, SAVER_H - 2 * i, C_BLACK);

  Display.drawRGBBitmap((int)SaverX, (int)SaverY, c.getBuffer(), SAVER_W, SAVER_H);
}

void WakeUp() {
  SaverOn = false;
  WaitRelease = true;  // the waking touch is not a button press
  LastTouchMs = millis();
  SetBacklight(cfg.brightness);
  MapOnScreen = false;
  DrawScreen();
}
