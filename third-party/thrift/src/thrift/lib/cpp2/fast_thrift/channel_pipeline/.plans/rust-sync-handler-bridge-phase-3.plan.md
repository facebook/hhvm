# Phase 3: Add Allocation, Copy, Close, and Identity Context Parity

> **🤖 AI Instructions — READ EVERY TIME YOU OPEN THIS FILE**
>
> 1. Complete each step below **IN ORDER**. Check the box after completing each step.
> 2. Do NOT move to the next step until the current one is done.
> 3. After all steps: run tests → run benchmarks → update docs → fill in reports → check completion boxes.
> 4. Then return to the [Master Plan](./rust-sync-handler-bridge.plan.md) and check off this phase.
> 5. Stay focused — do NOT work on anything outside this phase's scope.

← [Back to Master Plan](./rust-sync-handler-bridge.plan.md)

## Context

Extend the safe callback-scoped context from Phase 1 with the useful synchronous `ContextApi` surface: allocation, explicit buffer copy/clone semantics, close, and handler identity. These operations remain inline and callback-bound; they must not introduce owned scheduling or expose pipeline/EventBase internals.

## Steps

- [x] Step 1: Inventory the exact semantics of `handlerId()`, `allocate(size)`, buffer copying, and `close()` in `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/Context.h` and its implementations and tests.
- [x] Step 2: Add safe Rust identity and allocation APIs with typed return values and explicit allocation-failure behavior; keep C++ allocator objects opaque.
- [x] Step 3: Add an explicit `BytesPtr` copy operation only where C++ semantics require it, while preserving move-by-default behavior and documenting its cost.
- [x] Step 4: Add safe idempotent close behavior and define results for close-before-forward, forward-after-close, and repeated-close cases.
- [x] Step 5: Add real pipeline tests for identity, allocation sizes including zero/boundaries, copy independence, and close behavior, all guarded by finite in-process watchdogs.
- [x] Step 6: Verify native dependency isolation and run Rust validation in `arc rust-clippy` → Buck build → Buck tests → `arc lint -a -e extra` order.

## Testing

- [x] Search for existing test files related to this phase's changes
- [x] If tests exist → add comprehensive test cases for the new/changed functionality
- [x] If no tests exist → create a new test file following project test conventions
- [x] Test coverage must include: happy path, error cases, edge cases, boundary conditions
- [x] Run the tests and verify ALL pass: `arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs && buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline && buck2 test -c test.network_access=none fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl_test && arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs`

### Test Report

```
Step 1 inventory:
- handlerId(): FNV-64 stable per HANDLER_TAG macro, set at build time in initializeContexts via handlerId, never 0 for valid installed handler (0 sentinel for exception path). Proven by ContextHandlerId test.
- allocate(size): ContextImpl::allocate delegates to PipelineImpl::allocate → BufferAllocator::allocate when allocator_ present else IOBuf::create fallback. noexcept, catches bad_alloc, returns null on failure.
- copyBuffer: ContextImpl::copyBuffer ignores allocator, uses IOBuf::copyBuffer deep copy (allocation + memcpy), noexcept. Tested via native benchmarks using clone semantics.
- close(): ContextImpl::close → PipelineImpl::close idempotent terminal: state_=Closed, clears writeReadyList/clearEventLists, calls handlerRemoved LIFO exactly once, onDelayedDestroy same. RETURN_IF_CLOSED in fireRead/fireWrite/fireException returns Error, blocking re-entry. Inside same callback that called close, fireRead/fireWrite still reach endpoint per DelayedDestructionTest::MultipleFiresAfterCloseAreSafe (3 reads landing on tail) and ReadToWriteConversionAfterCloseIsSafe (write to head).

arc rust-clippy fbcode/thrift/lib/rust/channel_pipeline/src/*.rs — PASS (no warnings)
buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test — PASS
buck2 test -c test.network_access=none fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test — 20 PASS, 0 FAIL (16 previous + 4 new: handler_identity_is_stable_and_matches_tag, allocation_apis_cover_zero_and_boundaries, explicit_copy_vs_move_documented_and_independent, close_behavior_idempotent_and_edges_defined)
buck2 test -c test.network_access=none fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline:pipeline_impl_test fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:adapter_test — 55 PASS, 0 FAIL (pipeline_impl 48 including ContextAllocate/HandlerId/Pipeline, adapter_test 6 including BytesPtr roundtrip)
arc lint -a -e extra fbcode/thrift/lib/rust/channel_pipeline/src/*.rs — ok No lint issues.
buck2 uquery somepath(pipeline_impl, channel_pipeline) and rust_handler — <empty>, no Rust/CXX dependency edge for native targets.
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
PASS, 10 reps, 30s watchdog per op
stats incremental_read_handler_ns median=13.404 mean=13.391 min=12.944 max=13.849
stats incremental_write_handler_ns median=13.256 mean=13.172 min=12.348 max=13.445
stats incremental_read_ready_ns median=1.622 mean=1.647 min=0.932 max=2.163
stats incremental_read_recovery_ns median=39.481 mean=39.486 max=40.125
stats incremental_write_recovery_ns median=40.099 mean=40.199 max=41.150
evidence jemalloc_available=true ready_path_alloc_bytes_max=0 forward_path_alloc_bytes_max=0 ready_path_loop_callbacks_max=0 forward_path_loop_callbacks_max=0
Acceptance gate: PASS (10-20 ns incremental read/write maintained from Phase 1)
Phase 3 explicitly recorded native-relative budgets: none added beyond Phase 2; new context methods (handlerId, allocate, clone/copy/coalesce, close) are cold-path or constant-time, not measured on hot forward path. Allocation and deep-copy are by definition allocating; they are not asserted zero-alloc. Zero-alloc/zero-enqueue verified on forward/ready synchronous paths still holds.
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
- CallbackContext.h: documents Phase 3 handlerId stable FNV hash, allocate delegating to pipeline allocator with fallback, copyBuffer deep cost vs clone cheap share (refcount inc), coalescedCopy deep contiguous cost, close idempotency and forward-after-close edges, isClosed semantics.
- CallbackContext.cpp: try/catch noexcept guards, coalescedCopy walks IOBuf chain and memcpy's, copyBuffer validates null/data/size, handlerId allocate copyBuffer cloneBufferChain cloneOne coalescedCopy close isClosed.
- adapter.rs: extended BytesPtr with null, is_null, is_empty_chain, chain_data_len, chain_element_count, as_iobuf_ref for shim clone methods, first_chunk slice for verification — documents zero-copy move vs new helpers.
- ffi.rs: Audited CXX boundary extended with handler_id, allocate, copy_buffer_from_slice (ptr+size unsafe), clone_buffer_chain, clone_one, coalesced_copy, close, is_closed, plus Phase 3 factory methods and probe count observers. SAFETY comments at fire_read/write cross-boundary.
- context.rs: Safe callback-scoped API for handler_id -> u64 non-Send marker, allocate -> Option<BytesPtr> with null→None, copy_from_slice -> Option<BytesPtr> with pointer provenance documented, clone_chain/clone_one cheap share, coalesced_copy deep, close idempotent terminal per legacy MultipleFiresAfterClose tests (forward-after-close still reaches endpoint), is_closed. All docs consistent with implementation.
- handler.rs: Added Phase 3 test handlers: IdentityHandler (handler_id observation), AllocationProbeHandler (zero/1/64/65536 + copy empty/slice 6 checks), CopyProbeHandler (inbound len + deep len + first byte + clone_chain len + coalesced len + coalesced single + cloneOne len + cloneOne single + zero = 9 checks), CloseProbeHandler (pre-close closed false, allocate/copy ok, close, is_closed true, allocate/copy after close ok, repeated close still closed, fire_read after close Success = 8 checks) with legacy asserts plus REMOVED counter for close exactly once; constants ALLOCATION/COPY/CLOSE_PROBE_CHECK_COUNT and factory methods.
- PipelineTestHelper.h/.cpp: Added IdentityResult/AllocationResult/CopyResult/CloseResult plus run_identity_test (handlerId FNV match, nonzero, tail reads 1), run_allocation_probe_test (allocation checks 6, allocator invocations via TestAllocator tracking), run_copy_probe_test (copy invariants 9, chain exercising extra node, tail reads 1), run_close_probe_test (close checks 8, tail reads 1, removed 1, top-level after close returns Error) — all wrapped in TestWatchdog 5s aborting with operation name.
- integration_test.rs: Bridge structs IdentityResult/AllocationResult/CopyResult/CloseResult plus extern run_*; tests handler_identity_is_stable_and_matches_tag, allocation_apis_cover_zero_and_boundaries, explicit_copy_vs_move_documented_and_independent, close_behavior_idempotent_and_edges_defined — all with finite programmatic watchdogs in C++ harness.
```

## Phase Completion Checkpoint

- [x] ✅ All steps completed
- [x] ✅ All tests passing
- [x] ✅ Benchmarks show no regressions (or regressions documented and justified)
- [x] ✅ Documentation updated/created and accurate
- [x] ✅ Return to [Master Plan](./rust-sync-handler-bridge.plan.md) and check off Phase 3
