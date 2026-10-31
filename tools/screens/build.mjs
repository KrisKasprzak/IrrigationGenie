// IrrigationGenie TFT screenshot generator
//
// Compiles the real firmware UI code (Gfx.ino, UI.ino, Valves.ino, Sound.ino and parts of
// Storage.ino) together with Adafruit GFX against small stubs, using clang built for
// WebAssembly (no compiler install needed), runs it and writes PNGs of each screen.
//
// usage:  cd tools/screens && npm install && npm run tft
// output: docs/screenshots/tft-*.png

import { runClang } from '@yowasp/clang';
import { WASI } from 'node:wasi';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import zlib from 'node:zlib';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '..', '..');
const sketch = path.join(root, 'IrrigationGenie');
const gfxDir = process.env.GFX_DIR || 'C:/Arduino_Sketches/libraries/Adafruit_GFX_Library';
const outDir = path.join(root, 'docs', 'screenshots');
const SCALE = 2;

const read = f => fs.readFileSync(f, 'utf8');

// cut one top-level function (signature line to matching brace) out of a source file
function extractFunction(src, name) {
  const m = new RegExp(`^[\\w][\\w\\s\\*&:<>]*?\\b${name}\\s*\\([^;{]*\\)\\s*\\{`, 'm').exec(src);
  if (!m) throw new Error(`function ${name} not found`);
  let i = m.index + m[0].length, depth = 1;
  while (depth && i < src.length) {
    if (src[i] === '{') depth++;
    else if (src[i] === '}') depth--;
    i++;
  }
  return src.slice(m.index, i) + '\n';
}

// Arduino generates prototypes for .ino files, do the same so call order does not matter
function prototypes(src) {
  const re = /^((?:const\s+)?[A-Za-z_][\w:<>]*[\s\*&]+)([A-Za-z_]\w*)\s*\(([^;{}()]*)\)\s*\{\s*$/gm;
  const skip = new Set(['if', 'for', 'while', 'switch', 'return']);
  const out = [];
  let m;
  while ((m = re.exec(src))) {
    if (skip.has(m[2]) || m[1].trim() === 'else') continue;
    out.push(`${m[1].trim()} ${m[2]}(${m[3]});`);
  }
  return out.join('\n');
}

// ---- assemble one translation unit
const mainIno = read(path.join(sketch, 'IrrigationGenie.ino'));
const globals = mainIno.slice(mainIno.indexOf('#define FONT_BIG'), mainIno.indexOf('void setup()'));
const storage = read(path.join(sketch, 'Storage.ino'));
const storageParts = ['SetDefaults', 'DaysFromCivil', 'TodayNum', 'UsageSlot', 'DayGallons', 'UsageTotals']
  .map(n => extractFunction(storage, n)).join('\n');
const tabs = ['Gfx.ino', 'UI.ino', 'Valves.ino', 'Sound.ino'].map(f => `// ---- ${f}\n` + read(path.join(sketch, f))).join('\n');
const firmware = storageParts + tabs;

const unit = `
#include "Arduino.h"
#include "Devices.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "Config.h"
#include "Plan.h"

// fixed clock: Sat Oct 31 2026 10:42, shown as US Central daylight time
static time_t g_now = ${Date.UTC(2026, 9, 31, 10, 42) / 1000};
inline time_t mock_time(time_t *t) { if (t) *t = g_now; return g_now; }
inline struct tm *mock_localtime_r(const time_t *t, struct tm *r) { gmtime_r(t, r); r->tm_isdst = 1; return r; }
inline time_t mock_mktime(struct tm *t) { return timegm(t); }
inline size_t mock_strftime(char *b, size_t n, const char *f, const struct tm *t) {
  std::string s;
  for (const char *p = f; *p; p++) {
    if (p[0] == '%' && p[1] == 'Z') { s += "CDT"; p++; }
    else if (p[0] == '%' && p[1] == 'z') { s += "-0500"; p++; }
    else s += *p;
  }
  return strftime(b, n, s.c_str(), t);
}
#define time(x) mock_time(x)
#define localtime_r mock_localtime_r
#define mktime mock_mktime
#define strftime mock_strftime

${globals}

// prototypes
void MarkConfigDirty();
void SaveConfig();
void SaveUsage();
bool JoinNetwork(const char *ssid, const char *pass);
void StartAP();
void StopAP();
${prototypes(firmware)}

${firmware}

${read(path.join(here, 'scenes.cpp'))}
`;

const dir = (d, filter) => Object.fromEntries(fs.readdirSync(d).filter(filter).map(f => [f, read(path.join(d, f))]));
const tree = {
  'stubs': { ...dir(path.join(here, 'stubs'), f => f.endsWith('.h')), 'Adafruit_I2CDevice.h': '', 'Adafruit_SPIDevice.h': '' },
  'gfx': {
    'Adafruit_GFX.h': read(path.join(gfxDir, 'Adafruit_GFX.h')),
    'Adafruit_GFX.cpp': read(path.join(gfxDir, 'Adafruit_GFX.cpp')),
    'gfxfont.h': read(path.join(gfxDir, 'gfxfont.h')),
    'glcdfont.c': read(path.join(gfxDir, 'glcdfont.c')),
    'fonts': dir(path.join(gfxDir, 'Fonts'), f => /^FreeSans(9|12|18)pt7b\.h$/.test(f)),
  },
  'src': { 'Config.h': read(path.join(sketch, 'Config.h')), 'Plan.h': read(path.join(sketch, 'Plan.h')) },
  'main.cpp': unit,
};

console.log('compiling firmware UI to WebAssembly...');
const flags = ['-DARDUINO=100', '-std=c++17', '-O1', '-fno-exceptions', '-Wno-everything', '-Istubs', '-Igfx', '-Isrc'];
let result;
try {
  result = await runClang(['clang++', ...flags, 'main.cpp', 'gfx/Adafruit_GFX.cpp', '-o', 'screens.wasm'], tree, {
    stderr: b => b && process.stderr.write(Buffer.from(b)),
  });
} catch (e) {
  fs.writeFileSync(path.join(os.tmpdir(), 'irrigationgenie_screens.cpp'), unit);
  console.error(`compile failed (source saved to ${path.join(os.tmpdir(), 'irrigationgenie_screens.cpp')})`);
  process.exit(1);
}

// ---- run, frames land in a temp dir as raw RGB565
const rawDir = fs.mkdtempSync(path.join(os.tmpdir(), 'ig-screens-'));
const wasi = new WASI({ version: 'preview1', preopens: { '/out': rawDir } });
const instance = await WebAssembly.instantiate(await WebAssembly.compile(result['screens.wasm']), { wasi_snapshot_preview1: wasi.wasiImport });
wasi.start(instance);

// ---- RGB565 -> PNG
const crcT = Array.from({ length: 256 }, (_, n) => { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1; return c >>> 0; });
const crc = b => { let c = 0xffffffff; for (const v of b) c = crcT[(c ^ v) & 255] ^ (c >>> 8); return (c ^ 0xffffffff) >>> 0; };
const chunk = (t, d) => { const l = Buffer.alloc(4); l.writeUInt32BE(d.length); const td = Buffer.concat([Buffer.from(t), d]); const c = Buffer.alloc(4); c.writeUInt32BE(crc(td)); return Buffer.concat([l, td, c]); };
function png(raw, w, h, s) {
  const W = w * s, H = h * s, row = W * 3 + 1, data = Buffer.alloc(row * H);
  for (let y = 0; y < H; y++) for (let x = 0; x < W; x++) {
    const v = raw.readUInt16LE(((y / s | 0) * w + (x / s | 0)) * 2), o = y * row + 1 + x * 3;
    data[o] = (v >> 11) * 255 / 31; data[o + 1] = ((v >> 5) & 63) * 255 / 63; data[o + 2] = (v & 31) * 255 / 31;
  }
  const ihdr = Buffer.alloc(13); ihdr.writeUInt32BE(W, 0); ihdr.writeUInt32BE(H, 4); ihdr[8] = 8; ihdr[9] = 2;
  return Buffer.concat([Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]), chunk('IHDR', ihdr), chunk('IDAT', zlib.deflateSync(data, { level: 9 })), chunk('IEND', Buffer.alloc(0))]);
}

fs.mkdirSync(outDir, { recursive: true });
const frames = fs.readdirSync(rawDir).filter(f => f.endsWith('.raw')).map(f => ({ name: f.slice(0, -4), raw: fs.readFileSync(path.join(rawDir, f)) }));
for (const f of frames) fs.writeFileSync(path.join(outDir, f.name + '.png'), png(f.raw, 320, 240, SCALE));

// overview sheet, screens in the order the scenes rendered them
const order = read(path.join(here, 'scenes.cpp')).match(/Dump\("([^"]+)"\)/g).map(m => m.slice(6, -2));
const sheet = order.map(n => frames.find(f => f.name === n)).filter(Boolean);
const cols = 4, gap = 12, rows = Math.ceil(sheet.length / cols), SW = cols * 320 + (cols + 1) * gap, SH = rows * 240 + (rows + 1) * gap;
const big = Buffer.alloc(SW * SH * 2);
for (let i = 0; i < SW * SH; i++) big.writeUInt16LE(0x0841, i * 2);
sheet.forEach((f, i) => {
  const ox = gap + (i % cols) * (320 + gap), oy = gap + (i / cols | 0) * (240 + gap);
  for (let y = 0; y < 240; y++) f.raw.copy(big, ((oy + y) * SW + ox) * 2, y * 640, y * 640 + 640);
});
fs.writeFileSync(path.join(outDir, 'tft-overview.png'), png(big, SW, SH, 1));
fs.rmSync(rawDir, { recursive: true, force: true });
console.log(`screenshots written to ${path.relative(root, outDir)}`);
