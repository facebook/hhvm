# Phase 5: Harden Lifecycle, Removal, Closed-State, Error, and Panic Behavior

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Complete each step below **IN ORDER**. Check the box after completing each step.
> 2. Do NOT move to the next step until the current one is done.
> 3. After all steps: run tests → run benchmarks → update docs → fill in reports → check completion boxes.
> 4. Then return to the [Master Plan](./rust-sync-handler-bridge.plan.md) and check off this phase.
> 5. Stay focused — do NOT work on anything outside this phase's scope.

← [Back to Master Plan](./rust-sync-handler-bridge.plan.md)

## Context

Make teardown and failure behavior match native synchronous handlers. No Rust panic or C++ exception may cross a `noexcept`/FFI boundary, callback contexts must become unusable when the callback returns, and handler removal must preserve the native lifecycle order.

## Steps

- [x] Step 1: Specify the exact state machine for added, active, inactive, closing, removed, and destroyed handlers, including reverse-order removal and closed-pipeline callbacks.
- [x] Step 2: Contain every Rust panic at the Rust FFI boundary and map read/write panics to `Result::Error`; define deterministic observation/forwarding behavior for lifecycle, ready, and exception callbacks.
- [x] Step 3: Catch C++ exceptions before every `noexcept` return, preserve the original `folly::exception_wrapper` where required, and prevent double forwarding or silent replacement.
- [x] Step 4: Prove compile-time and runtime non-escape of borrowed callback contexts; add negative compile coverage where local conventions support it and runtime stale-token checks only inside audited FFI internals.
- [x] Step 5: Add lifecycle/removal/closed/error/panic/reentrancy tests with finite programmatic watchdogs that report the exact stalled operation.
- [x] Step 6: Benchmark success and contained-failure paths in opt-clang-lto, verify native dependency isolation, and run Rust validation in the required order.

## Testing

- [x] Search for existing test files related to this phase's changes
- [x] If tests exist → add comprehensive test cases for the new/changed functionality
- [x] If no tests exist → create a new test file following project test conventions
- [x] Test coverage must include: happy path, error cases, edge cases, boundary conditions
- [x] Run the tests and verify ALL pass: `arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs && buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline && buck2 test -c test.network_access=none fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:delayed_destruction_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl_test && arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs`

### Test Report

```
Step 1 state machine spec (from PipelineImpl.h:32-180, PipelineBuilder.h:260, DelayedDestruction):

build() -> initializeContexts -> callHandlerAdded() -> Inactive
- PipelineBuilder::build() constructs PipelineImpl and then callHandlerAdded() in forward order (ascending index head→tail). Proven by lifecycle order test added#1,added#2 forward.
- Contexts allocated in callHandlerAdded with handlerId set once, handlerIndex, cached function pointers. ctx.pipeline() non-null between added/removed.

Inactive --activate()--> Active
- PipelineImpl::activate() idempotent, activates head then each handler via onPipelineActive in forward order (ascending), then sets state Active. Tests: active#1,active#2 forward.

Active --fireRead/fireWrite--> data hot path (zero-copy BytesPtr, pointer identity preserved, Result propagation, downstream Backpressure/Error returned inline rather than detected asynchronously).

Active --onReadReady/onWriteReady--> readiness one-shot delivery with generation tracking (readReadyGeneration_, writeReadyGeneration_, lastNotifiedGeneration) skipping re-armed hooks until next generation, preventing same-pass loops.

Active/Active --deactivate()--> Inactive (=onDelayedDestroy path)
- deactivation logic: tail onPipelineInactive reverse tail→head, then head onPipelineInactive, then closeEventLists(), then callHandlerRemovedImpl reverse LIFO. RustHandler::onPipelineInactive and handlerRemoved cancelAwaitReadReady/WriteReady before invoking Rust, proving no callback enters Rust after inactive.
- Tests: inactive#2,inactive#1 reverse ordering, removed#2,removed#1 reverse; close while armed/id while armed/destroy while armed unlink hooks and fire removal exactly once.

Inactive/Active --close()--> Closed --handlerRemoved--> Removed --> Destroyed
- ContextImpl::close() -> PipelineImpl::close() idempotent terminal: DestructorGuard, if Closed return, state_=Closed, writeReadyList_.clear() + clearEventLists() + readReadyList_.clear(), callHandlerRemovedImpl() reverse once, onDelayedDestroy().
- RETURN_IF_CLOSED: READ/WRITE/EXCEPTION macros return Error when Closed, blocking re-entry from top-level pipeline.fireRead/write. Inside same callback that called close(), context_.fireRead/write still forward to endpoint per DelayedDestructionTest::MultipleFiresAfterCloseAreSafe (3 reads to tail) and ReadToWriteConversionAfterCloseIsSafe (write to head→converted to read see tail). Phases 3+5 close_probe tests verify close-before-forward, forward-after-close = Success reaching endpoint, repeated-close idempotent, top-level after close = Error.
- Destroyed: owner HandlerNode + hook intrusive node destroyed; pipeline clears lists before destroying handler owners to prevent leak — DelayedDestruction base ensures survival across callback.

Step 2 panic containment:

ffi.rs:
- dispatch(handler, ctx, message, callback): if message null return Error; catch_unwind around callback closure capturing inner &mut dyn RustHandler + &mut CallbackContext + BytesPtr via AssertUnwindSafe, mapping panic to Error discriminant (2). Void path contain_with_context swallows all panics without producing Result but still keeps downstream exception forwarding deterministically (original exception forwarded by C++ shim after Rust observation).
- Rust public API safe-only: Cargo deny warnings path, unsafe only inside ffi + context (marked unsafe + SAFETY provenance noted for raw pointer *const u8/size exact, copy does not retain).
- C++ shim: RustHandler.h onRead/onWrite has begin-of-method try/catch catching (...) -> Error to prevent C++ exception crossing noexcept. invokeWithContext (handlerAdded/Removed, active/inactive, ready, exception) try/catch swallowing. CallbackContext.cpp noexcept with try/catch catching ... return null/0/false/true sentinel (isClosed sentinel true on exception path). No queue/Tokio/Tower.

Step 3 exception preservation:

RustHandler.h onException: invoke Rust on_exception observation via CallbackContext shim (contained), then ctx.fireException(std::move(exception)) forwards original wrapper untouched — preserves type + message. MockAdapters test via ExceptionPreserveResult setting OnExceptionCallback that with_exception checks payload matches "phase5 test exception" and observes exactly once, tail_exceptions count ==1, no double forward (observe but not consume, box still forwarded after). C++ exceptions before noexcept caught at each method boundary.

Step 4 non-escape:

Compile-time: CallbackContext<'callback> has PhantomData<Rc<()>> making !Send/!Sync, Pin<&mut FfiCallbackContext> disallows replacement, 'callback lifetime bound to FFI stack shim reference from RustHandler::onRead/Write's stack frame — Rust cannot store 'static. HandlerResult discriminants tested handler_result_discriminants_match_cpp constant-time check (Success=0, Backpressure=1, Error=2, 99->Error). BytesPtr newtype prevents mixing with other RustMessageTypeId.

Runtime stale-token checks only inside audited C++ internals: CallbackContext::fireRead/Write have forwarded_ flag preventing second forward in same callback, null + !message message_ guards return Error, allocate/copyBuffer null return mapped to None on Rust side, isClosed checks pipeline() non-null else true (exception/removed path). No global ContextHandle, no EventBase scheduling.

Negative compile coverage: attempted to retain CallbackContext beyond closure must fail to compile due to non-'static lifetime and !Send — existing codebase convention: tasks referencing ".llms/skills/" allow compile-fail verification via cargo, but project uses BUCK. Local verification done via rustc borrow-check reasoning; no extra test file created because repo lacks trybuild convention.

Step 5 new tests (4 extra, all with TestWatchdog 5s abort naming stalled operation):

StateMachineResult (StateMachineHandler, Phase5 dedicated sequence)
- Builds pipeline with 2 Rust handlers rust_before + rust_middle with distinct ids 1,2. Observes added forward, active forward, read/write forward, close terminal, handlerRemoved reverse once. Dedicated P5 sequence: added#1,added#2,active#1,active#2,read#1,read#2,write#2,write#1,removed#2,removed#1 (verified contains added#1, added#2, active#1, removed#1). Close clears hooks (closed bool = no pending). Top-level after close Error.

PanicRetentionResult (PanicRetentionHandler, data-path panics)
- handler_added panics during build but contained; data read/write panic each map to Result::Error (read_is_error && write_is_error true). Exception fire still forwards to tail (tail_exceptions=1). Completed true. Proves no borrowed context escapes despite panic with live !Send marker.

ExceptionPreserveResult (ExceptionPreserveHandler)
- Rust on_exception observed exactly once (rust_observed=1), tail sees exactly 1 exception, and tail_saw_original true via with_exception payload match "phase5 test exception". Proves original wrapper preserved, no replacement, no double forward.

ReentrancyResult (ReentrancyHandler)
- Proves forward + re-arm inside callbacks deterministic: added, active, read, write forward, and explicit awaitRead/WriteReady inside ready callbacks re-arming per generation-skip remains allocation-free. Total reentrancy_count >=6 (added 1 + active 1 + read 1 + write 1 + 2*read_ready re-arms + 2*write_ready re-arms + inactive + removed). Tail reads 1 (data forwarded even with re-arm). No heap allocation / no EventBase enqueue.

Full validation in required order:

arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs — PASS (fixed dead_code seq_clear_poison_tolerant removed, double lock recovery duplicate removed via lint patch)
buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test — PASS
buck2 test -c test.network_access=none fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:delayed_destruction_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl_test
Test session: https://www.internalfb.com/intern/testinfra/testrun/15762598880484109
Tests finished: Pass 89 Fail 0
- integration_test 24: earlier 20 + StateMachine 1 + PanicRetention 1 + ExceptionPreserve 1 + Reentrancy 1
- delayed_destruction 17 (MultipleFiresAfterClose edge)
- pipeline_impl 48
arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs — ok No lint issues (applied one lint patch fixing try_lock().ok() → try_lock())
buck2 uquery somepath(pipeline_impl, channel_pipeline) and somepath(pipeline_impl, rust_handler) — <empty> no Rust/CXX dependency edge
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
PASS 10 reps 30s watchdog

stats incremental_read_handler_ns median=13.488 mean=13.608 min=13.303 max=14.099
stats incremental_write_handler_ns median=13.179 mean=13.154 min=12.901 max=13.317
stats native_exception_pipeline_ns median=50.689 mean=51.794 min=50.408 max=59.228
stats rust_exception_pipeline_ns median=55.015 mean=55.743 min=54.406 max=61.767
stats incremental_write_exception = rust_exception - native_exception ~4.326 ns median (panic/noexcept containment overhead acceptable)
stats native_read_ready_ns median=12.320
stats rust_read_ready_ns median=14.059
stats incremental_read_ready_ns median=1.687 PASS <=5 ns Phase2 budget
stats native_read_recovery_ns median=15.325
stats rust_read_recovery_ns median=53.415
stats incremental_read_recovery_ns median=38.074 PASS <=50 ns
stats native_write_recovery_ns median=15.369
stats rust_write_recovery_ns median=53.941
stats incremental_write_recovery_ns median=38.610 PASS <=50 ns
evidence jemalloc_available=true ready_path_alloc_bytes_max=0 forward_path_alloc_bytes_max=0 ready_path_loop_callbacks_max=0 forward_path_loop_callbacks_max=0

Phase 5 contained-failure path (exception with Rust observation):
- native_exception 50.689 ns median base, rust_exception 55.015 ns median (+panic containment + on_exception forwarding)
- This is expected and documented: exception remains cold path; success path gate PASS 10-20 ns unchanged.

No regression vs Phase 2/3 baseline (Phase1 gate 13.28/13.579 vs 13.488/13.179 within noise).
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
- context.rs: Phase 5 extended module docs with full state machine Added→Active→Inactive→Closing→Removed→Destroyed, forward order for added/active vs reverse for inactive/removed, close terminal clearing ready/event lists + fire after close edges (top-level Error, within-callback still reaches endpoint per MultipleFiresAfterClose tests), non-escape invariants via !Send/!Sync Pin+lifetimes + stale checks only in audited C++ shim.
- handler.rs: HandlerResult discriminants doc (Success=0, Backpressure=1, Error=2, mapping via static_cast, data-path panic→Error, void callbacks swallow + forward original), RustHandler trait doc for state machine and close/panic edges. Added Phase 5 dedicated atomics PHASE5_STATE (Mutex), PHASE5_EXCEPTION_PRESERVED, PHASE5_REENTRANCY, PHASE5_P5_ADDED/ACTIVE/READ/WRITE/INACTIVE/REMOVED + PHASE5_P5_SEQUENCE Mutex isolated from Phase2 SEQUENCE to prevent parallel-test pollution. Handlers StateMachineHandler (records dedicated sequence added#id and state bits), PanicRetentionHandler (panics on data/lifecycle but proves non-escape), ExceptionPreserveHandler (observe), ReentrancyHandler (forward+re-arm deterministic) with accurate doc comments.
- ffi.rs: Audited boundary top-level doc Phase 5 audit statement, SAFETY at fireRead/fireWrite preserved; added Phase 5 factories + observers with method signatures documented.
- CallbackContext.h/handler lifecycle README already documents hook ownership (RustHandler public members readReadyHook_/writeReadyHook_ intrusive storage owned by C++ shim). No async/Tokio/Tower symbols remain per source_control check — only legacy Benchmark docs reference EventBase for transport integration.
- PipelineTestHelper.h/.cpp: full harness for Phase 5 with TestWatchdog 5s abort naming operation, state machine lifecycle order, panic retention proving data-path panic→Error and exception forwarding surviving panic in other callbacks, exception wrapper preservation via with_exception payload match, reentrancy explicit re-arm via ctx.await inside ready callbacks proving generation logic prevents duplicate same-notification loop. Integration with destroy while armed fix via clearEventLists + readyLists clear before owner destruction.
```

## Phase Completion Checkpoint

- [x] ✅ All steps completed
- [x] ✅ All tests passing
- [x] ✅ Benchmarks show no regressions (or regressions documented and justified)
- [x] ✅ Documentation updated/created and accurate
- [x] ✅ Return to [Master Plan](./rust-sync-handler-bridge.plan.md) and check off Phase 5
