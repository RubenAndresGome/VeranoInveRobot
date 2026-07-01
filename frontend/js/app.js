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
