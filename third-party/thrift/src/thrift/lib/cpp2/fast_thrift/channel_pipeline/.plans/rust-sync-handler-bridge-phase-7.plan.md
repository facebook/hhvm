# Phase 7: Finalize Safety, Dependency Isolation, Documentation, and Performance

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Complete each step below **IN ORDER**. Check the box after completing each step.
> 2. Do NOT move to the next step until the current one is done.
> 3. After all steps: run tests → run benchmarks → update docs → fill in reports → check completion boxes.
> 4. Then return to the [Master Plan](./rust-sync-handler-bridge.plan.md) and check off this phase.
> 5. Stay focused — do NOT work on anything outside this phase's scope.

← [Back to Master Plan](./rust-sync-handler-bridge.plan.md)

## Context

Close the synchronous roadmap with an auditable safety argument, full validation, native zero-cost proof, and accurate user documentation. This phase must remove any residual ContextHandle/async/Tokio/Tower claims or dead bridge code and must not broaden scope.

## Steps

- [x] Step 1: Audit every `unsafe extern "C++"`, raw/internal pointer conversion, lifetime cast, and panic/exception boundary; require local SAFETY documentation and prove the public Rust API contains no unsafe callable surface.
- [x] Step 2: Remove dead ContextHandle bridge files, factories, test counters, scheduling dependencies, and obsolete documentation; confirm no async/Tokio/Tower symbols or owned callback-context APIs remain.
- [x] Step 3: Add/verify finite programmatic watchdogs for every bridge integration test and benchmark so hangs fail in-process with the exact operation and deadline.
- [x] Step 4: Prove native zero cost using Buck dependency queries/link inspection and the native pipeline benchmark: no Rust/CXX dependency edge, no added runtime branch, and no native regression beyond measurement noise.
- [x] Step 5: Run the complete Rust validation order and all C++/Rust tests, including debug coverage for `TypeErasedBox` constraints and applicable sanitizer modes.
- [x] Step 6: Run repeated opt-clang-lto comparisons and require the Rust handler incremental read/write latency to remain within 10–20 ns of the equivalent C++ handler; treat failure as a blocker, not a documented exception.
- [x] Step 7: Publish final rustdoc and local design/usage documentation covering synchronous scope, borrowed `!Send` context, handler construction, adapters, lifecycle, errors, safety invariants, performance, and explicit async exclusion.

## Testing

- [ ] Search for existing test files related to this phase's changes
- [ ] If tests exist → add comprehensive test cases for the new/changed functionality
- [ ] If no tests exist → create a new test file following project test conventions
- [ ] Test coverage must include: happy path, error cases, edge cases, boundary conditions
- [ ] Run the tests and verify ALL pass: `arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs && buck2 build fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test && buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline: fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:adapter_test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test && arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs`

### Test Report

<!-- AI: Paste test results here after running tests -->

```
<test output goes here>
```

## Microbenchmarks

- [ ] Search for existing benchmark files related to this phase's changes
- [ ] If benchmarks exist → add benchmark cases for the new/changed functionality
- [ ] If no benchmarks exist → create a new benchmark file following project conventions
- [ ] Run benchmarks in **opt-clang-lto mode** (NEVER debug mode): `buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench && buck2 run @fbcode//mode/opt-clang-lto fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_benchmark`
- [ ] Compare with baseline if available — note any regressions

### Benchmark Report

<!-- AI: Paste benchmark results here -->
<!-- Flag any regressions with percentage and explanation -->

```
<benchmark output goes here>
```

## Documentation

- [x] Search for existing documentation related to this phase's changes (READMEs, .llms/skills/, doc comments, wikis)
- [x] If docs exist → update them to reflect the changes made in this phase
- [x] If docs exist but contain information that is no longer accurate → correct them
- [x] If no docs exist for the new functionality → create appropriate documentation (README section, .llms/skill, or inline doc comments)
- [x] Ensure all public APIs changed or added in this phase have doc comments
- [x] Verify documentation is consistent with the actual implementation

### Documentation Changes

<!-- AI: List what documentation was updated or created -->

```
- thrift/lib/rust/channel_pipeline/src/lib.rs: documented synchronous scope, quick start, async exclusion, measured performance, safety, and native link isolation.
- thrift/lib/rust/channel_pipeline/src/context.rs: documented borrowed callback lifetime, !Send/!Sync invariants, lifecycle, forwarding, and readiness APIs.
- thrift/lib/rust/channel_pipeline/src/handler.rs: documented result ABI, handler callbacks, panic containment, backpressure, and typed-event exclusion.
- thrift/lib/rust/channel_pipeline/src/adapter.rs: documented adapter ownership, stable type IDs, zero-copy BytesPtr conversion, and failure behavior.
- thrift/lib/rust/channel_pipeline/src/ffi.rs: documented the opaque handler boundary and FFI safety responsibilities.
- thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustHandler.h: documented C++ construction, registration, synchronous behavior, result mapping, and panic containment.
- thrift/lib/cpp2/fast_thrift/channel_pipeline/.llms/skills/channel_pipeline.md: added the final Rust bridge design and usage guide.
```

## Phase Completion Checkpoint

- [ ] ✅ All steps completed
- [ ] ✅ All tests passing
- [ ] ✅ Benchmarks show no regressions (or regressions documented and justified)
- [ ] ✅ Documentation updated/created and accurate
- [ ] ✅ Return to [Master Plan](./rust-sync-handler-bridge.plan.md) and check off Phase 7
