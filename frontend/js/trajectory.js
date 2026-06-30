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
