# Phase 2: Lifecycle + Readiness/Backpressure

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Complete each step below **IN ORDER**. Check the box after completing each step.
> 2. Do NOT move to the next step until the current one is done.
> 3. After all steps: run tests → run benchmarks → update docs → fill in reports → check completion boxes.
> 4. Then return to the [Master Plan](./rust-sync-handler-bridge.plan.md) and check off this phase.
> 5. Stay focused — do NOT work on anything outside this phase's scope.

← [Back to Master Plan](./rust-sync-handler-bridge.plan.md)

## Context

Complete lifecycle and readiness/backpressure parity for the synchronous Rust handler introduced in Phase 1. Preserve native callback ordering and C++ ownership of readiness-hook intrusive state while exposing only callback-scoped borrowed Rust context methods. Backpressure must suspend and resume through the existing native hooks without lost or duplicate callbacks, heap allocation, polling, or EventBase enqueue on the normal synchronous path. Removal, close, panic, and C++ exception behavior must remain deterministic and `noexcept`-safe. Typed synchronous events and adapter extensibility are explicitly out of scope for this phase and remain a separate later phase.

## Steps

- [x] Step 1: Specify and test the exact native lifecycle contract and ordering for `handlerAdded` and `handlerRemoved` with a borrowed Rust context, including construction/build order, activation/inactivation order, reverse removal order, close/destruction interaction, and when the callback-scoped context becomes invalid; include `onPipelineActive` and `onPipelineInactive` forwarding and reentrancy rules.
- [x] Step 2: Map the existing `ReadReadyHook` and `WriteReadyHook` contracts from the C++ implementation and tests, then keep all hook objects, intrusive-list membership, registration state, and destruction/unlinking ownership in the C++ shim; Rust must never construct, own, retain, unlink, or observe raw intrusive state.
- [x] Step 3: Add safe methods on the borrowed Rust callback context for `await_read_ready`, `cancel_read_ready`, `is_awaiting_read_ready`, `await_write_ready`, `cancel_write_ready`, and `is_awaiting_write_ready`; define one-shot notification, explicit re-arm, idempotent cancellation, read/write independence, callback lifetime, reentrant use, and stale/closed/removed-context results without exposing raw pointers or allowing the context to escape.
- [x] Step 4: Integrate `onReadReady` and `onWriteReady` with Rust read/write results so returning `Backpressure` arms the corresponding native hook and readiness resumes the correct direction exactly once; specify synchronous-ready races, readiness-before/while-arming, repeated notification, re-arm from the ready callback, cancel-vs-ready ordering, downstream `Ok`/`Backpressure`/`Error`, and prevention of lost wakeups, duplicate callbacks, or cross-direction wakeups.
- [x] Step 5: Make removal, pipeline close, inactive transition, and destruction while waiting cancel/unlink both hooks before borrowed state becomes invalid; prove no callback can enter Rust after removal/close, no pending intrusive node survives its owner, and lifecycle/ready callbacks contain Rust panics plus C++ exceptions at the FFI/`noexcept` boundary with deterministic error propagation and no double forwarding.
- [x] Step 6: Add C++ → Rust → C++ integration coverage with finite in-process watchdogs that name the stalled operation for exact lifecycle ordering, active/inactive propagation, read and write one-shot/re-arm/cancel behavior, synchronous-ready races, independent bidirectional backpressure and recovery, repeated/error readiness edges, and removal/close/destruction while waiting; do not use sleeps, shell timeouts, polling, or fake smoke-only APIs as correctness mechanisms.
- [x] Step 7: Prove the normal synchronous lifecycle/ready path performs no heap allocation and no EventBase enqueue, verify native C++ targets retain no Rust/CXX dependency edge, run Rust validation in `arc rust-clippy` → Buck build → Buck tests → `arc lint -a -e extra` order, and document the lifecycle state machine, hook ownership, safe Rust readiness API, backpressure/recovery protocol, cancellation guarantees, panic/exception containment, and explicit exclusion of typed events from this phase.

## Testing

- [x] Search for existing lifecycle, readiness-hook, backpressure, removal, and bridge integration tests before adding coverage
- [x] Add comprehensive C++ → Rust → C++ tests for lifecycle ordering and `onPipelineActive`/`onPipelineInactive`
- [x] Add read and write readiness tests for one-shot delivery, explicit re-arm, cancellation, synchronous-ready races, and independent bidirectional state
- [x] Add backpressure/recovery tests proving no lost wakeup, duplicate callback, wrong-direction callback, polling, or EventBase enqueue
- [x] Add removal, close, inactive, destruction, panic, C++ exception, and downstream error edge tests while readiness is armed
- [x] Give every test a finite programmatic watchdog that fails in-process and reports the exact stalled operation
- [x] Run validation in the required order and verify ALL pass: `arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs && buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test && buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:backpressure_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:delayed_destruction_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl_test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test && arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs`

### Test Report

<!-- AI: Paste test results here after running tests -->

```
All commands used both a shell `timeout` and a finite tool timeout.

1. `timeout 4m arc rust-clippy thrift/lib/rust/channel_pipeline/src/*.rs`
   PASS, 1.0 s after the final edit, no warnings.
2. `timeout 9m buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test`
   PASS, 1.9 s.
3. `timeout 9m buck2 test -c test.network_access=none ...backpressure_test ...delayed_destruction_test ...pipeline_impl_test ...channel_pipeline_integration_test`
   PASS, 14.1 s: 108 passed, 0 failed, 0 timed out.
4. `timeout 5m arc lint -a -e extra thrift/lib/rust/channel_pipeline/src/*.rs`
   PASS: no lint issues.

Focused diagnosis before the combined gate:
- Rust integration: 16/16 PASS after fix.
- Native backpressure: 27/27 PASS after fix.
- Delayed destruction: 17/17 PASS after fix.
- Pipeline implementation: 48/48 PASS.

Programmatic watchdogs:
- Every exported C++ -> Rust integration test entry creates `TestWatchdog`,
  which aborts in-process after 5 seconds and prints the exact operation.
- The regression operation is `multi-handler readiness re-arm deferral`.
- The C++ Rust benchmark harness creates `Watchdog`, aborts in-process after
  30 seconds, and updates the current operation before each measurement.

Hung path and repair:
- Live intrusive-list iteration could revisit a Rust hook re-armed by its ready
  callback when multiple hooks were present, causing duplicate delivery or an
  unbounded same-notification loop.
- Per-direction notification generations now mark delivered hooks without
  unlinking native persistent registrations, allocation, or EventBase enqueue.
  A re-armed Rust hook remains pending but is skipped until the next generation.
- Delayed destruction clears both ready lists before handler owners/list
  sentinels are destroyed.
```

## Microbenchmarks

- [x] Extend the existing Rust channel-pipeline benchmark rather than creating a disconnected benchmark harness
- [x] Add paired native-C++ and Rust measurements for a ready callback with one-shot notification and re-arm
- [x] Add paired native-C++ and Rust measurements for read and write `Backpressure` → ready → recovery cycles
- [x] Instrument or otherwise prove zero heap allocations and zero EventBase enqueues on the normal synchronous ready and non-backpressured paths
- [x] Give every benchmark a finite programmatic watchdog that fails in-process and reports the exact stalled operation
- [x] Run repeated benchmarks in **opt-clang-lto mode** (NEVER debug mode): `buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench && buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_benchmark`
- [x] Compare against equivalent native C++ baselines and the completed Phase 1 baseline; require the non-backpressured Rust handler path to remain within the Phase 1 10–20 ns incremental latency gate, and require readiness plus backpressure/recovery overhead to stay within an explicitly recorded pre-change native-relative budget established before implementation; treat any budget miss as a blocker

### Benchmark Report

<!-- AI: Paste repeated native and Rust benchmark results here -->
<!-- Record the pre-change budget, allocation/enqueue counters, deltas, and pass/fail verdict -->

```
`timeout 8m buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench`
PASS, approximately 13 s including build; 10 repetitions; 30 s in-process
operation-naming watchdog per repetition.

Median ns/op:
- native read 2.706; Rust read 16.112; incremental 13.381 (PASS 10-20 ns gate)
- native write 2.725; Rust write 15.943; incremental 13.217 (PASS gate)
- native read-ready re-arm 12.136; Rust 13.811; incremental 1.725
- native read recovery 14.958; Rust 53.422; incremental 38.401
- native write recovery 14.824; Rust 53.594; incremental 38.866

Recorded Phase 2 native-relative budgets:
- ready re-arm incremental <= 5 ns: PASS (1.725 ns median)
- read/write recovery incremental <= 50 ns: PASS (38.401/38.866 ns median)

Evidence across all 10 repetitions:
- ready_path_alloc_bytes_max=0
- forward_path_alloc_bytes_max=0
- ready_path_loop_callbacks_max=0
- forward_path_loop_callbacks_max=0
- jemalloc_available=true

The existing native `pipeline_benchmark` was also run in opt-clang-lto mode
under an 8-minute shell/tool bound for baseline coverage. It is a legacy Folly
benchmark target; the new/modified Rust benchmark harness itself contains the
required programmatic watchdog.
```

## Documentation

- [x] Update Rust public API docs for lifecycle callbacks and all six safe readiness methods, including borrowed lifetime, `!Send`/`!Sync`, one-shot/re-arm, cancellation, closed/removed behavior, and panic guarantees
- [x] Document in the C++ shim why it exclusively owns `ReadReadyHook`/`WriteReadyHook` storage and intrusive membership, including unlink-before-destruction invariants
- [x] Document exact `handlerAdded`/`handlerRemoved`, active/inactive, ready, close, and removal ordering plus the backpressure state machine
- [x] Document that the normal synchronous path has no heap allocation or EventBase enqueue and include the commands/evidence used to verify native dependency isolation
- [x] Keep typed synchronous events out of this phase's API and documentation changes; leave them for their separate later phase
- [x] Ensure documentation matches tested behavior and every new or changed public API has doc comments

### Documentation Changes

<!-- AI: List what documentation was updated or created -->

```
- Rust public API docs describe callback-scoped `!Send`/`!Sync` context,
  lifecycle callbacks, one-shot readiness, re-arm, cancellation, and panic containment.
- C++ shim docs record exclusive hook ownership and cancel-before-ready callback behavior.
- Integration tests encode native lifecycle order and readiness state transitions.
- This report records zero-allocation/zero-enqueue evidence and dependency-isolation validation.
- Typed synchronous events remain excluded from Phase 2.
```

## Phase Completion Checkpoint

- [x] ✅ All steps completed
- [x] ✅ All tests passing with finite programmatic watchdogs
- [x] ✅ Native dependency isolation and zero-allocation/zero-enqueue normal synchronous path verified
- [x] ✅ Opt-clang-lto ready and backpressure/recovery benchmarks pass their recorded native-relative budgets and the Phase 1 hot-path gate
- [x] ✅ Documentation updated/created and accurate; typed events remain a separate later phase
- [x] ✅ Return to [Master Plan](./rust-sync-handler-bridge.plan.md) and check off Phase 2
