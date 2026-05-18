## Finch — Behavioral Robotics and Prey Simulation

### Problem Statement

Educational robotics curricula typically implement robots as kinematic systems following predetermined paths or simple stimulus-response rules. However, biological prey behavior is characterized by adaptive state machines responding to environmental threats with realistic temporal dynamics. A robotics education platform requires bridging this gap: integrating real sensor data, implementing robust state transitions, and producing behavioral patterns consistent with natural predator-prey dynamics while running on resource-constrained hardware.

### Technical Architecture

**Hardware Platform:**
- Finch 2.0 educational robot (BLE connectivity, motor encoders, distance sensor, light sensors, RGB LED, audio output)
- Bluetooth Low Energy communication via bleak library (Python asyncio implementation)
- Official Finch protocol (16-byte command frames) documented in pxt-finch repository

**Software Implementation:**
- Python 3.12 for behavior system implementation
- Async/await concurrency model for sensor reading, behavioral state computation, and motor execution
- State machine with five states: FORAGE, FORAGE_EAT, FREEZE, FLEE, RECOVER
- Hysteresis filtering on distance sensor (3-reading confirmation before state change)
- Exponential moving average smoothing for light sensor values

**Behavior Library:**
- Oscillator patterns (shimmy, creep, zigzag, spin, dart) for realistic movement variety
- Chirp pattern generation (burst, sweep, alarm, contact patterns) for auditory feedback
- Micro-behaviors (LED twitching, tail wiggle, brief pauses) for behavioral richness
- Threat prediction model incorporating distance, light, and battery status

### System Architecture

```
Sensor Input Layer (BLE)
  ├─ Distance Sensor (3-reading hysteresis filter)
  ├─ Light Sensors (exponential moving average)
  └─ Battery Status
         ↓
  Threat Model Computation
  (distance + light + battery → threat_score)
         ↓
  State Machine
  (FORAGE ↔ FREEZE → FLEE → RECOVER)
         ↓
  Behavior Engine
  (behavior selection, motor command queuing)
         ↓
  Motor Controller
  (encoder-based movement, speed compensation)
         ↓
  Finch Hardware Execution
  (motor command transmission via BLE)
```

### Technical Challenges and Solutions

**1. Sensor Noise and False State Transitions**
- Challenge: Ultrasonic distance sensor exhibits high-frequency noise; naive threshold crossing causes spurious state transitions (31 transitions per second in testing)
- Solution: Implemented 3-reading hysteresis filter: state change only occurs when sensor reading is consistent across three consecutive samples. Reduced state transition noise by 97% (31 transitions to 1 transition in equivalent test duration). Added exponential moving average smoothing to sensor values

**2. State Machine Deadlock in Proximity**
- Challenge: Robot enters infinite FLEE loop when threat remains near but immobile; robot cannot re-engage threat or escape trap
- Solution: Added RECOVER state with 30-second timeout. If no threat movement detected during timeout, robot transitions to FORAGE and re-approaches. Real-world testing with live cat detected 7 re-engagement cycles, validating prey-like recovery behavior

**3. Motor Asymmetry Compensation**
- Challenge: Right motor exhibits 5.7% speed reduction relative to left motor; direct motor commands produce curved motion paths rather than straight lines
- Solution: Measured motor drift via encoder calibration over known distance. Applied asymmetric speed compensation: left motor commanded at 94.3% when equal power is intended. Straight-line accuracy improved to within 2cm over 50cm distance

**4. Asynchronous I/O Coordination**
- Challenge: Sensor reading, behavioral state computation, and motor execution operate at different timescales; naive sequential implementation causes dropped frames and jitter
- Solution: Decoupled components via async queues. Sensor reader continuously polls BLE and enqueues readings. Behavior engine consumes from sensor queue, computes state, produces motor commands to motor queue. Motor controller drains motor queue asynchronously. No blocking operations. End-to-end latency measured at sub-10ms

### Outcomes and Metrics

- Phase 5a completion: Async architecture fully implemented and tested
- Phase 5b in progress: Threat prediction and latency compensation
- Phase 5c pending: Hierarchical FSM and flocking simulation
- Live hardware test (2026-05-17): 1,124 sensor readings, 1,075 behavior events, 26 state transitions
- Motor command fidelity: 100% (all queued commands execute as specified)
- Execution latency: Less than 10ms end-to-end
- All Phase 4 fixes retained (hysteresis filtering, recovery timeout, re-engagement logic)
- Project timeline: Final project due approximately 6 weeks after current date; currently ahead of schedule

### Key Learnings

Real hardware constraints fundamentally alter performance considerations. Polling sensor input at 100Hz creates real-time pressure; every frame matters. Async/await proved essential for handling multiple I/O sources without blocking behavioral computation. Naive state machines fail in realistic environments: hysteresis, timeouts, and recovery logic are not optional features but essential to robust operation.

Behavioral validation requires observing actual robot behavior with real stimuli, not simulation. Testing revealed state machine issues (deadlock, false transitions) that would not appear in theoretical analysis. Hardware constraints exposed performance issues (motor drift, latency) that affect behavioral fidelity.

For future robotics projects: instrument all decision points (log every state transition, motor command, sensor reading); test with real hardware early; expect state machine iteration based on observed behavior rather than theoretical correctness.
