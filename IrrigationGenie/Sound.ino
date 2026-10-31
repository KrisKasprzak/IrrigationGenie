/*
  IrrigationGenie - Sound.ino
  non-blocking piezo sound sequencer and backlight PWM (esp32 core 3.x LEDC API)
*/

// each sound is a list of notes ending with {0, 0}, freq 0 = rest
const Note SND_STARTUP[] = { { 523, 90 }, { 659, 90 }, { 784, 90 }, { 1047, 220 }, { 0, 0 } };
const Note SND_ZONE_ON[] = { { 880, 60 }, { 1175, 90 }, { 0, 0 } };
const Note SND_ZONE_OFF[] = { { 1175, 60 }, { 880, 90 }, { 0, 0 } };
const Note SND_WARNING[] = { { 440, 180 }, { 1, 80 }, { 440, 180 }, { 1, 80 }, { 330, 400 }, { 0, 0 } };
const Note SND_OK[] = { { 1319, 70 }, { 1568, 70 }, { 2093, 140 }, { 0, 0 } };
const Note SND_ERROR[] = { { 220, 250 }, { 1, 60 }, { 220, 250 }, { 0, 0 } };
const Note SND_WINTER[] = { { 1568, 90 }, { 1319, 90 }, { 1047, 90 }, { 784, 250 }, { 0, 0 } };

#define BL_FREQ 5000
#define BL_BITS 8

const Note *SoundPtr = nullptr;
uint32_t NoteEndMs = 0;

void SetupSound() {
  ledcAttach(PIN_BUZZER, 2000, 8);
  ledcWriteTone(PIN_BUZZER, 0);
}

void PlaySound(const Note *snd) {
  if (!cfg.sound && snd != SND_WARNING) {
    return;  // warnings always sound
  }
  SoundPtr = snd;
  NoteEndMs = 0;
  ServiceSound();
}

void StopSound() {
  SoundPtr = nullptr;
  NoteEndMs = 0;
  ledcWriteTone(PIN_BUZZER, 0);
}

// touch click: PWM on, wait CLICK_MS, off (set CLICK_HZ / CLICK_MS / CLICK_PWM in Config.h)
void Click() {
  if (!cfg.sound) return;
  StopSound();
  ledcChangeFrequency(PIN_BUZZER, CLICK_HZ, 8);  // tones change frequency and resolution, reset to 8 bit
  ledcWrite(PIN_BUZZER, CLICK_PWM);               // ESP32 equivalent of analogWrite(pin, PWM)
  delay(CLICK_MS);
  ledcWrite(PIN_BUZZER, 0);
}

void ServiceSound() {
  if (SoundPtr == nullptr) {
    return;
  }
  if (NoteEndMs != 0 && (int32_t)(millis() - NoteEndMs) < 0) {
    return;
  }
  if (NoteEndMs != 0) {
    SoundPtr++;
  }
  if (SoundPtr->ms == 0) {
    ledcWriteTone(PIN_BUZZER, 0);
    SoundPtr = nullptr;
    NoteEndMs = 0;
    return;
  }
  ledcWriteTone(PIN_BUZZER, SoundPtr->freq > 1 ? SoundPtr->freq : 0);
  NoteEndMs = millis() + SoundPtr->ms;
}

// backlight is driven through a 2N3906, the pin only feeds its base (default drive strength)
void SetupBacklight() {
  ledcAttach(PIN_LED, BL_FREQ, BL_BITS);
  SetBacklight(255);
}

// level 255 = full brightness (the core turns max duty into constant on, 0 into constant off)
void SetBacklight(uint8_t level) {
  if (level < 10) level = 10;
  ledcWrite(PIN_LED, BL_ACTIVE_LOW ? 255 - level : level);
}
