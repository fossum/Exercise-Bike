# Design

## Context

See `proposal.md` for motivation. The initial KiCad project in `pcb/echelon_ex5s_controller.*` was created with earlier GPIO assignments (`GPIO 4/5/6` for `ENC_A/B/SW`, `GPIO 7` for `CADENCE`, `GPIO 18/19/21` for `AIN1/AIN2/PWMA`, and no `STBY` net) and unrouted/unupdated netlist state in `echelon_ex5s_controller.kicad_pcb`. Meanwhile, `include/pins.h` and `docs/pinouts.md` on `main` finalized the GPIO mapping including `PIN_STBY = GPIO 7`.

## Goals / Non-Goals

**Goals:**
- Establish `controller-pcb` as the single hardware contract matching `include/pins.h` and `docs/pinouts.md`.
- Align `pcb/echelon_ex5s_controller.kicad_sch` and `pcb/echelon_ex5s_controller.kicad_pcb` with `controller-pcb` (adding `STBY` on `GPIO 7`, moving `AIN1` to `GPIO 20`, moving `ENC_A` to `GPIO 6`, moving `CADENCE` to `GPIO 4`, and consolidating bike connectors into `J1` 9-pin and `J2` 3-pin 2.54 mm headers matching the bike harnesses 1:1).
- Extend `Project_Footprints.pretty` with 9-pin (`PinHeader_1x09_P2.54mm_Vertical`) and 3-pin (`PinHeader_1x03_P2.54mm_Vertical`) 2.54 mm vertical footprints while keeping custom module footprints intact.

**Non-Goals:**
- Modifying firmware pin assignments in `include/pins.h` or behavioral firmware specs.
- Adding new sensors or changing the 12V -> 5V -> 3.3V power topology.

## Decisions

### 1. Reconcile KiCad schematic to `include/pins.h` instead of changing firmware
- **Decision**: Update `U2` (`ESP32-C6-DevKitC-1`) and `U3` (`Adafruit_TB6612`) in `pcb/echelon_ex5s_controller.kicad_sch` to match `include/pins.h` and `docs/pinouts.md`, adding the `STBY` global label on `GPIO 7` and removing `ENC_SW` from `GPIO 6` (or keeping `J3` `ENC_SW` as an unpopulated/optional pin).
- **Rationale**: `include/pins.h` and `docs/pinouts.md` reflect the tested breadboard/prototype wiring and `PIN_STBY` support. Changing the KiCad project before PCB fabrication avoids a mismatch between firmware and manufactured boards.
- **Alternatives considered**: Keeping the initial schematic pinout and changing `include/pins.h` — rejected because `STBY` is required by the firmware motor driver and `GPIO 4/5/6/7/19/20/21` is already verified on hardware.

### 2. Match bike harnesses 1:1 with `J1` (9-pin) and `J2` (3-pin) 2.54 mm headers
- **Decision**: Replace the initial four-connector split (`J1` 2P 12V, `J2` 8P motor/pot, `J3` 6P encoder, `J4` 2P cadence) with two 2.54 mm (0.1 in) pitch vertical headers: `J1` (`PinHeader_1x09_P2.54mm_Vertical`, 9-pin bike harness carrying motor, pot GND/wiper/+3V3, cadence, and `+12V`/`GND`) and `J2` (`PinHeader_1x03_P2.54mm_Vertical`, 3-pin encoder carrying `ENC_A`, `ENC_B`, `GND`, omitting `ENC_SW` since `ENC_PIN_SW = -1`).
- **Rationale**: Matches the physical 2.54 mm bike connectors documented in `docs/pinouts.md` 1:1 with no adapter harness needed and no unused `NC` pins.

## Risks / Trade-offs

- **KiCad symbol/footprint pin-number alignment**: Updating `Custom:ESP32-C6-DevKitC-1` and `Custom:Adafruit_TB6612` in `echelon_ex5s_controller.kicad_sch` requires verifying physical header pin numbers on the DevKitC-1 and TB6612 breakout footprints. -> Mitigation: Run KiCad ERC and inspect footprint pad-to-net mapping in `pcbnew` after updating the schematic and netlist.
