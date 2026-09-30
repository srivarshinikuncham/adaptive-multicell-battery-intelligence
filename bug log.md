# Bug Log

**Project:** Adaptive Multi-Cell Battery Intelligence Engine

## Purpose

This document records the technical issues encountered during project development, debugging, testing, RTL implementation, and system integration.

| No. | Issue Encountered | Resolution / Action |
|---|---|---|
| 1 | Online ESP32 simulator build-server availability issue | Continued development using the Velxio ESP32 simulation environment. |
| 2 | Initial cloud telemetry integration returned unsuccessful HTTP responses | Changed the final cloud telemetry implementation to Blynk IoT. |
| 3 | Blynk dashboard rejected certain string datastream widgets | Used compatible numerical health and protection status codes for dashboard visualization. |
| 4 | Battery simulation values required calibration | Adjusted the simulation input mapping and verified battery states using different slider positions. |
| 5 | Cloud connectivity needed recovery handling | Incorporated Wi-Fi reconnection and pending telemetry synchronization logic. |
| 6 | RTL-level protection logic was required for verification | Developed a SystemVerilog RTL protection controller implementing the NORMAL, DEGRADED, FAILSAFE, and SHUTDOWN states. |
| 7 | RTL state transitions required functional verification | Developed a SystemVerilog testbench to apply minor fault, critical fault, pack failure, and recovery conditions. |
| 8 | RTL simulation environment was required for waveform verification | Simulated the RTL design using Icarus Verilog through EDA Playground and verified the resulting waveforms using EPWave. |
| 9 | ModelSim was not available on the development system | Used Icarus Verilog with EDA Playground as the alternative RTL simulation environment. ModelSim-specific verification was not performed. |

## Testing Performed

The following functional and verification scenarios were performed:

- Healthy battery condition
- Minor imbalance condition
- Critical imbalance condition
- Pack failure condition
- Recovery to normal operation
- Live Blynk dashboard value updates
- Wi-Fi connectivity verification
- Blynk cloud connectivity verification
- RTL reset to NORMAL state
- NORMAL to DEGRADED transition
- DEGRADED to FAILSAFE transition
- FAILSAFE to SHUTDOWN transition
- SHUTDOWN to NORMAL recovery
- Protection activation during FAILSAFE and SHUTDOWN
- RTL waveform verification using EPWave

## Remaining Limitations

- Battery inputs are simulated rather than measured from physical lithium-ion cells.
- Real hardware deployment requires appropriate electrical protection circuitry, including a proper relay/MOSFET driver and protection components.
- The RTL implementation represents the battery protection control and FSM logic rather than the complete ESP32, sensor, LCD, and Blynk system.
- ModelSim was not available on the development system; RTL simulation was performed using Icarus Verilog through EDA Playground.

## Conclusion

The documented issues were addressed through debugging, design modifications, alternative simulation environments, and functional verification. The final project integrates the embedded BMS prototype, fault detection and protection logic, Blynk cloud telemetry, dashboard visualization, and RTL-based FSM verification.
