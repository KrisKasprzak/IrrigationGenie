/*
  IrrigationGenie - Net.ino
  WiFi, setup access point, time / time zone, weather (Open-Meteo), web server + API
*/

#define OPEN_METEO "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f" \
                   "&current=temperature_2m,relative_humidity_2m,precipitation,weather_code,wind_speed_10m" \
                   "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_sum," \
                   "precipitation_probability_max,et0_fao_evapotranspiration,wind_speed_10m_max," \
                   "relative_humidity_2m_mean,sunrise,sunset" \
                   "&timezone=auto&past_days=2&forecast_days=4&timeformat=unixtime"

#define IP_LOCATE "http://ip-api.com/json/?fields=status,city,regionName,countryCode,lat,lon,timezone,offset"

// fallback IANA -> POSIX rules, used only when the phone has not sent a POSIX string
struct TZRule {
  const char *iana;
  const char *posix;
};
const TZRule TZ_TABLE[] = {
  { "America/New_York", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Detroit", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Indiana/Indianapolis", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Kentucky/Louisville", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Toronto", "EST5EDT,M3.2.0,M11.1.0" },
  { "America/Chicago", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Winnipeg", "CST6CDT,M3.2.0,M11.1.0" },
  { "America/Mexico_City", "CST6" },
  { "America/Denver", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Boise", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Edmonton", "MST7MDT,M3.2.0,M11.1.0" },
  { "America/Phoenix", "MST7" },
  { "America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Vancouver", "PST8PDT,M3.2.0,M11.1.0" },
  { "America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0" },
  { "Pacific/Honolulu", "HST10" },
  { "America/Halifax", "AST4ADT,M3.2.0,M11.1.0" },
  { "America/Puerto_Rico", "AST4" },
  { "America/Sao_Paulo", "<-03>3" },
  { "Europe/London", "GMT0BST,M3.5.0/1,M10.5.0" },
  { "Europe/Dublin", "IST-1GMT0,M10.5.0,M3.5.0/1" },
  { "Europe/Lisbon", "WET0WEST,M3.5.0/1,M10.5.0" },
  { "Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Madrid", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Vienna", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Zurich", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Warsaw", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Stockholm", "CET-1CEST,M3.5.0,M10.5.0/3" },
  { "Europe/Athens", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Helsinki", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
  { "Europe/Moscow", "MSK-3" },
  { "Asia/Kolkata", "IST-5:30" },
  { "Asia/Shanghai", "CST-8" },
  { "Asia/Tokyo", "JST-9" },
  { "Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { "Australia/Melbourne", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
  { "Australia/Brisbane", "AEST-10" },
  { "Australia/Perth", "AWST-8" },
  { "Pacific/Auckland", "NZST-12NZDT,M9.5.0,M4.1.0/3" },
};

// POSIX string for a zone name, falls back to a fixed UTC offset (no DST)
void PosixFor(const char *iana, int32_t offsetSec, char *out, size_t len) {
  for (const TZRule &r : TZ_TABLE) {
    if (strcmp(r.iana, iana) == 0) {
      strlcpy(out, r.posix, len);
      return;
    }
  }
  // e.g. UTC-5 -> "<-05>5", UTC+5:30 -> "<+0530>-5:30" (POSIX sign is reversed)
  int32_t m = offsetSec / 60;
  char name = m >= 0 ? '+' : '-';
  const char *sign = m > 0 ? "-" : "";
  int h = abs(m) / 60, mm = abs(m) % 60;
  if (mm) {
    snprintf(out, len, "<%c%02d%02d>%s%d:%02d", name, h, mm, sign, h, mm);
  } else {
    snprintf(out, len, "<%c%02d>%s%d", name, h, sign, h);
  }
}

// ---------------------------------------------------------------------------------
// WiFi
// ---------------------------------------------------------------------------------

bool ConnectWiFi(uint32_t timeoutMs) {
  WiFi.persistent(false);
  WiFi.mode(APMode ? WIFI_AP_STA : WIFI_STA);
  WiFi.setHostname(MDNS_NAME);
  WiFi.setAutoReconnect(true);
  WiFi.begin(cfg.ssid, cfg.pass);
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < timeoutMs) {
    delay(100);
    ServiceSound();
  }
  WifiOK = WiFi.status() == WL_CONNECTED;
  if (WifiOK) {
    MDNS.end();
    MDNS.begin(MDNS_NAME);
    MDNS.addService("http", "tcp", 80);
    Serial.print(F("Connected, IP "));
    Serial.println(WiFi.localIP());
  }
  return WifiOK;
}

// access point so the phone can also do the WiFi setup (http://192.168.4.1)
void StartAP() {
  APMode = true;
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID);
  Serial.print(F("Setup AP started, IP "));
  Serial.println(WiFi.softAPIP());
}

void StopAP() {
  if (!APMode) return;
  APMode = false;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
}

// called when new credentials arrive (touch keyboard or phone)
bool JoinNetwork(const char *ssid, const char *pass) {
  strlcpy(cfg.ssid, ssid, sizeof(cfg.ssid));
  strlcpy(cfg.pass, pass, sizeof(cfg.pass));
  WiFi.disconnect();
  bool ok = ConnectWiFi(15000);
  if (ok) {
    SaveConfig();
    StopAP();
    StartTime();
    if (!cfg.hasLocation) NetWantIPLocate = true;
    RequestWeather();
    PlaySound(SND_OK);
  } else {
    PlaySound(SND_ERROR);
  }
  return ok;
}

// connection watchdog, warns once when the internet goes away
void CheckWiFi() {
  bool ok = WiFi.status() == WL_CONNECTED;
  if (ok != WifiOK) {
    WifiOK = ok;
    if (ok) {
      MDNS.end();
      MDNS.begin(MDNS_NAME);
      MDNS.addService("http", "tcp", 80);
      RequestWeather();
    }
    ScreenChanged();
  }

  bool inet = WifiOK && NetFailCount < 3;
  if (APMode && !WifiOK) inet = true;  // no warnings while being set up
  if (!inet) {
    if (WifiLostMs == 0) WifiLostMs = millis();
    if (InternetOK && millis() - WifiLostMs > WIFI_LOST_WARN_SEC * 1000UL) {
      InternetOK = false;
      PlaySound(SND_WARNING);
      ScreenChanged();
    }
  } else {
    WifiLostMs = 0;
    if (!InternetOK) {
      InternetOK = true;
      PlaySound(SND_OK);
      ScreenChanged();
    }
  }
}

// ---------------------------------------------------------------------------------
// time: SNTP keeps the ESP32 clock, POSIX TZ string handles the offset and DST
// ---------------------------------------------------------------------------------

void ApplyTimeZone() {
  if (strlen(cfg.posix) == 0) return;
  setenv("TZ", cfg.posix, 1);
  tzset();
}

// sync now (startup and every midnight), SNTP also keeps syncing in the background
void StartTime() {
  if (!WifiOK) return;
  configTzTime(strlen(cfg.posix) ? cfg.posix : "UTC0", NTP_SERVER1, NTP_SERVER2);
  struct tm lt;
  getLocalTime(&lt, 5000);
  TimeValid = time(nullptr) > 1700000000;
  if (TimeValid) {
    LastDay = lt.tm_mday;
    LastMinute = lt.tm_min;
  }
}

// ---------------------------------------------------------------------------------
// network task on core 0 so the touch screen stays responsive during downloads
// ---------------------------------------------------------------------------------

void RequestWeather() {
  WeatherTimer = millis();
  if (!cfg.hasLocation) return;
  NetLat = cfg.lat;
  NetLon = cfg.lon;
  NetWantWeather = true;
}

void StartNetTask() {
  xTaskCreatePinnedToCore(NetTask, "net", 16384, nullptr, 1, nullptr, 0);
}

void NetTask(void *param) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      if (NetWantIPLocate && !NetLocateReady) {
        NetWantIPLocate = false;
        NetBusy = true;
        FetchIPLocation();
        NetBusy = false;
      }
      if (NetWantWeather && !NetWeatherReady) {
        NetWantWeather = false;
        NetBusy = true;
        bool ok = FetchWeather(NetLat, NetLon, NetWx);
        NetFailCount = ok ? 0 : NetFailCount + 1;
        if (ok) NetWeatherReady = true;
        NetBusy = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

bool FetchWeather(float lat, float lon, Weather &w) {

  char url[512];
  snprintf(url, sizeof(url), OPEN_METEO, lat, lon);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(8000);
  // HTTP/1.0 so the reply is not chunked, getStream() hands raw bytes to the JSON parser
  http.useHTTP10(true);
  if (!http.begin(client, url)) return false;
  int code = http.GET();
  if (code != 200) {
    Serial.printf("Weather HTTP %d\n", code);
    http.end();
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) {
    Serial.print(F("Weather JSON: "));
    Serial.println(err.c_str());
    return false;
  }

  memset(&w, 0, sizeof(w));
  w.utcOffset = doc["utc_offset_seconds"] | 0;
  strlcpy(w.tzName, doc["timezone"] | "", sizeof(w.tzName));
  JsonObject c = doc["current"];
  w.curTemp = c["temperature_2m"] | 0.0f;
  w.curRH = c["relative_humidity_2m"] | 0.0f;
  w.curPrecip = c["precipitation"] | 0.0f;
  w.curWind = c["wind_speed_10m"] | 0.0f;
  w.curCode = c["weather_code"] | 0;

  JsonObject d = doc["daily"];
  for (int i = 0; i < WX_DAYS; i++) {
    WxDay &x = w.day[i];
    x.date = d["time"][i] | 0;
    x.tmax = d["temperature_2m_max"][i] | 0.0f;
    x.tmin = d["temperature_2m_min"][i] | 0.0f;
    x.rain = d["precipitation_sum"][i] | 0.0f;
    x.pop = d["precipitation_probability_max"][i] | 0;
    x.et0 = d["et0_fao_evapotranspiration"][i] | 0.0f;
    x.wind = d["wind_speed_10m_max"][i] | 0.0f;
    x.rh = d["relative_humidity_2m_mean"][i] | 0;
    x.code = d["weather_code"][i] | 0;
    x.sunrise = d["sunrise"][i] | 0;
    x.sunset = d["sunset"][i] | 0;
  }
  w.fetched = time(nullptr);
  w.valid = w.day[WX_TODAY].date != 0;
  return w.valid;
}

void FetchIPLocation() {
  HTTPClient http;
  http.setTimeout(6000);
  http.useHTTP10(true);
  if (!http.begin(IP_LOCATE)) return;
  int code = http.GET();
  if (code == 200) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getStream()) && strcmp(doc["status"] | "", "success") == 0) {
      NetLoc.lat = doc["lat"];
      NetLoc.lon = doc["lon"];
      snprintf(NetLoc.place, sizeof(NetLoc.place), "%s, %s, %s", doc["city"] | "", doc["regionName"] | "", doc["countryCode"] | "");
      strlcpy(NetLoc.tzName, doc["timezone"] | "UTC", sizeof(NetLoc.tzName));
      NetLoc.offset = doc["offset"] | 0;
      NetLoc.ok = true;
      NetLocateReady = true;
    }
  }
  http.end();
}

// hand results from the net task to the main loop
void ServiceNetResults() {

  if (NetLocateReady) {
    if (NetLoc.ok) {
      cfg.lat = NetLoc.lat;
      cfg.lon = NetLoc.lon;
      strlcpy(cfg.place, NetLoc.place, sizeof(cfg.place));
      strlcpy(cfg.tzName, NetLoc.tzName, sizeof(cfg.tzName));
      PosixFor(cfg.tzName, NetLoc.offset, cfg.posix, sizeof(cfg.posix));
      cfg.hasLocation = true;
      cfg.locFromIP = true;
      MarkConfigDirty();
      StartTime();
      RequestWeather();
    }
    NetLocateReady = false;
  }

  if (NetWeatherReady) {
    memcpy(&wx, &NetWx, sizeof(wx));
    NetWeatherReady = false;
    WxStamp++;

    // weather knows the zone of the location; fill in a time zone if we never got one
    if (strlen(cfg.posix) == 0 && strlen(wx.tzName) > 0) {
      strlcpy(cfg.tzName, wx.tzName, sizeof(cfg.tzName));
      PosixFor(cfg.tzName, wx.utcOffset, cfg.posix, sizeof(cfg.posix));
      MarkConfigDirty();
      StartTime();
    }
    UpdateSmartPreview();
    ScreenChanged();
  }
}

// ---------------------------------------------------------------------------------
// web server
// ---------------------------------------------------------------------------------

File UploadFile;
bool UploadOK = true;  // cleared by any failed part of a multipart upload

void StartWebServer() {
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-cache");
    server.send_P(200, "text/html", WEB_PAGE);
  });
  server.on("/api/state", HTTP_GET, ApiState);
  server.on("/api/config", HTTP_GET, ApiGetConfig);
  server.on("/api/config", HTTP_POST, ApiPostConfig);
  server.on("/api/weather", HTTP_GET, ApiWeather);
  server.on("/api/usage", HTTP_GET, ApiUsage);
  server.on("/api/cmd", HTTP_POST, ApiCommand);
  server.on("/api/location", HTTP_POST, ApiLocation);
  server.on("/api/wifi", HTTP_POST, ApiWifi);
  server.on("/api/scan", HTTP_GET, ApiScan);
  server.on("/api/map", HTTP_POST, ApiMapDone, ApiMapUpload);
  server.on("/api/map", HTTP_DELETE, ApiMapDelete);
  server.on("/map.jpg", HTTP_GET, []() {
    if (!LittleFS.exists(MAP_JPG)) {
      server.send(404, "text/plain", "no map");
      return;
    }
    File f = LittleFS.open(MAP_JPG, "r");
    server.sendHeader("Cache-Control", "no-cache");
    server.streamFile(f, "image/jpeg");
    f.close();
  });
  server.onNotFound([]() {
    // anything else goes to the app (helps phones that probe for captive portals)
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
}

void SendJson(JsonDocument &doc) {
  String out;
  serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "application/json", out);
}

void SendOK() {
  server.send(200, "application/json", "{\"ok\":true}");
}

bool ReadBody(JsonDocument &doc) {
  if (!server.hasArg("plain") || deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"bad json\"}");
    return false;
  }
  return true;
}

void ApiState() {
  JsonDocument doc;
  time_t now = time(nullptr);
  char buf[32];
  doc["t"] = (uint32_t)now;
  doc["tv"] = TimeValid;
  if (TimeValid) {
    doc["time"] = FmtClock(now, buf);
    struct tm lt;
    localtime_r(&now, &lt);
    strftime(buf, sizeof(buf), "%a %b %d", &lt);
    doc["date"] = buf;
    strftime(buf, sizeof(buf), "%Z", &lt);
    doc["tzAbbr"] = buf;
    doc["dst"] = lt.tm_isdst > 0;
    long off = 0;
    strftime(buf, sizeof(buf), "%z", &lt);
    off = atol(buf);
    doc["utcOff"] = (off / 100) * 60 + (off % 100);
  }
  doc["mode"] = cfg.mode;
  doc["winter"] = cfg.winter;
  doc["drain"] = Draining;
  doc["drainLeft"] = DrainSecondsLeft();
  doc["master"] = MasterOn;
  doc["masterManual"] = MasterManual;
  JsonArray z = doc["z"].to<JsonArray>();
  for (int i = 0; i < NUM_ZONES; i++) {
    JsonObject o = z.add<JsonObject>();
    o["on"] = run[i].on;
    o["left"] = ZoneSecondsLeft(i);
    o["tot"] = run[i].durMs / 1000;
    o["src"] = run[i].source;
  }
  doc["prog"] = prog.active;
  doc["progPct"] = prog.pct;
  JsonObject s = doc["smart"].to<JsonObject>();
  s["pct"] = smart.pct;
  s["skip"] = smart.skip;
  s["reason"] = smart.reason;
  time_t nr = NextRun();
  doc["next"] = (uint32_t)nr;
  if (nr) doc["nextStr"] = FmtNextRun(nr, buf);
  doc["rainDelay"] = RainDelayActive() ? cfg.rainDelayUntil - (uint32_t)now : 0;
  doc["wifi"] = WifiOK;
  doc["inet"] = InternetOK;
  doc["ap"] = APMode;
  doc["rssi"] = WiFi.RSSI();
  doc["ssid"] = WiFi.SSID();
  doc["ip"] = WifiOK ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  doc["up"] = (millis() - BootMs) / 1000;
  doc["heap"] = ESP.getFreeHeap();
  doc["fw"] = CODE_VERSION;
  doc["cfgStamp"] = ConfigStamp;
  doc["wxStamp"] = WxStamp;
  doc["mapStamp"] = MapStamp;
  doc["hasMap"] = LittleFS.exists(MAP_JPG);
  SendJson(doc);
}

void AddPlanPoly(JsonObject o, const char *key, const PlanPoly &p) {
  JsonArray a = o[key].to<JsonArray>();
  for (int i = 0; i < p.n; i++) {
    JsonArray xy = a.add<JsonArray>();
    xy.add(p.pts[i][0]);
    xy.add(p.pts[i][1]);
  }
}

void ApiGetConfig() {
  JsonDocument doc;
  ConfigToJson(doc.to<JsonObject>(), false);
  // fixed yard plan (house, driveway, property line) so the phone draws the same map
  JsonObject plan = doc["plan"].to<JsonObject>();
  AddPlanPoly(plan, "property", PLAN_PROPERTY);
  AddPlanPoly(plan, "house", PLAN_HOUSE);
  AddPlanPoly(plan, "drive", PLAN_DRIVE);
  JsonArray hl = plan["houseLabel"].to<JsonArray>();
  hl.add(PLAN_HOUSE_LABEL[0]);
  hl.add(PLAN_HOUSE_LABEL[1]);
  JsonArray dl = plan["driveLabel"].to<JsonArray>();
  dl.add(PLAN_DRIVE_LABEL[0]);
  dl.add(PLAN_DRIVE_LABEL[1]);
  SendJson(doc);
}

void ApiPostConfig() {
  JsonDocument doc;
  if (!ReadBody(doc)) return;
  bool wasWinter = cfg.winter;
  uint8_t bright = cfg.brightness;
  ConfigFromJson(doc.as<JsonObjectConst>(), false);

  // winter mode must go through the drain sequence
  if (cfg.winter != wasWinter) {
    bool want = cfg.winter;
    cfg.winter = wasWinter;
    if (want) EnterWinter();
    else ExitWinter();
  }
  if (bright != cfg.brightness) SetBacklight(cfg.brightness);
  MarkConfigDirty();
  UpdateSmartPreview();
  ScreenChanged();
  SendOK();
}

void ApiWeather() {
  JsonDocument doc;
  doc["valid"] = wx.valid;
  doc["fetched"] = wx.fetched;
  doc["temp"] = wx.curTemp;
  doc["rh"] = wx.curRH;
  doc["wind"] = wx.curWind;
  doc["code"] = wx.curCode;
  doc["today"] = WX_TODAY;
  JsonArray a = doc["days"].to<JsonArray>();
  for (int i = 0; i < WX_DAYS; i++) {
    WxDay &d = wx.day[i];
    JsonObject o = a.add<JsonObject>();
    o["date"] = d.date;
    o["hi"] = d.tmax;
    o["lo"] = d.tmin;
    o["rain"] = d.rain;
    o["pop"] = d.pop;
    o["et0"] = d.et0;
    o["wind"] = d.wind;
    o["rh"] = d.rh;
    o["code"] = d.code;
    o["rise"] = d.sunrise;
    o["set"] = d.sunset;
  }
  JsonObject s = doc["smart"].to<JsonObject>();
  s["pct"] = smart.pct;
  s["skip"] = smart.skip;
  s["reason"] = smart.reason;
  s["et"] = smart.et;
  s["rainPast"] = smart.rainPast;
  s["rainFcst"] = smart.rainFcst;
  SendJson(doc);
}

void ApiUsage() {
  JsonDocument doc;
  int32_t today = TimeValid ? TodayNum() : 0;
  doc["today"] = today;
  JsonArray a = doc["days"].to<JsonArray>();
  for (int i = USAGE_DAYS - 1; i >= 0; i--) {
    int32_t dn = today - i;
    UsageDay &u = usage[((dn % USAGE_DAYS) + USAGE_DAYS) % USAGE_DAYS];
    JsonObject o = a.add<JsonObject>();
    o["day"] = dn;
    JsonArray s = o["sec"].to<JsonArray>();
    bool valid = u.dayNum == dn;
    for (int z = 0; z < NUM_ZONES; z++) {
      s.add(valid ? u.sec[z] : 0);
    }
    o["base"] = valid ? u.baseGal : 0;
    o["prog"] = valid ? u.progGal : 0;
    o["pct"] = valid ? u.pct : 255;
  }
  SendJson(doc);
}

void ApiCommand() {
  JsonDocument doc;
  if (!ReadBody(doc)) return;
  const char *cmd = doc["cmd"] | "";

  if (!strcmp(cmd, "zone")) {
    int z = (doc["zone"] | 0) - 1;
    if (doc["on"] | false) {
      if (!StartZone(z, (uint32_t)(doc["min"] | (int)cfg.manualMin) * 60, SRC_MANUAL)) {
        server.send(409, "application/json", "{\"ok\":false,\"err\":\"winter mode\"}");
        return;
      }
    } else {
      StopZone(z);
    }
  } else if (!strcmp(cmd, "stopall")) {
    StopAll();
  } else if (!strcmp(cmd, "master")) {
    SetMasterManual(doc["on"] | false);
  } else if (!strcmp(cmd, "mode")) {
    cfg.mode = constrain(doc["mode"] | 0, 0, 2);
    MarkConfigDirty();
    UpdateSmartPreview();
    ScreenChanged();
  } else if (!strcmp(cmd, "winter")) {
    if (doc["on"] | false) EnterWinter();
    else ExitWinter();
  } else if (!strcmp(cmd, "runnow")) {
    if (cfg.mode == MODE_SMART) {
      ComputeSmart(WX_TODAY, smart);
      StartProgram(smart.skip ? 100 : smart.pct);
    } else {
      StartProgram(100);
    }
    ScreenChanged();
  } else if (!strcmp(cmd, "raindelay")) {
    int h = doc["hours"] | 0;
    cfg.rainDelayUntil = h > 0 ? (uint32_t)time(nullptr) + h * 3600UL : 0;
    MarkConfigDirty();
    ScreenChanged();
  } else if (!strcmp(cmd, "refresh")) {
    RequestWeather();
  } else if (!strcmp(cmd, "locateip")) {
    NetWantIPLocate = true;
  } else if (!strcmp(cmd, "reboot")) {
    SendOK();
    SaveConfig();
    SaveUsage();
    delay(300);
    ESP.restart();
  } else {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"unknown cmd\"}");
    return;
  }
  SendOK();
}

// phone sends GPS / searched location, IANA zone and the POSIX rule it computed
void ApiLocation() {
  JsonDocument doc;
  if (!ReadBody(doc)) return;
  if (!doc["lat"].is<float>() || !doc["lon"].is<float>()) {
    server.send(400, "application/json", "{\"ok\":false}");
    return;
  }
  cfg.lat = doc["lat"];
  cfg.lon = doc["lon"];
  strlcpy(cfg.place, doc["place"] | "", sizeof(cfg.place));
  strlcpy(cfg.tzName, doc["tz"] | "", sizeof(cfg.tzName));
  const char *px = doc["posix"] | "";
  if (strlen(px) > 0) {
    strlcpy(cfg.posix, px, sizeof(cfg.posix));
  } else {
    PosixFor(cfg.tzName, (int32_t)(doc["offset"] | 0) * 60, cfg.posix, sizeof(cfg.posix));
  }
  cfg.hasLocation = true;
  cfg.locFromIP = false;
  MarkConfigDirty();
  ApplyTimeZone();
  StartTime();
  RequestWeather();
  ScreenChanged();
  SendOK();
}

void ApiWifi() {
  JsonDocument doc;
  if (!ReadBody(doc)) return;
  SendOK();
  delay(200);
  JoinNetwork(doc["ssid"] | "", doc["pass"] | "");
  if (WifiOK) GoScreen(SCR_HOME);
}

void ApiScan() {
  JsonDocument doc;
  int n = WiFi.scanNetworks();
  JsonArray a = doc.to<JsonArray>();
  for (int i = 0; i < n && i < 20; i++) {
    JsonObject o = a.add<JsonObject>();
    o["ssid"] = WiFi.SSID(i);
    o["rssi"] = WiFi.RSSI(i);
    o["lock"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
  SendJson(doc);
}

// multipart upload: field "rgb" = raw RGB565 for the TFT, field "jpg" = photo for the phone
void ApiMapUpload() {
  HTTPUpload &up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    const char *path = up.name == "rgb" ? MAP_FILE ".tmp" : MAP_JPG ".tmp";
    UploadFile = LittleFS.open(path, "w");
    if (!UploadFile) UploadOK = false;
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (UploadFile) UploadFile.write(up.buf, up.currentSize);
  } else if (up.status == UPLOAD_FILE_END) {
    if (UploadFile) {
      String path = UploadFile.path();
      size_t size = UploadFile.size();
      UploadFile.close();
      String dest = path.substring(0, path.length() - 4);
      if (dest == MAP_FILE && size != (size_t)MAP_W * MAP_H * 2) {
        LittleFS.remove(path);
        UploadOK = false;
        return;
      }
      LittleFS.remove(dest);
      LittleFS.rename(path, dest);
    }
  } else if (up.status == UPLOAD_FILE_ABORTED) {
    if (UploadFile) UploadFile.close();
    UploadOK = false;
  }
}

void ApiMapDone() {
  if (!UploadOK) {
    UploadOK = true;
    server.send(500, "application/json", "{\"ok\":false,\"err\":\"upload failed\"}");
    return;
  }
  MapStamp++;
  ScreenChanged();
  SendOK();
}

void ApiMapDelete() {
  LittleFS.remove(MAP_FILE);
  LittleFS.remove(MAP_JPG);
  MapStamp++;
  ScreenChanged();
  SendOK();
}
