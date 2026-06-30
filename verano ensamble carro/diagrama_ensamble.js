// ═══════════════════════════════════════════════════════════════
// DIAGRAMA_ENSAMBLE.JS — Buses + Net Labels + Filtros + Zonas
// Supervisor Metadialéctico Escolástico · VeranoInve · Rev 3.1
// ═══════════════════════════════════════════════════════════════

(() => {
  'use strict';

  // ── Estado global ──
  const state = {
    viewMode: 'wires',
    activeNet: null,
    activeBus: null,
    netTags: [],
    pinElements: [],
    currentZoom: 1,
    sidebarCollapsed: false,
    filters: { power: true, gnd: true, signal: true }
  };

  // ── Definición de redes ──
  const NETS = {
    ENA_PWM:     { color: 'net-tag-pwm',    type: 'output', label: 'ENA (PWM)', gpio: 15, bus: 'PWM' },
    IN1_DIR:     { color: 'net-tag-output', type: 'output', label: 'IN1 (Dir L)', gpio: 16, bus: 'DIR_L' },
    IN2_DIR:     { color: 'net-tag-output', type: 'output', label: 'IN2 (Dir L)', gpio: 17, bus: 'DIR_L' },
    ENB_PWM:     { color: 'net-tag-pwm',    type: 'output', label: 'ENB (PWM)', gpio: 18, bus: 'PWM' },
    IN3_DIR:     { color: 'net-tag-output', type: 'output', label: 'IN3 (Dir R)', gpio: 8,  bus: 'DIR_R' },
    IN4_DIR:     { color: 'net-tag-output', type: 'output', label: 'IN4 (Dir R)', gpio: 9,  bus: 'DIR_R' },
    ENC_IZQ:     { color: 'net-tag-input',  type: 'input',  label: 'ENC IZQ',    gpio: 4,  bus: 'ENC' },
    ENC_DER:     { color: 'net-tag-input',  type: 'input',  label: 'ENC DER',    gpio: 5,  bus: 'ENC' },
    SDA_BUS:     { color: 'net-tag-bus',    type: 'bidir',  label: 'SDA (I2C)',  gpio: 6,  bus: 'I2C' },
    SCL_BUS:     { color: 'net-tag-bus',    type: 'bidir',  label: 'SCL (I2C)',  gpio: 7,  bus: 'I2C' },
    LED_IDLE:    { color: 'net-tag-output', type: 'output', label: 'LED IDLE',   gpio: 38, bus: 'LED' },
    LED_MOVING:  { color: 'net-tag-output', type: 'output', label: 'LED MOVE',   gpio: 39, bus: 'LED' },
    LED_ERROR:   { color: 'net-tag-output', type: 'output', label: 'LED ERR',    gpio: 40, bus: 'LED' },
    ESTOP_SIG:   { color: 'net-tag-input',  type: 'input',  label: 'E-STOP',     gpio: 41, bus: 'SAFE' },

    VCC_3V3:     { color: 'net-tag-power',  type: 'power',  label: '3.3V',       gpio: 0,  bus: 'PWR' },
    VCC_5V:      { color: 'net-tag-power',  type: 'power',  label: '5V',         gpio: 0,  bus: 'PWR' },
    GND:         { color: 'net-tag-gnd',    type: 'ground', label: 'GND',        gpio: 0,  bus: 'GND' },
    VMS_12V:     { color: 'net-tag-power',  type: 'power',  label: '12V VMS',    gpio: 0,  bus: 'PWR' },
    MOTOR_L:     { color: 'net-tag-output', type: 'power',  label: 'MOTOR L',    gpio: 0,  bus: 'MOT_L' },
    MOTOR_R:     { color: 'net-tag-output', type: 'power',  label: 'MOTOR R',    gpio: 0,  bus: 'MOT_R' }
  };

  // ── Definición de Buses ──
  const BUSES = {
    PWM: {
      color: '#f97316',
      label: 'BUS_PWM',
      signals: ['ENA_PWM', 'ENB_PWM'],
      path: [[855, 240], [980, 240], [980, 170], [1165, 170]],
      stubs: [
        { from: [980, 240], to: [855, 240], net: 'ENA_PWM' },
        { from: [980, 330], to: [855, 330], net: 'ENB_PWM' },
        { from: [980, 170], to: [1165, 170], net: 'ENA_PWM' },
        { from: [980, 265], to: [1165, 265], net: 'ENB_PWM' }
      ]
    },
    DIR_L: {
      color: '#eab308',
      label: 'BUS_DIR_L',
      signals: ['IN1_DIR', 'IN2_DIR'],
      path: [[855, 270], [990, 270], [990, 200], [1165, 200]],
      stubs: [
        { from: [990, 270], to: [855, 270], net: 'IN1_DIR' },
        { from: [990, 300], to: [855, 300], net: 'IN2_DIR' },
        { from: [990, 200], to: [1165, 200], net: 'IN1_DIR' },
        { from: [990, 230], to: [1165, 230], net: 'IN2_DIR' }
      ]
    },
    DIR_R: {
      color: '#06b6d4',
      label: 'BUS_DIR_R',
      signals: ['IN3_DIR', 'IN4_DIR'],
      path: [[855, 370], [1000, 370], [1000, 295], [1165, 295]],
      stubs: [
        { from: [1000, 370], to: [855, 370], net: 'IN3_DIR' },
        { from: [1000, 400], to: [855, 400], net: 'IN4_DIR' },
        { from: [1000, 295], to: [1165, 295], net: 'IN3_DIR' },
        { from: [1000, 325], to: [1165, 325], net: 'IN4_DIR' }
      ]
    },
    I2C: {
      color: '#22c55e',
      label: 'BUS_I2C',
      signals: ['SDA_BUS', 'SCL_BUS'],
      path: [[655, 310], [500, 310], [500, 260], [285, 260]],
      stubs: [
        { from: [500, 310], to: [655, 310], net: 'SDA_BUS' },
        { from: [500, 340], to: [655, 340], net: 'SCL_BUS' },
        { from: [500, 260], to: [285, 260], net: 'SDA_BUS' },
        { from: [500, 285], to: [285, 285], net: 'SCL_BUS' }
      ]
    },
    PWR: {
      color: '#ef4444',
      label: 'BUS_PWR',
      signals: ['VMS_12V', 'VCC_5V', 'VCC_3V3'],
      path: [[1395, 700], [1320, 700], [1320, 350], [920, 350], [920, 180], [855, 180]],
      stubs: [
        { from: [1320, 700], to: [1395, 700], net: 'VMS_12V' },
        { from: [1320, 350], to: [1320, 345], net: 'VCC_5V' },
        { from: [920, 180], to: [855, 180], net: 'VCC_5V' }
      ]
    }
  };

  // ── Conexiones Físicas (cables individuales) ──
  const wires = [
    // ESP32 → L298N (Motor Control)
    { id: 'gp15', color: '#f97316', label: 'ENA (PWM Motor Izquierdo)', bus: 'PWM', type: 'signal',
      path: [[855,240],[980,240],[980,170],[1165,170]] },
    { id: 'gp16', color: '#eab308', label: 'IN1 (Sentido Motor Izquierdo)', bus: 'DIR_L', type: 'signal',
      path: [[855,270],[990,270],[990,200],[1165,200]] },
    { id: 'gp17', color: '#eab308', label: 'IN2 (Sentido Motor Izquierdo)', bus: 'DIR_L', type: 'signal',
      path: [[855,300],[1000,300],[1000,230],[1165,230]] },
    { id: 'gp18', color: '#f97316', label: 'ENB (PWM Motor Derecho)', bus: 'PWM', type: 'signal',
      path: [[855,330],[1010,330],[1010,265],[1165,265]] },
    { id: 'gp8',  color: '#06b6d4', label: 'IN3 (Sentido Motor Derecho)', bus: 'DIR_R', type: 'signal',
      path: [[855,370],[1020,370],[1020,295],[1165,295]] },
    { id: 'gp9',  color: '#06b6d4', label: 'IN4 (Sentido Motor Derecho)', bus: 'DIR_R', type: 'signal',
      path: [[855,400],[1030,400],[1030,325],[1165,325]] },
    
    // L298N → Motors
    { id: 'out1', color: '#fbbf24', label: 'OUT1→Motor L M1A', bus: 'MOT_L', type: 'signal',
      path: [[1345,170],[1460,170],[1460,105],[1565,105]] },
    { id: 'out2', color: '#fbbf24', label: 'OUT2→Motor L M1B', bus: 'MOT_L', type: 'signal',
      path: [[1345,200],[1470,200],[1470,130],[1565,130]] },
    { id: 'out3', color: '#22d3ee', label: 'OUT3→Motor R M2A', bus: 'MOT_R', type: 'signal',
      path: [[1345,265],[1565,265]] },
    { id: 'out4', color: '#22d3ee', label: 'OUT4→Motor R M2B', bus: 'MOT_R', type: 'signal',
      path: [[1345,295],[1480,295],[1480,290],[1565,290]] },
    
    // ESP32 → Encoders
    { id: 'gp4',  color: '#22c55e', label: 'ENC IZQ (GP4)', bus: 'ENC', type: 'signal',
      path: [[655,240],[530,240],[530,460],[305,460]] },
    { id: 'gp5',  color: '#3b82f6', label: 'ENC DER (GP5)', bus: 'ENC', type: 'signal',
      path: [[655,270],[540,270],[540,490],[305,490]] },
    
    // ESP32 → MPU6050 (I2C)
    { id: 'gp6',  color: '#22c55e', label: 'SDA (GP6)', bus: 'I2C', type: 'signal',
      path: [[655,310],[510,310],[510,260],[285,260]] },
    { id: 'gp7',  color: '#3b82f6', label: 'SCL (GP7)', bus: 'I2C', type: 'signal',
      path: [[655,340],[500,340],[500,285],[285,285]] },
    
    // ESP32 → LEDs
    { id: 'gp38', color: '#22c55e', label: 'LED IDLE (GP38)', bus: 'LED', type: 'signal',
      path: [[855,440],[1050,440],[1050,480],[1165,480]] },
    { id: 'gp39', color: '#3b82f6', label: 'LED MOVING (GP39)', bus: 'LED', type: 'signal',
      path: [[855,470],[1060,470],[1060,510],[1165,510]] },
    { id: 'gp40', color: '#ef4444', label: 'LED ERROR (GP40)', bus: 'LED', type: 'signal',
      path: [[855,500],[1070,500],[1070,540],[1165,540]] },
    
    // ESP32 → E-STOP
    { id: 'gp41', color: '#ef4444', label: 'E-STOP (GP41)', bus: 'SAFE', type: 'signal',
      path: [[655,420],[480,420],[480,890],[285,890]] },
    
    // Power wires
    { id: 'pwr_12v', color: '#ef4444', label: 'Batería 12V → L298N VMS', bus: 'PWR', type: 'power',
      path: [[1395,700],[1200,700],[1200,345]] },
    { id: 'pwr_gnd', color: '#475569', label: 'Batería GND → L298N GND', bus: 'GND', type: 'gnd',
      path: [[1395,730],[1260,730],[1260,345]] },
    { id: 'pwr_5v',  color: '#ef4444', label: 'Regulador 5V → ESP32 VIN', bus: 'PWR', type: 'power',
      path: [[1320,345],[1320,380],[920,380],[920,180],[855,180]] },
    { id: 'esp_gnd',  color: '#475569', label: 'GND Común ESP32-L298N', bus: 'GND', type: 'gnd',
      path: [[855,205],[910,205],[910,360],[1260,360],[1260,345]] }
  ];

  // ── Inicializar ──
  function init() {
    drawWires();
    drawBuses();
    buildNetTags();
    setViewMode(state.viewMode);
    bindEvents();
  }

  // ── Calcular coordenadas lógicas del pin ──
  function getLogicalCoords(pin) {
    const canvas = document.getElementById('schematicCanvas');
    let x = 0, y = 0, curr = pin;
    while (curr && curr !== canvas) {
      x += curr.offsetLeft;
      y += curr.offsetTop;
      curr = curr.offsetParent;
    }
    return { x: x + pin.offsetWidth / 2, y: y + pin.offsetHeight / 2 };
  }

  // ── Dibujar Cables SVG ──
  function drawWires() {
    const svg = document.getElementById('wiresLayer');
    if (!svg) return;
    svg.innerHTML = '';

    wires.forEach(wire => {
      // Filtrar por tipo
      if (wire.type === 'power' && !state.filters.power) return;
      if (wire.type === 'gnd' && !state.filters.gnd) return;
      if (wire.type === 'signal' && !state.filters.signal) return;

      const pathData = wire.path.map((p, i) => (i === 0 ? `M${p[0]},${p[1]}` : `L${p[0]},${p[1]}`)).join(' ');

      // Sombra
      const shadow = document.createElementNS('http://www.w3.org/2000/svg', 'path');
      shadow.setAttribute('d', pathData);
      shadow.setAttribute('fill', 'none');
      shadow.setAttribute('stroke', wire.color);
      shadow.setAttribute('stroke-width', '5');
      shadow.setAttribute('stroke-linecap', 'round');
      shadow.setAttribute('stroke-linejoin', 'round');
      shadow.setAttribute('opacity', '0.12');
      shadow.setAttribute('class', 'wire-shadow');
      shadow.setAttribute('data-wire-id', wire.id);
      shadow.setAttribute('data-bus', wire.bus);
      svg.appendChild(shadow);

      // Cable principal
      const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
      path.setAttribute('d', pathData);
      path.setAttribute('fill', 'none');
      path.setAttribute('stroke', wire.color);
      path.setAttribute('stroke-width', '2.5');
      path.setAttribute('stroke-linecap', 'round');
      path.setAttribute('stroke-linejoin', 'round');
      path.setAttribute('class', 'wire');
      path.setAttribute('data-wire-id', wire.id);
      path.setAttribute('data-bus', wire.bus);
      path.setAttribute('data-label', wire.label);
      path.style.pointerEvents = 'stroke';
      path.style.cursor = 'pointer';

      path.addEventListener('mouseenter', (e) => showTooltip(e, wire.label, `Bus: ${wire.bus} · ID: ${wire.id.toUpperCase()}`));
      path.addEventListener('mousemove', moveTooltip);
      path.addEventListener('mouseleave', hideTooltip);
      path.addEventListener('click', (e) => { e.stopPropagation(); highlightWire(wire.id); });

      svg.appendChild(path);

      // Puntos de esquina
      wire.path.forEach((p, i) => {
        if (i > 0 && i < wire.path.length - 1) {
          const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
          circle.setAttribute('cx', p[0]);
          circle.setAttribute('cy', p[1]);
          circle.setAttribute('r', '3');
          circle.setAttribute('fill', wire.color);
          circle.setAttribute('class', 'wire-junction');
          circle.setAttribute('data-wire-id', wire.id);
          circle.setAttribute('data-bus', wire.bus);
          svg.appendChild(circle);
        }
      });
    });
  }

  // ── Dibujar Buses ──
  function drawBuses() {
    const svg = document.getElementById('busesLayer');
    if (!svg) return;
    svg.innerHTML = '';

    Object.entries(BUSES).forEach(([busName, bus]) => {
      const pathData = bus.path.map((p, i) => (i === 0 ? `M${p[0]},${p[1]}` : `L${p[0]},${p[1]}`)).join(' ');

      // Bus principal (línea gruesa)
      const busLine = document.createElementNS('http://www.w3.org/2000/svg', 'path');
      busLine.setAttribute('d', pathData);
      busLine.setAttribute('fill', 'none');
      busLine.setAttribute('stroke', bus.color);
      busLine.setAttribute('class', 'bus-line');
      busLine.setAttribute('data-bus-name', busName);
      busLine.style.pointerEvents = 'stroke';
      busLine.style.cursor = 'pointer';

      busLine.addEventListener('mouseenter', (e) => showTooltip(e, bus.label, `Señales: ${bus.signals.join(', ')}`));
      busLine.addEventListener('mousemove', moveTooltip);
      busLine.addEventListener('mouseleave', hideTooltip);
      busLine.addEventListener('click', (e) => { e.stopPropagation(); highlightBus(busName); });

      svg.appendChild(busLine);

      // Etiqueta del bus
      const midPoint = bus.path[Math.floor(bus.path.length / 2)];
      const label = document.createElementNS('http://www.w3.org/2000/svg', 'text');
      label.setAttribute('x', midPoint[0]);
      label.setAttribute('y', midPoint[1] - 10);
      label.setAttribute('class', 'bus-label');
      label.setAttribute('fill', bus.color);
      label.textContent = bus.label;
      svg.appendChild(label);

      // Stubs (conexiones individuales al bus)
      bus.stubs.forEach(stub => {
        const stubPath = document.createElementNS('http://www.w3.org/2000/svg', 'line');
        stubPath.setAttribute('x1', stub.from[0]);
        stubPath.setAttribute('y1', stub.from[1]);
        stubPath.setAttribute('x2', stub.to[0]);
        stubPath.setAttribute('y2', stub.to[1]);
        stubPath.setAttribute('stroke', bus.color);
        stubPath.setAttribute('class', 'bus-stub');
        stubPath.setAttribute('data-bus-name', busName);
        stubPath.setAttribute('data-net', stub.net);
        svg.appendChild(stubPath);
      });
    });
  }

  // ── Crear etiquetas de red ──
  function buildNetTags() {
    const canvas = document.getElementById('schematicCanvas');
    if (!canvas) return;

    const pins = document.querySelectorAll('[data-net]');
    state.pinElements = pins;

    pins.forEach(pin => {
      const netName = pin.dataset.net;
      const netDef = NETS[netName];
      if (!netDef) return;

      const coords = getLogicalCoords(pin);
      const isLeft = pin.style.left !== '' && parseFloat(pin.style.left) < 10;
      const isRight = pin.style.right !== '' && parseFloat(pin.style.right) < 10;
      const isBottom = pin.style.bottom !== '' && parseFloat(pin.style.bottom) < 10;

      const tag = document.createElement('span');
      tag.className = `net-tag ${netDef.color}`;
      tag.dataset.net = netName;
      tag.dataset.bus = netDef.bus;
      tag.textContent = `${netDef.label} [${netDef.bus}]`;

      tag.style.top = coords.y + 'px';
      if (isLeft) {
        tag.style.left = (coords.x - 10) + 'px';
        tag.style.transform = 'translate(-100%, -50%)';
        tag.classList.add('tag-left');
      } else if (isRight) {
        tag.style.left = (coords.x + 10) + 'px';
        tag.style.transform = 'translate(0, -50%)';
        tag.classList.add('tag-right');
      } else if (isBottom) {
        tag.style.left = coords.x + 'px';
        tag.style.top = (coords.y + 10) + 'px';
        tag.style.transform = 'translate(-50%, 0)';
        tag.classList.add('tag-bottom');
      } else {
        tag.style.left = (coords.x + 10) + 'px';
        tag.style.transform = 'translate(0, -50%)';
        tag.classList.add('tag-right');
      }

      tag.addEventListener('mouseenter', (e) => {
        showTooltip(e, `Red: ${netDef.label}`, `Bus: ${netDef.bus} · Tipo: ${netDef.type.toUpperCase()} · Pin: ${pin.dataset.pin.toUpperCase()}`);
      });
      tag.addEventListener('mousemove', moveTooltip);
      tag.addEventListener('mouseleave', hideTooltip);
      tag.addEventListener('click', (e) => { e.stopPropagation(); highlightNet(netName); });

      canvas.appendChild(tag);
      state.netTags.push(tag);
    });
  }

  // ── Resaltar Bus ──
  function highlightBus(busName) {
    if (state.activeBus === busName) { resetHighlights(); return; }
    state.activeBus = busName;
    state.activeNet = null;

    // Resaltar buses
    document.querySelectorAll('.bus-line, .bus-stub').forEach(el => {
      if (el.getAttribute('data-bus-name') === busName) {
        el.classList.add('highlighted');
        el.classList.remove('dimmed');
      } else {
        el.classList.add('dimmed');
        el.classList.remove('highlighted');
      }
    });

    // Resaltar cables del bus
    document.querySelectorAll('.wire, .wire-shadow, .wire-junction').forEach(w => {
      if (w.getAttribute('data-bus') === busName) {
        w.classList.add('highlighted');
        w.classList.remove('dimmed');
      } else {
        w.classList.add('dimmed');
        w.classList.remove('highlighted');
      }
    });

    // Resaltar net tags del bus
    state.netTags.forEach(t => {
      t.classList.toggle('glow', t.dataset.bus === busName);
    });

    // Resaltar filas de tabla
    document.querySelectorAll('.pin-row').forEach(row => {
      const busCell = row.querySelector('.pin-bus');
      if (busCell && busCell.textContent === busName) {
        row.classList.add('highlighted');
        row.classList.remove('dimmed');
      } else {
        row.classList.remove('highlighted');
        row.classList.add('dimmed');
      }
    });

    // Info panel
    const bus = BUSES[busName];
    if (bus) {
      document.getElementById('infoPanel').innerHTML = `
        <div style="border-left: 4px solid ${bus.color}; padding-left: 8px;">
          <h4 style="color:${bus.color}; margin-bottom:4px; font-size:12px;">BUS: ${bus.label}</h4>
          <p><strong>Señales:</strong> ${bus.signals.join(', ')}</p>
          <p><strong>Tipo:</strong> ${busName.includes('PWM') ? 'PWM' : busName.includes('I2C') ? 'I2C Bidireccional' : 'Control Digital'}</p>
          <p style="margin-top:6px; font-size:10px; color:#64748b;">${bus.signals.length} señales agrupadas en este bus.</p>
        </div>
      `;
    }
  }

  // ── Resaltar Red ──
  function highlightNet(netName) {
    if (state.activeNet === netName) { resetHighlights(); return; }
    state.activeNet = netName;
    state.activeBus = null;

    state.netTags.forEach(t => t.classList.toggle('glow', t.dataset.net === netName));
    state.pinElements.forEach(p => p.classList.toggle('pin-glow', p.dataset.net === netName));

    const allWires = document.querySelectorAll('.wire, .wire-shadow, .wire-junction');
    allWires.forEach(w => {
      const wireId = w.getAttribute('data-wire-id');
      const matchingWire = wires.find(item => item.id === wireId);
      if (matchingWire && getNetFromWireId(matchingWire.id) === netName) {
        w.classList.add('highlighted');
        w.classList.remove('dimmed');
      } else {
        w.classList.add('dimmed');
        w.classList.remove('highlighted');
      }
    });

    document.querySelectorAll('.pin-row').forEach(row => {
      const clickAttr = row.getAttribute('onclick') || '';
      const match = clickAttr.match(/'([^']+)'/);
      if (match) {
        const wireId = match[1];
        const rowNet = getNetFromWireId(wireId);
        row.classList.toggle('highlighted', rowNet === netName);
        row.classList.toggle('dimmed', rowNet !== netName);
      }
    });

    const net = NETS[netName];
    if (net) {
      const connectedCount = Array.from(state.pinElements).filter(p => p.dataset.net === netName).length;
      document.getElementById('infoPanel').innerHTML = `
        <div style="border-left: 4px solid #3b82f6; padding-left: 8px;">
          <h4 style="color:#60a5fa; margin-bottom:4px; font-size:12px;">NET: ${netName}</h4>
          <p><strong>Función:</strong> ${net.label}</p>
          <p><strong>Bus:</strong> ${net.bus}</p>
          <p><strong>Tipo:</strong> ${net.type.toUpperCase()}</p>
          <p><strong>ESP32 GPIO:</strong> ${net.gpio ? 'GP' + net.gpio : 'N/A'}</p>
          <p style="margin-top:6px; font-size:10px; color:#64748b;">${connectedCount} puntos de conexión.</p>
        </div>
      `;
    }
  }

  // ── Resaltar Cable ──
  function highlightWire(wireId) {
    const allWires = document.querySelectorAll('.wire, .wire-shadow, .wire-junction');
    allWires.forEach(w => {
      if (w.getAttribute('data-wire-id') === wireId) {
        w.classList.add('highlighted');
        w.classList.remove('dimmed');
      } else {
        w.classList.add('dimmed');
        w.classList.remove('highlighted');
      }
    });

    const netName = getNetFromWireId(wireId);
    if (netName) {
      state.netTags.forEach(t => t.classList.toggle('glow', t.dataset.net === netName));
      state.pinElements.forEach(p => p.classList.toggle('pin-glow', p.dataset.net === netName));
    }

    document.querySelectorAll('.pin-row').forEach(row => {
      const clickAttr = row.getAttribute('onclick') || '';
      row.classList.toggle('highlighted', clickAttr.includes(`'${wireId}'`));
      row.classList.toggle('dimmed', !clickAttr.includes(`'${wireId}'`));
    });

    const wire = wires.find(w => w.id === wireId);
    if (wire) {
      document.getElementById('infoPanel').innerHTML = `
        <div style="border-left: 4px solid ${wire.color}; padding-left: 8px;">
          <h4 style="color:${wire.color}; margin-bottom:4px; font-size:12px;">CONEXIÓN: ${wire.id.toUpperCase()}</h4>
          <p><strong>Línea:</strong> ${wire.label}</p>
          <p><strong>Bus:</strong> ${wire.bus}</p>
          <p><strong>Tipo:</strong> ${wire.type.toUpperCase()}</p>
          <p><strong>Color:</strong> <span style="display:inline-block; width:10px; height:10px; background:${wire.color}; border-radius:2px; vertical-align:middle;"></span> ${wire.color}</p>
        </div>
      `;
    }
  }

  // ── Mapeo ID → Red ──
  function getNetFromWireId(wireId) {
    const map = {
      gp15: 'ENA_PWM', gp16: 'IN1_DIR', gp17: 'IN2_DIR', gp18: 'ENB_PWM',
      gp8: 'IN3_DIR', gp9: 'IN4_DIR', gp4: 'ENC_IZQ', gp5: 'ENC_DER',
      gp6: 'SDA_BUS', gp7: 'SCL_BUS', gp38: 'LED_IDLE', gp39: 'LED_MOVING',
      gp40: 'LED_ERROR', gp41: 'ESTOP_SIG', pwr_12v: 'VMS_12V', pwr_gnd: 'GND', pwr_5v: 'VCC_5V',
      esp_gnd: 'GND', out1: 'MOTOR_L', out2: 'MOTOR_L', out3: 'MOTOR_R', out4: 'MOTOR_R'
    };
    return map[wireId] || null;
  }

  // ── Resetear resaltados ──
  function resetHighlights() {
    state.activeNet = null;
    state.activeBus = null;
    state.netTags.forEach(t => t.classList.remove('glow'));
    state.pinElements.forEach(p => p.classList.remove('pin-glow'));
    
    document.querySelectorAll('.wire, .wire-shadow, .wire-junction, .bus-line, .bus-stub').forEach(w => {
      w.classList.remove('dimmed', 'highlighted');
    });

    document.querySelectorAll('.pin-row').forEach(row => {
      row.classList.remove('highlighted', 'dimmed');
    });
  }

  // ── Toggle Filtros ──
  function toggleFilter(filterName) {
    state.filters[filterName] = !state.filters[filterName];
    const btn = document.querySelector(`.filter-btn[data-filter="${filterName}"]`);
    if (btn) {
      btn.classList.toggle('active', state.filters[filterName]);
      btn.classList.toggle('inactive', !state.filters[filterName]);
    }
    drawWires();
  }

  // ── Conmutar Vista ──
  function setViewMode(mode) {
    state.viewMode = mode;
    const wiresLayer = document.getElementById('wiresLayer');
    const busesLayer = document.getElementById('busesLayer');
    const isLabelsMode = (mode === 'labels');
    const isBusesMode = (mode === 'buses');

    if (wiresLayer) wiresLayer.style.display = isLabelsMode ? 'none' : 'block';
    if (busesLayer) busesLayer.style.display = isLabelsMode ? 'none' : 'block';

    state.netTags.forEach(tag => {
      tag.classList.toggle('visible', isLabelsMode || isBusesMode);
    });

    document.querySelectorAll('.toolbar button[data-mode]').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.mode === mode);
    });

    const modeNames = { wires: 'Cables Físicos', buses: 'Buses + Labels', labels: 'Esquema Puro' };
    document.getElementById('modeDisplay').textContent = modeNames[mode] || mode;

    resetHighlights();
  }

  // ── Toggle Panel ──
  function togglePanel() {
    state.sidebarCollapsed = !state.sidebarCollapsed;
    document.getElementById('sidePanel').classList.toggle('collapsed', state.sidebarCollapsed);
    document.getElementById('legend').classList.toggle('collapsed', state.sidebarCollapsed);
    document.getElementById('zoomControls').classList.toggle('shifted', state.sidebarCollapsed);
    const btn = document.getElementById('btnTogglePanel');
    if (btn) {
      btn.classList.toggle('active', state.sidebarCollapsed);
      btn.textContent = state.sidebarCollapsed ? 'Mostrar Panel' : 'Tabla';
    }
  }

  // ── Tooltip ──
  function showTooltip(e, title, detail) {
    const tooltip = document.getElementById('tooltip');
    if (!tooltip) return;
    document.getElementById('tooltipTitle').textContent = title;
    document.getElementById('tooltipDetail').textContent = detail;
    tooltip.style.display = 'block';
    moveTooltip(e);
  }
  function moveTooltip(e) {
    const tooltip = document.getElementById('tooltip');
    if (!tooltip) return;
    tooltip.style.left = (e.clientX + 14) + 'px';
    tooltip.style.top = (e.clientY - 12) + 'px';
  }
  function hideTooltip() {
    const tooltip = document.getElementById('tooltip');
    if (tooltip) tooltip.style.display = 'none';
  }

  // ── Zoom ──
  function zoomIn() { state.currentZoom = Math.min(state.currentZoom + 0.15, 2.5); applyZoom(); }
  function zoomOut() { state.currentZoom = Math.max(state.currentZoom - 0.15, 0.4); applyZoom(); }
  function resetView() {
    state.currentZoom = 1;
    applyZoom();
    const container = document.getElementById('canvasContainer');
    if (container) container.scrollTo({ top: 0, left: 0, behavior: 'smooth' });
  }
  function applyZoom() {
    const canvas = document.getElementById('schematicCanvas');
    const zoomDisplay = document.getElementById('zoomLevel');
    if (canvas) {
      canvas.style.transform = `scale(${state.currentZoom})`;
      canvas.style.transformOrigin = 'top left';
    }
    if (zoomDisplay) zoomDisplay.textContent = Math.round(state.currentZoom * 100) + '%';
  }

  // ── Eventos ──
  function bindEvents() {
    document.querySelectorAll('.toolbar button[data-mode]').forEach(btn => {
      btn.addEventListener('click', () => setViewMode(btn.dataset.mode));
    });

    window.zoomIn = zoomIn;
    window.zoomOut = zoomOut;
    window.resetView = resetView;
    window.togglePanel = togglePanel;
    window.highlightWire = highlightWire;
    window.toggleFilter = toggleFilter;

    document.querySelectorAll('.component').forEach(comp => {
      comp.addEventListener('mouseenter', (e) => showTooltip(e, comp.dataset.name, comp.dataset.info));
      comp.addEventListener('mousemove', moveTooltip);
      comp.addEventListener('mouseleave', hideTooltip);
      comp.addEventListener('click', (e) => {
        if (e.target.closest('.comp-pin') || e.target.closest('.net-tag')) return;
        e.stopPropagation();
        document.querySelectorAll('.component').forEach(c => c.classList.remove('highlight'));
        comp.classList.add('highlight');
        document.getElementById('infoPanel').innerHTML = `
          <div style="border-left: 4px solid #3b82f6; padding-left: 8px;">
            <h4 style="color:#60a5fa; margin-bottom:4px; font-size:12px;">${comp.dataset.name}</h4>
            <p>${comp.dataset.info}</p>
          </div>
        `;
      });
    });

    document.getElementById('schematicCanvas').addEventListener('click', (e) => {
      if (!e.target.closest('.component') && !e.target.closest('.wire') && !e.target.closest('.net-tag') && !e.target.closest('.bus-line')) {
        resetHighlights();
        document.querySelectorAll('.component').forEach(c => c.classList.remove('highlight'));
        document.getElementById('infoPanel').innerHTML = '<p>Haga clic en un componente, cable o bus para ver detalles.</p>';
      }
    });

    const container = document.getElementById('canvasContainer');
    const coordDisplay = document.getElementById('coordDisplay');
    if (container && coordDisplay) {
      container.addEventListener('mousemove', (e) => {
        const canvas = document.getElementById('schematicCanvas');
        const rect = canvas.getBoundingClientRect();
        const x = Math.round((e.clientX - rect.left) / state.currentZoom);
        const y = Math.round((e.clientY - rect.top) / state.currentZoom);
        if (x >= 0 && x <= 1600 && y >= 0 && y <= 1000) {
          coordDisplay.textContent = `X: ${x} Y: ${y}`;
        }
      });
    }

    let resizeTimer;
    window.addEventListener('resize', () => {
      clearTimeout(resizeTimer);
      resizeTimer = setTimeout(() => {
        state.netTags.forEach(t => t.remove());
        state.netTags = [];
        buildNetTags();
        setViewMode(state.viewMode);
      }, 250);
    });
  }

  // ── Arrancar ──
  document.addEventListener('DOMContentLoaded', init);

})();
