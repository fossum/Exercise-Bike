# Tasks

## 1. Schematic Pinout & Net Alignment (`pcb/echelon_ex5s_controller.kicad_sch`)

- [x] 1.1 Update the `U2` (`ESP32-C6-DevKitC-1`) symbol, connector symbols (`J1` 9-pin bike harness, `J2` 3-pin encoder), and global labels in `pcb/echelon_ex5s_controller.kicad_sch` to match `include/pins.h` (`GPIO 2`=`POT_WIPER`, `GPIO 4`=`CADENCE`, `GPIO 5`=`ENC_B`, `GPIO 6`=`ENC_A`, `GPIO 7`=`STBY`, `GPIO 19`=`AIN2`, `GPIO 20`=`AIN1`, `GPIO 21`=`PWMA`), leaving `GPIO 8`, `GPIO 12`, and `GPIO 13` unconnected; verify by inspecting the schematic nets and `include/pins.h`.
- [x] 1.2 Add the `STBY` pin/net connection on `U3` (`Adafruit_TB6612`) in `pcb/echelon_ex5s_controller.kicad_sch` so it is driven by `GPIO 7` (`STBY`), and run KiCad Electrical Rules Check (ERC) with zero unresolved errors.

## 2. PCB Layout & Verification (`pcb/echelon_ex5s_controller.kicad_pcb`)

- [x] 2.1 Update `pcb/echelon_ex5s_controller.kicad_pcb` from the updated schematic netlist, verify all footprints (`U1`–`U3`, `J1`–`J2`, `C1`–`C3`) and power/signal nets (`+12V`, `+5V`, `+3V3`, `GND`, `AIN1`, `AIN2`, `PWMA`, `STBY`, `POT_WIPER`, `ENC_A`, `ENC_B`, `CADENCE`, `MOT_RED`, `MOT_BLK`) are routed, and run KiCad Design Rules Check (DRC) with zero errors.
- [x] 2.2 Cross-check `docs/pinouts.md` connector pin tables (`J1`–`J2`, 9-pin bike harness & 3-pin encoder) against the finalized KiCad schematic and PCB netlist to confirm 100% alignment.
