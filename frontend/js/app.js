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
