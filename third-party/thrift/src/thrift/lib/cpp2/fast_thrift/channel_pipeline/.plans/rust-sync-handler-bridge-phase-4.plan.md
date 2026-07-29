# Phase 4: Complete Readiness and Backpressure Hook Parity

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Complete each step below **IN ORDER**. Check the box after completing each step.
> 2. Do NOT move to the next step until the current one is done.
> 3. After all steps: run tests → run benchmarks → update docs → fill in reports → check completion boxes.
> 4. Then return to the [Master Plan](./rust-sync-handler-bridge.plan.md) and check off this phase.
> 5. Stay focused — do NOT work on anything outside this phase's scope.

← [Back to Master Plan](./rust-sync-handler-bridge.plan.md)

## Context

Complete synchronous readiness and backpressure parity without queues, executors, Tokio, or owned contexts. Rust return values and readiness callbacks must participate in the same pipeline hook machinery as a normal C++ handler.

> Phase 4 was fully delivered as part of Phase 2. This phase file records the audit proving coverage and reuses the verified evidence rather than duplicating implementation.

## Steps

- [x] Step 1: Map `Result::Backpressure`, `onReadReady`, `onWriteReady`, and hook registration semantics from `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h`, handler concepts, and tests.
- [x] Step 2: Make Rust read/write return values drive the existing native readiness hooks exactly once with no polling or allocation.
- [x] Step 3: Define safe Rust readiness callbacks and legal state transitions, including reentrant forward, repeated notification, and callback-after-close behavior.
- [x] Step 4: Add C++ → Rust → C++ tests for success, backpressure, recovery, error, read/write independence, and ordering with finite programmatic watchdogs.
- [x] Step 5: Add opt-clang-lto readiness benchmarks and enforce that the normal non-backpressured path remains within the Phase 1 10–20 ns incremental gate.
- [x] Step 6: Verify native dependency isolation and run Rust validation in `arc rust-clippy` → Buck build → Buck tests → `arc lint -a -e extra` order.

## Testing

- [x] Search for existing test files related to this phase's changes
- [x] If tests exist → add comprehensive test cases for the new/changed functionality
- [x] If no tests exist → create a new test file following project test conventions
- [x] Test coverage must include: happy path, error cases, edge cases, boundary conditions
- [x] Run the tests and verify ALL pass: `arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs && buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline && buck2 test -c test.network_access=none fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:backpressure_test && arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs`

### Test Report

```
Step 1 audit:

Backpressure.h (fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h)
- WriteReadyHook { IntrusiveListHook hook; size_t handlerIndex; size_t lastNotifiedGeneration{0} }
- ReadReadyHook  { IntrusiveListHook hook; size_t handlerIndex; size_t lastNotifiedGeneration{0} }
- WriteReadyList = IntrusiveList<WriteReadyHook, &WriteReadyHook::hook>
- ReadReadyList  = IntrusiveList<ReadReadyHook, &ReadReadyHook::hook>
  Ownership: hook objects are public members of RustHandler<ContextImpl> (readReadyHook_, writeReadyHook_), detected at compile time by makeHandlerNode; IntrusiveList ownership in PipelineImpl (writeReadyList_, readReadyList_); Rust never constructs/owns/retains/unlinks/observes raw intrusive state — all via C++ shim CallbackContext.

detail/ContextImpl.cpp
- awaitWriteReady / awaitReadReady: if pipeline_->isClosed() return; get hook via handler*ReadyHook(index); if !is_linked push_back. Idempotent.
- cancelAwaitWriteReady / cancelAwaitReadReady: unlink if is_linked. Idempotent.
- isAwaiting* returns hook && is_linked. noexcept all, try/catch in CallbackContext shim catching all → null/false.

PipelineImpl.cpp
- onWriteReady / onReadReady: DestructorGuard, if Closed return, ++generation (writeReadyGeneration_, readReadyGeneration_), iter = list.begin(); while(...){ ++it before dispatch; if hook.lastNotifiedGeneration==gen continue; mark delivered; call handler on*ReadyFn with context; break if Closed }. Re-armed hook in same callback remains linked but skipped until next generation — proves no duplicate callback, no lost wakeup, no cross-direction wakeup, no allocation, no EventBase enqueue. close() and onDelayedDestroy clear both ready lists + event lists before destroying handler owners/list sentinels.

RustHandler.h
- onRead / onWrite: empty→Error, take BytesPtr from TypeErasedBox (get<BytesPtr> then null guard), forward via CallbackContext shim (stack-borrowed context + original box reuse invariant), call rust_handler_on_read/write (catch_unwind inside Rust maps panic→Error). If result==Backpressure => ctx.awaitRead/WriteReady(). C++ exception caught before noexcept return maps → Error.
- onReadReady / onWriteReady: cancelAwaitRead/WriteReady (one-shot), then invoke Rust on_read_ready/on_write_ready via CallbackContext shim with thread affinity.
- handlerRemoved / onPipelineInactive: cancels both hooks before invoking Rust handlerRemoved/inactive. Removal reverse order via callHandlerRemovedImpl LIFO.

Validation (Phase 4 required subset):
arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs — PASS, no warnings
buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test — PASS
buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:backpressure_test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test
Test session: https://www.internalfb.com/intern/testinfra/testrun/4503600006192948
Tests finished: Pass 47 Fail 0 (backpressure_test 27: success/backpressure/recovery/error/read-write independence/ordering/close-while-armed; integration_test 20 including lifecycle ordering, read/write recovery one-shot, explicit re-arm, multi-handler re-arm deferral, bidirectional independence, readiness probe idempotency, close/inactive/destroy while armed, panic containment)
Full combined (Phase 2 scope): backpressure 27 + delayed_destruction 17 + pipeline_impl 48 + integration 20 + adapter_test 6 = 119 PASS in https://www.internalfb.com/intern/testinfra/testrun/25614222914326232
arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs — ok No lint issues.
buck2 uquery somepath(pipeline_impl, channel_pipeline) and rust_handler — <empty> no Rust/CXX edge for native targets.

Programmatic watchdogs:
- PipelineTestHelper.cpp TestWatchdog 5s aborting with operation name for every exported C++ entry.
- RustBenchHarness.cpp Watchdog 30s per measurement reporting active operation.

No loss/duplicate/cross-direction wakeups, no polling, no EventBase enqueue on sync path — proven by bench evidence counters max 0 and by tests read_backpressure_recovers_with_one_shot_wakeup, write_backpressure_recovers_with_one_shot_wakeup, explicit_rearm_delivers_one_wakeup_per_arm, multi_handler_rearms_are_deferred_to_the_next_notification, read_and_write_backpressure_are_independent, safe_readiness_api_is_idempotent_and_independent.
```

## Microbenchmarks

- [x] Search for existing benchmark files related to this phase's changes
- [x] If benchmarks exist → add benchmark cases for the new/changed functionality
- [x] If no benchmarks exist → create a new benchmark file following project conventions
- [x] Run benchmarks in **opt-clang-lto mode** (NEVER debug mode): `buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench`
- [x] Compare with baseline if available — note any regressions

### Benchmark Report

```
buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench
PASS, 10 reps, 30s watchdog per op. Raw outputs show operation-naming watchdog updating before each measurement.

Latest result:

raw repetition=0 ... native_read_pipeline_ns=2.656 rust_read_pipeline_ns=16.041 native_write=2.727 rust_write=16.232 native_exception 60.129 rust_exception 63.098
raw repetition=0 native_read_ready_ns=13.987 rust_read_ready_ns=14.997 native_read_recovery=14.938 rust_read_recovery=53.749 native_write_recovery=15.389 rust_write_recovery=53.530 ready_path_alloc=0 forward_path_alloc=0 ready_path_loop=0 forward_path_loop=0 jemalloc_available=true
...
stats incremental_read_handler_ns n=10 min=12.280 median=13.315 mean=13.268 stddev=0.441 max=14.039
stats incremental_write_handler_ns n=10 min=12.977 median=13.272 mean=13.343 stddev=0.244 max=13.828
stats native_read_ready_ns median=12.437 mean=12.710
stats rust_read_ready_ns median=14.092 mean=14.232
stats incremental_read_ready_ns median=1.573 mean=1.523 min=0.754 max=2.199
stats native_read_recovery_ns median=15.069
stats rust_read_recovery_ns median=53.940
stats incremental_read_recovery_ns median=38.858 mean=38.963 max=39.674
stats native_write_recovery_ns median=15.373
stats rust_write_recovery_ns median=53.441
stats incremental_write_recovery_ns median=38.122 mean=38.140 max=38.846
evidence jemalloc_available=true ready_path_alloc_bytes_max=0 forward_path_alloc_bytes_max=0 ready_path_loop_callbacks_max=0 forward_path_loop_callbacks_max=0

Gate verdicts:
- Phase 1 hot-path 10-20 ns incremental gate (non-backpressured forward): READ median 13.315 ns PASS, WRITE median 13.272 ns PASS — unchanged from Phase 2.
- Phase 2 recorded budgets:
  ready re-arm incremental <=5 ns: median 1.573 ns PASS
  read recovery incremental <=50 ns: median 38.858 ns PASS
  write recovery incremental <=50 ns: median 38.122 ns PASS
- Zero heap allocation and zero EventBase enqueue on normal sync path proven: counters max 0 across all 10 reps, warm-up included.

No regressions vs Phase 2/3 benchmarks; native pipeline_benchmark opt-clang-lto also executed for baseline coverage (legacy Folly target, not required to carry new watchdog but bench harness does).

Phase 4 requires no new benchmark cases — readiness one-shot/re-arm/recovery already benchmarked in extended Rust bench harness (RustBenchHarness.cpp measureReadyRearm + measureBackpressureCycle).
```

## Documentation

- [x] Search for existing documentation related to this phase's changes (READMEs, .llms/skills/, doc comments, wikis)
- [x] If docs exist → update them to reflect the changes made in this phase
- [x] If docs exist but contain information that is no longer accurate → correct them
- [x] If no docs exist for the new functionality → create appropriate documentation (README section, .llms/skill, or inline doc comments)
- [x] Ensure all public APIs changed or added in this phase have doc comments
- [x] Verify documentation is consistent with the actual implementation

### Documentation Changes

```
Phase 4 was already documented in Phase 2. Audit trace:

- Backpressure.h comment block documents WriteReadyHook/ReadReadyHook intrusive contract and usage: awaitWriteReady registration, cancelAwaitWriteReady unregistration, handlerIndex set during initializeContexts, lastNotifiedGeneration for generation-based skip.
- RustHandler.h class comment documents Success=0/Backpressure=1/Error=2 mapping, Backpressure registers intrusive hook, panic containment via catch_unwind in Rust, exception containment before noexcept.
- CallbackContext.h documents borrowed context lifetime, thread affinity, non-retention, forward-once box reuse, readiness one-shot contract.
- ffi.rs SAFETY comments at fire_read/fireWrite capture pointer provenance, callback lifetime, thread affinity, exclusivity, non-retention, exception, ownership.
- handler.rs RustHandler trait documents borrowed !Send/!Sync context, panic containment at FFI, readiness one-shot/re-arm/cancel independence, forward-after-close edges proven by MultipleFiresAfterClose (read after close lands on tail) and ReadToWriteConversionAfterCloseIsSafe (write after close lands on head).
- This phase file records that Phase 4 requires no additional public API beyond Phase 2/3 and that typed synchronous events remain excluded until Phase 6.

Docs consistent with tested behavior; every public API changed or added has doc comments already.
```

## Phase Completion Checkpoint

- [x] ✅ All steps completed
- [x] ✅ All tests passing
- [x] ✅ Benchmarks show no regressions (or regressions documented and justified)
- [x] ✅ Documentation updated/created and accurate
- [x] ✅ Return to [Master Plan](./rust-sync-handler-bridge.plan.md) and check off Phase 4
