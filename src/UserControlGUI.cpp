#include "UserControlGUI.h"

const char* HTML_PAGE = R"EOF(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Robot Autonomo S3</title>
    <style>
*,::before,::after{box-sizing:border-box;border-width:0;border-style:solid}
:root{--sl-900:#0f172a;--sl-800:#1e293b;--sl-700:#334155;--sl-600:#475569;--sl-500:#64748b;--sl-400:#94a3b8;--sl-300:#cbd5e1;--bl-900:#1e3a8a;--bl-600:#2563eb;--bl-500:#3b82f6;--bl-400:#60a5fa;--bl-300:#93c5fd;--bl-200:#bfdbfe;--bl-100:#dbeafe;--pu-600:#9333ea;--pu-500:#a855f7;--pu-400:#c084fc;--pu-300:#d8b4fe;--rd-600:#dc2626;--rd-500:#ef4444;--rd-400:#f87171;--am-600:#d97706;--am-500:#f59e0b;--gr-500:#22c55e;--gr-400:#4ade80;--em-900:#064e3b;--em-600:#059669;--em-500:#10b981;--em-400:#34d399;--em-300:#6ee7b7;--in-600:#4f46e5;--in-500:#6366f1;--ye-400:#facc15;--cy-400:#22d3ee;--cy-700:#0e7490}
body{margin:0;font-family:'Courier New',Consolas,monospace;background:var(--sl-900);background-image:linear-gradient(rgba(34,211,238,0.04) 1px,transparent 1px),linear-gradient(90deg,rgba(34,211,238,0.04) 1px,transparent 1px);background-size:40px 40px;color:#e2e8f0;overflow-x:hidden}
.panel-tech{background:var(--sl-900);border:1px solid var(--cy-700);clip-path:polygon(0 8px,8px 0,calc(100% - 8px) 0,100% 8px,100% calc(100% - 8px),calc(100% - 8px) 100%,8px 100%,0 calc(100% - 8px))}
.panel-header{background:linear-gradient(90deg,var(--cy-700),transparent);border-bottom:1px solid var(--cy-700);padding:6px 12px;font-size:11px;letter-spacing:2px;text-transform:uppercase;color:var(--cy-400);display:flex;justify-content:space-between;align-items:center}
.data-value{font-family:'Courier New',monospace;color:var(--cy-400);font-weight:700}
.data-label{color:var(--sl-500);font-size:11px;text-transform:uppercase;letter-spacing:1px}
.fsm-badge{padding:3px 8px;font-size:11px;letter-spacing:2px;border:1px solid;text-transform:uppercase}
.led-dot{width:10px;height:10px;border-radius:50%;display:inline-block;transition:.3s}
.led-on{box-shadow:0 0 8px currentColor}
.led-off{opacity:.2}
.btn-cmd{background:var(--sl-800);border:1px solid var(--cy-700);color:#e2e8f0;padding:8px 14px;font-family:inherit;font-size:12px;letter-spacing:1px;text-transform:uppercase;cursor:pointer;transition:.2s;clip-path:polygon(6px 0,100% 0,calc(100% - 6px) 100%,0 100%)}
.btn-cmd:hover{background:var(--cy-700);color:#fff;box-shadow:0 0 12px rgba(34,211,238,.4)}
.btn-cmd:active{transform:scale(.97)}
.btn-estop{background:rgba(239,68,68,.15);border:2px solid var(--rd-500);color:var(--rd-500);padding:12px 24px;font-size:14px;font-weight:700;letter-spacing:3px;text-transform:uppercase;cursor:pointer;animation:pulse-red 2s infinite;transition:.2s}
.btn-estop:hover{background:var(--rd-500);color:#fff;box-shadow:0 0 20px rgba(239,68,68,.6)}
@keyframes pulse-red{0%,100%{box-shadow:0 0 5px rgba(239,68,68,.3)}50%{box-shadow:0 0 20px rgba(239,68,68,.6)}}
.slider-tech{-webkit-appearance:none;appearance:none;width:100%;height:6px;background:var(--sl-800);border:1px solid var(--cy-700);outline:none}
.slider-tech::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:14px;height:18px;background:var(--cy-400);border:1px solid #67e8f9;cursor:pointer;clip-path:polygon(0 20%,50% 0,100% 20%,100% 100%,0 100%)}
.slider-tech::-moz-range-thumb{width:14px;height:18px;background:var(--cy-400);border:1px solid #67e8f9;cursor:pointer}
@keyframes spin-cw{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}
@keyframes spin-ccw{from{transform:rotate(0deg)}to{transform:rotate(-360deg)}}
.motor-spin-cw{animation:spin-cw var(--motor-dur,2s) linear infinite}
.motor-spin-ccw{animation:spin-ccw var(--motor-dur,2s) linear infinite}
.motor-stop{animation:none}
.motor-ring{border-color:var(--sl-600);box-shadow:none;transition:.3s}
.motor-fwd{border-color:var(--cy-400);box-shadow:0 0 12px rgba(34,211,238,.5)}
.motor-rev{border-color:var(--rd-500);box-shadow:0 0 12px rgba(239,68,68,.5)}
.log-cmd{color:var(--cy-400)}.log-sys{color:var(--sl-400)}.log-err{color:var(--rd-400)}.log-warn{color:var(--am-400)}.log-motor{color:var(--gr-400)}
.log-container{overflow-y:auto;font-size:10px;font-family:'Courier New',monospace}
.log-container::-webkit-scrollbar{width:4px}
.log-container::-webkit-scrollbar-thumb{background:var(--cy-700)}
.card-content{flex:1;overflow-y:auto;padding:14px}
.card-local-log{height:40px;overflow-y:auto;font-size:9px;padding:4px 8px;border-top:1px solid var(--cy-700);background:rgba(2,6,23,.5);flex-shrink:0}
.card-local-log::-webkit-scrollbar{width:3px}
.card-local-log::-webkit-scrollbar-thumb{background:var(--cy-700)}
.fs-btn{font-size:10px;padding:1px 6px;border:1px solid var(--sl-600);color:var(--sl-400);background:none;cursor:pointer;border-radius:2px;transition:.2s}
.fs-btn:hover{border-color:var(--cy-400);color:var(--cy-400)}
:fullscreen .panel-tech{height:100vh;width:100vw;display:flex;flex-direction:column;border-radius:0;clip-path:none;}
:fullscreen .traj-container{flex:1;}
:fullscreen .canvas-container{height:100%;border-radius:0;border:none;}
:fullscreen .card-local-log{height:60px}
:-webkit-full-screen .panel-tech{height:100vh;width:100vw;display:flex;flex-direction:column;border-radius:0;clip-path:none;}
:-webkit-full-screen .traj-container{flex:1;}
:-webkit-full-screen .canvas-container{height:100%;border-radius:0;border:none;}
:-webkit-full-screen .card-local-log{height:60px}

/* Custom Utility & Layout Classes for index.html */
.app-header { border-bottom:1px solid rgba(34,211,238,.3); background:rgba(15,23,42,.8); padding:10px 20px; display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:10px; }
.header-logo-container { display:flex; align-items:center; gap:10px; }
.logo-dot { width:10px; height:10px; background:var(--cy-400); border-radius:50%; box-shadow:0 0 10px var(--cy-400); animation:pulse-cyan 2s infinite; }
@keyframes pulse-cyan { 0%, 100% { box-shadow: 0 0 5px rgba(34,211,238,0.3); } 50% { box-shadow: 0 0 15px rgba(34,211,238,0.8); } }
.header-title { color:var(--cy-400); font-size:18px; font-weight:700; letter-spacing:3px; }
.header-info { display:flex; align-items:center; gap:16px; font-size:11px; }
.fsm-badge-idle { border-color:var(--cy-500); color:var(--cy-400); }
.main-container { max-width:1600px; margin:20px auto; padding:0 15px; }

.panel-col { padding:20px; display:flex; flex-direction:column; gap:20px; }
.panel-title { color:white; font-size:18px; font-weight:bold; margin-bottom:10px; }

.btn-dir-turn { background:var(--pu-600); border-color:var(--pu-400); color:white; }
.btn-dir-rev { background:var(--am-600); border-color:var(--am-400); color:white; }

.pwm-widget { background:rgba(15,23,42,0.6); padding:16px; border-radius:12px; border:1px solid var(--sl-700); }
.pwm-header { display:flex; justify-content:space-between; align-items:center; margin-bottom:10px; }
.pwm-title { color:white; font-weight:bold; }
.pwm-value { background:rgba(255,255,255,0.1); padding:4px 10px; border-radius:6px; color:var(--cy-400); }
.pwm-slider-container { display:flex; align-items:center; gap:10px; }
.pwm-icon { font-size:24px; color:var(--ye-400); }

.estop-container { margin-top:auto; }
.btn-estop-full { width:100%; border-radius:8px; }

.grid-2col-gap16 { display:grid; grid-template-columns: 1fr 1fr; gap:16px; }
.data-label-mb4 { margin-bottom:4px; }
.text-pu-300 { color:var(--pu-300); }

.traj-container { flex:1; display:flex; flex-direction:column; gap:8px; }
.flex-between { display:flex; justify-content:space-between; }
.data-value-sm { font-size:10px; }
.canvas-container { position:relative; background:#020617; border:1px solid var(--sl-700); border-radius:8px; height:200px; width:100%; overflow:hidden; }
.canvas-layer { position:absolute; top:0; left:0; width:100%; height:100%; touch-action:none; }

.btn-glass-fs { position:absolute; top:10px; right:10px; z-index:10; background:linear-gradient(135deg, rgba(255,255,255,0.1), rgba(255,255,255,0)); backdrop-filter:blur(10px); -webkit-backdrop-filter:blur(10px); border:1px solid rgba(255,255,255,0.18); border-radius:8px; color:white; width:36px; height:36px; font-size:18px; display:flex; align-items:center; justify-content:center; cursor:pointer; box-shadow:0 8px 32px 0 rgba(0,0,0,0.37); transition:all 0.3s ease; outline:none; }
.btn-glass-fs:hover { background:linear-gradient(135deg, rgba(255,255,255,0.2), rgba(255,255,255,0.05)); border:1px solid rgba(34,211,238,0.5); color:var(--cy-400); box-shadow:0 0 15px rgba(34,211,238,0.3); transform:translateY(-2px); }
.btn-glass-fs:active { transform:scale(0.95); }

.zoom-controls { position:absolute; bottom:10px; right:10px; z-index:10; display:flex; flex-direction:column; gap:4px; }
.btn-glass-zoom { background:linear-gradient(135deg, rgba(255,255,255,0.1), rgba(255,255,255,0)); backdrop-filter:blur(10px); -webkit-backdrop-filter:blur(10px); border:1px solid rgba(255,255,255,0.18); border-radius:6px; color:white; width:28px; height:28px; font-size:16px; display:flex; align-items:center; justify-content:center; cursor:pointer; transition:all 0.2s; }
.btn-glass-zoom:hover { color:var(--cy-400); border-color:rgba(34,211,238,0.5); }


.pwm-bar-row { display:grid; grid-template-columns: 60px 1fr 40px; align-items:center; gap:10px; margin-top:8px; }
.pwm-bar-label { color:var(--sl-300); font-size:12px; }
.pwm-bar-val { font-size:12px; color:var(--cy-400); text-align:right; }
.enc-row { display:flex; justify-content:space-between; margin-top:8px; font-size:10px; color:var(--sl-500); }
.w-0 { width: 0%; }

.input-tech { width:100%; background:#020617; border:1px solid var(--sl-700); color:white; padding:8px; border-radius:4px; outline:none; }
.grid-2col-gap10 { display:grid; grid-template-columns: 1fr 1fr; gap:10px; }
.btn-cmd-add { background:var(--pu-600); border-color:var(--pu-400); }
.btn-cmd-go { background:var(--em-600); border-color:var(--em-400); }

.waypoints-container { flex:1; background:rgba(2,6,23,0.5); border:1px solid var(--sl-700); border-radius:8px; padding:10px; display:flex; flex-direction:column; }
.data-label-mb8 { margin-bottom:8px; }
.waypoints-list { flex:1; overflow-y:auto; font-size:11px; color:var(--sl-300); display:flex; flex-direction:column; gap:4px; }
.wp-item { background:rgba(255,255,255,0.03); border:1px solid var(--sl-800); border-radius:6px; padding:6px 8px; border-left:3px solid var(--pu-500); }
.wp-item-header { display:flex; justify-content:space-between; color:var(--pu-300); margin-bottom:2px; font-weight:bold; }
.wp-item-details { display:flex; justify-content:space-between; color:var(--sl-400); }
.waypoints-empty { text-align:center; margin-top:20px; opacity:0.5; padding:10px; }
.btn-cmd-clear { width:100%; margin-top:10px; background:var(--sl-700); }

/* --- NUEVAS CLASES PARA FSM DEBUG Y CRM NOTIFICACIONES --- */
.fsm-debug-container { display:flex; align-items:center; gap:8px; margin-left:16px; border-left:1px solid var(--sl-700); padding-left:16px; }
.fsm-select { background:rgba(2,6,23,0.8); border:1px solid var(--sl-600); color:var(--sl-200); font-family:monospace; font-size:10px; border-radius:4px; outline:none; cursor:pointer; }
.fsm-force-btn { position:static !important; width:28px !important; height:28px !important; font-size:12px !important; box-shadow:none !important; border-radius:4px !important; margin-left:4px; }
.btn-glass-crm { position:relative; background:linear-gradient(135deg, rgba(255,255,255,0.1), rgba(255,255,255,0.02)); backdrop-filter:blur(8px); -webkit-backdrop-filter:blur(8px); border:1px solid rgba(255,255,255,0.15); border-radius:50%; color:var(--cy-400); width:40px; height:40px; display:flex; align-items:center; justify-content:center; cursor:pointer; margin-left:16px; transition:0.3s; }
.btn-glass-crm:hover { box-shadow:0 0 15px rgba(34,211,238,0.3); transform:translateY(-1px); }
.notif-badge { position:absolute; top:-2px; right:-2px; background:var(--ye-500); color:#000; font-size:9px; font-weight:bold; border-radius:10px; padding:2px 5px; }

/* Offcanvas Notifications Panel */
.notifications-panel { position:fixed; top:60px; right:20px; width:300px; max-height:calc(100vh - 80px); background:rgba(15,23,42,0.95); backdrop-filter:blur(12px); border:1px solid var(--cy-700); border-radius:12px; z-index:100; display:flex; flex-direction:column; box-shadow:0 10px 40px rgba(0,0,0,0.5); overflow:hidden; }
.notif-header { display:flex; justify-content:space-between; align-items:center; padding:12px 16px; background:linear-gradient(90deg, rgba(14,165,233,0.2), transparent); border-bottom:1px solid var(--sl-700); font-weight:bold; color:var(--cy-300); }
.notif-list { flex:1; overflow-y:auto; padding:12px; display:flex; flex-direction:column; gap:8px; }
.notif-item { font-size:11px; padding:8px; background:rgba(255,255,255,0.03); border-radius:6px; border-left:3px solid var(--sl-500); }
.notif-item-time { color:var(--sl-400); font-size:9px; margin-bottom:2px; }
.notif-footer { padding:12px; border-top:1px solid var(--sl-700); background:rgba(2,6,23,0.5); }


</style>
    <style>
        .grid-3col { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 16px; }
        .control-pad { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 8px; margin-bottom: 20px; }
        .btn-dir { background: var(--bl-900); color: var(--bl-300); padding: 16px; border: 1px solid var(--bl-600); border-radius: 8px; text-align: center; cursor: pointer; -webkit-user-select: none; user-select: none; transition: 0.1s; font-weight: bold; text-transform: uppercase; }
        .btn-dir:active { transform: scale(0.95); background: var(--bl-600); color: white; }
        .btn-stop { background: rgba(239, 68, 68, 0.2); color: var(--rd-400); border: 1px solid var(--rd-500); }
        .btn-stop:active { background: var(--rd-500); color: white; }
        
        .huge-number { font-size: 28px; font-weight: 700; color: var(--pu-300); background: #020617; padding: 12px; border: 1px solid var(--sl-700); border-radius: 6px; text-align: center; }
        .progress-bar-container { background: #020617; border: 1px solid var(--sl-700); border-radius: 12px; height: 24px; padding: 3px; position: relative; overflow: hidden; display: flex; align-items: center; justify-content: space-between; }
        .progress-bar-fill { height: 100%; background: linear-gradient(90deg, var(--cy-700), var(--cy-400)); border-radius: 8px; transition: 0.2s width; }
        
        @media (max-width: 1000px) { .grid-3col { grid-template-columns: 1fr; } }
    </style>
</head>
<body>

<header class="app-header">
    <div class="header-logo-container">
        <div class="logo-dot"></div>
        <span class="header-title">ROBOT-AUTONOMO-S3</span>
    </div>
    <div class="header-info">
        <span class="data-label">IP:</span><span class="data-value" id="hdr-ip">192.168.4.1</span>
        
        <!-- FSM SELECTOR & BADGE -->
        <div class="fsm-debug-container">
            <span class="data-label">FSM:</span>
            <span id="fsm-badge" class="fsm-badge fsm-badge-idle">IDLE</span>
            <select id="fsm-override-select" class="fsm-select">
                <option value="INIT">INIT</option>
                <option value="CALIBRATING">CALIBRATING</option>
                <option value="IDLE">IDLE</option>
                <option value="ADVANCING">ADVANCING</option>
                <option value="TURNING">TURNING</option>
                <option value="BRAKING">BRAKING</option>
                <option value="ESTOP">ESTOP</option>
                <option value="MANUAL">MANUAL</option>
            </select>
            <button class="btn-glass-fs fsm-force-btn" onclick="window.app.forceFSM()" title="Forzar Estado FSM">⚙️</button>
        </div>

        <!-- CRM NOTIFICATIONS BUTTON -->
        <button class="btn-glass-crm" onclick="window.app.toggleNotifications()">
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"></path><path d="M13.73 21a2 2 0 0 1-3.46 0"></path></svg>
            <span id="notif-badge" class="notif-badge" style="display:none">0</span>
        </button>
    </div>
</header>

<!-- OFFCANVAS NOTIFICATIONS (Hidden by default) -->
<div id="notifications-panel" class="notifications-panel" style="display:none">
    <div class="notif-header">
        <span>Log de Sistema</span>
        <button class="btn-glass-fs" onclick="window.app.toggleNotifications()" style="position:static; width:28px; height:28px">✖</button>
    </div>
    <div id="notif-list" class="notif-list"></div>
    <div class="notif-footer">
        <button class="btn-cmd btn-cmd-go" style="width:100%" onclick="window.app.downloadLogsTXT()">Descargar TXT</button>
    </div>
</div>

<main class="main-container">
    <div class="grid-3col">
        
        <!-- COLUMNA 1: CONTROL MANUAL -->
        <div class="panel-tech panel-col">
            <div class="panel-title">Control Manual</div>
            
            <div class="control-pad">
                <div></div>
                <div class="btn-dir" id="btn-fwd">Avanzar</div>
                <div></div>
                <div class="btn-dir btn-dir-turn" id="btn-left">Girar</div>
                <div class="btn-dir btn-stop" id="btn-stop">Stop</div>
                <div class="btn-dir btn-dir-turn" id="btn-right">Girar</div>
                <div></div>
                <div class="btn-dir btn-dir-rev" id="btn-rev">Retroceder</div>
                <div></div>
            </div>

            <!-- PWM Control Widget Style -->
            <div class="pwm-widget">
                <div class="pwm-header">
                    <span class="pwm-title">Velocidad PWM</span>
                    <span id="lbl-pwm-val" class="pwm-value">150</span>
                </div>
                <div class="pwm-slider-container">
                    <span class="pwm-icon">⚡</span>
                    <input type="range" min="0" max="255" value="150" class="slider-tech" id="sl-pwm-gral" title="Velocidad PWM" oninput="window.ui.setPWM('i',this.value); window.ui.setPWM('d',this.value); document.getElementById('lbl-pwm-val').textContent=this.value;">
                </div>
            </div>

            <div class="estop-container">
                <button class="btn-estop btn-estop-full" onclick="window.ui.cmd('estop')">EMERGENCY STOP</button>
            </div>
        </div>

        <!-- COLUMNA 2: TELEMETRIA -->
        <div class="panel-tech panel-col">
            <div class="panel-title">Telemetría en Vivo</div>
            
            <div class="grid-2col-gap16">
                <div>
                    <div class="data-label data-label-mb4">DISTANCIA (CM)</div>
                    <div class="huge-number" id="tel-dist">0.0</div>
                </div>
                <div>
                    <div class="data-label data-label-mb4">ANGULO (°)</div>
                    <div class="huge-number text-pu-300" id="tel-ang">0.0</div>
                </div>
            </div>

            <div class="traj-container">
                <div class="flex-between">
                    <span class="data-label">TRAYECTORIA (X,Y CM)</span>
                    <span class="data-value data-value-sm"><span id="tel-posx">0.0</span>, <span id="tel-posy">0.0</span></span>
                </div>
                <!-- OPTIMIZACION DE CANVAS (Doble capa) -->
                <div class="canvas-container" id="telemetry-fullscreen-container">
                    <canvas id="canvas-bg" class="canvas-layer"></canvas>
                    <canvas id="canvas-fg" class="canvas-layer"></canvas>
                    <button class="btn-glass-fs" onclick="window.app.toggleFullscreen(document.getElementById('telemetry-fullscreen-container'))" title="Pantalla Completa">⛶</button>
                    <div class="zoom-controls">
                        <button class="btn-glass-zoom" onclick="if(window.app && window.app.canvasTrayectoria) window.app.canvasTrayectoria.zoomIn()" title="Zoom In">+</button>
                        <button class="btn-glass-zoom" onclick="if(window.app && window.app.canvasTrayectoria) window.app.canvasTrayectoria.zoomOut()" title="Zoom Out">-</button>
                    </div>
                </div>
            </div>

            <div>
                <span class="data-label">MOTORES (PWM)</span>
                <div class="pwm-bar-row">
                    <span class="pwm-bar-label">Izquierdo</span>
                    <div class="progress-bar-container"><div class="progress-bar-fill w-0" id="bar-pwm-i"></div></div>
                    <span id="lbl-pwm-i" class="pwm-bar-val">0</span>
                </div>
                <div class="pwm-bar-row">
                    <span class="pwm-bar-label">Derecho</span>
                    <div class="progress-bar-container"><div class="progress-bar-fill w-0" id="bar-pwm-d"></div></div>
                    <span id="lbl-pwm-d" class="pwm-bar-val">0</span>
                </div>
                <div class="enc-row">
                    <span>Pulsos L: <span id="enc-i">0</span></span>
                    <span>Pulsos R: <span id="enc-d">0</span></span>
                </div>
            </div>
        </div>

        <!-- COLUMNA 3: LABERINTO -->
        <div class="panel-tech panel-col">
            <div class="panel-title">Laberinto X/Y</div>
            
            <div class="grid-2col-gap16">
                <div>
                    <div class="data-label data-label-mb4">DESTINO X (CM)</div>
                    <input type="number" id="inp-dest-x" value="0" title="Destino X" class="input-tech">
                </div>
                <div>
                    <div class="data-label data-label-mb4">DESTINO Y (CM)</div>
                    <input type="number" id="inp-dest-y" value="0" title="Destino Y" class="input-tech">
                </div>
            </div>

            <div class="grid-2col-gap10">
                <button class="btn-cmd btn-cmd-add" onclick="window.app.agregarPuntoXY()">Agregar</button>
                <button class="btn-cmd btn-cmd-go" onclick="window.app.enviarRutaRobot()">Ir al Destino</button>
            </div>

            <div class="waypoints-container">
                <div class="flex-between data-label-mb8">
                    <span class="data-label">WAYPOINTS PROGRAMADOS</span>
                    <div style="display:flex; gap:4px">
                        <button class="btn-cmd btn-cmd-add" style="padding:2px 8px; font-size:10px" onclick="window.app.exportJSON()">↓ JSON</button>
                        <button class="btn-cmd btn-cmd-add" style="padding:2px 8px; font-size:10px" onclick="document.getElementById('file-json-import').click()">↑ JSON</button>
                        <input type="file" id="file-json-import" style="display:none" accept=".json" onchange="window.app.importJSON(event)">
                    </div>
                </div>
                <div id="waypoints-list" class="waypoints-list">
                    <div class="waypoints-empty">Ingrese X,Y destino arriba</div>
                </div>
                <button class="btn-cmd btn-cmd-clear" onclick="window.app.limpiarRuta()">Limpiar</button>
            </div>
        </div>

    </div>
</main>

<script>

// --- js/ui.js ---
window.ui = {
    // ── GLOBALES DE ESTADO ──
    S: {fsm:'IDLE',pwmI:150,pwmD:150,estI:'stop',estD:'stop',estop:false,logs:[]},
    
    // ── FSM ──
    FSM_STYLES: {INIT:'amber',CALIBRATING:'amber',IDLE:'green',ADVANCING:'cyan',TURNING:'cyan',BRAKING:'amber','E-STOP':'red',MANUAL:'cyan'},

    toggleLED(id, on){
        const e=document.getElementById(id)
        if(!e) return
        if(on){e.classList.remove('led-off');e.classList.add('led-on')}
        else{e.classList.add('led-off');e.classList.remove('led-on')}
    },

    updFSM(est){
        this.S.fsm=est
        const b=document.getElementById('fsm-badge'),c=this.FSM_STYLES[est]||'slate'
        const col={'amber':['#f59e0b','amber'],'green':['#22c55e','green'],'cyan':['#22d3ee','cyan'],'red':['#ef4444','red']}[c]||['#64748b','slate']
        b.style.borderColor=col[0];b.style.color=col[0];b.textContent=est
        this.toggleLED('led-idle',est==='IDLE'||est==='INIT'||est==='CALIBRATING')
        this.toggleLED('led-moving',est==='ADVANCING'||est==='TURNING'||est==='MANUAL')
        this.toggleLED('led-error',est==='E-STOP')
    },

    // ── CARD LOGS ──
    cardLog(id, msg){
        const el=document.getElementById('log-'+id)
        if(!el)return
        const t=new Date().toTimeString().substring(0,8)
        const d=document.createElement('div')
        d.textContent='['+t+'] '+msg
        el.appendChild(d)
        el.scrollTop=el.scrollHeight
        while(el.children.length>20)el.removeChild(el.firstChild)
    },

    // ── LOGS GLOBALES ──
    log(tipo, msg){
        const t=new Date().toTimeString().substring(0,8)
        const txt=`[${t}] ${tipo}: ${msg}`
        this.S.logs.push({texto:txt,timestamp:new Date().toISOString(),tipo,mensaje:msg})
        const c=document.getElementById('log-container'),d=document.createElement('div')
        d.className='log-'+tipo.toLowerCase()
        d.textContent=txt
        c.appendChild(d);c.scrollTop=c.scrollHeight
        if(c.children.length>40)c.removeChild(c.firstChild)
    },

    limpiarLog(){
        this.S.logs=[];
        // AP-001 XSS innerHTML fix
        document.getElementById('log-container').replaceChildren();
        this.log('SYS','Logs limpiados')
    },

    exportLog(){
        if(!this.S.logs.length){this.log('WARN','No hay logs');return}
        let txt='LOGS ROBOT\n'+'='.repeat(50)+'\n'
        this.S.logs.forEach(l=>txt+=l.texto+'\n')
        const b=new Blob([txt],{type:'text/plain'}),a=document.createElement('a')
        a.href=URL.createObjectURL(b);a.download='robot_logs.txt';a.click()
        this.log('SYS','Logs exportados')
    },

    // ── MOTORES ──
    calcAnim(pwm){return pwm<=0?'0s':(5-(pwm/255)*4.7).toFixed(2)+'s'},
    
    updMotor(lado, est, pwm){
        const r=document.getElementById('rotor-'+lado),m=document.getElementById('marker-'+lado),s=document.getElementById('status-m'+lado)
        if(!r || !m || !s) return
        r.classList.remove('motor-spin-cw','motor-spin-ccw','motor-stop')
        const ring=r.querySelector('.motor-ring')
        if(ring) ring.classList.remove('motor-fwd','motor-rev')
        
        if(est==='stop'||pwm===0){
            r.classList.add('motor-stop')
            m.style.background='var(--sl-600)';s.textContent='STOP';s.style.color='var(--sl-500)'
        }else if(est==='fwd'){
            r.classList.add('motor-spin-cw')
            r.style.setProperty('--motor-dur',this.calcAnim(pwm))
            if(ring) ring.classList.add('motor-fwd')
            m.style.background='var(--cy-400)';s.textContent='FWD '+pwm;s.style.color='var(--cy-400)'
        }else{
            r.classList.add('motor-spin-ccw')
            r.style.setProperty('--motor-dur',this.calcAnim(pwm))
            if(ring) ring.classList.add('motor-rev')
            m.style.background='var(--rd-500)';s.textContent='REV '+pwm;s.style.color='var(--rd-500)'
        }
        if(lado==='i'){this.S.estI=est;this.cardLog('motor-i',est+(pwm>0?' '+pwm:''))}
        else{this.S.estD=est;this.cardLog('motor-d',est+(pwm>0?' '+pwm:''))}
        this.updAnimSpeed()
    },

    updAnimSpeed(){
        const prom=(this.S.pwmI+this.S.pwmD)/2
        document.getElementById('anim-speed').textContent=prom===0?'— detenido':this.calcAnim(prom)+'/rev'
    },

    // ── PWM ──
    setPWM(lado, val){
        const pwm=parseInt(val)
        if(lado==='i'){
            this.S.pwmI=pwm;
            document.getElementById('lbl-pwm-i').textContent=pwm;
            this.cardLog('pwm','I='+pwm);
            if(this.S.estI!=='stop'&&!this.S.estop) this.updMotor('i',this.S.estI,pwm);
        } else {
            this.S.pwmD=pwm;
            document.getElementById('lbl-pwm-d').textContent=pwm;
            this.cardLog('pwm','D='+pwm);
            if(this.S.estD!=='stop'&&!this.S.estop) this.updMotor('d',this.S.estD,pwm);
        }
        this.updAnimSpeed();
    },

    // ── COMANDOS DE ACCION ──
    cmd(tipo){
        if(this.S.estop&&tipo!=='reset'){this.log('WARN','E-STOP activo.');return}
        if(tipo==='estop'){
            this.S.estop=true;
            this.updMotor('i','stop',0);
            this.updMotor('d','stop',0);
            this.updFSM('E-STOP');
            this.log('ERR','!!! E-STOP ACTIVADO !!!');
            this.cardLog('control','⚠ E-STOP ACTIVADO');
            this.cardLog('motor-i','E-STOP');
            this.cardLog('motor-d','E-STOP');
            if(window.app && window.app.ws && window.app.ws.readyState===WebSocket.OPEN) window.app.ws.send('ESTOP');
            return;
        }
        
        const pI=this.S.pwmI,pD=this.S.pwmD;
        let p=new URLSearchParams({tipo,valor1:pI,valor2:pD});
        fetch('/cmd',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p.toString()})
        .then(r=>{if(r.status===503)this.log('WARN','Cola llena!')}).catch(()=>this.log('ERR','Error de red'));
    }
};


// --- js/websocket.js ---
window.wsClient = {
    ws: null,
    wsFail: 0,
    pollInterval: null,
    pollFail: 0,
    wsReconnecting: false,

    FSM_MAP: ['INIT','CALIBRATING','IDLE','ADVANCING','TURNING','BRAKING','ESTOP','MANUAL'],

    decodeBinaryTelemetry(buffer){
        if (!buffer || buffer.byteLength < 2) return null;
        const v = new DataView(buffer);
        if (v.getUint8(0) !== 0x01) return null; // version mismatch
        return {
            estado:   this.FSM_MAP[v.getUint8(1)] || 'IDLE',
            distancia:v.getFloat32(2, true),
            angulo:   v.getFloat32(6, true),
            pulsosIzq:v.getInt32(10, true),
            pulsosDer:v.getInt32(14, true),
            pwmIzq:   v.getUint16(18, true),
            pwmDer:   v.getUint16(20, true),
            posX:     v.getFloat32(22, true),
            posY:     v.getFloat32(26, true),
            orientacion:v.getFloat32(30, true),
            pidCorr:  v.getFloat32(34, true),
            distRestante:v.getFloat32(38, true),
            targetDist:  v.getFloat32(42, true),
            targetVel:   v.getUint16(46, true)
        };
    },

    applyTelemetry(t){
        if(!t)return
        
        const setText = (id, val) => { const el = document.getElementById(id); if(el) el.textContent = val; };
        
        setText('tel-dist', (t.distancia||0).toFixed(1));
        setText('tel-ang', (t.angulo||0).toFixed(1));
        setText('tel-posx', (t.posX||0).toFixed(1));
        setText('tel-posy', (t.posY||0).toFixed(1));
        setText('enc-i', t.pulsosIzq||0);
        setText('enc-d', t.pulsosDer||0);
        setText('lbl-pwm-i', t.pwmIzq||0);
        setText('lbl-pwm-d', t.pwmDer||0);
        
        const barI = document.getElementById('bar-pwm-i');
        if(barI) barI.style.width = ((t.pwmIzq||0) / 255 * 100) + '%';
        const barD = document.getElementById('bar-pwm-d');
        if(barD) barD.style.width = ((t.pwmDer||0) / 255 * 100) + '%';
        
        const est=t.estado||'IDLE'
        if(est!==window.ui.S.fsm){
            if(window.ui.S.fsm!=='MANUAL') window.ui.updFSM(est);
        }
    },

    init(){
        const proto=window.location.protocol==='https:'?'wss:':'ws:'
        const host=window.location.hostname||'192.168.4.1'
        const url=proto+'//'+host+'/ws'
        window.ui.log('SYS','WS: Conectando a '+url)
        this.ws=new WebSocket(url)
        this.ws.binaryType = 'arraybuffer'
        
        this.ws.onopen=()=>{
            window.ui.log('SYS','WS: Conectado');
            this.wsFail=0;
            if(this.pollInterval){clearInterval(this.pollInterval);this.pollInterval=null}
            document.getElementById('hdr-ip').style.color='var(--cy-400)';
        }
        this.ws.onclose=()=>{
            window.ui.log('WARN','WS: Desconectado. Usando fallback HTTP y reintentando...');
            document.getElementById('hdr-ip').style.color='var(--rd-500)';
            if(!this.wsReconnecting){
                this.wsReconnecting=true;
                setTimeout(()=>{this.wsReconnecting=false;this.init()},3000);
            }
            if(!this.pollInterval)this.startPollFallback();
        }
        this.ws.onerror=()=>{
            this.wsFail++;
            if(this.wsFail===1){
                window.ui.log('WARN','WS: Error de conexion');
                if(!this.pollInterval)this.startPollFallback();
            }
        }
        this.ws.onmessage=(e)=>{
            if (e.data instanceof ArrayBuffer) {
                const t = this.decodeBinaryTelemetry(e.data);
                if (t) {
                    this.applyTelemetry(t);
                    if(window.app && window.app.canvasTrayectoria) {
                        window.app.canvasTrayectoria.setRobotPos(t.posX || 0, t.posY || 0, t.angulo || 0);
                    }
                    const needle = document.getElementById('gyro-needle');
                    if(needle) needle.style.transform = `rotate(${t.angulo || 0}deg)`;
                    const gyroVal = document.getElementById('gyro-value');
                    if(gyroVal) gyroVal.textContent = (t.angulo || 0).toFixed(1) + '\u00B0';
                }
                return;
            }
            try{
                const d=JSON.parse(e.data)
                if(d.tipo==='telem') {
                    this.applyTelemetry(d);
                    if(window.app && window.app.canvasTrayectoria) {
                        window.app.canvasTrayectoria.setRobotPos(d.posX || 0, d.posY || 0, d.angulo || 0);
                    }
                    const needle = document.getElementById('gyro-needle');
                    if(needle) needle.style.transform = `rotate(${d.angulo || 0}deg)`;
                    const gyroVal = document.getElementById('gyro-value');
                    if(gyroVal) gyroVal.textContent = (d.angulo || 0).toFixed(1) + '\u00B0';
                }
                if(d.tipo==='estop'){
                    window.ui.log('ERR','E-STOP recibido del servidor');
                }
            }catch(err){}
        }
        return this.ws;
    },

    startPollFallback(){
        window.ui.log('SYS','Polling HTTP iniciado (fallback)')
        this.pollInterval=setInterval(()=>{
            fetch('/status').then(r=>r.json()).then(d=>{
                this.pollFail=0
                if(d.telemetria) this.applyTelemetry(d.telemetria)
            }).catch(()=>{this.pollFail++;if(this.pollFail>5)window.ui.log('WARN','Polling: sin respuesta')})
        },500)
    }
};


// --- js/joystick.js ---
class VirtualJoystick {
    constructor(containerId, knobId, callback) {
        this.container = document.getElementById(containerId);
        this.knob = document.getElementById(knobId);
        this.callback = callback;
        this.active = false;
        this.maxDist = 55; // Radio en pixeles
        this.bindEvents();
    }
    bindEvents() {
        ['touchstart', 'mousedown'].forEach(evt => 
            this.container.addEventListener(evt, e => this.start(e), {passive: false}));
        ['touchmove', 'mousemove'].forEach(evt => 
            window.addEventListener(evt, e => this.move(e), {passive: false}));
        ['touchend', 'mouseup', 'mouseleave'].forEach(evt => 
            window.addEventListener(evt, e => this.end(e)));
    }
    getCoords(e) {
        const touch = e.touches ? e.touches[0] : e;
        const rect = this.container.getBoundingClientRect();
        return {
            x: touch.clientX - rect.left - rect.width / 2,
            y: touch.clientY - rect.top - rect.height / 2
        };
    }
    start(e) {
        if(window.ui.S.estop) return;
        e.preventDefault();
        this.active = true;
        this.move(e);
    }
    move(e) {
        if (!this.active || window.ui.S.estop) return;
        e.preventDefault();
        let { x, y } = this.getCoords(e);
        let dist = Math.sqrt(x*x + y*y);
        
        if (dist > this.maxDist) {
            const ratio = this.maxDist / dist;
            x *= ratio; y *= ratio; dist = this.maxDist;
        }
        
        this.knob.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;
        
        const speed = Math.round((dist / this.maxDist) * 255);
        const angle = Math.atan2(-y, x) * 180 / Math.PI;
        this.callback(speed, angle);
    }
    end(e) {
        if (!this.active) return;
        this.active = false;
        this.knob.style.transform = 'translate(-50%, -50%)';
        this.callback(0, 0);
    }
}

window.VirtualJoystick = VirtualJoystick;


// --- js/trajectory.js ---
class TrayectoriaCanvas {
    constructor(canvasBgId, canvasFgId) {
        this.canvasBg = document.getElementById(canvasBgId);
        this.ctxBg = this.canvasBg.getContext('2d', { alpha: false });
        
        this.canvasFg = document.getElementById(canvasFgId);
        this.ctxFg = this.canvasFg.getContext('2d');
        
        this.colaPasos = [];
        this.robotPos = { x: 0, y: 0, angulo: 0 };
        this.escala = 1.2; // pixels per cm
        this.offsetX = 0;
        this.offsetY = 0;
        
        this._lastTouch = 0;
        this.isDragging = false;
        this.dragStartX = 0;
        this.dragStartY = 0;
        
        this.resize();
        window.addEventListener('resize', () => this.resize());
        
        // Eventos solo en el FG
        this.canvasFg.addEventListener('mousedown', (e) => {
            if(Date.now() - this._lastTouch < 300) return;
            this.isDragging = true;
            this.dragStartX = e.clientX;
            this.dragStartY = e.clientY;
        });
        this.canvasFg.addEventListener('mousemove', (e) => {
            if (this.isDragging) {
                this.offsetX += e.clientX - this.dragStartX;
                this.offsetY += e.clientY - this.dragStartY;
                this.dragStartX = e.clientX;
                this.dragStartY = e.clientY;
                this.drawBackground();
                this.drawForeground();
            }
        });
        this.canvasFg.addEventListener('mouseup', (e) => {
            if(this.isDragging) {
                this.isDragging = false;
                // Si el movimiento fue minimo, tratarlo como click (add waypoint)
                const dist = Math.hypot(e.clientX - this.dragStartX, e.clientY - this.dragStartY);
                if(dist < 5) {
                    this.click(e);
                }
            }
        });
        this.canvasFg.addEventListener('mouseleave', () => this.isDragging = false);

        this.canvasFg.addEventListener('wheel', (e) => {
            e.preventDefault();
            const zoomFactor = 1.1;
            if (e.deltaY < 0) {
                this.zoomIn();
            } else {
                this.zoomOut();
            }
        });
        
        this.canvasFg.addEventListener('click', (e) => {
            if(Date.now() - this._lastTouch < 300) return;
            this.click(e);
        });
        this.canvasFg.addEventListener('touchstart', (e) => {
            this._lastTouch = Date.now();
            this.click(e);
        }, {passive: true});
    }

    zoomIn() {
        this.escala *= 1.2;
        if(this.escala > 10) this.escala = 10;
        this.drawBackground();
        this.drawForeground();
    }
    
    zoomOut() {
        this.escala /= 1.2;
        if(this.escala < 0.1) this.escala = 0.1;
        this.drawBackground();
        this.drawForeground();
    }
    
    resize() {
        const rect = this.canvasFg.parentElement.getBoundingClientRect();
        this.canvasBg.width = rect.width;
        this.canvasBg.height = rect.height;
        this.canvasFg.width = rect.width;
        this.canvasFg.height = rect.height;
        
        // Redibujar cuadrícula estática
        this.drawBackground();
        this.drawForeground();
    }
    
    click(e) {
        if (window.ui && window.ui.S.estop) return;
        const rect = this.canvasFg.getBoundingClientRect();
        const touch = e.touches ? e.touches[0] : e;
        const clickX = touch.clientX - rect.left;
        const clickY = touch.clientY - rect.top;
        
        const centroX = (this.canvasFg.width / 2) + this.offsetX;
        const centroY = (this.canvasFg.height / 2) + this.offsetY;
        
        const rx = (clickX - centroX) / this.escala;
        const ry = (centroY - clickY) / this.escala;
        
        this.agregarPunto(rx, ry);
    }
    
    agregarPunto(rx, ry) {
        this.colaPasos.push({ x: rx, y: ry });
        this.drawForeground();
        // Llamar hook de UI si existe
        if(window.app && window.app.onWaypointAdded) {
            window.app.onWaypointAdded();
        }
    }
    
    limpiar() {
        this.colaPasos = [];
        this.drawForeground();
    }
    
    enviarRutaBinaria() {
        if (this.colaPasos.length === 0) return;
        
        const N = this.colaPasos.length;
        const headerSize = 2;
        const waypointSize = 16; // sizeof(PuntoRuta) = float+float+int32+uint32
        const totalSize = headerSize + N * waypointSize;
        
        const buffer = new ArrayBuffer(totalSize);
        const view = new DataView(buffer);
        view.setUint16(0, N, true); // count LE
        
        let offset = headerSize;
        let prevX = this.robotPos.x || 0;
        let prevY = this.robotPos.y || 0;
        let prevHeading = 0;
        
        this.colaPasos.forEach(p => {
            const dx = p.x - prevX;
            const dy = p.y - prevY;
            const dist = Math.sqrt(dx*dx + dy*dy);
            const targetHeading = Math.atan2(dy, dx);
            let diffAng = (targetHeading - prevHeading) * 180 / Math.PI;
            // Normalize to [-180, 180]
            while (diffAng > 180) diffAng -= 360;
            while (diffAng < -180) diffAng += 360;
            
            // If turn needed, encode as separate waypoint
            if (Math.abs(diffAng) > 1.0) {
                // Giro waypoint
                view.setFloat32(offset, 0, true); offset += 4;   // distancia=0
                view.setFloat32(offset, diffAng, true); offset += 4; // angulo
                view.setInt32(offset, 130, true); offset += 4;   // velocidad
                view.setUint32(offset, 5000, true); offset += 4; // duracion
            }
            
            if (dist > 0.1) {
                // Avance waypoint
                view.setFloat32(offset, dist, true); offset += 4;
                view.setFloat32(offset, 0, true); offset += 4;
                view.setInt32(offset, 150, true); offset += 4;
                view.setUint32(offset, Math.round(dist / 5.0 * 1000) + 2000, true); offset += 4;
            }
            
            prevX = p.x;
            prevY = p.y;
            prevHeading = targetHeading;
        });
        
        // Trim to actual size
        const final = buffer.slice(0, offset);
        
        if (window.wsClient && window.wsClient.ws && window.wsClient.ws.readyState === WebSocket.OPEN) {
            window.wsClient.ws.send(final);
            if (window.ui) window.ui.log('CMD', 'Ruta binaria enviada (' + N + ' waypoints, ' + final.byteLength + ' bytes)');
        }
        
        this.limpiar();
    }
    
    setRobotPos(x, y, angulo) {
        this.robotPos.x = x;
        this.robotPos.y = y;
        this.robotPos.angulo = angulo;
        this.drawForeground();
        // Disparar re-renderizado de wp para actualizar dist/ang relativos
        if(window.app && window.app.onWaypointAdded) {
            window.app.onWaypointAdded();
        }
    }
    
    drawBackground() {
        const w = this.canvasBg.width;
        const h = this.canvasBg.height;
        const ctx = this.ctxBg;
        
        ctx.fillStyle = '#020617';
        ctx.fillRect(0, 0, w, h);
        
        const centroX = (w / 2) + this.offsetX;
        const centroY = (h / 2) + this.offsetY;
        
        // Rejilla de Fondo adaptativa a escala (cada 20cm logicos)
        ctx.strokeStyle = 'rgba(34, 211, 238, 0.06)';
        ctx.lineWidth = 0.5;
        const logicalGridSize = 20; // 20 cm
        const pxGridSize = logicalGridSize * this.escala;
        
        ctx.beginPath();
        // Lineas verticales
        for (let x = centroX % pxGridSize; x < w; x += pxGridSize) {
            ctx.moveTo(x, 0); ctx.lineTo(x, h);
        }
        // Lineas horizontales
        for (let y = centroY % pxGridSize; y < h; y += pxGridSize) {
            ctx.moveTo(0, y); ctx.lineTo(w, y);
        }
        ctx.stroke();
        
        // Ejes X e Y principales
        ctx.strokeStyle = 'rgba(34, 211, 238, 0.25)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        // Eje X
        ctx.moveTo(0, centroY); ctx.lineTo(w, centroY);
        // Eje Y
        ctx.moveTo(centroX, 0); ctx.lineTo(centroX, h);
        ctx.stroke();

        // Numeracion en ejes
        ctx.fillStyle = 'rgba(148, 163, 184, 0.7)'; // sl-400
        ctx.font = '10px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'top';
        
        // Marcadores Eje X
        let startXLogical = Math.ceil(-centroX / pxGridSize) * logicalGridSize;
        for (let xLog = startXLogical; xLog * this.escala + centroX < w; xLog += logicalGridSize) {
            if (xLog !== 0) {
                const px = xLog * this.escala + centroX;
                ctx.fillText(xLog, px, centroY + 4);
                ctx.beginPath(); ctx.moveTo(px, centroY - 3); ctx.lineTo(px, centroY + 3); ctx.stroke();
            }
        }
        
        // Marcadores Eje Y
        ctx.textAlign = 'right';
        ctx.textBaseline = 'middle';
        let startYLogical = Math.ceil((centroY - h) / pxGridSize) * logicalGridSize;
        for (let yLog = startYLogical; centroY - yLog * this.escala < h; yLog += logicalGridSize) {
            if (yLog !== 0) {
                const py = centroY - yLog * this.escala;
                ctx.fillText(yLog, centroX - 4, py);
                ctx.beginPath(); ctx.moveTo(centroX - 3, py); ctx.lineTo(centroX + 3, py); ctx.stroke();
            }
        }
    }
    
    drawForeground() {
        const w = this.canvasFg.width;
        const h = this.canvasFg.height;
        const ctx = this.ctxFg;
        
        // Limpiar FG
        ctx.clearRect(0, 0, w, h);
        
        const centroX = (w / 2) + this.offsetX;
        const centroY = (h / 2) + this.offsetY;
        
        // Dibujar ruta planificada
        if (this.colaPasos.length > 0) {
            ctx.strokeStyle = '#a855f7';
            ctx.lineWidth = 1.5;
            ctx.setLineDash([4, 2]);
            ctx.beginPath();
            
            const startX = centroX + this.robotPos.x * this.escala;
            const startY = centroY - this.robotPos.y * this.escala;
            ctx.moveTo(startX, startY);
            
            this.colaPasos.forEach(p => {
                const px = centroX + p.x * this.escala;
                const py = centroY - p.y * this.escala;
                ctx.lineTo(px, py);
            });
            ctx.stroke();
            ctx.setLineDash([]);
            
            // Círculos waypoints
            this.colaPasos.forEach((p, idx) => {
                const px = centroX + p.x * this.escala;
                const py = centroY - p.y * this.escala;
                ctx.fillStyle = '#c084fc';
                ctx.beginPath();
                ctx.arc(px, py, 3, 0, 2 * Math.PI);
                ctx.fill();
                ctx.fillStyle = '#cbd5e1';
                ctx.font = '8px monospace';
                ctx.fillText(idx + 1, px + 5, py - 5);
            });
        }
        
        // Dibujar robot en coordenadas actuales
        const rx = centroX + this.robotPos.x * this.escala;
        const ry = centroY - this.robotPos.y * this.escala;
        
        ctx.save();
        ctx.translate(rx, ry);
        ctx.rotate(-this.robotPos.angulo * Math.PI / 180);
        
        ctx.fillStyle = 'rgba(34, 211, 238, 0.15)';
        ctx.strokeStyle = '#22d3ee';
        ctx.lineWidth = 1.2;
        ctx.beginPath();
        ctx.rect(-8, -12, 16, 24);
        ctx.fill();
        ctx.stroke();
        
        ctx.fillStyle = '#22d3ee';
        ctx.beginPath();
        ctx.moveTo(0, -15);
        ctx.lineTo(-4, -12);
        ctx.lineTo(4, -12);
        ctx.closePath();
        ctx.fill();
        
        ctx.restore();
    }
}
window.TrayectoriaCanvas = TrayectoriaCanvas;


// --- js/app.js ---
window.app = {
    canvasTrayectoria: null,
    ws: null,
    notifLogs: [],
    unreadNotifs: 0,

    limpiarRuta() {
        if(this.canvasTrayectoria) this.canvasTrayectoria.limpiar();
        document.getElementById('waypoints-list').innerHTML = '<div style="text-align:center; margin-top:20px; opacity:0.5;">Ingrese X,Y destino arriba</div>';
    },

    agregarPuntoXY() {
        const x = parseFloat(document.getElementById('inp-dest-x').value) || 0;
        const y = parseFloat(document.getElementById('inp-dest-y').value) || 0;
        if(this.canvasTrayectoria) {
            this.canvasTrayectoria.agregarPunto(x, y);
        }
    },

    onWaypointAdded() {
        const list = document.getElementById('waypoints-list');
        if(!this.canvasTrayectoria) return;
        list.innerHTML = '';
        this.canvasTrayectoria.colaPasos.forEach((p, index) => {
            // Calcular vector y ángulo respecto al robot actual
            const dx = p.x - this.canvasTrayectoria.robotPos.x;
            const dy = p.y - this.canvasTrayectoria.robotPos.y;
            const dist = Math.sqrt(dx*dx + dy*dy);
            const angRad = Math.atan2(dy, dx);
            let angDeg = angRad * 180 / Math.PI;
            if(angDeg < 0) angDeg += 360;

            const div = document.createElement('div');
            div.className = 'wp-item';
            div.innerHTML = `
                <div class="wp-item-header">
                    <span>WP ${index + 1}</span>
                    <span>Dest: X=${p.x.toFixed(1)}, Y=${p.y.toFixed(1)}</span>
                </div>
                <div class="wp-item-details">
                    <span><span style="color:var(--cy-400)">Δd:</span> ${dist.toFixed(1)} cm</span>
                    <span><span style="color:var(--ye-400)">θ:</span> ${angDeg.toFixed(1)}°</span>
                </div>
            `;
            list.appendChild(div);
        });
    },

    enviarRutaRobot() {
        if(!this.canvasTrayectoria || this.canvasTrayectoria.colaPasos.length === 0) return;
        const payload = "RUTA:" + this.canvasTrayectoria.colaPasos.map(p => `X${p.x.toFixed(1)}Y${p.y.toFixed(1)}`).join(';');
        if(this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(payload);
            this.limpiarRuta();
        }
    },

    toggleFullscreen(elem) {
        if (!document.fullscreenElement && !document.mozFullScreenElement && !document.webkitFullscreenElement && !document.msFullscreenElement) {
            if (elem.requestFullscreen) {
                elem.requestFullscreen();
            } else if (elem.msRequestFullscreen) {
                elem.msRequestFullscreen();
            } else if (elem.mozRequestFullScreen) {
                elem.mozRequestFullScreen();
            } else if (elem.webkitRequestFullscreen) {
                elem.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);
            }
        } else {
            if (document.exitFullscreen) {
                document.exitFullscreen();
            } else if (document.msExitFullscreen) {
                document.msExitFullscreen();
            } else if (document.mozCancelFullScreen) {
                document.mozCancelFullScreen();
            } else if (document.webkitExitFullscreen) {
                document.webkitExitFullscreen();
            }
        }
        // Force resize del canvas poco despues de la transicion de fullscreen
        setTimeout(() => { if(this.canvasTrayectoria) this.canvasTrayectoria.resize(); }, 100);
        setTimeout(() => { if(this.canvasTrayectoria) this.canvasTrayectoria.resize(); }, 500);
    },

    // --- CRM NOTIFICACIONES ---
    addNotification(msg, type="info") {
        const timeStr = new Date().toLocaleTimeString();
        this.notifLogs.push(`[${timeStr}] [${type.toUpperCase()}] ${msg}`);
        
        const list = document.getElementById('notif-list');
        const div = document.createElement('div');
        div.className = 'notif-item';
        let borderColor = 'var(--sl-500)';
        if(type === 'error' || type === 'estop') borderColor = 'var(--re-500)';
        else if(type === 'warn') borderColor = 'var(--ye-500)';
        div.style.borderLeftColor = borderColor;
        div.innerHTML = `<div class="notif-item-time">${timeStr}</div><div>${msg}</div>`;
        list.prepend(div);
        
        if (this.notifLogs.length > 100) {
            this.notifLogs.shift(); // Evitar leak de RAM array JS
            if (list.lastChild) {
                list.removeChild(list.lastChild); // Evitar leak de nodos DOM
            }
        }
        
        this.unreadNotifs++;
        const badge = document.getElementById('notif-badge');
        badge.textContent = this.unreadNotifs;
        badge.style.display = 'block';
    },

    toggleNotifications() {
        const panel = document.getElementById('notifications-panel');
        if (panel.style.display === 'none') {
            panel.style.display = 'flex';
            this.unreadNotifs = 0;
            document.getElementById('notif-badge').style.display = 'none';
        } else {
            panel.style.display = 'none';
        }
    },

    downloadLogsTXT() {
        if(this.notifLogs.length === 0) return;
        const text = this.notifLogs.join('\n');
        const blob = new Blob([text], {type: "text/plain"});
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `robot_log_${new Date().getTime()}.txt`;
        a.click();
        URL.revokeObjectURL(url);
    },

    // --- FSM DEBUG ---
    forceFSM() {
        const select = document.getElementById('fsm-override-select');
        const estado = select.value;
        if(this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(`FSM:${estado}`);
            this.addNotification(`Forzando estado FSM: ${estado}`, "warn");
        }
    },

    // --- LABERINTO JSON I/O ---
    exportJSON() {
        if(!this.canvasTrayectoria || this.canvasTrayectoria.colaPasos.length === 0) {
            this.addNotification("No hay waypoints para exportar", "warn");
            return;
        }
        const data = JSON.stringify(this.canvasTrayectoria.colaPasos, null, 2);
        const blob = new Blob([data], {type: "application/json"});
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `waypoints_${new Date().getTime()}.json`;
        a.click();
        URL.revokeObjectURL(url);
        this.addNotification("Waypoints exportados a JSON");
    },

    importJSON(event) {
        const file = event.target.files[0];
        if(!file) return;
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const parsed = JSON.parse(e.target.result);
                if(Array.isArray(parsed)) {
                    if(this.canvasTrayectoria) {
                        this.canvasTrayectoria.colaPasos = parsed;
                        this.canvasTrayectoria.drawForeground();
                        this.onWaypointAdded();
                        this.addNotification(`Importados ${parsed.length} waypoints`, "info");
                    }
                }
            } catch (err) {
                this.addNotification("Error parseando JSON", "error");
            }
        };
        reader.readAsText(file);
        event.target.value = ''; // Reset input
    },

    sendCmd(dir) {
        if (dir === 'S') {
            if(this.ws && this.ws.readyState === WebSocket.OPEN) this.ws.send('MV:S,0');
            window.ui.updFSM('IDLE');
        } else {
            const pwm = document.getElementById('sl-pwm-gral').value;
            if(this.ws && this.ws.readyState === WebSocket.OPEN) this.ws.send(`MV:${dir},${pwm}`);
            window.ui.updFSM('MANUAL');
        }
    },

    bindManualButtons() {
        const btnFwd = document.getElementById('btn-fwd');
        const btnRev = document.getElementById('btn-rev');
        const btnLeft = document.getElementById('btn-left');
        const btnRight = document.getElementById('btn-right');
        const btnStop = document.getElementById('btn-stop');

        const bindBtn = (btn, dir) => {
            if(!btn) return;
            const start = (e) => { e.preventDefault(); this.sendCmd(dir); };
            const end = (e) => { e.preventDefault(); this.sendCmd('S'); };
            
            btn.addEventListener('mousedown', start);
            btn.addEventListener('touchstart', start, {passive:false});
            
            btn.addEventListener('mouseup', end);
            btn.addEventListener('mouseleave', end);
            btn.addEventListener('touchend', end, {passive:false});
            btn.addEventListener('touchcancel', end, {passive:false});
        };

        bindBtn(btnFwd, 'F');
        bindBtn(btnRev, 'B');
        bindBtn(btnLeft, 'L');
        bindBtn(btnRight, 'R');
        
        if(btnStop) {
            btnStop.addEventListener('mousedown', (e)=>{ e.preventDefault(); window.ui.cmd('estop'); });
            btnStop.addEventListener('touchstart', (e)=>{ e.preventDefault(); window.ui.cmd('estop'); }, {passive:false});
        }
    },

    initKeyboard() {
        const teclasPresionadas = {};
        document.addEventListener('keydown', e => {
            if(window.ui.S.estop) return;
            const key = e.key.toLowerCase();
            if(teclasPresionadas[key]) return;
            teclasPresionadas[key] = true;
            let dir = null;
            switch(key){
                case 'w': dir = 'F'; break;
                case 's': dir = 'B'; break;
                case 'a': dir = 'L'; break;
                case 'd': dir = 'R'; break;
                case ' ': e.preventDefault(); window.ui.cmd('estop'); return;
            }
            if(dir) this.sendCmd(dir);
        });

        document.addEventListener('keyup', e => {
            const key = e.key.toLowerCase();
            if(teclasPresionadas[key]) {
                delete teclasPresionadas[key];
                if(!teclasPresionadas['w'] && !teclasPresionadas['s'] && !teclasPresionadas['a'] && !teclasPresionadas['d']) {
                    this.sendCmd('S');
                }
            }
        });
    },

    init() {
        this.canvasTrayectoria = new window.TrayectoriaCanvas('canvas-bg', 'canvas-fg');
        window.ui.updFSM('INIT');
        
        this.ws = window.wsClient.init();
        this.bindManualButtons();
        this.initKeyboard();
        
        fetch('/network').then(r=>r.text()).then(h=>{
            const m=h.match(/http:\/\/([^\/<]+)/)
            if(m) document.getElementById('hdr-ip').textContent=m[1]
        }).catch(()=>{})
    }
};

window.onload = () => {
    window.app.init();
};


</script>
</body>
</html>

)EOF";

const char* get_html_page() {
    return HTML_PAGE;
}
