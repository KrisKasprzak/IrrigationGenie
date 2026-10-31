// IrrigationGenie screenshot scenes. Appended after the real firmware UI code by build.mjs.
// Each scene sets up state, calls the firmware's own drawing functions and dumps the frame.

uint32_t g_millis = 1000000;
SerialStub Serial;
WiFiClass WiFi;
FSStub LittleFS;

// firmware functions that live in tabs the harness does not compile
void MarkConfigDirty() {}
void SaveConfig() {}
void SaveUsage() {}
bool JoinNetwork(const char *, const char *) { return true; }
void StartAP() { APMode = true; }
void StopAP() { APMode = false; }

static void Dump(const char *name) {
  char path[64];
  snprintf(path, sizeof(path), "/out/%s.raw", name);
  FILE *f = fopen(path, "wb");
  fwrite(Display.getBuffer(), 2, SCREEN_W * SCREEN_H, f);
  fclose(f);
  printf("rendered %s\n", name);
}

static void SetupState() {
  SetDefaults();
  const uint8_t mins[NUM_ZONES] = { 12, 15, 10, 8, 6, 10, 5 };
  for (int i = 0; i < NUM_ZONES; i++) cfg.zone[i].minutes = mins[i];
  cfg.mode = MODE_SMART;
  cfg.calOK = true;
  cfg.hasLocation = true;
  cfg.lat = 39.7817f;
  cfg.lon = -89.6501f;
  strcpy(cfg.place, "Springfield, IL, US");
  strcpy(cfg.tzName, "America/Chicago");
  strcpy(cfg.posix, "CST6CDT,M3.2.0,M11.1.0");

  TimeValid = WifiOK = InternetOK = true;
  BootMs = 0;
  g_millis = 3 * 86400000UL + 5 * 3600000UL + 17 * 60000UL;
  LastTouchMs = g_millis;

  // weather: 2 past days, today, 3 forecast days (local midnight epochs)
  time_t today = g_now - g_now % 86400;
  const float hi[WX_DAYS] = { 29.1f, 30.4f, 31.2f, 30.0f, 33.4f, 26.6f };
  const float lo[WX_DAYS] = { 17.2f, 18.0f, 19.4f, 16.1f, 14.0f, 15.8f };
  const float rain[WX_DAYS] = { 0, 0.4f, 0, 0, 0, 6.5f };
  const uint8_t pop[WX_DAYS] = { 0, 10, 15, 20, 10, 80 };
  const float et0[WX_DAYS] = { 4.6f, 4.9f, 5.2f, 4.8f, 5.6f, 2.4f };
  const uint8_t code[WX_DAYS] = { 0, 1, 2, 2, 0, 63 };
  wx.valid = true;
  wx.fetched = g_now - 600;
  wx.curTemp = 26.4f;
  wx.curRH = 41;
  wx.curWind = 11;
  wx.curCode = 1;
  for (int i = 0; i < WX_DAYS; i++) {
    WxDay &d = wx.day[i];
    d.date = today + (i - WX_TODAY) * 86400;
    d.tmax = hi[i];
    d.tmin = lo[i];
    d.rain = rain[i];
    d.pop = pop[i];
    d.et0 = et0[i];
    d.wind = 14;
    d.rh = 48;
    d.code = code[i];
    d.sunrise = d.date + 7 * 3600 + 2 * 60;
    d.sunset = d.date + 17 * 3600 + 51 * 60;
  }

  // 30 days of usage: program every other day, a couple of skipped days
  int32_t t = TodayNum();
  for (int i = 0; i < USAGE_DAYS; i++) {
    int32_t dn = t - i;
    UsageDay &u = UsageSlot(dn);
    if (i % 2 == 1) {
      uint8_t pct = 70 + (i * 13) % 60;
      float base = 0, prog = 0;
      for (int z = 0; z < NUM_ZONES; z++) {
        u.sec[z] = cfg.zone[z].minutes * 60 * pct / 100;
        base += cfg.zone[z].minutes * cfg.zone[z].gpm;
        prog += u.sec[z] / 60.0f * cfg.zone[z].gpm;
      }
      u.baseGal = base;
      u.progGal = prog;
      u.pct = pct;
    } else if (i == 6 || i == 14) {
      for (int z = 0; z < NUM_ZONES; z++) u.baseGal += cfg.zone[z].minutes * cfg.zone[z].gpm;
      u.pct = 0;
    } else if (i == 10) {
      u.sec[1] = 900;  // a manual run
    }
  }
  UpdateSmartPreview();
}

int main() {
  SetupState();

  DrawSplash();
  SplashProgress(70, "Getting internet time");
  Dump("tft-splash");

  GoScreen(SCR_HOME);
  Dump("tft-home");

  GoScreen(SCR_SMART);
  Dump("tft-smart");

  // zone 3 running from the program
  prog.active = true;
  prog.pct = smart.pct;
  prog.current = 2;
  run[2].on = true;
  run[2].source = SRC_PROGRAM;
  run[2].durMs = 600000;
  run[2].startMs = g_millis - 337000;
  ApplyOutputs();
  GoScreen(SCR_HOME);
  Dump("tft-home-watering");

  Pulse = true;
  GoScreen(SCR_ZONES);
  Dump("tft-zones");

  PopZone = 2;
  PopMinutes = 15;
  OpenPopup(POP_ZONE);
  Dump("tft-zone-popup");

  memset(&prog, 0, sizeof(prog));
  memset(run, 0, sizeof(run));
  ApplyOutputs();

  GoScreen(SCR_SCHED);
  Dump("tft-schedule");

  PopStart = 0;
  PopTimeMin = cfg.startMin[0];
  PopEnabled = true;
  OpenPopup(POP_TIME);
  Dump("tft-time-popup");

  GoScreen(SCR_USAGE);
  SelBar = 28;
  DrawScreen();
  Dump("tft-usage");

  GoScreen(SCR_INFO);
  Dump("tft-info");

  GoScreen(SCR_SETTINGS);
  Dump("tft-settings");

  OpenPopup(POP_CONFIRM_WINTER);
  Dump("tft-winter-popup");

  GoScreen(SCR_WIFI);
  Dump("tft-wifi");

  KbTarget = "HomeNetwork";
  strcpy(KbText, "garden");
  KbShow = true;
  GoScreen(SCR_KEYBOARD);
  Dump("tft-keyboard");

  // screen saver: idle long enough, then one frame
  GoScreen(SCR_HOME);
  g_millis += cfg.saverSec * 1000UL + 10;
  ServiceScreenSaver();
  SaverX = 92;
  SaverY = 118;
  g_millis += 40;
  ServiceScreenSaver();
  Dump("tft-saver");
  return 0;
}
