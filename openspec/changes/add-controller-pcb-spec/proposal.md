# Proposal

## Why

An initial KiCad PCB project (`pcb/echelon_ex5s_controller.*`) was added for the Exercise-Bike controller carrier board, but no hardware specification exists yet and the schematic's initial GPIO assignments (`GPIO 4/5/6/7/18/19/21`, no `STBY`) have drifted from the finalized firmware pinout in `include/pins.h` and `docs/pinouts.md`. Adding a formal PCB specification establishes the electrical, connector, power-rail, and GPIO contract between the carrier board and the firmware before schematic capture and PCB layout are finalized.

## What Changes

- Introduce a new `controller-pcb` capability specification defining:
  - Power distribution (`+12V` DC input via `J1` Pins 8–9, `Buck_12V_to_5V` step-down to `+5V`, `ESP32-C6-DevKitC-1` onboard regulation to `+3V3` for logic and potentiometer reference).
  - Power decoupling and ADC filtering (`220 uF` bulk + `100 nF` decoupling on motor `VM`, `100 nF` RC/bypass filtering on `POT_WIPER`).
  - Bike and peripheral connectors (`J1` 9-pin main bike harness for motor/potentiometer/cadence/`+12V` power, `J2` 3-pin rotary encoder harness).
  - Canonical `ESP32-C6-DevKitC-1` GPIO mapping aligned with `include/pins.h` and `docs/pinouts.md` (`GPIO 2` pot, `GPIO 4` cadence, `GPIO 5`/`6` encoder, `GPIO 7` TB6612 `STBY`, `GPIO 20`/`19`/`21` TB6612 `AIN1`/`AIN2`/`PWMA`), avoiding reserved pins (`GPIO 8` RGB LED, `GPIO 12/13` USB Serial/JTAG).
  - TB6612 H-bridge wiring (`VM`, `VCC`, `STBY`, `AIN1`, `AIN2`, `PWMA`, `MOT_RED`, `MOT_BLK`).
- Track KiCad schematic and PCB alignment tasks (updating `U2` pin nets and adding `STBY` in `pcb/echelon_ex5s_controller.kicad_sch`, updating `pcb/echelon_ex5s_controller.kicad_pcb` netlist/layout, and running KiCad ERC/DRC).

## Capabilities

### New Capabilities

- `controller-pcb`: Electrical architecture, power rails, filtering, 2.54 mm connector interfaces, TB6612 H-bridge integration, and ESP32-C6-DevKitC-1 GPIO assignments for the Echelon EX-5S controller carrier PCB.

### Modified Capabilities

<!-- None: firmware behavioral specs intentionally omit pin assignments. -->

## Impact

- **Hardware / KiCad**: `pcb/echelon_ex5s_controller.kicad_sch` and `pcb/echelon_ex5s_controller.kicad_pcb` (pinout alignment, `STBY` wiring, netlist sync, ERC/DRC).
- **Documentation**: `docs/pinouts.md` (already aligned with `include/pins.h`; verify consistency with final KiCad connector pinouts).
- **Specs**: New capability spec under `openspec/changes/add-controller-pcb-spec/specs/controller-pcb/spec.md`.
