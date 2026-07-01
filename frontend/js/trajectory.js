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
        
        let prevX = this.robotPos.x || 0;
        let prevY = this.robotPos.y || 0;
        let prevHeading = (this.robotPos.angulo || 0) * Math.PI / 180;
        
        // Pre-calcular cuantos waypoints de salida para asignar buffer exacto
        let outputWaypoints = 0;
        this.colaPasos.forEach(p => {
            const dx = p.x - prevX, dy = p.y - prevY;
            const targetHeading = Math.atan2(dy, dx);
            let diffAng = (targetHeading - prevHeading) * 180 / Math.PI;
            while (diffAng > 180) diffAng -= 360;
            while (diffAng < -180) diffAng += 360;
            if (Math.abs(diffAng) > 1.0) outputWaypoints++;
            if (Math.sqrt(dx*dx+dy*dy) > 0.1) outputWaypoints++;
            prevX = p.x; prevY = p.y; prevHeading = targetHeading;
        });
        
        if (outputWaypoints === 0) return;
        
        const headerSize = 2;
        const waypointSize = 16;
        const totalSize = headerSize + outputWaypoints * waypointSize;
        const buffer = new ArrayBuffer(totalSize);
        const view = new DataView(buffer);
        view.setUint16(0, outputWaypoints, true);
        
        let offset = headerSize;
        prevX = this.robotPos.x || 0;
        prevY = this.robotPos.y || 0;
        prevHeading = (this.robotPos.angulo || 0) * Math.PI / 180;
        
        this.colaPasos.forEach(p => {
            const dx = p.x - prevX, dy = p.y - prevY;
            const dist = Math.sqrt(dx*dx + dy*dy);
            const targetHeading = Math.atan2(dy, dx);
            let diffAng = (targetHeading - prevHeading) * 180 / Math.PI;
            while (diffAng > 180) diffAng -= 360;
            while (diffAng < -180) diffAng += 360;
            
            if (Math.abs(diffAng) > 1.0) {
                view.setFloat32(offset, 0, true); offset += 4;
                view.setFloat32(offset, diffAng, true); offset += 4;
                view.setInt32(offset, 130, true); offset += 4;
                view.setUint32(offset, 5000, true); offset += 4;
            }
            if (dist > 0.1) {
                view.setFloat32(offset, dist, true); offset += 4;
                view.setFloat32(offset, 0, true); offset += 4;
                view.setInt32(offset, 150, true); offset += 4;
                view.setUint32(offset, Math.round(dist / 5.0 * 1000) + 2000, true); offset += 4;
            }
            prevX = p.x; prevY = p.y; prevHeading = targetHeading;
        });
        
        if (window.wsClient && window.wsClient.ws && window.wsClient.ws.readyState === WebSocket.OPEN) {
            window.wsClient.ws.send(buffer);
            if (window.ui) window.ui.log('CMD', 'Ruta binaria enviada (' + outputWaypoints + ' waypoints, ' + buffer.byteLength + ' bytes)');
            this.limpiar();
        }
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
