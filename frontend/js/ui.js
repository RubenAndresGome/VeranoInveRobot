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
