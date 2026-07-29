# Phase 1: Replace ContextHandle Bridge with First Usable Synchronous Rust Handler

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Complete each step below **IN ORDER**. Check the box after completing each step.
> 2. Do NOT move to the next step until the current one is done.
> 3. After all steps: run tests → run benchmarks → update docs → fill in reports → check completion boxes.
> 4. Then return to the [Master Plan](./rust-sync-handler-bridge.plan.md) and check off this phase.
> 5. Stay focused — do NOT work on anything outside this phase's scope.

← [Back to Master Plan](./rust-sync-handler-bridge.plan.md)

## Context

Deliver the first genuinely usable Rust-authored synchronous handler, not a scaffold or smoke API. Delete or replace the owned scheduling implementation in `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/ContextHandleBridge.h`, `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/ContextHandleBridge.cpp`, and `/data/users/rroeser/fbsource/fbcode/thrift/lib/rust/channel_pipeline/src/context_handle.rs` before building the new path. The callback context is borrowed for one callback, explicitly `!Send`/`!Sync`, and cannot escape. `fire_read` and `fire_write` execute inline and return the actual downstream `Result`. Async/Tokio/Tower and owned `ContextHandle` scheduling remain out of scope.

## Steps

- [x] Step 1: Remove the ContextHandle/EventBase ownership, cross-thread Drop, worker-release test helpers, and related BUCK dependencies; retain only reusable CXX build wiring and the concrete `BytesPtr` ownership adapter.
- [x] Step 2: Define an audited C++ callback-context shim that borrows the live C++ context only for the Rust callback; document pointer provenance, callback lifetime, thread affinity, exclusivity, exception, and non-retention invariants at every unsafe FFI declaration.
- [x] Step 3: Expose a safe public Rust `RustHandler` API whose borrowed context is structurally `!Send`/`!Sync`, cannot be stored beyond the callback, and provides inline `fire_read(BytesPtr) -> Result` and `fire_write(BytesPtr) -> Result` without raw pointers or public `unsafe`.
- [x] Step 4: Implement one production `BytesPtr` adapter that moves `std::unique_ptr<folly::IOBuf>`/`iobuf::IOBuf` without copy, preserves pointer identity, rejects null/wrong/empty values, and never exposes `TypeErasedBox` across FFI.
- [x] Step 5: Make a Rust-authored duplex handler replace a normal C++ handler and support read, write, exception, handler-added/removed, pipeline-active/inactive, and read/write-ready callbacks with real `Result::Success`, `Result::Backpressure`, and `Result::Error` propagation.
- [x] Step 6: Add normal C++ → Rust → normal C++ integration tests and an opt-clang-lto benchmark with finite in-process watchdogs that fail with the timed-out operation; prohibit fake echo/arithmetic/smoke methods and require a 10–20 ns per-handler incremental latency acceptance gate against the equivalent C++ handler.
- [x] Step 7: Verify native C++ pipeline targets have no Rust/CXX dependency edge, size/link change, or runtime branch; run Rust validation in order: `arc rust-clippy`, Buck build, Buck tests, then `arc lint -a -e extra`.

## Testing

- [x] Search for existing test files related to this phase's changes
- [x] If tests exist → add comprehensive test cases for the new/changed functionality
- [x] If no tests exist → create a new test file following project test conventions (not applicable; existing integration and adapter tests were extended)
- [x] Test coverage must include: happy path, error cases, edge cases, boundary conditions
- [x] Run the tests and verify ALL pass: `arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs && buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test && buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:adapter_test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test && arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs`

### Test Report

<!-- AI: Paste test results here after running tests -->

```
arc rust-clippy thrift/lib/rust/channel_pipeline/src/*.rs
PASS

buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test
PASS (default ASAN/UBSAN dev configuration)

buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:adapter_test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test
Test session: https://www.internalfb.com/intern/testinfra/testrun/28428972668495976
Tests finished: Pass 12. Fail 0. Timeout 0. Fatal 0. Skip 0. Omit 0. Infra Failure 0. Build failure 0

arc lint -a -e extra thrift/lib/rust/channel_pipeline/src/*.rs
ok No lint issues.

buck2 uquery "somepath(fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl, fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline)"
<empty: no dependency path>

buck2 uquery "somepath(fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl, fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:rust_handler)"
<empty: no dependency path>
```

## Microbenchmarks

- [x] Search for existing benchmark files related to this phase's changes
- [x] If benchmarks exist → add benchmark cases for the new/changed functionality
- [x] If no benchmarks exist → create a new benchmark file following project conventions (not applicable; existing benchmark was corrected)
- [x] Run benchmarks in **opt-clang-lto mode** (NEVER debug mode): `buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench`
- [x] Compare with baseline if available — note any regressions

### Benchmark Report

<!-- AI: Paste benchmark results here -->
<!-- Flag any regressions with percentage and explanation -->

```
buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench

Independent task 11 rerun after adding direct paired-delta reporting:
stats adapter_round_trip_ns n=10 min=29.237 median=30.546 mean=32.389 stddev=3.619 max=40.293
stats native_read_pipeline_ns n=10 min=2.637 median=2.732 mean=3.158 stddev=0.706 max=4.291
stats rust_read_pipeline_ns n=10 min=15.871 median=16.059 mean=16.234 stddev=0.416 max=17.306
stats native_write_pipeline_ns n=10 min=2.720 median=2.726 mean=2.789 stddev=0.099 max=3.012
stats rust_write_pipeline_ns n=10 min=16.095 median=16.300 mean=17.710 stddev=2.347 max=22.497
stats incremental_read_handler_ns n=10 min=11.735 median=13.280 mean=13.077 stddev=0.789 max=14.669
stats incremental_write_handler_ns n=10 min=13.302 median=13.579 mean=14.921 stddev=2.297 max=19.772
stats native_exception_pipeline_ns n=10 min=50.153 median=50.722 mean=53.281 stddev=4.834 max=65.295
stats rust_exception_pipeline_ns n=10 min=52.797 median=54.042 mean=58.801 stddev=8.752 max=76.047

Median incremental handler latency is now computed per repetition before aggregation:
read  = median(rust_read - native_read) = 13.280 ns
write = median(rust_write - native_write) = 13.579 ns
Acceptance gate: PASS (required 10-20 ns).
The Rust pipeline total medians are 16.059 ns read and 16.300 ns write; they are not the incremental overhead. Setup and IOBuf allocation are outside timed regions. Every benchmark operation is protected by a terminating in-process watchdog that reports the active operation and deadline.
```

## Documentation

- [x] Search for existing documentation related to this phase's changes (READMEs, .llms/skills/, doc comments, wikis)
- [x] If docs exist → update them to reflect the changes made in this phase
- [x] If docs exist but contain information that is no longer accurate → correct them
- [x] If no docs exist for the new functionality → create appropriate documentation (inline module, public API, and audited FFI documentation added)
- [x] Ensure all public APIs changed or added in this phase have doc comments
- [x] Verify documentation is consistent with the actual implementation

### Documentation Changes

<!-- AI: List what documentation was updated or created -->

```
- /data/users/rroeser/fbsource/fbcode/thrift/lib/rust/channel_pipeline/src/context.rs: documents callback lifetime, structural !Send/!Sync, and inline Result-returning forwarding.
- /data/users/rroeser/fbsource/fbcode/thrift/lib/rust/channel_pipeline/src/handler.rs: documents the safe synchronous RustHandler contract and lifecycle callbacks.
- /data/users/rroeser/fbsource/fbcode/thrift/lib/rust/channel_pipeline/src/ffi.rs: records provenance, lifetime, thread-affinity, exclusivity, exception, ownership, and non-retention invariants at unsafe declarations.
- /data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/CallbackContext.h: documents the stack-borrowed C++ shim and original-box reuse invariant.
```

## Phase Completion Checkpoint

- [x] ✅ All steps completed
- [x] ✅ All tests passing
- [x] ✅ Benchmarks show no regressions (13.280 ns read / 13.579 ns write paired incremental median passes gate)
- [x] ✅ Documentation updated/created and accurate
- [x] ✅ Return to [Master Plan](./rust-sync-handler-bridge.plan.md) and check off Phase 1
