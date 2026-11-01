# Timer and interrupt boundary — WASM32

The WASM32 port does not receive a physical timer interrupt. The browser owns the event loop and provides a monotonic clock through the host import `rio_host_monotonic_millis()`.

## Runtime behavior

- `timer_millis()` and `timer_seconds()` derive elapsed time from the host monotonic clock.
- One-shot timer state is evaluated when the timer API is queried; it is not delivered by a guest IRQ.
- `rio_tick()` is called by the browser animation/event loop to refresh time-dependent UI state.
- Interrupt statistics remain zero because this target has no GIC or guest hardware interrupt delivery.
- `irq` reports that hardware IRQ/GIC control is unavailable instead of claiming a live timer IRQ.

## What is not implemented

The following are native AArch64 features and are not emulated by this port: exception vectors, GIC routing, IRQ masking, physical timer PPI, privileged exception entry/return, and interrupt-driven process preemption.

## Verify

Run `bash wasm/run-ui.sh` and observe clock updates in the UI. Use `version`, `stats`, and `irq status` from the browser terminal to inspect WASM runtime information. The expected IRQ report explicitly says hardware IRQs are unavailable.
