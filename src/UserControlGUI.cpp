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
:fullscreen .panel-tech{height:100vh;width:100vw;display:flex;flex-direction:column}
:fullscreen .card-content{flex:1}
:fullscreen .card-local-log{height:60px}
:-webkit-full-screen .panel-tech{height:100vh;width:100vw;display:flex;flex-direction:column}
:-webkit-full-screen .card-content{flex:1}
:-webkit-full-screen .card-local-log{height:60px}

</style>
    <style>
        .grid-3col { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 16px; }
        .control-pad { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 8px; margin-bottom: 20px; }
        .btn-dir { background: var(--bl-900); color: var(--bl-300); padding: 16px; border: 1px solid var(--bl-600); border-radius: 8px; text-align: center; cursor: pointer; user-select: none; transition: 0.1s; font-weight: bold; text-transform: uppercase; }
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

<header style="border-bottom:1px solid rgba(34,211,238,.3);background:rgba(15,23,42,.8);padding:10px 20px;display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:10px">
    <div style="display:flex;align-items:center;gap:10px">
        <div style="width:10px;height:10px;background:var(--cy-400);border-radius:50%;box-shadow:0 0 10px var(--cy-400);animation:pulse 2s infinite"></div>
        <span style="color:var(--cy-400);font-size:18px;font-weight:700;letter-spacing:3px">ROBOT-AUTONOMO-S3</span>
    </div>
    <div style="display:flex;align-items:center;gap:16px;font-size:11px">
        <span class="data-label">IP:</span><span class="data-value" id="hdr-ip">192.168.4.1</span>
        <span class="data-label">FSM:</span><span id="fsm-badge" class="fsm-badge" style="border-color:var(--cy-500);color:var(--cy-400);">IDLE</span>
    </div>
</header>

<main style="max-width:1600px;margin:20px auto;padding:0 15px;">
    <div class="grid-3col">
        
        <!-- COLUMNA 1: CONTROL MANUAL -->
        <div class="panel-tech" style="padding:20px; display:flex; flex-direction:column; gap:20px;">
            <div style="color:white; font-size:18px; font-weight:bold; margin-bottom:10px;">Control Manual</div>
            
            <div class="control-pad">
                <div></div>
                <div class="btn-dir" id="btn-fwd">Avanzar</div>
                <div></div>
                <div class="btn-dir" id="btn-left" style="background:var(--pu-600); border-color:var(--pu-400); color:white;">Girar</div>
                <div class="btn-dir btn-stop" id="btn-stop">Stop</div>
                <div class="btn-dir" id="btn-right" style="background:var(--pu-600); border-color:var(--pu-400); color:white;">Girar</div>
                <div></div>
                <div class="btn-dir" id="btn-rev" style="background:var(--am-600); border-color:var(--am-400); color:white;">Retroceder</div>
                <div></div>
            </div>

            <!-- PWM Control Widget Style -->
            <div style="background:rgba(15,23,42,0.6); padding:16px; border-radius:12px; border:1px solid var(--sl-700);">
                <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:10px;">
                    <span style="color:white; font-weight:bold;">Velocidad PWM</span>
                    <span id="lbl-pwm-val" style="background:rgba(255,255,255,0.1); padding:4px 10px; border-radius:6px; color:var(--cy-400);">150</span>
                </div>
                <div style="display:flex; align-items:center; gap:10px;">
                    <span style="font-size:24px; color:var(--ye-400);">⚡</span>
                    <input type="range" min="0" max="255" value="150" class="slider-tech" id="sl-pwm-gral" oninput="window.ui.setPWM('i',this.value); window.ui.setPWM('d',this.value); document.getElementById('lbl-pwm-val').textContent=this.value;">
                </div>
            </div>

            <div style="margin-top:auto;">
                <button class="btn-estop" style="width:100%; border-radius:8px;" onclick="window.ui.cmd('estop')">EMERGENCY STOP</button>
            </div>
        </div>

        <!-- COLUMNA 2: TELEMETRIA -->
        <div class="panel-tech" style="padding:20px; display:flex; flex-direction:column; gap:20px;">
            <div style="color:white; font-size:18px; font-weight:bold; margin-bottom:10px;">Telemetría en Vivo</div>
            
            <div style="display:grid; grid-template-columns: 1fr 1fr; gap:16px;">
                <div>
                    <div class="data-label" style="margin-bottom:4px;">DISTANCIA (CM)</div>
                    <div class="huge-number" id="tel-dist">0.0</div>
                </div>
                <div>
                    <div class="data-label" style="margin-bottom:4px;">ANGULO (°)</div>
                    <div class="huge-number" id="tel-ang" style="color:var(--pu-300);">0.0</div>
                </div>
            </div>

            <div style="flex:1; display:flex; flex-direction:column; gap:8px;">
                <div style="display:flex; justify-content:space-between;">
                    <span class="data-label">TRAYECTORIA (X,Y CM)</span>
                    <span class="data-value" style="font-size:10px;"><span id="tel-posx">0.0</span>, <span id="tel-posy">0.0</span></span>
                </div>
                <!-- OPTIMIZACION DE CANVAS (Doble capa) -->
                <div style="position:relative; background:#020617; border:1px solid var(--sl-700); border-radius:8px; height:180px; width:100%;">
                    <canvas id="canvas-bg" style="position:absolute; top:0; left:0; width:100%; height:100%;"></canvas>
                    <canvas id="canvas-fg" style="position:absolute; top:0; left:0; width:100%; height:100%;"></canvas>
                </div>
            </div>

            <div>
                <span class="data-label">MOTORES (PWM)</span>
                <div style="display:grid; grid-template-columns: 60px 1fr 40px; align-items:center; gap:10px; margin-top:8px;">
                    <span style="color:var(--sl-300); font-size:12px;">Izquierdo</span>
                    <div class="progress-bar-container"><div class="progress-bar-fill" id="bar-pwm-i" style="width:0%;"></div></div>
                    <span id="lbl-pwm-i" style="font-size:12px; color:var(--cy-400); text-align:right;">0</span>
                </div>
                <div style="display:grid; grid-template-columns: 60px 1fr 40px; align-items:center; gap:10px; margin-top:8px;">
                    <span style="color:var(--sl-300); font-size:12px;">Derecho</span>
                    <div class="progress-bar-container"><div class="progress-bar-fill" id="bar-pwm-d" style="width:0%;"></div></div>
                    <span id="lbl-pwm-d" style="font-size:12px; color:var(--cy-400); text-align:right;">0</span>
                </div>
                <div style="display:flex; justify-content:space-between; margin-top:8px; font-size:10px; color:var(--sl-500);">
                    <span>Pulsos L: <span id="enc-i">0</span></span>
                    <span>Pulsos R: <span id="enc-d">0</span></span>
                </div>
            </div>
        </div>

        <!-- COLUMNA 3: LABERINTO -->
        <div class="panel-tech" style="padding:20px; display:flex; flex-direction:column; gap:20px;">
            <div style="color:white; font-size:18px; font-weight:bold; margin-bottom:10px;">Laberinto X/Y</div>
            
            <div style="display:grid; grid-template-columns: 1fr 1fr; gap:16px;">
                <div>
                    <div class="data-label" style="margin-bottom:4px;">DESTINO X (CM)</div>
                    <input type="number" id="inp-dest-x" value="0" style="width:100%; background:#020617; border:1px solid var(--sl-700); color:white; padding:8px; border-radius:4px; outline:none;">
                </div>
                <div>
                    <div class="data-label" style="margin-bottom:4px;">DESTINO Y (CM)</div>
                    <input type="number" id="inp-dest-y" value="0" style="width:100%; background:#020617; border:1px solid var(--sl-700); color:white; padding:8px; border-radius:4px; outline:none;">
                </div>
            </div>

            <div style="display:grid; grid-template-columns: 1fr 1fr; gap:10px;">
                <button class="btn-cmd" style="background:var(--pu-600); border-color:var(--pu-400);" onclick="window.app.agregarPuntoXY()">Agregar</button>
                <button class="btn-cmd" style="background:var(--em-600); border-color:var(--em-400);" onclick="window.app.enviarRutaRobot()">Ir al Destino</button>
            </div>

            <div style="flex:1; background:rgba(2,6,23,0.5); border:1px solid var(--sl-700); border-radius:8px; padding:10px; display:flex; flex-direction:column;">
                <span class="data-label" style="margin-bottom:8px;">WAYPOINTS PROGRAMADOS</span>
                <div id="waypoints-list" style="flex:1; overflow-y:auto; font-size:12px; color:var(--sl-300);">
                    <div style="text-align:center; margin-top:20px; opacity:0.5;">Ingrese X,Y destino arriba</div>
                </div>
                <button class="btn-cmd" style="width:100%; margin-top:10px; background:var(--sl-700);" onclick="window.app.limpiarRuta()">Limpiar</button>
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
                window.ui.log('WARN','WS: Error de conexión');
                if(!this.pollInterval)this.startPollFallback();
            }
        }
        this.ws.onmessage=(e)=>{
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
                    if(gyroVal) gyroVal.textContent = (d.angulo || 0).toFixed(1) + '°';
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
        this.escala = 1.2; // 1.2 px/cm
        
        this._lastTouch = 0;
        this.resize();
        window.addEventListener('resize', () => this.resize());
        
        // Eventos solo en el FG
        this.canvasFg.addEventListener('mousedown', (e) => {
            if(Date.now() - this._lastTouch < 300) return;
            this.click(e);
        });
        this.canvasFg.addEventListener('touchstart', (e) => {
            this._lastTouch = Date.now();
            this.click(e);
        }, {passive: true});
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
        
        const centroX = this.canvasFg.width / 2;
        const centroY = this.canvasFg.height / 2;
        
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
    
    setRobotPos(x, y, angulo) {
        this.robotPos.x = x;
        this.robotPos.y = y;
        this.robotPos.angulo = angulo;
        this.drawForeground();
    }
    
    drawBackground() {
        const w = this.canvasBg.width;
        const h = this.canvasBg.height;
        const ctx = this.ctxBg;
        
        ctx.fillStyle = '#020617';
        ctx.fillRect(0, 0, w, h);
        
        // Rejilla de Fondo
        ctx.strokeStyle = 'rgba(34, 211, 238, 0.06)';
        ctx.lineWidth = 0.5;
        const gridSize = 20;
        for (let x = 0; x < w; x += gridSize) {
            ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke();
        }
        for (let y = 0; y < h; y += gridSize) {
            ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
        }
        
        const centroX = w / 2;
        const centroY = h / 2;
        
        // Ejes origen
        ctx.strokeStyle = 'rgba(34, 211, 238, 0.15)';
        ctx.lineWidth = 1;
        ctx.beginPath(); ctx.moveTo(centroX - 10, centroY); ctx.lineTo(centroX + 10, centroY); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(centroX, centroY - 10); ctx.lineTo(centroX, centroY + 10); ctx.stroke();
    }
    
    drawForeground() {
        const w = this.canvasFg.width;
        const h = this.canvasFg.height;
        const ctx = this.ctxFg;
        
        // Limpiar FG
        ctx.clearRect(0, 0, w, h);
        
        const centroX = w / 2;
        const centroY = h / 2;
        
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
        const pts = this.canvasTrayectoria.colaPasos;
        if(pts.length === 1) list.innerHTML = ''; // clean placeholder
        const p = pts[pts.length - 1];
        const div = document.createElement('div');
        div.style.padding = '4px 0';
        div.style.borderBottom = '1px solid var(--sl-700)';
        div.textContent = `WP ${pts.length}: X=${p.x.toFixed(1)} cm, Y=${p.y.toFixed(1)} cm`;
        list.appendChild(div);
    },

    enviarRutaRobot() {
        if(!this.canvasTrayectoria || this.canvasTrayectoria.colaPasos.length === 0) return;
        const payload = "RUTA:" + this.canvasTrayectoria.colaPasos.map(p => `X${p.x.toFixed(1)}Y${p.y.toFixed(1)}`).join(';');
        if(this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(payload);
            this.limpiarRuta();
        }
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
