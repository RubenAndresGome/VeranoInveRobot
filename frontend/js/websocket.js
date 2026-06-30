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
