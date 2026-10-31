/*
  IrrigationGenie - Storage.ino
  settings are kept as JSON in LittleFS, the same code feeds the phone API
  usage history is kept as a small binary ring buffer
*/


void SetDefaults() {

  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = MODE_SCHEDULE;
  cfg.winter = false;
  cfg.days = 0b0101010;  // Mon, Wed, Fri
  cfg.startEn[0] = true;
  cfg.startMin[0] = 6 * 60;
  cfg.startEn[1] = false;
  cfg.startMin[1] = 19 * 60;

  cfg.rainSkipMM = 6.0;
  cfg.freezeC = 2.0;
  cfg.windSkipKmh = 40.0;
  cfg.maxBoost = 160;
  cfg.minPct = 25;
  cfg.et0Ref = 5.0;

  cfg.manualMin = 15;
  cfg.maxRunMin = 120;
  cfg.drainMin = 20;
  cfg.brightness = 220;
  cfg.sound = true;
  cfg.metric = false;
  cfg.clock24 = false;
  cfg.allowMulti = false;
  cfg.saverSec = 60;
  cfg.costPer1000 = 5.0;
  cfg.rainDelayUntil = 0;

  cfg.hasLocation = false;
  cfg.calOK = false;

  for (int i = 0; i < NUM_ZONES; i++) {
    snprintf(cfg.zone[i].name, sizeof(cfg.zone[i].name), "Zone %d", i + 1);
    cfg.zone[i].enabled = true;
    cfg.zone[i].minutes = 10;
    cfg.zone[i].gpm = 3.0;
    // outlines and badges from the yard plan (Plan.h)
    cfg.zone[i].bx = PLAN_BADGE[i][0];
    cfg.zone[i].by = PLAN_BADGE[i][1];
    cfg.zone[i].npts = min((int)PLAN_ZONE[i].n, MAX_PTS);
    for (int p = 0; p < cfg.zone[i].npts; p++) {
      cfg.zone[i].px[p] = PLAN_ZONE[i].pts[p][0];
      cfg.zone[i].py[p] = PLAN_ZONE[i].pts[p][1];
    }
  }
}

// secrets = true writes wifi password and touch calibration (file only, never the API)
void ConfigToJson(JsonObject o, bool secrets) {

  o["mode"] = cfg.mode;
  o["winter"] = cfg.winter;
  o["days"] = cfg.days;
  JsonArray st = o["starts"].to<JsonArray>();
  for (int i = 0; i < 2; i++) {
    JsonObject s = st.add<JsonObject>();
    s["en"] = cfg.startEn[i];
    s["min"] = cfg.startMin[i];
  }

  o["rainSkip"] = cfg.rainSkipMM;
  o["freeze"] = cfg.freezeC;
  o["windSkip"] = cfg.windSkipKmh;
  o["maxBoost"] = cfg.maxBoost;
  o["minPct"] = cfg.minPct;
  o["et0Ref"] = cfg.et0Ref;

  o["manualMin"] = cfg.manualMin;
  o["maxRunMin"] = cfg.maxRunMin;
  o["drainMin"] = cfg.drainMin;
  o["bright"] = cfg.brightness;
  o["sound"] = cfg.sound;
  o["metric"] = cfg.metric;
  o["clock24"] = cfg.clock24;
  o["multi"] = cfg.allowMulti;
  o["saver"] = cfg.saverSec;
  o["cost"] = cfg.costPer1000;
  o["rainDelay"] = cfg.rainDelayUntil;

  o["hasLoc"] = cfg.hasLocation;
  o["locIP"] = cfg.locFromIP;
  o["lat"] = serialized(String(cfg.lat, 5));
  o["lon"] = serialized(String(cfg.lon, 5));
  o["place"] = cfg.place;
  o["tz"] = cfg.tzName;
  o["posix"] = cfg.posix;
  o["ssid"] = cfg.ssid;

  if (secrets) {
    o["pass"] = cfg.pass;
    o["calOK"] = cfg.calOK;
    JsonArray c = o["cal"].to<JsonArray>();
    for (int i = 0; i < 6; i++) {
      c.add(serialized(String(cfg.cal[i], 6)));
    }
  }

  JsonArray zs = o["zones"].to<JsonArray>();
  for (int i = 0; i < NUM_ZONES; i++) {
    ZoneCfg &z = cfg.zone[i];
    JsonObject j = zs.add<JsonObject>();
    j["name"] = z.name;
    j["en"] = z.enabled;
    j["min"] = z.minutes;
    j["gpm"] = z.gpm;
    j["bx"] = z.bx;
    j["by"] = z.by;
    JsonArray pts = j["pts"].to<JsonArray>();
    for (int p = 0; p < z.npts; p++) {
      JsonArray xy = pts.add<JsonArray>();
      xy.add(z.px[p]);
      xy.add(z.py[p]);
    }
  }
}

// only keys that are present are applied, so the phone can send partial updates
void ConfigFromJson(JsonObjectConst o, bool secrets) {

  if (o["mode"].is<int>()) cfg.mode = constrain(o["mode"].as<int>(), 0, 2);
  if (o["winter"].is<bool>()) cfg.winter = o["winter"];
  if (o["days"].is<int>()) cfg.days = o["days"].as<int>() & 0x7F;
  if (o["starts"].is<JsonArrayConst>()) {
    JsonArrayConst st = o["starts"];
    for (int i = 0; i < 2 && i < (int)st.size(); i++) {
      if (st[i]["en"].is<bool>()) cfg.startEn[i] = st[i]["en"];
      if (st[i]["min"].is<int>()) cfg.startMin[i] = constrain(st[i]["min"].as<int>(), 0, 1439);
    }
  }

  if (o["rainSkip"].is<float>()) cfg.rainSkipMM = constrain(o["rainSkip"].as<float>(), 1.0f, 50.0f);
  if (o["freeze"].is<float>()) cfg.freezeC = constrain(o["freeze"].as<float>(), -5.0f, 10.0f);
  if (o["windSkip"].is<float>()) cfg.windSkipKmh = constrain(o["windSkip"].as<float>(), 0.0f, 100.0f);
  if (o["maxBoost"].is<int>()) cfg.maxBoost = constrain(o["maxBoost"].as<int>(), 100, 250);
  if (o["minPct"].is<int>()) cfg.minPct = constrain(o["minPct"].as<int>(), 0, 80);
  if (o["et0Ref"].is<float>()) cfg.et0Ref = constrain(o["et0Ref"].as<float>(), 1.0f, 12.0f);

  if (o["manualMin"].is<int>()) cfg.manualMin = constrain(o["manualMin"].as<int>(), 1, 120);
  if (o["maxRunMin"].is<int>()) cfg.maxRunMin = constrain(o["maxRunMin"].as<int>(), 5, 240);
  if (o["drainMin"].is<int>()) cfg.drainMin = constrain(o["drainMin"].as<int>(), 0, 120);
  if (o["bright"].is<int>()) cfg.brightness = constrain(o["bright"].as<int>(), 10, 255);
  if (o["sound"].is<bool>()) cfg.sound = o["sound"];
  if (o["metric"].is<bool>()) cfg.metric = o["metric"];
  if (o["clock24"].is<bool>()) cfg.clock24 = o["clock24"];
  if (o["multi"].is<bool>()) cfg.allowMulti = o["multi"];
  if (o["saver"].is<int>()) cfg.saverSec = constrain(o["saver"].as<int>(), 0, 250);
  if (o["cost"].is<float>()) cfg.costPer1000 = o["cost"];
  if (o["rainDelay"].is<uint32_t>()) cfg.rainDelayUntil = o["rainDelay"];

  if (o["hasLoc"].is<bool>()) cfg.hasLocation = o["hasLoc"];
  if (o["locIP"].is<bool>()) cfg.locFromIP = o["locIP"];
  if (o["lat"].is<float>()) cfg.lat = o["lat"];
  if (o["lon"].is<float>()) cfg.lon = o["lon"];
  if (o["place"].is<const char *>()) strlcpy(cfg.place, o["place"], sizeof(cfg.place));
  if (o["tz"].is<const char *>()) strlcpy(cfg.tzName, o["tz"], sizeof(cfg.tzName));
  if (o["posix"].is<const char *>()) strlcpy(cfg.posix, o["posix"], sizeof(cfg.posix));

  if (secrets) {
    if (o["ssid"].is<const char *>()) strlcpy(cfg.ssid, o["ssid"], sizeof(cfg.ssid));
    if (o["pass"].is<const char *>()) strlcpy(cfg.pass, o["pass"], sizeof(cfg.pass));
    if (o["calOK"].is<bool>()) cfg.calOK = o["calOK"];
    if (o["cal"].is<JsonArrayConst>()) {
      for (int i = 0; i < 6; i++) {
        cfg.cal[i] = o["cal"][i] | 0.0f;
      }
    }
  }

  if (o["zones"].is<JsonArrayConst>()) {
    JsonArrayConst zs = o["zones"];
    for (int i = 0; i < NUM_ZONES && i < (int)zs.size(); i++) {
      JsonObjectConst j = zs[i];
      ZoneCfg &z = cfg.zone[i];
      if (j["name"].is<const char *>()) strlcpy(z.name, j["name"], sizeof(z.name));
      if (j["en"].is<bool>()) z.enabled = j["en"];
      if (j["min"].is<int>()) z.minutes = constrain(j["min"].as<int>(), 0, 120);
      if (j["gpm"].is<float>()) z.gpm = constrain(j["gpm"].as<float>(), 0.0f, 50.0f);
      if (j["bx"].is<int>()) z.bx = constrain(j["bx"].as<int>(), 0, 1000);
      if (j["by"].is<int>()) z.by = constrain(j["by"].as<int>(), 0, 1000);
      if (j["pts"].is<JsonArrayConst>()) {
        JsonArrayConst pts = j["pts"];
        z.npts = min((int)pts.size(), MAX_PTS);
        for (int p = 0; p < z.npts; p++) {
          z.px[p] = constrain(pts[p][0].as<int>(), 0, 1000);
          z.py[p] = constrain(pts[p][1].as<int>(), 0, 1000);
        }
      }
    }
  }
}

void LoadConfig() {

  SetDefaults();

  File f = LittleFS.open(CONFIG_FILE, "r");
  if (!f) {
    Serial.println(F("No config, using defaults"));
    return;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    Serial.print(F("Config parse error: "));
    Serial.println(err.c_str());
    return;
  }
  ConfigFromJson(doc.as<JsonObjectConst>(), true);
}

void SaveConfig() {

  JsonDocument doc;
  ConfigToJson(doc.to<JsonObject>(), true);
  File f = LittleFS.open(CONFIG_FILE, "w");
  if (!f) {
    Serial.println(F("Config save failed"));
    return;
  }
  serializeJson(doc, f);
  f.close();
#ifdef SHOW_DEBUG
  Serial.println(F("Config saved"));
#endif
}

// ---------------------------------------------------------------------------------
// usage history
// ---------------------------------------------------------------------------------

// days since 1970-01-01 for a civil date (Howard Hinnant's algorithm)
int32_t DaysFromCivil(int y, int m, int d) {
  y -= m <= 2;
  int32_t era = (y >= 0 ? y : y - 399) / 400;
  uint32_t yoe = (uint32_t)(y - era * 400);
  uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

int32_t TodayNum() {
  time_t now = time(nullptr);
  struct tm lt;
  localtime_r(&now, &lt);
  return DaysFromCivil(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
}

// slot for a day, cleared when a new day takes it over
UsageDay &UsageSlot(int32_t dayNum) {
  UsageDay &u = usage[((dayNum % USAGE_DAYS) + USAGE_DAYS) % USAGE_DAYS];
  if (u.dayNum != dayNum) {
    memset(&u, 0, sizeof(u));
    u.dayNum = dayNum;
    u.pct = 255;
  }
  return u;
}

void LoadUsage() {
  memset(usage, 0, sizeof(usage));
  for (int i = 0; i < USAGE_DAYS; i++) {
    usage[i].pct = 255;
  }
  File f = LittleFS.open(USAGE_FILE, "r");
  if (!f) {
    return;
  }
  if (f.size() == sizeof(usage)) {
    f.read((uint8_t *)usage, sizeof(usage));
  }
  f.close();
}

void SaveUsage() {
  File f = LittleFS.open(USAGE_FILE, "w");
  if (!f) {
    return;
  }
  f.write((const uint8_t *)usage, sizeof(usage));
  f.close();
  UsageDirty = false;
}

// called each second, adds on-time of running zones to today's slot
void AccumulateUsage() {
  if (!TimeValid) {
    return;
  }
  bool any = false;
  for (int i = 0; i < NUM_ZONES; i++) {
    if (run[i].on && run[i].source != SRC_DRAIN) {
      any = true;
      break;
    }
  }
  if (!any) {
    return;
  }
  UsageDay &u = UsageSlot(TodayNum());
  for (int i = 0; i < NUM_ZONES; i++) {
    if (run[i].on && run[i].source != SRC_DRAIN) {
      u.sec[i]++;
      if (run[i].source == SRC_PROGRAM) {
        u.progGal += cfg.zone[i].gpm / 60.0f;
      }
    }
  }
  UsageDirty = true;
}

float DayGallons(const UsageDay &u) {
  float g = 0;
  for (int i = 0; i < NUM_ZONES; i++) {
    g += u.sec[i] / 60.0f * cfg.zone[i].gpm;
  }
  return g;
}

// fills totals over the last 30 days
void UsageTotals(float &gal, uint32_t &sec, float &saved) {
  gal = 0;
  sec = 0;
  saved = 0;
  int32_t today = TimeValid ? TodayNum() : 0;
  for (int i = 0; i < USAGE_DAYS; i++) {
    UsageDay &u = usage[i];
    if (u.dayNum <= 0 || today - u.dayNum >= USAGE_DAYS || u.dayNum > today) {
      continue;
    }
    gal += DayGallons(u);
    for (int z = 0; z < NUM_ZONES; z++) {
      sec += u.sec[z];
    }
    if (u.pct != 255) {
      saved += u.baseGal - u.progGal;
    }
  }
}
