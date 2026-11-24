# Design

## Context

See `proposal.md` for motivation. Currently, `Motor::begin()` configures pin modes and delegates to `Motor::stop()`, and every command method (`forward`, `reverse`, `stop`, `setSpeedPercent`) unconditionally invokes `applyOutput()` even when `direction_` and `speedPercent_` already match the requested state.

## Goals / Non-Goals

**Goals:**
- Avoid redundant `digitalWrite` / `analogWrite` hardware calls in `forward()`, `reverse()`, `stop()`, and `setSpeedPercent()` (and by delegation `setModeChar()`) when effective `(direction_, speedPercent_)` is unchanged.
- Guarantee `begin()` always drives all configured pins (`AIN1`, `AIN2`, optional `PWM`, optional `STBY`) to the safe `Stopped` state regardless of member defaults.
- Keep `include/motor.h` public API unchanged.

**Non-Goals:**
- Comparing quantized integer PWM counts instead of clamped `speedPercent_`.
- Debouncing or rate-limiting genuine state transitions in `src/main.cpp`.
- Changing hardware writer injection semantics (`setHardwareWriters()`).

## Decisions

### 1. Direct initialization write in `begin()` instead of calling `stop()`
- **Decision**: Have `Motor::begin()` explicitly reset `direction_ = MotorDirection::Stopped; speedPercent_ = 0.0f;` and call `applyOutput()` directly rather than delegating to `stop()`.
- **Rationale**: Constructor initialization already sets `direction_ = Stopped` and `speedPercent_ = 0.0f`. Once `stop()` short-circuits on unchanged state, calling `stop()` from `begin()` would skip writing initial pin levels. Calling `applyOutput()` directly in `begin()` is minimal, explicit, and avoids extra state fields.
- **Alternatives considered**:
  - Adding an `initialized_` boolean member to `Motor`: adds extra state to track and reset with no benefit over `begin()` calling `applyOutput()` directly.
  - Adding a `stop(bool force)` overload/parameter: unnecessarily complicates the internal or public API for a single caller (`begin()`).

### 2. Clamp before comparing `(direction_, speedPercent_)`
- **Decision**: Compute `float clamped = clampPercent(speedPercent);` before checking whether `(direction_, speedPercent_)` changed:
  - `forward(speedPercent)`: if `clamped <= 0.0f` delegate to `stop()`; else if `direction_ == MotorDirection::Forward && speedPercent_ == clamped` return early; otherwise update and call `applyOutput()`.
  - `reverse(speedPercent)`: same with `MotorDirection::Reverse`.
  - `stop()`: if `direction_ == MotorDirection::Stopped && speedPercent_ == 0.0f` return early; otherwise set `Stopped`/`0.0f` and call `applyOutput()`.
  - `setSpeedPercent(speedPercent)`: if `clamped <= 0.0f || direction_ == MotorDirection::Stopped` delegate to `stop()`; else if `speedPercent_ == clamped` return early; otherwise update `speedPercent_` and call `applyOutput()`.
- **Rationale**: Comparing post-clamp values ensures calls like `forward(150.0f)` while already at `100.0%` or `setSpeedPercent(60.0f)` while `Stopped` cleanly short-circuit without redundant writes, and `setModeChar()` inherits the guard automatically.
- **Alternatives considered**:
  - Comparing raw unclamped `speedPercent`: would needlessly rewrite outputs when repeated out-of-range inputs clamp to the same `100.0f` or `0.0f`.
  - Comparing quantized `getPwmValue()`: could suppress updates to `speedPercent_` reported by `getSpeedPercent()` / `getSpeedNormalized()` when two distinct percentages round to the same PWM count.

## Risks / Trade-offs

- **External pin tampering**: If external code writes directly to the motor GPIO pins outside `Motor`, subsequent identical commands will not re-assert the pin states. -> Mitigation: `Motor` owns its pins; calling `begin()` unconditionally re-applies the safe stopped outputs if hardware re-sync is ever needed.
- **Exact float equality on `speedPercent_`**: Floating-point values computed differently (e.g., `80.0f` vs `80.0001f`) will not be considered equal and will conservatively write hardware outputs. -> Mitigation: Acceptable and safe—false negatives only trigger a benign hardware write, never a missed update.
