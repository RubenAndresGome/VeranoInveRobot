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
