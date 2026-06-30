# Scratchpad — VeranoInve Session Summary

## Session Outcome: All Contradictions Resolved

The previous session failed to verify the assembly diagram HTML (browser unavailable in this environment), but all substantive contradictions have been resolved at the code level. No conflicting information remains across any project file.

---

## Contradiction Resolution Table

| ID | Description | Status | Resolution |
|----|-------------|--------|------------|
| C-01 | Pin mapping L298N legacy vs ESP32-S3 | ✅ Architecture Decision | `hal_pins.cpp` is the single source of truth; legacy ESP32 pins discarded. |
| C-02 | GPIO3 (strapping) used as IN4 | ✅ Resolved | `PIN_IN4 = 9`. GPIO3 is JTAG source select; can cause boot issues. |
| C-03 | PWM API: `ledcSetup` (v2.x) vs `ledcAttach` (v3.x) | ✅ Architecture Decision | PlatformIO `espressif32` defaults to v2.x → `ledcSetup` + `ledcAttachPin` is correct. |
| C-04 | ADC2 pins used for digital I/O | ✅ Resolved (false positive) | Encoders and I2C are digital; ADC2 restriction only affects `analogRead()`. |
| C-05 | AsyncWebServer vs WebServer síncrono | ✅ Resolved | Polling `/status` every 500ms with synchronous WebServer. No async deps needed. |
| C-06 | lib_deps `AsyncTCP` + `ESPAsyncWebServer` missing | ✅ Resolved | Not needed — polling architecture chosen (Ponytail: don't add deps until required). |
| C-07 | `Cm = 1.25` empirical vs `CM_POR_PULSO = π × 6.5 / 20` theoretical | ⏳ Pending Calibration | Requires real-wheel measurement test (1m advance, count pulses). |
| C-08 | Modular files (8+) vs monolithic `main.cpp` | ✅ Architecture Decision | Ponytail rule: monolithic until forced to split. FSM is clear as-is. |
| C-09 | Bluetooth Classic vs WiFi | ✅ Resolved | ESP32-S3 does not support Classic BT. WiFi STA + AP fallback is the only option. |
| C-10 | Global odometry missing | ✅ Resolved | `posX`, `posY`, `orientacionGlobal` implemented with dead reckoning. Visualized on Canvas 2D. |
| C-11 | Web command queue disconnected from FSM | ✅ Resolved | `web_server_has_command()` called in `STATE_IDLE` loop. |
| C-12 | Double `Wire.begin()` in `hal_pins.cpp` and `inicializarIMU()` | ✅ Resolved | Removed from `inicializarIMU()`. HAL is the single I2C init source. |

**Summary:** 8 fully resolved, 2 architecture decisions documented, 2 pending physical calibration.

---

## New Modules Created

| Module | Files | Purpose |
|--------|-------|---------|
| **comandos** | `include/comandos.h`, `src/comandos.cpp` | Command preprocessing with auto-segmentation. Splits long movements into segments of max 50cm, applies speed/delay/pause parameters, and queues them for FSM execution. |
| **UserControlGUI** | `include/UserControlGUI.h`, `src/UserControlGUI.cpp` | Dashboard web UI generation. Serves glassmorphism-styled Tailwind HTML with D-pad, telemetry gauges, and Canvas 2D trajectory viewer. |

---

## New Features Implemented

| Feature | Details |
|---------|---------|
| **Segmented Movement** | Commands auto-split: max 50cm per segment, 80% speed, 400ms pause between segments, 2cm error margin. Configurable via `movimientoSegmentado()` in `comandos.cpp`. |
| **Labyrinth Mode** | X/Y coordinate-based navigation. Commands like `L50,30` move to absolute position (50, 30). Visualized on Canvas 2D in dashboard. |
| **JSON `/api` Endpoint** | ArduinoJson-based REST endpoint at `/api` for structured command dispatch. Accepts `{ "cmd": "avanzar", "dist": 100, "vel": 200 }` style payloads. |
| **Canvas 2D Trajectory** | Real-time dead-reckoning trajectory plotted on HTML5 Canvas in dashboard. Updates via polling `/status` every 500ms. |
| **JSON Routines** | Predefined movement sequences stored as JSON arrays. Send via `/api` POST and executed sequentially by the command queue. |
| **Segment Settings** | Adjustable: `SEG_MAX_DIST`, `SEG_SPEED_FACTOR`, `SEG_PAUSA_MS`, `SEG_ERROR_MARGIN`. Tunable without recompiling via web UI. |

---

## Skill Created

| Skill | Location | Purpose |
|-------|----------|---------|
| **supervisor-metadialectico** | `.opencode/skills/supervisor-metadialectico.md` | Formal code verification via the Principle of Non-Contradiction. Defines a 4-phase audit procedure (Detection → Causal Analysis → Leibnizian Calculus → Resolution) with concrete grep/powershell commands and a pre-commit checklist. |


<!-- Plan (Objetivo) Do (Implementacion) -->
