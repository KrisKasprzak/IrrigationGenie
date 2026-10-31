/*
  IrrigationGenie - Valves.ino
  master + zone outputs, program runner, schedule, winter mode, intelligent watering
*/

#define ZONE_GAP_MS 3000  // pause between zones in a program, master stays open

void SetupOutputs() {
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    // set the level first so the relay never blips on during boot
    digitalWrite(OUTPUT_PINS[i], RELAY_ACTIVE_LOW ? HIGH : LOW);
    pinMode(OUTPUT_PINS[i], OUTPUT);
    OutputState[i] = false;
  }
}

void WriteOutput(uint8_t idx, bool on) {
  if (OutputState[idx] == on) {
    return;
  }
  OutputState[idx] = on;
  digitalWrite(OUTPUT_PINS[idx], (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
#ifdef SHOW_DEBUG
  Serial.printf("Output %d %s\n", idx, on ? "ON" : "OFF");
#endif
}

bool AnyZoneOn() {
  for (int i = 0; i < NUM_ZONES; i++) {
    if (run[i].on) return true;
  }
  return false;
}

int RunningZone() {
  for (int i = 0; i < NUM_ZONES; i++) {
    if (run[i].on) return i;
  }
  return -1;
}

uint32_t ZoneSecondsLeft(int z) {
  if (!run[z].on || run[z].durMs == 0) {
    return 0;
  }
  uint32_t el = millis() - run[z].startMs;
  return el >= run[z].durMs ? 0 : (run[z].durMs - el + 999) / 1000;
}

// zone 0..6, seconds of run time
bool StartZone(int z, uint32_t secs, uint8_t source) {
  if (z < 0 || z >= NUM_ZONES) return false;
  if (cfg.winter && source != SRC_DRAIN) return false;

  uint32_t maxSec = (uint32_t)cfg.maxRunMin * 60;
  if (source != SRC_DRAIN && (secs == 0 || secs > maxSec)) {
    secs = min(secs == 0 ? (uint32_t)cfg.manualMin * 60 : secs, maxSec);
  }

  // one zone at a time unless allowed, a manual start takes over from a program
  if (source == SRC_MANUAL && !cfg.allowMulti) {
    prog.active = false;
    for (int i = 0; i < NUM_ZONES; i++) {
      if (i != z) run[i].on = false;
    }
  }

  run[z].on = true;
  run[z].source = source;
  run[z].startMs = millis();
  run[z].durMs = secs * 1000UL;
  if (source != SRC_DRAIN) {
    PlaySound(SND_ZONE_ON);
  }
  ApplyOutputs();
  ScreenChanged();
  return true;
}

void StopZone(int z) {
  if (z < 0 || z >= NUM_ZONES || !run[z].on) return;
  run[z].on = false;
  if (prog.active && prog.current == z) {
    prog.current = 255;
    prog.gapUntil = millis() + ZONE_GAP_MS;
  }
  PlaySound(SND_ZONE_OFF);
  ApplyOutputs();
  SaveUsage();
  ScreenChanged();
}

void StopAll() {
  prog.active = false;
  MasterManual = false;
  bool wasOn = AnyZoneOn();
  for (int i = 0; i < NUM_ZONES; i++) {
    run[i].on = false;
  }
  if (Draining) {
    Draining = false;
  }
  ApplyOutputs();
  if (wasOn) {
    PlaySound(SND_ZONE_OFF);
    SaveUsage();
  }
  ScreenChanged();
}

void SetMasterManual(bool on) {
  if (cfg.winter && on) return;
  MasterManual = on;
  MasterManualMs = millis();
  ApplyOutputs();
  ScreenChanged();
}

// master is open whenever a zone is open, except while draining in winter mode
void ApplyOutputs() {
  bool zoneOn = AnyZoneOn();
  bool master = (zoneOn || MasterManual || (prog.active && prog.current == 255)) && !cfg.winter;
  MasterOn = master;
  WriteOutput(0, master);
  for (int i = 0; i < NUM_ZONES; i++) {
    WriteOutput(i + 1, run[i].on);
  }
}

// called every loop: timers, program sequencing, safety cutoffs
void ServiceValves() {

  bool changed = false;
  uint32_t now = millis();

  for (int i = 0; i < NUM_ZONES; i++) {
    if (run[i].on && run[i].durMs > 0 && now - run[i].startMs >= run[i].durMs) {
      run[i].on = false;
      changed = true;
      if (run[i].source == SRC_PROGRAM && prog.active && prog.current == i) {
        prog.current = 255;
        prog.gapUntil = now + ZONE_GAP_MS;
      }
      if (run[i].source != SRC_DRAIN) {
        PlaySound(SND_ZONE_OFF);
      }
    }
  }

  // winter drain finished, close everything and stay in winter mode
  if (Draining && !AnyZoneOn()) {
    Draining = false;
    changed = true;
  }

  // master opened by hand also obeys the safety cutoff
  if (MasterManual && now - MasterManualMs >= (uint32_t)cfg.maxRunMin * 60000UL) {
    MasterManual = false;
    changed = true;
  }

  if (prog.active && prog.current == 255 && (int32_t)(now - prog.gapUntil) >= 0) {
    // next zone in the program
    int next = -1;
    for (int i = 0; i < NUM_ZONES; i++) {
      if (prog.secs[i] > 0) {
        next = i;
        break;
      }
    }
    if (next < 0) {
      prog.active = false;
      PlaySound(SND_OK);
    } else {
      prog.current = next;
      uint16_t s = prog.secs[next];
      prog.secs[next] = 0;
      StartZone(next, s, SRC_PROGRAM);
    }
    changed = true;
  }

  if (changed) {
    ApplyOutputs();
    SaveUsage();
    ScreenChanged();
  }
}

// seconds a zone runs in a program at pct, 0 = skipped
uint32_t ProgramSecs(int i, uint8_t pct) {
  ZoneCfg &z = cfg.zone[i];
  if (!z.enabled || z.minutes == 0) return 0;
  uint32_t s = (uint32_t)z.minutes * 60 * pct / 100;
  s = min(s, (uint32_t)cfg.maxRunMin * 60);
  return s >= 30 ? s : 0;  // anything under 30 seconds is not worth opening a valve
}

// "1-2-6-7", the zones the next program will water
char *NextZonesStr(uint8_t pct, char *buf) {
  buf[0] = 0;
  for (int i = 0; i < NUM_ZONES; i++) {
    if (ProgramSecs(i, pct) == 0) continue;
    if (buf[0]) strcat(buf, "-");
    char n[2] = { (char)('1' + i), 0 };
    strcat(buf, n);
  }
  return buf;
}

// pct = 100 runs the baseline minutes
void StartProgram(uint8_t pct) {
  if (cfg.winter) return;

  // a program replaces anything that is running
  for (int i = 0; i < NUM_ZONES; i++) {
    run[i].on = false;
  }
  memset(&prog, 0, sizeof(prog));
  float baseGal = 0;
  bool any = false;
  for (int i = 0; i < NUM_ZONES; i++) {
    ZoneCfg &z = cfg.zone[i];
    if (z.enabled) baseGal += z.minutes * z.gpm;
    prog.secs[i] = ProgramSecs(i, pct);
    if (prog.secs[i]) any = true;
  }

  if (TimeValid) {
    UsageDay &u = UsageSlot(TodayNum());
    u.baseGal += baseGal;
    u.pct = pct;
    UsageDirty = true;
  }

  if (!any) {
    ApplyOutputs();
    ScreenChanged();
    return;
  }
  prog.active = true;
  prog.pct = pct;
  prog.current = 255;
  prog.gapUntil = millis();
#ifdef SHOW_DEBUG
  Serial.printf("Program start %d%%\n", pct);
#endif
}

// record a skipped program so usage shows the savings
void LogSkip() {
  if (!TimeValid) return;
  UsageDay &u = UsageSlot(TodayNum());
  for (int i = 0; i < NUM_ZONES; i++) {
    if (cfg.zone[i].enabled) u.baseGal += cfg.zone[i].minutes * cfg.zone[i].gpm;
  }
  u.pct = 0;
  UsageDirty = true;
}

bool RainDelayActive() {
  return TimeValid && cfg.rainDelayUntil > (uint32_t)time(nullptr);
}

// called once a minute with local time
void CheckSchedule(struct tm &lt) {
  if (cfg.mode == MODE_MANUAL || cfg.winter) return;
  if (!(cfg.days & (1 << lt.tm_wday))) return;

  uint16_t nowMin = lt.tm_hour * 60 + lt.tm_min;
  for (int k = 0; k < 2; k++) {
    if (!cfg.startEn[k] || cfg.startMin[k] != nowMin) continue;

    uint32_t key = (uint32_t)lt.tm_yday * 1440 + nowMin + 1;
    if (key == LastTriggerKey) return;
    LastTriggerKey = key;

    if (RainDelayActive()) {
      LogSkip();
      return;
    }
    if (cfg.mode == MODE_SCHEDULE) {
      StartProgram(100);
    } else {
      ComputeSmart(WX_TODAY, smart);
      if (smart.skip) {
        LogSkip();
        PlaySound(SND_OK);
      } else {
        StartProgram(smart.pct);
      }
    }
    ScreenChanged();
    return;
  }
}

void EnterWinter() {
  StopAll();
  cfg.winter = true;
  MarkConfigDirty();
  // master stays closed, every zone opens to drain the lines
  for (int i = 0; i < NUM_ZONES; i++) {
    StartZone(i, (uint32_t)cfg.drainMin * 60, SRC_DRAIN);
  }
  Draining = true;
  DrainStartMs = millis();
  ApplyOutputs();
  PlaySound(SND_WINTER);
  ScreenChanged();
}

void ExitWinter() {
  for (int i = 0; i < NUM_ZONES; i++) {
    run[i].on = false;
  }
  Draining = false;
  cfg.winter = false;
  MarkConfigDirty();
  ApplyOutputs();
  PlaySound(SND_OK);
  ScreenChanged();
}

uint32_t DrainSecondsLeft() {
  if (!Draining || cfg.drainMin == 0) return 0;
  uint32_t el = (millis() - DrainStartMs) / 1000;
  uint32_t tot = (uint32_t)cfg.drainMin * 60;
  return el >= tot ? 0 : tot - el;
}

// next scheduled start as epoch, 0 = none
time_t NextRun() {
  if (!TimeValid || cfg.mode == MODE_MANUAL || cfg.winter || cfg.days == 0) return 0;
  if (!cfg.startEn[0] && !cfg.startEn[1]) return 0;

  time_t now = time(nullptr);
  time_t from = now;
  if (RainDelayActive()) from = cfg.rainDelayUntil;

  struct tm lt;
  localtime_r(&from, &lt);
  for (int d = 0; d < 8; d++) {
    struct tm day = lt;
    day.tm_mday += d;
    day.tm_hour = 0;
    day.tm_min = 0;
    day.tm_sec = 0;
    day.tm_isdst = -1;
    time_t mid = mktime(&day);
    struct tm chk;
    localtime_r(&mid, &chk);
    if (!(cfg.days & (1 << chk.tm_wday))) continue;

    time_t best = 0;
    for (int k = 0; k < 2; k++) {
      if (!cfg.startEn[k]) continue;
      struct tm s = chk;
      s.tm_hour = cfg.startMin[k] / 60;
      s.tm_min = cfg.startMin[k] % 60;
      s.tm_isdst = -1;
      time_t t = mktime(&s);
      if (t > from && (best == 0 || t < best)) best = t;
    }
    if (best) return best;
  }
  return 0;
}

// weather index for an epoch, -1 if outside the forecast
int WxIndexFor(time_t t) {
  if (!wx.valid) return -1;
  for (int i = 0; i < WX_DAYS; i++) {
    if ((uint32_t)t >= wx.day[i].date && (uint32_t)t < wx.day[i].date + 86400UL) return i;
  }
  return -1;
}

// ---------------------------------------------------------------------------------
// intelligent watering
//
// the baseline minutes are what the yard needs on a "reference" day
// (et0Ref, default 5 mm/day = hot, sunny summer day). each run is scaled by:
//   1. evapotranspiration (FAO-56 ET0 from Open-Meteo: temperature, humidity,
//      wind and solar radiation in one number), blended yesterday/today
//   2. rain credit: effective rain of the last 2 days + expected rain today
//      (amount x probability) + half of tomorrow's expected rain
//   3. heat wave and very dry air nudges
// and skipped for freeze risk, heavy rain, strong wind or too little demand
// ---------------------------------------------------------------------------------

float EffectiveRain(float mm) {
  // light rain mostly evaporates before it reaches the roots
  return mm < 1.0f ? 0.0f : mm * 0.85f;
}

void ComputeSmart(int d, SmartResult &r) {

  memset(&r, 0, sizeof(r));
  bool stale = !wx.valid || (TimeValid && (uint32_t)time(nullptr) - wx.fetched > 12UL * 3600UL);
  if (stale || d < 1 || d >= WX_DAYS) {
    r.pct = 100;
    strlcpy(r.reason, "No weather data, using schedule", sizeof(r.reason));
    return;
  }

  WxDay &day = wx.day[d];
  WxDay &prev = wx.day[d - 1];

  r.et = 0.35f * prev.et0 + 0.65f * day.et0;
  r.rainPast = EffectiveRain(prev.rain) + (d >= 2 ? 0.5f * EffectiveRain(wx.day[d - 2].rain) : 0);
  r.rainFcst = day.rain * day.pop / 100.0f;
  if (d + 1 < WX_DAYS) {
    r.rainFcst += 0.5f * wx.day[d + 1].rain * wx.day[d + 1].pop / 100.0f;
  }
  float credit = r.rainPast + r.rainFcst;

  char buf[24];

  // freeze protection
  float low = day.tmin;
  if (d == WX_TODAY && wx.curTemp < low) low = wx.curTemp;
  if (low <= cfg.freezeC) {
    r.skip = true;
    snprintf(r.reason, sizeof(r.reason), "Freeze risk, low %s", FmtTemp(low, buf));
    return;
  }

  if (credit >= cfg.rainSkipMM) {
    r.skip = true;
    snprintf(r.reason, sizeof(r.reason), "Rain skip, %s rain", FmtRain(credit, buf));
    return;
  }

  if (day.pop >= 70 && day.rain >= cfg.rainSkipMM * 0.5f) {
    r.skip = true;
    snprintf(r.reason, sizeof(r.reason), "Rain likely today (%d%%)", day.pop);
    return;
  }

  if (cfg.windSkipKmh > 0 && day.wind >= cfg.windSkipKmh) {
    r.skip = true;
    snprintf(r.reason, sizeof(r.reason), "Too windy, %s", FmtWind(day.wind, buf));
    return;
  }

  float pct = r.et / cfg.et0Ref * 100.0f;
  pct *= 1.0f - credit / cfg.rainSkipMM;

  const char *why;
  if (day.tmax >= 35.0f) {
    pct *= 1.10f;
    why = "Heat wave";
  } else if (day.rh > 0 && day.rh < 30) {
    pct *= 1.05f;
    why = "Very dry air";
  } else if (credit >= 1.0f) {
    why = "Rain credit";
  } else if (r.et >= cfg.et0Ref) {
    why = "Hot & dry";
  } else if (r.et < cfg.et0Ref * 0.6f) {
    why = "Cool, low demand";
  } else {
    why = "Mild";
  }

  pct = constrain(pct, 0.0f, (float)cfg.maxBoost);
  r.pct = (uint8_t)(pct + 0.5f);

  if (r.pct < cfg.minPct) {
    r.skip = true;
    // name what pushed it under the minimum: rain credit or just cool weather
    if (credit >= 1.0f) snprintf(r.reason, sizeof(r.reason), "Rain skip, %s rain", FmtRain(credit, buf));
    else snprintf(r.reason, sizeof(r.reason), "Low demand, ET %s", FmtRain(r.et, buf));
    return;
  }
  snprintf(r.reason, sizeof(r.reason), "%s, ET %s", why, FmtRain(r.et, buf));
}

// irrigation status of a forecast day (weather index): what the scheduler will do that day
DayPlan DayStatus(int idx) {
  if (cfg.mode == MODE_MANUAL || cfg.winter || idx < 1 || idx >= WX_DAYS) return PLAN_OFF;
  if (!cfg.startEn[0] && !cfg.startEn[1]) return PLAN_OFF;
  char zs[16];
  if (!NextZonesStr(100, zs)[0]) return PLAN_OFF;  // every zone off

  time_t noon = wx.day[idx].date + 43200;
  struct tm lt;
  localtime_r(&noon, &lt);
  if (!(cfg.days & (1 << lt.tm_wday))) return PLAN_OFF;  // not a watering day

  // rain delay covering that day's first start time
  uint16_t first = 1440;
  for (int k = 0; k < 2; k++) {
    if (cfg.startEn[k]) first = min(first, cfg.startMin[k]);
  }
  if (RainDelayActive() && wx.day[idx].date + first * 60UL < cfg.rainDelayUntil) return PLAN_SKIP;

  if (cfg.mode == MODE_SMART) {
    SmartResult r;
    ComputeSmart(idx, r);
    return r.skip ? PLAN_SKIP : PLAN_RUN;
  }
  return PLAN_RUN;
}

// smart result for the next run (or today) for display
void UpdateSmartPreview() {
  int d = WX_TODAY;
  time_t nr = NextRun();
  if (nr) {
    int i = WxIndexFor(nr);
    if (i >= 1) d = i;
  }
  ComputeSmart(d, smart);
}
