# Master Plan: Rust Synchronous Handler Bridge

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Work through phases **ONE AT A TIME** in order. Do NOT skip ahead.
> 2. Open the phase file → complete ALL checkboxes → run tests & benchmarks → return here.
> 3. Check off the phase below ONLY after all its checkboxes are done.
> 4. After ALL phases are checked, complete the **Final Validation** section at the bottom.
> 5. Keep your focus NARROW — only work on the current phase's scope.
> 6. If you lose context, re-read this file and the current phase file.

## Overview

Replace the current ContextHandle-coupled Rust bridge with a synchronous-only bridge that lets a real Rust-authored handler replace a normal C++ handler in a normal pipeline. Phase 1 is the first usable product slice: borrowed callback-scoped `!Send` context, inline `fire_read`/`fire_write` returning the actual downstream `Result`, `BytesPtr` zero-copy adaptation, lifecycle/read/write/exception/active/inactive/ready behavior, audited FFI invariants, real C++ → Rust → C++ tests, and a 10–20 ns opt-clang-lto acceptance gate. Later phases fill the remaining rich synchronous C++ context and handler parity. Async/Tokio/Tower and owned `ContextHandle` scheduling are explicitly out of scope. `unsafe` is allowed only inside audited FFI internals; the public Rust API must remain safe. Existing native C++ targets must not acquire Rust/CXX dependencies or hot-path cost.

## Phases

- [x] **Phase 1**: Replace ContextHandle bridge with first usable synchronous Rust handler → [📋 Phase 1 Details](./rust-sync-handler-bridge-phase-1.plan.md)
- [x] **Phase 2**: Lifecycle + Readiness/Backpressure → [📋 Phase 2 Details](./rust-sync-handler-bridge-phase-2.plan.md)
- [x] **Phase 3**: Add allocation, copy, close, and identity context parity → [📋 Phase 3 Details](./rust-sync-handler-bridge-phase-3.plan.md)
- [x] **Phase 4**: Complete readiness and backpressure hook parity → [📋 Phase 4 Details](./rust-sync-handler-bridge-phase-4.plan.md) (covered via Phase 2 — audited, no new code needed)
- [x] **Phase 5**: Harden lifecycle, removal, closed-state, error, and panic behavior → [📋 Phase 5 Details](./rust-sync-handler-bridge-phase-5.plan.md)
- [x] **Phase 6**: Add typed synchronous events and adapter extensibility where justified → [📋 Phase 6 Details](./rust-sync-handler-bridge-phase-6.plan.md)
- [ ] **Phase 7**: Finalize safety, dependency isolation, documentation, and performance → [📋 Phase 7 Details](./rust-sync-handler-bridge-phase-7.plan.md)

## Final Validation (do this AFTER all phases are checked off)

- [ ] Run **ALL** application tests: `buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline: fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:adapter_test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test`
- [ ] Run **ALL** benchmarks in opt-clang-lto mode: `buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench && buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_benchmark`
- [ ] Paste full test results in the **Final Test Report** section below
- [ ] Paste benchmark results in the **Final Benchmark Report** section below
- [ ] Review benchmark results for regressions — document and justify any regressions
- [ ] Review all documentation changes across phases for consistency and accuracy

### Final Test Report

<!-- AI: Run all tests and paste the full output here -->

```
<test output goes here>
```

### Final Benchmark Report

<!-- AI: Run all benchmarks in opt-clang-lto mode and paste results here -->
<!-- Flag any regressions compared to pre-plan baselines -->

```
<benchmark output goes here>
```

### Regression Analysis

<!-- AI: If any benchmarks regressed, document:
  1. Which benchmark regressed
  2. By how much (percentage)
  3. Why it's acceptable OR what follow-up is needed
-->
```
