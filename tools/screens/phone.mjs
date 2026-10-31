// IrrigationGenie phone app screenshots
//
// Serves the real page from IrrigationGenie/WebPage.h with a mock controller API
// (zone outlines from Plan.h, same sample data as the TFT shots) and captures each tab
// in a headless Chromium at phone size.
//
// usage:  cd tools/screens && npm install
//         npx @puppeteer/browsers install chrome-headless-shell@stable   (prints the exe path)
//         set CHROME_PATH=<that path> && npm run phone
// output: docs/screenshots/phone-*.png

import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import puppeteer from 'puppeteer-core';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '..', '..');
const outDir = path.join(root, 'docs', 'screenshots');
const chrome = process.env.CHROME_PATH;
if (!chrome) {
  console.error('set CHROME_PATH to a Chrome / chrome-headless-shell executable');
  process.exit(1);
}

const page = fs.readFileSync(path.join(root, 'IrrigationGenie', 'WebPage.h'), 'utf8').split('R"rawliteral(')[1].split(')rawliteral"')[0];

// ---- yard plan from Plan.h
const planH = fs.readFileSync(path.join(root, 'IrrigationGenie', 'Plan.h'), 'utf8');
const pts = s => [...s.matchAll(/\{ (-?\d+), (-?\d+) \}/g)].map(m => [+m[1], +m[2]]);
const poly = name => pts(planH.match(new RegExp(`${name} = \\{ \\d+, \\{(.*?)\\} \\};`))[1]);
const pair = name => planH.match(new RegExp(`${name}\\[2\\] = \\{ (\\d+), (\\d+) \\}`)).slice(1).map(Number);
const zonePolys = planH.split('PLAN_ZONE[NUM_ZONES] = {')[1].split('};')[0].split('\n').filter(l => l.includes('{ {')).map(pts);
const badges = pts(planH.split('PLAN_BADGE[NUM_ZONES][2] = {')[1].split(';')[0]);
const plan = { property: poly('PLAN_PROPERTY'), house: poly('PLAN_HOUSE'), drive: poly('PLAN_DRIVE'), houseLabel: pair('PLAN_HOUSE_LABEL'), driveLabel: pair('PLAN_DRIVE_LABEL') };

// ---- sample controller data (matches tools/screens/scenes.cpp)
const mins = [12, 15, 10, 8, 6, 10, 5];
const zones = zonePolys.map((p, i) => ({ name: `Zone ${i + 1}`, en: true, min: mins[i], gpm: 3, bx: badges[i][0], by: badges[i][1], pts: p }));
const cfg = {
  mode: 2, winter: false, days: 42, starts: [{ en: true, min: 360 }, { en: false, min: 1140 }],
  rainSkip: 6, freeze: 2, windSkip: 40, maxBoost: 160, minPct: 25, et0Ref: 5,
  manualMin: 15, maxRunMin: 120, drainMin: 20, bright: 220, sound: true, metric: false, clock24: false, multi: false,
  saver: 60, cost: 5, rainDelay: 0, hasLoc: true, locIP: false, lat: 39.7817, lon: -89.6501,
  place: 'Springfield, IL, US', tz: 'America/Chicago', posix: 'CST6CDT,M3.2.0,M11.1.0', ssid: 'HomeNetwork', zones, plan,
};
const now = Date.UTC(2026, 9, 31, 15, 42) / 1000, today = Date.UTC(2026, 9, 31, 5) / 1000;
const state = (running) => ({
  t: now, tv: true, time: '10:42 AM', date: 'Sat Oct 31', tzAbbr: 'CDT', dst: true, utcOff: -300,
  mode: 2, winter: false, drain: false, drainLeft: 0, master: running, masterManual: false,
  z: zones.map((_, i) => ({ on: running && i === 2, left: running && i === 2 ? 263 : 0, tot: running && i === 2 ? 600 : 0, src: 2 })),
  prog: running, progPct: 60, smart: { pct: 60, skip: false, reason: 'Rain credit, ET 0.21 in' },
  next: now + 156000, nextStr: 'Mon 6:00 AM', rainDelay: 0, wifi: true, inet: true, ap: false, rssi: -48, ssid: 'HomeNetwork',
  ip: '192.168.1.50', up: 278220, heap: 182000, fw: '1.0', cfgStamp: 3, wxStamp: 1, mapStamp: 1, hasMap: false,
});
const hi = [29.1, 30.4, 31.2, 30.0, 33.4, 26.6], lo = [17.2, 18.0, 19.4, 16.1, 14.0, 15.8], rain = [0, 0.4, 0, 0, 0, 6.5];
const pop = [0, 10, 15, 20, 10, 80], et0 = [4.6, 4.9, 5.2, 4.8, 5.6, 2.4], code = [0, 1, 2, 2, 0, 63];
const weather = {
  valid: true, fetched: now - 600, temp: 26.4, rh: 41, wind: 11, code: 1, today: 2,
  days: hi.map((_, i) => { const d = today + (i - 2) * 86400; return { date: d, hi: hi[i], lo: lo[i], rain: rain[i], pop: pop[i], et0: et0[i], wind: 14, rh: 48, code: code[i], rise: d + 7.03 * 3600, set: d + 17.85 * 3600 }; }),
  smart: { pct: 60, skip: false, reason: 'Rain credit, ET 0.21 in', et: 5.3, rainPast: 0, rainFcst: 2.9 },
};
const dayNum = Math.floor(today / 86400);
const usage = {
  today: dayNum,
  days: [...Array(30)].map((_, k) => {
    const i = 29 - k, pct = 70 + (i * 13) % 60, ran = i % 2 === 1;
    const sec = mins.map((m, z) => ran ? Math.round(m * 60 * pct / 100) : (i === 10 && z === 1 ? 900 : 0));
    const base = mins.reduce((a, m) => a + m * 3, 0);
    return { day: dayNum - i, sec, base: ran || i === 6 || i === 14 ? base : 0, prog: ran ? sec.reduce((a, s) => a + s / 20, 0) : 0, pct: ran ? pct : (i === 6 || i === 14 ? 0 : 255) };
  }),
};

// ---- mock controller
let running = false;
const server = http.createServer((req, res) => {
  const p = req.url.split('?')[0];
  const json = o => { res.writeHead(200, { 'Content-Type': 'application/json' }); res.end(JSON.stringify(o)); };
  if (p === '/') { res.writeHead(200, { 'Content-Type': 'text/html' }); res.end(page); }
  else if (p === '/api/state') json(state(running));
  else if (p === '/api/config') json(cfg);
  else if (p === '/api/weather') json(weather);
  else if (p === '/api/usage') json(usage);
  else json({ ok: true });
});
await new Promise(r => server.listen(0, '127.0.0.1', r));
const url = `http://127.0.0.1:${server.address().port}/`;

// ---- capture
const browser = await puppeteer.launch({ executablePath: chrome, headless: 'shell', args: ['--no-sandbox'] });
const tab = await browser.newPage();
await tab.setViewport({ width: 390, height: 844, deviceScaleFactor: 2, isMobile: true, hasTouch: true });
await tab.emulateTimezone('America/Chicago');
fs.mkdirSync(outDir, { recursive: true });
const wait = ms => new Promise(r => setTimeout(r, ms));
const shot = async name => { await wait(700); await tab.screenshot({ path: path.join(outDir, `${name}.png`) }); console.log('captured', name); };
const go = async v => { await tab.evaluate(v => { go(v); window.scrollTo(0, 0); }, v); await wait(400); };

await tab.goto(url, { waitUntil: 'networkidle0' });
await shot('phone-home');

running = true;
await wait(2300);  // next poll picks up the running zone
await shot('phone-home-watering');

await go('map');
await shot('phone-map');

await tab.click('#mapBox .badge[data-z="2"]');
await shot('phone-zone-sheet');
await tab.evaluate(() => closeSheet());
running = false;

await go('sched');
await shot('phone-schedule');

await go('usage');
await wait(500);
// hover yesterday's bar to show the tooltip
await tab.evaluate(() => document.querySelector('#chartWrap g[data-i="28"] .hit').dispatchEvent(new PointerEvent('pointerdown', { bubbles: true })));
await shot('phone-usage');

await go('set');
await tab.evaluate(() => { const el = [...document.querySelectorAll('#v-set h3')].find(h => h.textContent.includes('Location')); el.closest('.card').scrollIntoView(); window.scrollBy(0, -70); });
await shot('phone-settings');

await browser.close();
server.close();
console.log(`phone screenshots written to ${path.relative(root, outDir)}`);
