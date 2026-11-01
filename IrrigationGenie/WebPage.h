/*
  IrrigationGenie - WebPage.h
  phone web app, served from flash at http://irrigationgenie.local
  single file, no external libraries (weather icons and charts are inline SVG)
*/

#pragma once

const char WEB_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b1426">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<title>IrrigationGenie</title>
<style>
:root{--bg:#0b1426;--card:rgba(22,35,61,.72);--card2:rgba(33,51,82,.85);--line:rgba(148,178,220,.14);--text:#eef4fb;--sub:#9fb3c8;--muted:#6b7f96;
--acc:#22d3ee;--acc2:#0e7490;--good:#22c55e;--warn:#f59e0b;--bad:#ef4444;--ice:#7dd3fc;--r:22px;color-scheme:dark}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;min-height:100%;background:var(--bg);color:var(--text);font:15px/1.4 -apple-system,BlinkMacSystemFont,"SF Pro Text","Segoe UI",Inter,Roboto,sans-serif}
body{background:radial-gradient(1200px 600px at 10% -10%,#0e4a63 0,transparent 55%),radial-gradient(900px 500px at 110% 10%,#1e2f6b 0,transparent 50%),var(--bg);background-attachment:fixed}
button,input,select{font:inherit;color:inherit}
button{cursor:pointer;border:0;background:none}
.hidden{display:none!important}
header.top{position:sticky;top:0;z-index:20;display:flex;align-items:center;gap:10px;padding:calc(10px + env(safe-area-inset-top)) 18px 10px;background:rgba(11,20,38,.6);backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);border-bottom:1px solid var(--line)}
.logo{display:flex;align-items:center;gap:8px;font-weight:700;font-size:18px;letter-spacing:.2px}
.logo svg{filter:drop-shadow(0 0 8px rgba(34,211,238,.6))}
.pill{margin-left:auto;display:flex;align-items:center;gap:6px;font-size:12px;color:var(--sub);background:var(--card2);padding:5px 10px;border-radius:99px}
.dot{width:8px;height:8px;border-radius:50%;background:var(--good);box-shadow:0 0 8px var(--good)}
.dot.off{background:var(--bad);box-shadow:0 0 8px var(--bad)}
main{padding:6px 16px calc(96px + env(safe-area-inset-bottom));max-width:720px;margin:0 auto}
.view{display:none;animation:fade .25s ease}
.view.active{display:block}
@keyframes fade{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:none}}
h2{font-size:24px;margin:18px 4px 6px;letter-spacing:-.3px}
h3{font-size:13px;font-weight:600;color:var(--sub);text-transform:uppercase;letter-spacing:.8px;margin:0 0 12px}
.card{background:var(--card);backdrop-filter:blur(18px);-webkit-backdrop-filter:blur(18px);border:1px solid var(--line);border-radius:var(--r);padding:16px;margin:12px 0;box-shadow:0 10px 30px rgba(0,0,0,.25)}
.row{display:flex;align-items:center;gap:10px}
.between{justify-content:space-between}
.sub{color:var(--sub)}.muted{color:var(--muted)}.small{font-size:12px}
.btn{display:inline-flex;align-items:center;justify-content:center;gap:6px;padding:11px 18px;border-radius:14px;background:var(--card2);font-weight:600;transition:transform .1s,opacity .2s}
.btn:active{transform:scale(.96)}
.btn.primary{background:linear-gradient(135deg,#22d3ee,#0ea5e9);color:#04202b}
.btn.danger{background:linear-gradient(135deg,#f87171,#dc2626);color:#fff}
.btn.good{background:linear-gradient(135deg,#4ade80,#16a34a);color:#04210f}
.btn.ghost{background:transparent;border:1px solid var(--line)}
.btn.block{width:100%}
.btn:disabled{opacity:.4}
/* hero */
.hero{display:flex;gap:18px;align-items:center}
.ring{position:relative;width:132px;height:132px;flex:none}
.ring svg{transform:rotate(-90deg)}
.ring .trk{stroke:rgba(148,178,220,.12)}
.ring .arc{transition:stroke-dashoffset .8s cubic-bezier(.2,.8,.2,1),stroke .4s}
.ringTxt{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center}
.ringTxt b{font-size:30px;letter-spacing:-1px}
.ringTxt span{font-size:11px;color:var(--sub);text-transform:uppercase;letter-spacing:.6px}
.eyebrow{font-size:11px;font-weight:700;letter-spacing:1px;color:var(--acc);text-transform:uppercase}
.big{font-size:22px;font-weight:700;margin:2px 0}
.heroInfo{min-width:0;flex:1}
.heroInfo .btn{margin-top:10px}
/* segmented */
.seg{position:relative;display:grid;grid-template-columns:repeat(3,1fr);background:var(--card);border:1px solid var(--line);border-radius:16px;padding:4px;margin:12px 0}
.seg .sl{position:absolute;top:4px;bottom:4px;left:4px;width:calc((100% - 8px)/3);border-radius:12px;background:linear-gradient(135deg,#0e7490,#155e75);box-shadow:0 4px 14px rgba(34,211,238,.25);transition:transform .35s cubic-bezier(.2,.8,.2,1)}
.seg button{position:relative;padding:10px 0;font-weight:600;color:var(--sub);transition:color .2s}
.seg button.on{color:#fff}
.banner{border-radius:16px;padding:12px 16px;margin:12px 0;font-weight:600;display:flex;gap:10px;align-items:center}
.banner.ice{background:rgba(125,211,252,.12);color:var(--ice);border:1px solid rgba(125,211,252,.3)}
.banner.warn{background:rgba(245,158,11,.12);color:var(--warn);border:1px solid rgba(245,158,11,.3)}
/* weather */
.wxNow{display:flex;align-items:center;gap:14px}
.wxNow .t{font-size:44px;font-weight:300;letter-spacing:-2px}
.wxStats{display:grid;grid-template-columns:1fr 1fr;gap:2px 14px;font-size:13px;color:var(--sub);margin-left:auto}
.fc{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-top:14px}
.fc>div{background:rgba(255,255,255,.04);border-radius:14px;padding:10px 4px;text-align:center;font-size:13px}
.fc .d{color:var(--sub);font-size:12px}
.fc .p{color:var(--ice);font-size:12px}
.reason{display:flex;gap:10px;align-items:flex-start;margin-top:14px;padding:12px;border-radius:14px;background:rgba(34,211,238,.07);font-size:14px}
/* zones grid */
.zgrid{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:10px}
.zt{display:flex;align-items:center;gap:10px;padding:12px;border-radius:16px;background:rgba(255,255,255,.04);border:1px solid transparent;text-align:left;transition:all .2s}
.zt:active{transform:scale(.97)}
.zt .n{width:30px;height:30px;border-radius:50%;display:grid;place-items:center;font-weight:700;background:var(--c);flex:none;color:#fff}
.zt.on{border-color:var(--acc);background:rgba(34,211,238,.1)}
.zt.on .n{animation:pulse 1.4s infinite}
.zt .nm{font-weight:600;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.zt .st{font-size:12px;color:var(--sub)}
@keyframes pulse{0%{box-shadow:0 0 0 0 rgba(34,211,238,.7)}70%{box-shadow:0 0 0 10px rgba(34,211,238,0)}100%{box-shadow:0 0 0 0 rgba(34,211,238,0)}}
.actions{display:grid;grid-template-columns:1fr 1fr;gap:10px}
/* map */
.mapBox{position:relative;width:100%;aspect-ratio:320/176;border-radius:16px;overflow:hidden;background:#0b1426;touch-action:manipulation}
.mapBox>img,.mapBox>svg,.mapBox>canvas{position:absolute;inset:0;width:100%;height:100%}
.mapBox>svg *{vector-effect:non-scaling-stroke;stroke-width:1.6;stroke-linejoin:round}
.mapBox .zp{cursor:pointer;transition:fill-opacity .3s}
.mapBox .zp.on{animation:glow 1.6s infinite}
@keyframes glow{50%{fill-opacity:.85}}
.badge{position:absolute;transform:translate(-50%,-50%);width:28px;height:28px;border-radius:50%;background:var(--c);border:2px solid #fff;font-weight:800;font-size:13px;color:#fff;display:grid;place-items:center;box-shadow:0 2px 8px rgba(0,0,0,.5);z-index:2}
.badge.on{background:var(--acc);color:#04202b;animation:pulse 1.4s infinite}
.mlbl{position:absolute;transform:translate(-50%,-50%);font-size:12px;font-weight:600;color:#cbd5e1;pointer-events:none;white-space:nowrap}
.mlbl.drive{font-size:10px;color:#94a3b8}
.north{position:absolute;right:2.5%;top:50%;transform:translateY(-60%);color:var(--acc);font-weight:800;font-size:12px;text-align:center;pointer-events:none}
.legend{display:flex;flex-wrap:wrap;gap:6px 14px;font-size:13px;color:var(--sub);margin-top:12px}
.legend span{display:inline-flex;align-items:center;gap:6px}
.sw{width:10px;height:10px;border-radius:3px;background:var(--c);display:inline-block}
/* forms */
.field{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:12px 0;border-bottom:1px solid var(--line)}
.field:last-child{border-bottom:0}
.field label{flex:1}
.field .v{color:var(--sub);font-variant-numeric:tabular-nums;min-width:64px;text-align:right}
input[type=text],input[type=number],input[type=password],input[type=time],input[type=search],select{background:rgba(255,255,255,.06);border:1px solid var(--line);border-radius:12px;padding:10px 12px;outline:none;min-width:0}
input:focus{border-color:var(--acc)}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:6px;border-radius:9px;background:linear-gradient(90deg,var(--acc) var(--p,50%),rgba(255,255,255,.1) var(--p,50%));outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:24px;height:24px;border-radius:50%;background:#fff;box-shadow:0 2px 8px rgba(0,0,0,.4)}
input[type=range]::-moz-range-thumb{width:24px;height:24px;border-radius:50%;background:#fff;border:0}
.sw2,.field label.sw2{position:relative;width:50px;height:30px;flex:none}
.sw2 input{opacity:0;width:0;height:0}
.sw2 span{position:absolute;inset:0;border-radius:99px;background:rgba(255,255,255,.12);transition:.25s}
.sw2 span:before{content:"";position:absolute;width:24px;height:24px;left:3px;top:3px;border-radius:50%;background:#fff;transition:.25s cubic-bezier(.2,.8,.2,1)}
.sw2 input:checked+span{background:var(--good)}
.sw2 input:checked+span:before{transform:translateX(20px)}
.modes{display:grid;gap:10px}
.mc{display:flex;gap:14px;align-items:center;text-align:left;padding:16px;border-radius:18px;background:var(--card);border:1.5px solid var(--line);transition:all .2s}
.mc.on{border-color:var(--acc);background:rgba(34,211,238,.09);box-shadow:0 0 0 4px rgba(34,211,238,.1)}
.mc .ic{width:44px;height:44px;border-radius:14px;display:grid;place-items:center;background:rgba(255,255,255,.06);flex:none}
.mc b{display:block;font-size:16px}
.mc small{color:var(--sub)}
.days{display:grid;grid-template-columns:repeat(7,1fr);gap:6px}
.days button{padding:12px 0;border-radius:12px;background:rgba(255,255,255,.05);font-weight:600;color:var(--sub);transition:all .2s}
.days button.on{background:linear-gradient(135deg,#0e7490,#155e75);color:#fff}
.zr{display:grid;grid-template-columns:14px 1fr auto;gap:4px 10px;align-items:center;padding:10px 0;border-bottom:1px solid var(--line)}
.zr:last-child{border-bottom:0}
.zr .sw{width:12px;height:12px;border-radius:50%}
.zr input[type=range]{grid-column:2/4}
.sticky{position:sticky;bottom:calc(84px + env(safe-area-inset-bottom));z-index:5}
/* usage */
.kpis{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.kpi{background:var(--card);border:1px solid var(--line);border-radius:18px;padding:14px}
.kpi span{font-size:11px;font-weight:700;letter-spacing:.8px;color:var(--sub);text-transform:uppercase}
.kpi b{display:block;font-size:26px;letter-spacing:-.5px;margin-top:4px}
.kpi b small{font-size:13px;color:var(--sub);font-weight:500;margin-left:3px}
.chartWrap{position:relative}
.chart{width:100%;height:auto;display:block}
.tip{position:absolute;pointer-events:none;background:rgba(10,18,34,.95);border:1px solid var(--line);border-radius:12px;padding:10px 12px;font-size:12px;min-width:150px;transform:translate(-50%,-100%);box-shadow:0 8px 24px rgba(0,0,0,.4);z-index:5}
.tip .r{display:flex;justify-content:space-between;gap:12px}
.hb{display:grid;grid-template-columns:1fr auto;gap:4px 10px;align-items:center;margin:10px 0}
.hb .track{grid-column:1/3;height:8px;border-radius:6px;background:rgba(255,255,255,.06);overflow:hidden}
.hb .track i{display:block;height:100%;border-radius:6px;background:var(--c)}
table{width:100%;border-collapse:collapse;font-size:13px}
td,th{padding:6px 4px;text-align:right;border-bottom:1px solid var(--line)}
td:first-child,th:first-child{text-align:left}
th{color:var(--sub);font-weight:600}
/* settings */
.kv{display:grid;grid-template-columns:auto 1fr;gap:8px 14px;font-size:14px}
.kv span:nth-child(odd){color:var(--sub)}
.kv span:nth-child(even){text-align:right;word-break:break-word}
.results button{display:block;width:100%;text-align:left;padding:12px;border-radius:12px;background:rgba(255,255,255,.04);margin-top:6px}
/* tab bar */
nav.tabs{position:fixed;left:0;right:0;bottom:0;z-index:30;display:grid;grid-template-columns:repeat(5,1fr);padding:8px 8px calc(8px + env(safe-area-inset-bottom));background:rgba(11,20,38,.75);backdrop-filter:blur(24px);-webkit-backdrop-filter:blur(24px);border-top:1px solid var(--line)}
nav.tabs button{display:flex;flex-direction:column;align-items:center;gap:3px;font-size:11px;color:var(--muted);padding:4px 0;transition:color .2s}
nav.tabs button.on{color:var(--acc)}
nav.tabs svg{width:24px;height:24px}
/* sheet */
#scrim{position:fixed;inset:0;background:rgba(0,0,0,.5);opacity:0;pointer-events:none;transition:opacity .3s;z-index:40}
#scrim.on{opacity:1;pointer-events:auto}
.sheet{position:fixed;left:0;right:0;bottom:0;z-index:41;max-height:88vh;overflow:auto;background:#111d33;border-radius:26px 26px 0 0;border-top:1px solid var(--line);padding:10px 20px calc(24px + env(safe-area-inset-bottom));transform:translateY(105%);transition:transform .38s cubic-bezier(.2,.8,.2,1);max-width:720px;margin:0 auto}
.sheet.on{transform:none}
.grab{width:44px;height:5px;border-radius:9px;background:rgba(255,255,255,.2);margin:0 auto 14px}
.shHead{display:flex;align-items:center;gap:12px}
.shHead .n{width:42px;height:42px;border-radius:50%;background:var(--c);display:grid;place-items:center;font-weight:800;font-size:18px}
.bigTime{font-size:48px;font-weight:200;letter-spacing:-2px;text-align:center;font-variant-numeric:tabular-nums;margin:10px 0 4px}
/* editor */
.overlay{position:fixed;inset:0;z-index:50;background:var(--bg);overflow:auto;padding:calc(12px + env(safe-area-inset-top)) 16px 30px}
.edWrap{position:relative;touch-action:none;user-select:none}
.edWrap .pt{fill:#fff;stroke:#000}
.chips{display:flex;gap:6px;overflow-x:auto;padding:4px 0}
.chips button{flex:none;padding:8px 14px;border-radius:99px;background:rgba(255,255,255,.06);font-weight:600;border:1.5px solid transparent}
.chips button.on{border-color:var(--c,var(--acc));background:rgba(255,255,255,.12)}
.stepper{display:flex;gap:6px;margin:10px 0}
.stepper button{flex:1;padding:10px;border-radius:12px;background:rgba(255,255,255,.05);font-weight:600;color:var(--sub)}
.stepper button.on{background:var(--acc2);color:#fff}
#toast{position:fixed;left:50%;bottom:calc(96px + env(safe-area-inset-bottom));transform:translate(-50%,20px);opacity:0;background:#1f3150;padding:12px 18px;border-radius:14px;font-weight:600;transition:all .3s;z-index:60;box-shadow:0 10px 30px rgba(0,0,0,.4);max-width:90vw}
#toast.on{opacity:1;transform:translate(-50%,0)}
#toast.err{background:#7f1d1d}
</style>
</head>
<body>
<header class="top">
  <div class="logo"><svg width="22" height="22" viewBox="0 0 24 24"><path d="M12 2.5C9 7 6 10.2 6 14a6 6 0 0 0 12 0c0-3.8-3-7-6-11.5z" fill="#22d3ee"/><circle cx="10" cy="14.5" r="2" fill="#a5f3fc"/></svg>IrrigationGenie</div>
  <div class="pill"><span class="dot off" id="connDot"></span><span id="clock">--:--</span></div>
</header>

<main>
<!-- HOME -->
<section class="view active" id="v-home">
  <div id="apBanner" class="banner warn hidden">Setup mode. Connect IrrigationGenie to your home WiFi in Settings.</div>
  <div id="inetBanner" class="banner warn hidden">No internet. Smart mode is using the schedule as is.</div>
  <div class="card hero">
    <div class="ring"><svg width="132" height="132" viewBox="0 0 132 132"><circle class="trk" cx="66" cy="66" r="56" fill="none" stroke-width="12"/>
      <circle id="ringArc" class="arc" cx="66" cy="66" r="56" fill="none" stroke="#22d3ee" stroke-width="12" stroke-linecap="round" stroke-dasharray="351.9" stroke-dashoffset="351.9"/></svg>
      <div class="ringTxt"><b id="ringVal">--</b><span id="ringLbl">Water budget</span></div></div>
    <div class="heroInfo"><div class="eyebrow" id="heroEye">Next run</div><div class="big" id="heroTitle">--</div><div class="sub small" id="heroSub"></div>
      <button class="btn primary" id="heroBtn">Run now</button></div>
  </div>
  <div class="seg" id="modeSeg"><div class="sl"></div><button data-m="0">Manual</button><button data-m="1">Schedule</button><button data-m="2">Smart</button></div>
  <div id="winterBanner" class="banner ice hidden"><svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2v20M4 7l16 10M20 7L4 17"/></svg><span id="winterTxt">Winter mode</span></div>
  <div class="card">
    <h3>Weather</h3>
    <div class="wxNow"><div id="wxIcon"></div><div class="t" id="wxTemp">--</div>
      <div class="wxStats"><span id="wxHL">H -- L --</span><span id="wxRH">Hum --</span><span id="wxWind">Wind --</span><span id="wxET">ET --</span></div></div>
    <div class="fc" id="fc"></div>
    <div class="reason"><svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="#22d3ee" stroke-width="2"><path d="M12 3l2.5 5.5L20 9l-4 4 1 6-5-3-5 3 1-6-4-4 5.5-.5z"/></svg><div><b id="smartPct">--</b> <span id="smartReason" class="sub">Waiting for weather</span></div></div>
  </div>
  <div class="card"><div class="row between"><h3>Zones</h3><button class="btn ghost small hidden" id="masterBtn">Master</button></div><div class="zgrid" id="zgrid"></div></div>
  <div class="actions">
    <button class="btn" id="delayBtn">Rain delay</button>
    <button class="btn danger" id="stopBtn">Stop all</button>
  </div>
</section>

<!-- MAP -->
<section class="view" id="v-map">
  <h2>Yard map</h2>
  <div class="card">
    <div class="mapBox" id="mapBox"></div>
    <div class="legend" id="mapLegend"></div>
  </div>
  <div class="actions"><button class="btn" id="editMap">Edit map</button><button class="btn" id="masterBtn2">Master valve</button></div>
  <p class="small muted" style="margin:14px 4px">Tap a zone to water it. The touch screen shows the same map.</p>
</section>

<!-- SCHEDULE -->
<section class="view" id="v-sched">
  <h2>Watering</h2>
  <div class="modes" id="modeCards">
    <button class="mc" data-m="0"><div class="ic"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#9fb3c8" stroke-width="2"><path d="M8 13V4.5a1.5 1.5 0 0 1 3 0V12m0-1.5a1.5 1.5 0 0 1 3 0V12m0-.5a1.5 1.5 0 0 1 3 0V13m0-.5a1.5 1.5 0 0 1 3 0V16a6 6 0 0 1-6 6h-1a6 6 0 0 1-5-2.7L5 15.5a1.6 1.6 0 0 1 2.6-1.9L8 14"/></svg></div><div><b>Manual</b><small>You turn zones on and off</small></div></button>
    <button class="mc" data-m="1"><div class="ic"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#22d3ee" stroke-width="2"><rect x="3" y="5" width="18" height="16" rx="3"/><path d="M3 10h18M8 3v4M16 3v4"/></svg></div><div><b>Schedule</b><small>Fixed days, times and run times</small></div></button>
    <button class="mc" data-m="2"><div class="ic"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#22c55e" stroke-width="2"><path d="M12 3l2.5 5.5L20 9l-4 4 1 6-5-3-5 3 1-6-4-4 5.5-.5z"/></svg></div><div><b>Smart</b><small>Schedule adjusted daily by the weather forecast</small></div></button>
  </div>
  <div class="card" id="smartCard">
    <h3>Smart watering</h3>
    <p class="small sub" style="margin-top:-4px">Run times below are for a hot, dry day. Each run is scaled by evapotranspiration (temperature, humidity, wind, sun) and credited for recent and expected rain.</p>
    <div class="field"><label>Skip if chance of rain above</label><span class="v" id="vPop"></span></div><input type="range" id="sPop" min="0" max="100" step="5">
    <div class="field"><label>Skip after rain days in a row</label><span class="v" id="vStreak"></span></div><input type="range" id="sStreak" min="0" max="3" step="1">
    <div class="field"><label>Rain skip</label><span class="v" id="vRain"></span></div><input type="range" id="sRain" min="2" max="25" step="0.5">
    <div class="field"><label>Freeze skip at or below</label><span class="v" id="vFreeze"></span></div><input type="range" id="sFreeze" min="-3" max="8" step="0.5">
    <div class="field"><label>Wind skip above</label><span class="v" id="vWind"></span></div><input type="range" id="sWind" min="0" max="80" step="2">
    <div class="field"><label>Max boost on hot days</label><span class="v" id="vBoost"></span></div><input type="range" id="sBoost" min="100" max="250" step="5">
    <div class="field"><label>Skip runs below</label><span class="v" id="vMin"></span></div><input type="range" id="sMin" min="0" max="60" step="5">
    <div class="field"><label>Reference ET (100%)</label><span class="v" id="vEt"></span></div><input type="range" id="sEt" min="2" max="10" step="0.25">
  </div>
  <div class="card">
    <h3>Start times</h3>
    <div class="field"><label class="sw2"><input type="checkbox" id="st0"><span></span></label><label style="padding-left:12px">Start 1</label><input type="time" id="tm0"></div>
    <div class="field"><label class="sw2"><input type="checkbox" id="st1"><span></span></label><label style="padding-left:12px">Start 2</label><input type="time" id="tm1"></div>
  </div>
  <div class="card"><h3>Days</h3><div class="days" id="days"></div></div>
  <div class="card"><div class="row between"><h3>Run time per zone</h3><span class="small sub" id="totRun"></span></div><div id="zRows"></div></div>
  <div class="sticky"><button class="btn primary block" id="saveSched">Save schedule</button></div>
</section>

<!-- USAGE -->
<section class="view" id="v-usage">
  <h2>Water usage</h2>
  <div class="kpis" id="kpis"></div>
  <div class="card">
    <div class="row between"><h3>Last 30 days</h3><button class="btn ghost small" id="tblBtn">Table</button></div>
    <div class="chartWrap" id="chartWrap"></div>
    <div class="legend" id="usageLegend"></div>
    <div id="usageTable" class="hidden" style="margin-top:12px"></div>
  </div>
  <div class="card"><h3>By zone</h3><div id="byZone"></div></div>
  <p class="small muted" style="margin:0 4px">Usage is estimated from valve on-time and each zone's flow rate (set it in Settings).</p>
</section>

<!-- SETTINGS -->
<section class="view" id="v-set">
  <h2>Settings</h2>
  <div class="card" id="wifiCard">
    <h3>WiFi</h3>
    <div class="row"><input type="text" id="wSsid" placeholder="Network name" style="flex:1" list="ssids"><button class="btn" id="wScan">Scan</button></div>
    <datalist id="ssids"></datalist>
    <input type="password" id="wPass" placeholder="Password" style="width:100%;margin-top:8px">
    <button class="btn primary block" id="wSave" style="margin-top:10px">Connect</button>
  </div>
  <div class="card">
    <h3>Location &amp; time</h3>
    <div class="kv" id="locKv"></div>
    <div class="row" style="margin-top:14px"><button class="btn primary" id="gpsBtn" style="flex:1">Use my phone's location</button></div>
    <p class="small muted hidden" id="gpsNote">Your browser only shares GPS with secure (https) pages. Search for your city or ZIP code instead. The time zone still comes from your phone.</p>
    <div class="row" style="margin-top:10px"><input type="search" id="locQ" placeholder="City or ZIP code" style="flex:1"><button class="btn" id="locGo">Search</button></div>
    <div class="results" id="locRes"></div>
    <button class="btn ghost block" id="ipBtn" style="margin-top:10px">Detect from internet address</button>
  </div>
  <div class="card">
    <h3>Zones</h3>
    <div id="zSet"></div>
  </div>
  <div class="card">
    <h3>Preferences</h3>
    <div class="field"><label>Metric units</label><label class="sw2"><input type="checkbox" id="pMetric"><span></span></label></div>
    <div class="field"><label>24-hour clock</label><label class="sw2"><input type="checkbox" id="pClock"><span></span></label></div>
    <div class="field"><label>Sounds</label><label class="sw2"><input type="checkbox" id="pSound"><span></span></label></div>
    <div class="field"><label>Allow several zones at once</label><label class="sw2"><input type="checkbox" id="pMulti"><span></span></label></div>
    <div class="field"><label>Screen brightness</label><span class="v" id="vBright"></span></div><input type="range" id="pBright" min="15" max="255" step="5">
    <div class="field"><label>Screen saver after</label><select id="pSaver"><option value="30">30 s</option><option value="60">1 min</option><option value="120">2 min</option><option value="240">4 min</option><option value="0">Never</option></select></div>
    <div class="field"><label>Manual run default (min)</label><input type="number" id="pManual" min="1" max="120" style="width:80px"></div>
    <div class="field"><label>Safety cutoff (min)</label><input type="number" id="pMaxRun" min="5" max="240" style="width:80px"></div>
    <div class="field"><label>Water cost per 1000 <span class="vu">gal</span></label><input type="number" id="pCost" min="0" step="0.1" style="width:80px"></div>
  </div>
  <div class="card">
    <h3>Winter mode</h3>
    <p class="small sub" style="margin-top:-4px">Closes the master valve and opens every zone to drain the lines. No watering until you turn it off.</p>
    <div class="field"><label>Drain time (min, 0 = stay open)</label><input type="number" id="pDrain" min="0" max="120" style="width:80px"></div>
    <div class="field"><label><b>Winter mode</b></label><label class="sw2"><input type="checkbox" id="pWinter"><span></span></label></div>
  </div>
  <div class="card">
    <h3>Device</h3>
    <div class="kv" id="devKv"></div>
    <div class="actions" style="margin-top:14px"><button class="btn" id="wxRefresh">Refresh weather</button><button class="btn" id="rebootBtn">Restart</button></div>
  </div>
</section>
</main>

<nav class="tabs" id="tabs">
  <button data-v="home" class="on"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linejoin="round"><path d="M3 11l9-7 9 7v9a1 1 0 0 1-1 1h-5v-6h-6v6H4a1 1 0 0 1-1-1z"/></svg>Home</button>
  <button data-v="map"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linejoin="round"><path d="M9 4L3 6v14l6-2 6 2 6-2V4l-6 2zM9 4v14M15 6v14"/></svg>Map</button>
  <button data-v="sched"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="5" width="18" height="16" rx="3"/><path d="M3 10h18M8 3v4M16 3v4"/></svg>Schedule</button>
  <button data-v="usage"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M5 20v-6M12 20V8M19 20V4"/></svg>Usage</button>
  <button data-v="set"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1.1-1.5 1.7 1.7 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.5-1.1 1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.8.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8V9a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z"/></svg>Settings</button>
</nav>

<div id="scrim"></div>
<div class="sheet" id="sheet"><div class="grab"></div><div id="sheetBody"></div></div>

<div class="overlay hidden" id="editor">
  <div class="row between"><h2 style="margin:6px 0">Edit map</h2><button class="btn ghost" id="edClose">Cancel</button></div>
  <div class="stepper"><button data-s="1" class="on">1. Background</button><button data-s="2">2. Zones</button></div>
  <div id="edStep1">
    <div class="chips" id="edSrc"><button data-src="plan">Yard plan</button><button data-src="sat">Satellite</button><button data-src="img">Photo</button></div>
    <input type="file" id="edFile" accept="image/*" class="hidden">
    <p class="small muted" id="edHint1">Drag to move, pinch or use +/- to zoom.</p>
  </div>
  <div id="edStep2" class="hidden">
    <div class="chips" id="edZones"></div>
    <div class="row" style="margin:6px 0"><div class="seg" style="margin:0;flex:1" id="edMode"><div class="sl" style="width:calc((100% - 8px)/2)"></div><button data-k="0" class="on">Outline</button><button data-k="1">Badge</button><span></span></div></div>
    <p class="small muted">Outline: tap corners of the zone (up to 12). Badge: tap where the number goes.</p>
  </div>
  <div class="mapBox edWrap" id="edWrap"><canvas id="edCanvas" width="960" height="528"></canvas><svg id="edSvg" viewBox="0 0 1000 1000" preserveAspectRatio="none"></svg><div id="edBadges"></div></div>
  <div class="row between" style="margin-top:8px"><span class="small muted" id="edAttr"></span><div class="row" id="edZoom"><button class="btn" id="edOut">-</button><button class="btn" id="edIn">+</button></div></div>
  <div class="actions hidden" id="edTools" style="margin-top:8px"><button class="btn" id="edUndo">Undo point</button><button class="btn" id="edClear">Clear zone</button></div>
  <button class="btn primary block" id="edSave" style="margin-top:14px">Save map</button>
</div>

<div id="toast"></div>

<script>
const ZC=['#3987e5','#d95926','#199e70','#c98500','#d55181','#008300','#9085e9'];
const $=(s,r=document)=>r.querySelector(s),$$=(s,r=document)=>[...r.querySelectorAll(s)];
let S=null,C=null,W=null,U=null,D=null,lastCfg=-1,lastWx=-1,lastMap=-1,view='home',sheetZ=-1,failCount=0,draftDirty=false;

async function api(p,body,method){
  const o={method:method||(body?'POST':'GET')};
  if(body){o.body=JSON.stringify(body);o.headers={'Content-Type':'application/json'}}
  const r=await fetch(p,o);
  if(!r.ok)throw new Error((await r.text())||r.status);
  return (r.headers.get('content-type')||'').includes('json')?r.json():r.text();
}
async function cmd(c,x){try{await api('/api/cmd',Object.assign({cmd:c},x||{}));poll()}catch(e){toast('Failed: '+e.message,1)}}
async function saveCfg(o,msg){try{await api('/api/config',o);if(msg!==false)toast(msg||'Saved');await loadConfig();poll()}catch(e){toast('Save failed',1)}}
function toast(m,err){const t=$('#toast');t.textContent=m;t.className='on'+(err?' err':'');clearTimeout(t._h);t._h=setTimeout(()=>t.className='',2600)}

// ---------- units
const MET=()=>C&&C.metric;
const T=c=>Math.round(MET()?c:c*1.8+32)+'°';
const RN=mm=>MET()?mm.toFixed(1)+' mm':(mm/25.4).toFixed(2)+' in';
const WS=k=>MET()?Math.round(k)+' km/h':Math.round(k/1.609)+' mph';
const VOL=g=>{const v=MET()?g*3.785:g;return v>=10000?(v/1000).toFixed(1)+'k':Math.round(v).toLocaleString()};
const VU=()=>MET()?'L':'gal';
const pad=n=>String(n).padStart(2,'0');
const dur=s=>{s=Math.max(0,s|0);const h=s/3600|0,m=(s/60|0)%60;return h?h+':'+pad(m)+':'+pad(s%60):m+':'+pad(s%60)};
const hm=min=>{const h=min/60|0,m=min%60;return C&&C.clock24?pad(h)+':'+pad(m):(h%12||12)+':'+pad(m)+(h<12?' AM':' PM')};
const dayName=(unix,i)=>{if(i===0)return 'Today';try{return new Date((unix+43200)*1000).toLocaleDateString('en-US',{weekday:'short',timeZone:C.tz||undefined})}catch(e){return new Date((unix+43200)*1000).toLocaleDateString('en-US',{weekday:'short'})}};
const esc=s=>String(s).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));

// ---------- weather icons (WMO codes)
function wxIcon(code,size){
  size=size||40;
  const sun='<circle cx="12" cy="12" r="4.2" fill="#facc15"/><g stroke="#facc15" stroke-width="1.8" stroke-linecap="round"><path d="M12 2.5v2M12 19.5v2M2.5 12h2M19.5 12h2M5.3 5.3l1.4 1.4M17.3 17.3l1.4 1.4M5.3 18.7l1.4-1.4M17.3 6.7l1.4-1.4"/></g>';
  const cloud=(c,y)=>`<path transform="translate(0 ${y||0})" d="M7 18.5h10.5a4 4 0 0 0 .3-8 6 6 0 0 0-11.4 1.6A3.3 3.3 0 0 0 7 18.5z" fill="${c||'#cbd5e1'}"/>`;
  let b;
  if(code===0)b=sun;
  else if(code<=2)b=`<g transform="translate(-3 -3) scale(.8)">${sun}</g>`+cloud('#e2e8f0',2);
  else if(code===3)b=cloud();
  else if(code===45||code===48)b='<g stroke="#cbd5e1" stroke-width="2" stroke-linecap="round"><path d="M4 9h16M3 13h18M5 17h14"/></g>';
  else if((code>=71&&code<=77)||code===85||code===86)b=cloud('#cbd5e1',-3)+'<g fill="#fff"><circle cx="8" cy="20" r="1.3"/><circle cx="12" cy="21.5" r="1.3"/><circle cx="16" cy="20" r="1.3"/></g>';
  else if(code>=95)b=cloud('#64748b',-3)+'<path d="M12.5 14l-3 5h2.5l-1.5 4 4.5-6h-2.6l1.6-3z" fill="#facc15"/>';
  else b=cloud('#cbd5e1',-3)+'<g stroke="#7dd3fc" stroke-width="1.8" stroke-linecap="round"><path d="M8 18.5l-1 3M12 18.5l-1 3M16 18.5l-1 3"/></g>';
  return `<svg width="${size}" height="${size}" viewBox="0 0 24 24">${b}</svg>`;
}

// ---------- time zone: POSIX rule computed from the IANA name in the browser
function tzOff(tz,d){
  const f=tzOff.f[tz]||(tzOff.f[tz]=new Intl.DateTimeFormat('en-US',{timeZone:tz,hourCycle:'h23',year:'numeric',month:'numeric',day:'numeric',hour:'numeric',minute:'numeric',second:'numeric'}));
  const p={};f.formatToParts(d).forEach(x=>p[x.type]=x.value);
  return Math.round((Date.UTC(+p.year,+p.month-1,+p.day,+p.hour%24,+p.minute,+p.second)-Math.floor(d.getTime()/1000)*1000)/60000);
}
tzOff.f={};
function posixTZ(tz){
  try{
    const y=new Date().getFullYear(),t0=Date.UTC(y,0,1),tr=[];
    let prev=tzOff(tz,new Date(t0));
    for(let d=1;d<=366;d++){
      const t=t0+d*864e5,o=tzOff(tz,new Date(t));
      if(o!==prev){let a=t-864e5;while(tzOff(tz,new Date(a))===prev)a+=9e5;tr.push({t:a,from:prev,to:o});prev=o}
    }
    const abbr=(d,off)=>{const n=new Intl.DateTimeFormat('en-US',{timeZone:tz,timeZoneName:'short'}).formatToParts(d).find(x=>x.type==='timeZoneName').value;
      if(/^[A-Za-z]{3,6}$/.test(n))return n;const a=Math.abs(off);return '<'+(off>=0?'+':'-')+pad(a/60|0)+(a%60?pad(a%60):'')+'>'};
    const po=m=>{const s=-m,a=Math.abs(s);return (s<0?'-':'')+(a/60|0)+(a%60?':'+pad(a%60):'')};
    if(tr.length<2){const o=tzOff(tz,new Date());return abbr(new Date(),o)+po(o)}
    const st=tr.find(x=>x.to>x.from),en=tr.find(x=>x.to<x.from);
    const rule=x=>{const d=new Date(x.t+x.from*6e4),day=d.getUTCDate(),dim=new Date(Date.UTC(d.getUTCFullYear(),d.getUTCMonth()+1,0)).getUTCDate();
      let w=Math.ceil(day/7);if(day+7>dim)w=5;const mins=d.getUTCHours()*60+d.getUTCMinutes();
      return 'M'+(d.getUTCMonth()+1)+'.'+w+'.'+d.getUTCDay()+'/'+(mins/60|0)+(mins%60?':'+pad(mins%60):'')};
    const std=st.from,dst=st.to;
    return abbr(new Date(st.t-864e5),std)+po(std)+abbr(new Date(st.t+864e5),dst)+(dst-std!==60?po(dst):'')+','+rule(st)+','+rule(en);
  }catch(e){return ''}
}

// ---------- data
async function loadConfig(){C=await api('/api/config');lastCfg=S?S.cfgStamp:lastCfg;if(!draftDirty)D=JSON.parse(JSON.stringify(C));renderAll()}
async function loadWeather(){try{W=await api('/api/weather');renderHome()}catch(e){}}
async function loadUsage(){try{U=await api('/api/usage');renderUsage()}catch(e){}}
async function poll(){
  try{
    S=await api('/api/state');failCount=0;$('#connDot').classList.remove('off');
    $('#clock').textContent=S.tv?S.time:'--:--';
    if(S.cfgStamp!==lastCfg){lastCfg=S.cfgStamp;await loadConfig()}
    if(S.wxStamp!==lastWx){lastWx=S.wxStamp;await loadWeather()}
    if(S.mapStamp!==lastMap){lastMap=S.mapStamp;renderMap()}
    renderLive();
  }catch(e){if(++failCount>1)$('#connDot').classList.add('off')}
}

// ---------- rendering
function renderAll(){if(!C)return;$$('.vu').forEach(e=>e.textContent=VU());renderHome();renderMap();renderSched();renderSettings();if(U)renderUsage()}

function renderLive(){
  if(!S||!C)return;
  renderHome();
  if(view==='map')updateMapLive();
  if(sheetZ>=0)updateSheet();
  if(view==='set')renderDevice();
  $('#apBanner').classList.toggle('hidden',!S.ap);
  $('#inetBanner').classList.toggle('hidden',S.inet||S.ap);
}

// zones the next program waters at pct, same rule as the controller (enabled, scaled run of 30 s or more)
const nextZones=pct=>C.zones.map((z,i)=>z.en&&Math.min(z.min*60*pct/100,(C.maxRunMin||120)*60)>=30?i+1:0).filter(Boolean).join('-');
function setRing(frac,color){const a=$('#ringArc');a.style.strokeDashoffset=351.9*(1-Math.max(0,Math.min(1,frac)));a.style.stroke=color}

function renderHome(){
  if(!S||!C)return;
  // mode segment
  $$('#modeSeg button').forEach(b=>b.classList.toggle('on',+b.dataset.m===S.mode));
  $('#modeSeg .sl').style.transform=`translateX(${S.mode*100}%)`;
  $('#modeSeg').classList.toggle('hidden',S.winter);
  $('#winterBanner').classList.toggle('hidden',!S.winter);
  $('#winterTxt').textContent=S.drain?'Winter mode, draining lines'+(S.drainLeft?' ('+dur(S.drainLeft)+')':''):'Winter mode, watering is off';

  const zi=S.z.findIndex(z=>z.on);
  const eye=$('#heroEye'),title=$('#heroTitle'),sub=$('#heroSub'),btn=$('#heroBtn');
  if(S.drain){
    setRing(1,'#7dd3fc');$('#ringVal').textContent=S.drainLeft?dur(S.drainLeft):'open';$('#ringLbl').textContent='Draining';
    eye.textContent='Winter';title.textContent='Draining lines';sub.textContent='Master closed, all zones open';btn.textContent='Stop';btn.className='btn danger';btn.disabled=false;btn.onclick=()=>cmd('stopall');
  }else if(zi>=0){
    const z=S.z[zi];setRing(z.tot?(z.tot-z.left)/z.tot:1,ZC[zi]);$('#ringVal').textContent=dur(z.left);$('#ringLbl').textContent='Zone '+(zi+1);
    eye.textContent=S.prog?'Program running':'Watering';title.textContent=C.zones[zi].name;sub.textContent=S.prog?'Smart '+S.progPct+'% of baseline':'Manual run';
    btn.textContent='Stop';btn.className='btn danger';btn.disabled=false;btn.onclick=()=>cmd('stopall');
  }else{
    const sm=S.smart,mb=C.maxBoost||160;
    if(S.mode===2){setRing(sm.skip?0:sm.pct/mb,sm.skip?'#f59e0b':sm.pct>120?'#f59e0b':'#22c55e');$('#ringVal').textContent=sm.skip?'Skip':sm.pct+'%';$('#ringLbl').textContent='Water budget'}
    else if(S.mode===1){setRing(100/mb,'#22d3ee');$('#ringVal').textContent='100%';$('#ringLbl').textContent='Fixed'}
    else{setRing(0,'#22d3ee');$('#ringVal').textContent='Off';$('#ringLbl').textContent='Manual'}
    eye.textContent=S.mode===2&&!S.winter?(nextZones(100)?'Next '+nextZones(100):'Next - no zones'):'Next run';
    title.textContent=S.winter?'None':S.mode===0?'Manual mode':(S.nextStr||'Not scheduled');
    sub.textContent=S.rainDelay?'Rain delay, '+Math.ceil(S.rainDelay/3600)+' h left':S.mode===2?sm.reason:S.mode===1?C.zones.filter(z=>z.en).reduce((a,z)=>a+z.min,0)+' min total':'Tap a zone to water';
    btn.textContent='Run now';btn.className='btn primary';btn.disabled=S.winter;btn.onclick=()=>cmd('runnow').then(()=>toast('Program started'));
  }

  // weather
  if(W&&W.valid){
    const d=W.days[W.today];
    $('#wxIcon').innerHTML=wxIcon(W.code,52);$('#wxTemp').textContent=T(W.temp);
    $('#wxHL').textContent='H '+T(d.hi)+'  L '+T(d.lo);$('#wxRH').textContent='Hum '+Math.round(W.rh)+'%';
    $('#wxWind').textContent='Wind '+WS(W.wind);$('#wxET').textContent='ET '+RN(d.et0);
    $('#fc').innerHTML=W.days.slice(W.today,W.today+4).map((d,i)=>`<div><div class="d">${dayName(d.date,i)}</div>${wxIcon(d.code,30)}<div>${T(d.hi)} <span class="muted">${T(d.lo)}</span></div><div class="p">${d.pop}% · ${RN(d.rain)}</div></div>`).join('');
  }else{$('#wxIcon').innerHTML=wxIcon(3,52);$('#fc').innerHTML='';$('#wxTemp').textContent='--'}
  const sm=S.smart;$('#smartPct').textContent=S.mode===2?(sm.skip?'Skip':sm.pct+'%'):'Smart';
  $('#smartReason').textContent=sm.reason+(S.mode!==2?' (smart mode off)':'');

  // zones
  $('#zgrid').innerHTML=C.zones.map((z,i)=>{const r=S.z[i];return `<button class="zt${r.on?' on':''}" data-z="${i}" style="--c:${z.en?ZC[i]:'#64748b'}"><span class="n">${i+1}</span><span style="min-width:0"><div class="nm">${esc(z.name)}</div><div class="st">${r.on?dur(r.left)+' left':z.en?z.min+' min':'disabled'}</div></span></button>`}).join('');
  const mb=$('#masterBtn');mb.classList.remove('hidden');mb.textContent='Master '+(S.master?'open':'closed');mb.style.color=S.master?'#22c55e':'';
  $('#delayBtn').textContent=S.rainDelay?'Rain delay '+Math.ceil(S.rainDelay/3600)+'h':'Rain delay';
}

// ---------- map
function mapHTML(edit){
  const P=C.plan,hasImg=S&&S.hasMap&&!edit,zs=edit?ED.zones:C.zones;
  const pts=a=>a.map(p=>p.join(',')).join(' ');
  let svg='';
  if(!hasImg&&!edit)svg+=`<polygon points="${pts(P.property)}" fill="#13261d" stroke="#3b5b4a"/>`;
  zs.forEach((z,i)=>{if(z.pts.length<3)return;const on=!edit&&S&&S.z[i].on,c=(edit||C.zones[i].en)?ZC[i]:'#64748b';
    const op=edit?(i===ED.sel?.45:.25):hasImg?(on?.5:.25):(on?.8:.4);
    svg+=`<polygon class="zp${on?' on':''}" data-z="${i}" points="${pts(z.pts)}" fill="${c}" fill-opacity="${op}" stroke="${c}"${edit&&i===ED.sel?' style="stroke-width:3"':''}/>`});
  if(edit){const z=zs[ED.sel];if(z.pts.length&&z.pts.length<3)svg+=`<polyline points="${pts(z.pts)}" fill="none" stroke="${ZC[ED.sel]}" style="stroke-width:3"/>`;
    z.pts.forEach(p=>svg+=`<ellipse class="pt" cx="${p[0]}" cy="${p[1]}" rx="6" ry="11"/>`);return svg}
  if(!hasImg)svg+=`<polygon points="${pts(P.drive)}" fill="#2b3442" stroke="#64748b"/><polygon points="${pts(P.house)}" fill="#33415c" stroke="#9fb3c8"/>`;
  let h=(hasImg?`<img src="/map.jpg?${S.mapStamp}" alt="Aerial view">`:'')+`<svg viewBox="0 0 1000 1000" preserveAspectRatio="none">${svg}</svg>`;
  if(!hasImg){h+=`<div class="mlbl" style="left:${P.houseLabel[0]/10}%;top:${P.houseLabel[1]/10}%">House</div><div class="mlbl drive" style="left:${P.driveLabel[0]/10}%;top:${P.driveLabel[1]/10}%">Driveway</div>`;
    h+='<div class="north"><svg width="14" height="18" viewBox="0 0 14 18"><path d="M7 0l7 16-7-4-7 4z" fill="#22d3ee"/></svg><div>N</div></div>'}
  return h+badgesHTML(zs,edit);
}
function badgesHTML(zs,edit){return zs.map((z,i)=>{const on=!edit&&S&&S.z[i].on;return `<button class="badge${on?' on':''}" data-z="${i}" style="left:${z.bx/10}%;top:${z.by/10}%;--c:${(edit||C.zones[i].en)?ZC[i]:'#64748b'}">${i+1}</button>`}).join('')}
let mapSig='';
function renderMap(){if(!C||!S)return;mapSig='';updateMapLive(true);
  $('#mapLegend').innerHTML=C.zones.map((z,i)=>`<span style="--c:${z.en?ZC[i]:'#64748b'}"><i class="sw"></i>${i+1} ${esc(z.name)}</span>`).join('')}
function updateMapLive(force){const sig=S.z.map(z=>z.on?1:0).join('')+S.hasMap+S.mapStamp;if(!force&&sig===mapSig)return;mapSig=sig;$('#mapBox').innerHTML=mapHTML(false);
  $('#masterBtn2').textContent='Master valve: '+(S.master?'open':'closed')}

// ---------- zone sheet
function openSheet(i){
  sheetZ=i;const z=C.zones[i];const max=C.maxRunMin||120;
  $('#sheetBody').innerHTML=`<div class="shHead" style="--c:${ZC[i]}"><div class="n">${i+1}</div><div style="flex:1;min-width:0"><div class="big" style="margin:0">${esc(z.name)}</div><div class="sub small" id="shSt"></div></div></div>
  <div class="bigTime" id="shTime">${C.manualMin}:00</div>
  <div class="field"><label>Run for</label><span class="v" id="shMinV">${C.manualMin} min</span></div>
  <input type="range" id="shMin" min="1" max="${Math.min(max,90)}" value="${C.manualMin}">
  <div class="actions" style="margin-top:16px" id="shBtns"><button class="btn good" id="shStart">Start</button><button class="btn danger" id="shStop">Stop</button></div>
  <div class="card" style="margin-top:16px"><div class="kv"><span>Last 30 days</span><span id="shUse">--</span><span>Baseline run</span><span>${z.min} min</span><span>Flow rate</span><span>${z.gpm} gpm</span></div></div>
  <div class="field"><label>Name</label><input type="text" id="shName" value="${esc(z.name)}" maxlength="23" style="width:60%"></div>
  <div class="field"><label>Flow (gal/min)</label><input type="number" id="shGpm" value="${z.gpm}" step="0.1" min="0" style="width:90px"></div>
  <div class="field"><label>Enabled</label><label class="sw2"><input type="checkbox" id="shEn" ${z.en?'checked':''}><span></span></label></div>
  <button class="btn block" id="shSave" style="margin-top:10px">Save zone</button>`;
  const r=$('#shMin');rangeFill(r);
  r.oninput=()=>{rangeFill(r);$('#shMinV').textContent=r.value+' min';if(!S.z[i].on)$('#shTime').textContent=r.value+':00'};
  $('#shStart').onclick=async()=>{try{await api('/api/cmd',{cmd:'zone',zone:i+1,on:true,min:+r.value});poll()}catch(e){toast(S.winter?'Winter mode is on':'Failed',1)}};
  $('#shStop').onclick=()=>cmd('zone',{zone:i+1,on:false});
  $('#shSave').onclick=()=>{const zs=C.zones.map(()=>({}));zs[i]={name:$('#shName').value,gpm:+$('#shGpm').value,en:$('#shEn').checked};saveCfg({zones:zs},'Zone saved')};
  if(U){const s=U.days.reduce((a,d)=>a+d.sec[i],0);$('#shUse').textContent=VOL(s/60*z.gpm)+' '+VU()+' · '+Math.round(s/60)+' min'}
  else loadUsage().then(()=>sheetZ===i&&openSheet(i));
  updateSheet();
  $('#sheet').classList.add('on');$('#scrim').classList.add('on');
}
function updateSheet(){const r=S.z[sheetZ];if(!r)return;
  $('#shSt').textContent=S.winter?'Winter mode':r.on?'Running · '+(r.src===2?'program':'manual'):'Off';
  $('#shSt').style.color=r.on?'#22c55e':'';
  $('#shTime').textContent=r.on?dur(r.left):$('#shMin').value+':00';
  $('#shStart').textContent=r.on?'Restart':'Start';$('#shStart').disabled=S.winter;$('#shStop').disabled=!r.on}
function closeSheet(){sheetZ=-1;$('#sheet').classList.remove('on');$('#scrim').classList.remove('on')}

// ---------- schedule
function rangeFill(r){r.style.setProperty('--p',((r.value-r.min)/(r.max-r.min)*100)+'%')}
const DAYS=['S','M','T','W','T','F','S'];
function renderSched(){
  if(!D)return;
  $$('#modeCards .mc').forEach(b=>b.classList.toggle('on',+b.dataset.m===C.mode));
  $('#smartCard').classList.toggle('hidden',C.mode!==2);
  for(let k=0;k<2;k++){$('#st'+k).checked=D.starts[k].en;$('#tm'+k).value=pad(D.starts[k].min/60|0)+':'+pad(D.starts[k].min%60)}
  $('#days').innerHTML=DAYS.map((d,i)=>`<button class="${D.days&1<<i?'on':''}" data-d="${i}">${d}</button>`).join('');
  $('#zRows').innerHTML=D.zones.map((z,i)=>`<div class="zr"><i class="sw" style="--c:${ZC[i]}"></i><span>${esc(z.name)}${z.en?'':' <span class="muted small">(disabled)</span>'}</span><span class="v sub" id="zv${i}"></span><input type="range" min="0" max="60" value="${z.min}" data-z="${i}"></div>`).join('');
  $$('#zRows input').forEach(r=>{rangeFill(r);r.oninput=()=>{D.zones[r.dataset.z].min=+r.value;draftDirty=true;rangeFill(r);schedTotals()}});
  smartSliders();schedTotals();
}
function schedTotals(){let t=0,g=0;D.zones.forEach((z,i)=>{$('#zv'+i).textContent=z.min?z.min+' min · '+VOL(z.min*z.gpm)+' '+VU():'off';if(z.en){t+=z.min;g+=z.min*z.gpm}});
  $('#totRun').textContent=t+' min · '+VOL(g)+' '+VU()}
const SM=[['sPop','popSkip',v=>v+'%'],['sStreak','rainDays',v=>v?v+(v>1?' days':' day'):'off'],['sRain','rainSkip',v=>RN(v)],['sFreeze','freeze',v=>T(v)],['sWind','windSkip',v=>v?WS(v):'off'],['sBoost','maxBoost',v=>v+'%'],['sMin','minPct',v=>v+'%'],['sEt','et0Ref',v=>RN(v)+'/day']];
function smartSliders(){SM.forEach(([id,k,f])=>{const r=$('#'+id);r.value=D[k];rangeFill(r);$('#v'+id.slice(1)).textContent=f(D[k]);
  r.oninput=()=>{D[k]=+r.value;draftDirty=true;rangeFill(r);$('#v'+id.slice(1)).textContent=f(D[k])}})}

// ---------- usage chart
function renderUsage(){
  if(!U||!C)return;
  const days=U.days,gal=d=>d.sec.reduce((a,s,i)=>a+s/60*C.zones[i].gpm,0);
  const tot=days.reduce((a,d)=>a+gal(d),0),sec=days.reduce((a,d)=>a+d.sec.reduce((x,y)=>x+y,0),0);
  const saved=Math.max(0,days.reduce((a,d)=>a+(d.pct!==255?d.base-d.prog:0),0)),base=days.reduce((a,d)=>a+d.base,0);
  const cost=(MET()?tot*3.785:tot)/1000*(C.cost||0);
  $('#kpis').innerHTML=`<div class="kpi"><span>30-day use</span><b>${VOL(tot)}<small>${VU()}</small></b></div><div class="kpi"><span>Run time</span><b>${(sec/3600).toFixed(1)}<small>hrs</small></b></div>
   <div class="kpi"><span>Smart savings</span><b style="color:#22c55e">${VOL(saved)}<small>${VU()}${base?' · '+Math.round(saved/base*100)+'%':''}</small></b></div><div class="kpi"><span>Est. cost</span><b>$${cost.toFixed(2)}</b></div>`;
  // stacked bars, one axis, 2px gaps between segments
  const Wd=340,Hd=180,L=34,B=158,top=10,slot=(Wd-L-4)/30,bw=slot-2;
  let mx=Math.max(1,...days.map(gal));const nice=[10,20,50,100,200,250,500,1000,2000,5000,10000,50000];mx=nice.find(n=>n>=mx)||mx;
  const y=v=>B-(v/mx)*(B-top);
  let s=`<svg class="chart" viewBox="0 0 ${Wd} ${Hd}" role="img" aria-label="Daily water use for the last 30 days, stacked by zone">`;
  [0,.5,1].forEach(f=>{const yy=y(mx*f);s+=`<line x1="${L}" x2="${Wd-4}" y1="${yy}" y2="${yy}" stroke="rgba(148,178,220,${f?0.12:0.25})" stroke-dasharray="${f?'3 3':''}"/><text x="${L-6}" y="${yy+3}" fill="#6b7f96" font-size="9" text-anchor="end">${VOL(mx*f)}</text>`});
  days.forEach((d,i)=>{let base=B;const x=L+i*slot+1;
    s+=`<g data-i="${i}">`;
    d.sec.forEach((sv,z)=>{const g=sv/60*C.zones[z].gpm;if(g<=0)return;const h=Math.max(1,(g/mx)*(B-top)),yy=base-h,gap=base<B?2:0;
      s+=`<rect x="${x}" y="${yy}" width="${bw}" height="${Math.max(.5,h-gap)}" rx="1.5" fill="${ZC[z]}"/>`;base=yy});
    if(d.pct===0)s+=`<circle cx="${x+bw/2}" cy="${B-3}" r="2" fill="#7dd3fc"/>`;
    s+=`<rect class="hit" x="${L+i*slot}" y="${top}" width="${slot}" height="${B-top+12}" fill="transparent"/></g>`});
  const lbl=n=>new Date(n*864e5).toLocaleDateString('en-US',{month:'short',day:'numeric',timeZone:'UTC'});
  s+=`<text x="${L}" y="${Hd-6}" fill="#6b7f96" font-size="9">${lbl(days[0].day)}</text><text x="${Wd-4}" y="${Hd-6}" fill="#6b7f96" font-size="9" text-anchor="end">Today</text></svg><div class="tip hidden" id="tip"></div>`;
  $('#chartWrap').innerHTML=s;
  const tip=$('#tip'),wrap=$('#chartWrap');
  const show=e=>{const g=e.target.closest('g[data-i]');if(!g){tip.classList.add('hidden');return}const d=days[+g.dataset.i];
    const rows=d.sec.map((sv,z)=>sv?`<div class="r"><span><i class="sw" style="--c:${ZC[z]}"></i> ${esc(C.zones[z].name)}</span><span>${VOL(sv/60*C.zones[z].gpm)} · ${Math.round(sv/60)}m</span></div>`:'').join('');
    tip.innerHTML=`<b>${new Date(d.day*864e5).toLocaleDateString('en-US',{weekday:'short',month:'short',day:'numeric',timeZone:'UTC'})}</b> · ${VOL(gal(d))} ${VU()}${d.pct===0?' · <span style="color:#7dd3fc">skipped</span>':d.pct!==255?' · smart '+d.pct+'%':''}${rows||'<div class="muted">No watering</div>'}`;
    const r=wrap.getBoundingClientRect(),gr=g.getBoundingClientRect(),bars=[...g.querySelectorAll('rect:not(.hit)')].map(b=>b.getBoundingClientRect().top),top=bars.length?Math.min(...bars):gr.bottom-20;
    tip.style.left=Math.min(Math.max(gr.left+gr.width/2-r.left,80),r.width-80)+'px';tip.style.top=(top-r.top-8)+'px';tip.classList.remove('hidden')};
  wrap.onpointermove=show;wrap.onpointerdown=show;wrap.onpointerleave=()=>tip.classList.add('hidden');
  $('#usageLegend').innerHTML=C.zones.map((z,i)=>`<span style="--c:${ZC[i]}"><i class="sw"></i>${esc(z.name)}</span>`).join('')+'<span><svg width="8" height="8"><circle cx="4" cy="4" r="3" fill="#7dd3fc"/></svg>Skipped by smart / rain delay</span>';
  const per=C.zones.map((z,i)=>({i,s:days.reduce((a,d)=>a+d.sec[i],0)})).map(o=>({...o,g:o.s/60*C.zones[o.i].gpm})),pm=Math.max(1,...per.map(o=>o.g));
  $('#byZone').innerHTML=per.map(o=>`<div class="hb" style="--c:${ZC[o.i]}"><span>${o.i+1} · ${esc(C.zones[o.i].name)}</span><span class="sub small">${VOL(o.g)} ${VU()} · ${Math.round(o.s/60)} min</span><div class="track"><i style="width:${o.g/pm*100}%"></i></div></div>`).join('');
  $('#usageTable').innerHTML='<table><tr><th>Day</th>'+C.zones.map((z,i)=>`<th>${i+1}</th>`).join('')+`<th>${VU()}</th></tr>`+days.slice().reverse().filter(d=>gal(d)>0||d.pct!==255).map(d=>`<tr><td>${lbl(d.day)}</td>${d.sec.map(s=>`<td>${s?Math.round(s/60):''}</td>`).join('')}<td>${VOL(gal(d))}</td></tr>`).join('')+'</table><p class="small muted">Minutes per zone.</p>';
}

// ---------- settings
function kv(el,rows){el.innerHTML=rows.map(([k,v])=>`<span>${k}</span><span>${esc(v)}</span>`).join('')}
function renderSettings(){
  if(!C)return;
  const off=S&&S.utcOff!==undefined?'UTC'+(S.utcOff>=0?'+':'-')+(Math.abs(S.utcOff)/60|0)+(Math.abs(S.utcOff)%60?':'+pad(Math.abs(S.utcOff)%60):''):'';
  let sun='-';if(W&&W.valid){const d=W.days[W.today],f=u=>{try{return new Date(u*1000).toLocaleTimeString('en-US',{hour:'numeric',minute:'2-digit',hour12:!C.clock24,timeZone:C.tz||undefined})}catch(e){return '-'}};sun=f(d.rise)+' / '+f(d.set)}
  kv($('#locKv'),[['Place',C.hasLoc?(C.place||'Set')+(C.locIP?' (approx)':''):'Not set'],['Coordinates',C.hasLoc?(+C.lat).toFixed(4)+', '+(+C.lon).toFixed(4):'-'],['Time zone',C.tz||'-'],['Offset',S&&S.tv?(S.tzAbbr+' · '+off+(S.dst?' · DST':'')):'-'],['Sunrise / sunset',sun],['Phone zone',Intl.DateTimeFormat().resolvedOptions().timeZone]]);
  const zs=$('#zSet');
  zs.innerHTML=C.zones.map((z,i)=>`<div class="field" style="--c:${ZC[i]}"><i class="sw" style="width:12px;height:12px;border-radius:50%"></i><input type="text" value="${esc(z.name)}" data-z="${i}" data-k="name" maxlength="23" style="flex:1"><input type="number" value="${z.gpm}" step="0.1" min="0" data-z="${i}" data-k="gpm" style="width:70px" title="Flow rate gal/min"><label class="sw2"><input type="checkbox" ${z.en?'checked':''} data-z="${i}" data-k="en"><span></span></label></div>`).join('')+'<p class="small muted">Name, flow rate (gal/min) and enabled. Flow rate sets the usage estimate.</p>';
  $$('input',zs).forEach(inp=>inp.onchange=()=>{const a=C.zones.map(()=>({}));a[inp.dataset.z][inp.dataset.k]=inp.type==='checkbox'?inp.checked:inp.type==='number'?+inp.value:inp.value;saveCfg({zones:a})});
  $('#pMetric').checked=C.metric;$('#pClock').checked=C.clock24;$('#pSound').checked=C.sound;$('#pMulti').checked=C.multi;
  $('#pBright').value=C.bright;rangeFill($('#pBright'));$('#vBright').textContent=Math.round(C.bright/2.55)+'%';
  $('#pSaver').value=C.saver;$('#pManual').value=C.manualMin;$('#pMaxRun').value=C.maxRunMin;$('#pCost').value=C.cost;$('#pDrain').value=C.drainMin;$('#pWinter').checked=C.winter;
  $('#wSsid').value=$('#wSsid').value||C.ssid||'';
  $('#gpsNote').classList.toggle('hidden',window.isSecureContext);
  renderDevice();
}
function renderDevice(){if(!S)return;const u=S.up;kv($('#devKv'),[['Address','http://'+S.ip],['Also','http://irrigationgenie.local'],['WiFi',S.wifi?S.ssid+' ('+S.rssi+' dBm)':S.ap?'Setup access point':'Disconnected'],['Internet',S.inet?'OK':'Lost'],['Weather',W&&W.valid?'Updated '+new Date(W.fetched*1000).toLocaleTimeString([], {hour:'numeric',minute:'2-digit'}):'No data'],['Uptime',(u/86400|0)+'d '+((u/3600|0)%24)+'h '+((u/60|0)%60)+'m'],['Firmware','v'+S.fw],['Free memory',Math.round(S.heap/1024)+' KB']])}

// ---------- location
async function sendLocation(lat,lon,place,tz){
  const posix=posixTZ(tz);
  const off=-new Date().getTimezoneOffset();
  try{await api('/api/location',{lat:+(+lat).toFixed(5),lon:+(+lon).toFixed(5),place:place.slice(0,47),tz,posix,offset:off});toast('Location set: '+place);$('#locRes').innerHTML='';await loadConfig();setTimeout(loadWeather,4000)}
  catch(e){toast('Could not set location',1)}
}
function useGPS(){
  const tz=Intl.DateTimeFormat().resolvedOptions().timeZone;
  if(!navigator.geolocation||!window.isSecureContext){$('#gpsNote').classList.remove('hidden');toast('GPS needs https, search below instead',1);$('#locQ').focus();return}
  toast('Getting location...');
  navigator.geolocation.getCurrentPosition(async p=>{
    const {latitude:la,longitude:lo}=p.coords;let place=la.toFixed(3)+', '+lo.toFixed(3);
    try{const r=await (await fetch(`https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=${la}&longitude=${lo}&localityLanguage=en`)).json();place=[r.city||r.locality,r.principalSubdivisionCode?r.principalSubdivisionCode.split('-').pop():r.principalSubdivision,r.countryCode].filter(Boolean).join(', ')}catch(e){}
    sendLocation(la,lo,place,tz)},e=>{$('#gpsNote').classList.remove('hidden');toast('Location blocked: '+e.message,1)},{enableHighAccuracy:true,timeout:15000})
}
async function searchLoc(){
  const q=$('#locQ').value.trim();if(!q)return;
  try{const r=await (await fetch('https://geocoding-api.open-meteo.com/v1/search?count=6&language=en&format=json&name='+encodeURIComponent(q))).json();
    const res=r.results||[];
    $('#locRes').innerHTML=res.length?res.map((x,i)=>`<button data-i="${i}"><b>${esc(x.name)}</b> <span class="sub small">${esc([x.admin1,x.country].filter(Boolean).join(', '))} · ${esc(x.timezone||'')}</span></button>`).join(''):'<p class="muted small">No match.</p>';
    $$('#locRes button').forEach(b=>b.onclick=()=>{const x=res[+b.dataset.i];sendLocation(x.latitude,x.longitude,[x.name,x.admin1_code||x.admin1,x.country_code].filter(Boolean).join(', '),x.timezone||Intl.DateTimeFormat().resolvedOptions().timeZone)});
  }catch(e){toast('Search needs internet on your phone',1)}
}

// ---------- map editor
const ED={step:1,src:'plan',img:null,wx:0,wy:0,z:0,tiles:new Map(),zones:null,sel:0,badge:false,bgChanged:false};
const cv=$('#edCanvas'),ctx=cv.getContext('2d'),EW=960,EH=528;
function openEditor(){
  ED.zones=C.zones.map(z=>({pts:z.pts.map(p=>p.slice()),bx:z.bx,by:z.by}));ED.sel=0;ED.bgChanged=false;ED.badge=false;
  if(S.hasMap){const im=new Image();im.onload=()=>{ED.img=im;ED.src='img';ED.wx=im.naturalWidth/2;ED.wy=im.naturalHeight/2;ED.z=Math.log2(Math.max(EW/im.naturalWidth,EH/im.naturalHeight));edDraw()};im.src='/map.jpg?'+Date.now();ED.src='img'}
  else ED.src='plan';
  $('#editor').classList.remove('hidden');document.body.style.overflow='hidden';edStep(1);edDraw();
}
function closeEditor(){$('#editor').classList.add('hidden');document.body.style.overflow=''}
function edStep(n){ED.step=n;$$('.stepper button').forEach(b=>b.classList.toggle('on',+b.dataset.s===n));$('#edStep1').classList.toggle('hidden',n!==1);$('#edStep2').classList.toggle('hidden',n!==2);
  $('#edTools').classList.toggle('hidden',n!==2);$('#edZoom').classList.toggle('hidden',n!==1||ED.src==='plan');edChips();edOverlay()}
function edChips(){$$('#edSrc button').forEach(b=>b.classList.toggle('on',b.dataset.src===ED.src));
  $('#edZones').innerHTML=ED.zones.map((z,i)=>`<button data-z="${i}" class="${i===ED.sel?'on':''}" style="--c:${ZC[i]}"><i class="sw" style="--c:${ZC[i]}"></i> ${i+1} ${esc(C.zones[i].name)}</button>`).join('');
  $('#edAttr').textContent=ED.src==='sat'?'Imagery © Esri, Maxar, Earthstar Geographics':ED.src==='plan'?'Your cleaned up yard plan':'';
  $('#edHint1').textContent=ED.src==='plan'?'The yard plan is drawn by IrrigationGenie, nothing to upload.':'Drag to move, pinch or use +/- to zoom.'}
function edDraw(){
  ctx.fillStyle='#0b1426';ctx.fillRect(0,0,EW,EH);
  if(ED.src==='plan'){const P=C.plan,poly=(a,f,s)=>{ctx.beginPath();a.forEach((p,i)=>ctx[i?'lineTo':'moveTo'](p[0]/1000*EW,p[1]/1000*EH));ctx.closePath();ctx.fillStyle=f;ctx.fill();ctx.strokeStyle=s;ctx.lineWidth=2;ctx.stroke()};
    poly(P.property,'#13261d','#3b5b4a');poly(P.drive,'#2b3442','#64748b');poly(P.house,'#33415c','#9fb3c8')}
  else if(ED.src==='img'&&ED.img){const s=2**ED.z;ctx.drawImage(ED.img,EW/2-ED.wx*s,EH/2-ED.wy*s,ED.img.naturalWidth*s,ED.img.naturalHeight*s)}
  else if(ED.src==='sat')edTiles();
}
function edTiles(){
  const zt=Math.min(19,Math.ceil(ED.z)),ts=2**(ED.z-zt),n=2**zt,x0=ED.wx*n-EW/2/ts,y0=ED.wy*n-EH/2/ts;
  for(let ty=Math.floor(y0/256);ty<=Math.floor((y0+EH/ts)/256);ty++)for(let tx=Math.floor(x0/256);tx<=Math.floor((x0+EW/ts)/256);tx++){
    const k=zt+'/'+ty+'/'+tx;let im=ED.tiles.get(k);
    if(!im){im=new Image();im.crossOrigin='anonymous';im.onload=()=>ED.src==='sat'&&edDraw();im.src=`https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/${zt}/${ty}/${tx}`;ED.tiles.set(k,im)}
    if(im.complete&&im.naturalWidth)ctx.drawImage(im,(tx*256-x0)*ts,(ty*256-y0)*ts,256*ts+.6,256*ts+.6)}
}
function edOverlay(){$('#edSvg').innerHTML=mapHTML(true);$('#edBadges').innerHTML=badgesHTML(ED.zones,true);$('#edBadges').style.pointerEvents='none';$$('#edBadges .badge').forEach(b=>b.style.pointerEvents='none')}
const clampZ=z=>ED.src==='sat'?Math.max(3,Math.min(21,z)):Math.max(-4,Math.min(5,z));
function edZoom(dz){ED.z=clampZ(ED.z+dz);ED.bgChanged=true;edDraw()}
function centroid(p){let a=0,x=0,y=0;for(let i=0,j=p.length-1;i<p.length;j=i++){const f=p[j][0]*p[i][1]-p[i][0]*p[j][1];a+=f;x+=(p[j][0]+p[i][0])*f;y+=(p[j][1]+p[i][1])*f}
  if(Math.abs(a)<1)return [p.reduce((s,q)=>s+q[0],0)/p.length|0,p.reduce((s,q)=>s+q[1],0)/p.length|0];return [Math.round(x/(3*a)),Math.round(y/(3*a))]}
(function(){
  const wrap=$('#edWrap'),ptr=new Map();let moved=false;
  wrap.onpointerdown=e=>{wrap.setPointerCapture(e.pointerId);ptr.set(e.pointerId,{x:e.clientX,y:e.clientY});moved=false};
  wrap.onpointermove=e=>{const p=ptr.get(e.pointerId);if(!p)return;const k=EW/wrap.clientWidth;
    if(ED.step===1&&ED.src!=='plan'){
      if(ptr.size===1){ED.wx-=(e.clientX-p.x)*k/2**ED.z;ED.wy-=(e.clientY-p.y)*k/2**ED.z}
      else if(ptr.size===2){const o=[...ptr.entries()].find(([id])=>id!==e.pointerId)[1],d0=Math.hypot(p.x-o.x,p.y-o.y),d1=Math.hypot(e.clientX-o.x,e.clientY-o.y);if(d0>0)ED.z=clampZ(ED.z+Math.log2(d1/d0))}
      ED.bgChanged=true;edDraw()}
    if(Math.abs(e.clientX-p.x)+Math.abs(e.clientY-p.y)>2)moved=true;ptr.set(e.pointerId,{x:e.clientX,y:e.clientY})};
  wrap.onpointerup=e=>{ptr.delete(e.pointerId);if(ED.step!==2||moved)return;
    const r=wrap.getBoundingClientRect(),x=Math.round((e.clientX-r.left)/r.width*1000),y=Math.round((e.clientY-r.top)/r.height*1000),z=ED.zones[ED.sel];
    if(ED.badge){z.bx=x;z.by=y}else{if(z.pts.length>=12)return toast('12 points max');z.pts.push([x,y]);if(z.pts.length>=3)[z.bx,z.by]=centroid(z.pts)}
    edOverlay()};
  wrap.onpointercancel=e=>ptr.delete(e.pointerId);
  wrap.onwheel=e=>{if(ED.step===1&&ED.src!=='plan'){e.preventDefault();edZoom(-e.deltaY*.002)}};
})();
function lonlat2world(lon,lat){const s=Math.sin(lat*Math.PI/180);return [(lon+180)/360*256,(.5-Math.log((1+s)/(1-s))/(4*Math.PI))*256]}
async function edSave(){
  const b=$('#edSave');b.disabled=true;b.textContent='Saving...';
  try{
    if(ED.src==='plan'){if(S.hasMap)await api('/api/map',null,'DELETE')}
    else if(ED.bgChanged){
      const jpg=await new Promise(r=>cv.toBlob(r,'image/jpeg',.85));
      const sm=document.createElement('canvas');sm.width=320;sm.height=176;const sx=sm.getContext('2d');sx.imageSmoothingQuality='high';sx.drawImage(cv,0,0,320,176);
      const d=sx.getImageData(0,0,320,176).data,out=new Uint8Array(320*176*2);
      for(let i=0,j=0;i<d.length;i+=4,j+=2){const v=((d[i]&0xF8)<<8)|((d[i+1]&0xFC)<<3)|(d[i+2]>>3);out[j]=v&255;out[j+1]=v>>8}
      const fd=new FormData();fd.append('rgb',new Blob([out]),'map.rgb');fd.append('jpg',jpg,'map.jpg');
      const r=await fetch('/api/map',{method:'POST',body:fd});if(!r.ok)throw new Error('upload failed');
    }
    await api('/api/config',{zones:ED.zones.map(z=>({pts:z.pts,bx:z.bx,by:z.by}))});
    toast('Map saved');closeEditor();await loadConfig();await poll();renderMap();
  }catch(e){toast(e.name==='SecurityError'?'Browser blocked the satellite image, use Photo instead':'Save failed: '+e.message,1)}
  b.disabled=false;b.textContent='Save map';
}

// ---------- wiring
function go(v){view=v;$$('.view').forEach(s=>s.classList.toggle('active',s.id==='v-'+v));$$('#tabs button').forEach(b=>b.classList.toggle('on',b.dataset.v===v));window.scrollTo({top:0,behavior:'smooth'});
  if(v==='usage')loadUsage();if(v==='map')renderMap();if(v==='set')renderSettings()}
$$('#tabs button').forEach(b=>b.onclick=()=>go(b.dataset.v));
$('#modeSeg').onclick=e=>{const b=e.target.closest('button');if(b)cmd('mode',{mode:+b.dataset.m})};
$('#modeCards').onclick=e=>{const b=e.target.closest('.mc');if(b)cmd('mode',{mode:+b.dataset.m}).then(()=>toast(['Manual','Schedule','Smart'][b.dataset.m]+' mode'))};
document.addEventListener('click',e=>{const z=e.target.closest('[data-z]');if(z&&(z.closest('#zgrid')||z.closest('#mapBox')))openSheet(+z.dataset.z)});
$('#scrim').onclick=closeSheet;
$('#stopBtn').onclick=()=>cmd('stopall').then(()=>toast('All zones off'));
$('#masterBtn').onclick=$('#masterBtn2').onclick=()=>cmd('master',{on:!S.masterManual});
$('#delayBtn').onclick=()=>{const h=S.rainDelay?(S.rainDelay>48*3600?0:S.rainDelay>24*3600?72:48):24;cmd('raindelay',{hours:h}).then(()=>toast(h?'Rain delay '+h+' h':'Rain delay off'))};
$('#days').onclick=e=>{const b=e.target.closest('button');if(!b)return;D.days^=1<<b.dataset.d;draftDirty=true;b.classList.toggle('on')};
for(let k=0;k<2;k++){$('#st'+k).onchange=e=>{D.starts[k].en=e.target.checked;draftDirty=true};$('#tm'+k).onchange=e=>{const [h,m]=e.target.value.split(':');D.starts[k].min=h*60+ +m;draftDirty=true}}
$('#saveSched').onclick=async()=>{draftDirty=false;const o={days:D.days,starts:D.starts,zones:D.zones.map(z=>({min:z.min}))};SM.forEach(([,k])=>o[k]=D[k]);await saveCfg(o,'Schedule saved')};
$('#tblBtn').onclick=()=>{$('#usageTable').classList.toggle('hidden');$('#tblBtn').textContent=$('#usageTable').classList.contains('hidden')?'Table':'Chart only'};
$('#gpsBtn').onclick=useGPS;$('#locGo').onclick=searchLoc;$('#locQ').onkeydown=e=>{if(e.key==='Enter')searchLoc()};
$('#ipBtn').onclick=()=>cmd('locateip').then(()=>{toast('Locating...');setTimeout(loadConfig,4000)});
const pref=(id,k,f)=>$(id).onchange=e=>saveCfg({[k]:f?f(e.target):e.target.checked});
pref('#pMetric','metric');pref('#pClock','clock24');pref('#pSound','sound');pref('#pMulti','multi');
pref('#pSaver','saver',t=>+t.value);pref('#pManual','manualMin',t=>+t.value);pref('#pMaxRun','maxRunMin',t=>+t.value);pref('#pCost','cost',t=>+t.value);pref('#pDrain','drainMin',t=>+t.value);
$('#pBright').oninput=e=>{rangeFill(e.target);$('#vBright').textContent=Math.round(e.target.value/2.55)+'%'};pref('#pBright','bright',t=>+t.value);
$('#pWinter').onchange=e=>{const on=e.target.checked;if(!confirm(on?'Enter winter mode? The master valve closes and all zones open to drain the lines.':'Exit winter mode and resume watering?')){e.target.checked=!on;return}cmd('winter',{on})};
$('#wxRefresh').onclick=()=>cmd('refresh').then(()=>toast('Refreshing weather'));
$('#rebootBtn').onclick=()=>{if(confirm('Restart IrrigationGenie?'))cmd('reboot').then(()=>toast('Restarting...'))};
$('#wScan').onclick=async()=>{toast('Scanning...');try{const l=await api('/api/scan');$('#ssids').innerHTML=l.map(n=>`<option value="${esc(n.ssid)}">${n.rssi} dBm</option>`).join('');toast(l.length+' networks found')}catch(e){toast('Scan failed',1)}};
$('#wSave').onclick=async()=>{const ssid=$('#wSsid').value.trim();if(!ssid)return toast('Enter the network name',1);try{await api('/api/wifi',{ssid,pass:$('#wPass').value});toast('Connecting... rejoin your home WiFi and open irrigationgenie.local')}catch(e){toast('Failed',1)}};
$('#editMap').onclick=openEditor;$('#edClose').onclick=closeEditor;$('#edSave').onclick=edSave;
$$('.stepper button').forEach(b=>b.onclick=()=>edStep(+b.dataset.s));
$('#edSrc').onclick=e=>{const b=e.target.closest('button');if(!b)return;const s=b.dataset.src;
  if(s==='img'){$('#edFile').click();return}
  if(s==='sat'){if(!C.hasLoc)return toast('Set your location first (Settings)',1);[ED.wx,ED.wy]=lonlat2world(+C.lon,+C.lat);ED.z=19;ED.bgChanged=true}
  ED.src=s;edStep(1);edDraw()};
$('#edFile').onchange=e=>{const f=e.target.files[0];if(!f)return;const im=new Image();im.onload=()=>{ED.img=im;ED.src='img';ED.wx=im.naturalWidth/2;ED.wy=im.naturalHeight/2;ED.z=Math.log2(Math.max(EW/im.naturalWidth,EH/im.naturalHeight));ED.bgChanged=true;edStep(1);edDraw()};im.src=URL.createObjectURL(f)};
$('#edIn').onclick=()=>edZoom(.5);$('#edOut').onclick=()=>edZoom(-.5);
$('#edZones').onclick=e=>{const b=e.target.closest('button');if(b){ED.sel=+b.dataset.z;edChips();edOverlay()}};
$('#edMode').onclick=e=>{const b=e.target.closest('button');if(!b)return;ED.badge=b.dataset.k==='1';$$('#edMode button').forEach(x=>x.classList.toggle('on',x===b));$('#edMode .sl').style.transform=`translateX(${ED.badge?100:0}%)`};
$('#edUndo').onclick=()=>{const z=ED.zones[ED.sel];z.pts.pop();if(z.pts.length>=3)[z.bx,z.by]=centroid(z.pts);edOverlay()};
$('#edClear').onclick=()=>{ED.zones[ED.sel].pts=[];edOverlay()};

poll();setInterval(poll,2000);
</script>
</body></html>)rawliteral";
