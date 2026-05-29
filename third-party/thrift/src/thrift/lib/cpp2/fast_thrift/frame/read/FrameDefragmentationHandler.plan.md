# FrameDefragmentationHandler Implementation Plan

## Overview

Implement an inbound pipeline handler that reassembles fragmented RSocket frames per the RSocket protocol specification. The handler receives `ParsedFrame` instances, accumulates fragments for streams with `hasFollows() == true`, and emits complete `ParsedFrame` instances when all fragments arrive.

## Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Map Type | `folly::F14NodeMap<uint32_t, FragmentState>` | Reference stability during IOBufQueue appends |
| Payload Accumulation | `folly::IOBufQueue` with `cacheChainLength()` | O(1) append, O(1) length queries |
| Output Type | `ParsedFrame` | Downstream handlers expect uniform type |
| Metadata Handling | Combined with data in payload queue | Simpler; first fragment contains metadata |

## File Structure

```
thrift/lib/cpp2/fast_thrift/framing/reading/
├── FragmentState.h                      # Fragment accumulation state
├── FrameDefragmentationHandler.h        # Handler implementation
├── test/
│   ├── FragmentStateTest.cpp            # Unit tests for FragmentState
│   ├── FrameDefragmentationHandlerTest.cpp  # Handler tests
│   └── FrameDefragmentationHandlerBench.cpp # Microbenchmarks
└── BUCK                                 # Build targets (update)
```

---

## Phase 1: FragmentState

**Goal**: Create a struct to hold pending fragment data for a single stream.

### File: `FragmentState.h`

```cpp
#pragma once

#include <thrift/lib/cpp2/fast_thrift/framing/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/framing/reading/ParsedFrame.h>

#include <folly/io/IOBufQueue.h>

namespace apache::thrift::fast_thrift::framing {

struct FragmentState {
  // Original frame type from first fragment (REQUEST_*, not PAYLOAD)
  FrameType originalType{FrameType::RESERVED};

  // Original flags from first fragment
  uint16_t originalFlags{0};

  // Stream ID (for debugging/validation)
  uint32_t streamId{0};

  // Accumulated payload (metadata + data from all fragments)
  folly::IOBufQueue payload{folly::IOBufQueue::cacheChainLength()};

  // Total accumulated bytes (for metrics/limits)
  size_t accumulatedBytes{0};
};

} // namespace apache::thrift::fast_thrift::framing
```

### Acceptance Criteria

- [ ] Struct compiles and is default-constructible
- [ ] IOBufQueue uses `cacheChainLength()` for O(1) size queries
- [ ] All fields have sensible defaults

---

## Phase 2: FrameDefragmentationHandler

**Goal**: Implement the handler satisfying `InboundHandler` concept.

### File: `FrameDefragmentationHandler.h`

```cpp
#pragma once

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/framing/reading/FragmentState.h>
#include <thrift/lib/cpp2/fast_thrift/framing/reading/ParsedFrame.h>

#include <folly/container/F14Map.h>

namespace apache::thrift::fast_thrift::framing {

/**
 * FrameDefragmentationHandler - Reassembles fragmented RSocket frames.
 *
 * This handler implements the InboundHandler concept. It intercepts frames
 * with hasFollows() == true, accumulates fragments per stream ID in an
 * F14NodeMap, and emits complete ParsedFrame instances when the final
 * fragment (hasFollows() == false) arrives.
 *
 * Non-fragmented frames pass through unchanged (fast path).
 *
 * Input:  ParsedFrame (possibly fragmented)
 * Output: ParsedFrame (complete, defragmented)
 *
 * Thread Safety: Not thread-safe. Assumes single-threaded EventBase access.
 */
class FrameDefragmentationHandler {
 public:
  FrameDefragmentationHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    pending_.clear();
  }

  // === InboundHandler ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx,
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& frame = msg.get<ParsedFrame>();
    auto streamId = frame.streamId();

    auto it = pending_.find(streamId);
    bool hasPending = (it != pending_.end());

    // Fast path: complete frame with no pending fragments
    if (!hasPending && !frame.hasFollows()) {
      return ctx.fireRead(std::move(msg));
    }

    // First fragment for this stream
    if (!hasPending) {
      initPendingFragment(streamId, frame);
      return channel_pipeline::Result::Success;
    }

    // Continuation fragment
    appendToPending(it->second, frame);

    // Final fragment - assemble and emit
    if (!frame.hasFollows()) {
      auto assembled = assembleFrame(std::move(it->second));
      pending_.erase(it);

      channel_pipeline::TypeErasedBox box;
      box.emplace<ParsedFrame>(std::move(assembled));
      return ctx.fireRead(std::move(box));
    }

    return channel_pipeline::Result::Success;
  }

  template <typename Context>
  channel_pipeline::Result onException(
      Context& ctx,
      folly::exception_wrapper&& e) noexcept {
    return ctx.fireException(std::move(e));
  }

  // === Accessors (for testing) ===

  size_t pendingCount() const noexcept {
    return pending_.size();
  }

  bool hasPendingFragment(uint32_t streamId) const noexcept {
    return pending_.contains(streamId);
  }

 private:
  /**
   * Initialize pending state for the first fragment of a stream.
   * Stores original frame type/flags and extracts initial payload.
   */
  void initPendingFragment(uint32_t streamId, const ParsedFrame& frame);

  /**
   * Append a continuation fragment's payload to pending state.
   */
  void appendToPending(FragmentState& state, const ParsedFrame& frame);

  /**
   * Assemble a complete ParsedFrame from accumulated fragments.
   * Consumes the FragmentState.
   */
  ParsedFrame assembleFrame(FragmentState&& state);

  // Pending fragments keyed by stream ID
  // F14NodeMap for reference stability during IOBufQueue operations
  folly::F14NodeMap<uint32_t, FragmentState> pending_;
};

} // namespace apache::thrift::fast_thrift::framing
```

### Private Method Implementations

#### `initPendingFragment`

```cpp
void FrameDefragmentationHandler::initPendingFragment(
    uint32_t streamId,
    const ParsedFrame& frame) {
  FragmentState state;
  state.originalType = frame.type();
  state.originalFlags = frame.metadata.flags_;
  state.streamId = streamId;

  // Extract payload from first fragment
  auto payloadSize = frame.payloadSize();
  if (payloadSize > 0) {
    auto cursor = frame.payloadCursor();
    state.payload.append(cursor.cloneAtMost(payloadSize));
    state.accumulatedBytes = payloadSize;
  }

  pending_.emplace(streamId, std::move(state));
}
```

#### `appendToPending`

```cpp
void FrameDefragmentationHandler::appendToPending(
    FragmentState& state,
    const ParsedFrame& frame) {
  // Continuation fragments only have data (no metadata per RSocket spec)
  auto dataSize = frame.dataSize();
  if (dataSize > 0) {
    auto cursor = frame.dataCursor();
    state.payload.append(cursor.cloneAtMost(dataSize));
    state.accumulatedBytes += dataSize;
  }
}
```

#### `assembleFrame`

```cpp
ParsedFrame FrameDefragmentationHandler::assembleFrame(FragmentState&& state) {
  // Build reassembled buffer: header + payload
  // The header must reflect the original frame type, not PAYLOAD

  auto payload = state.payload.move();
  auto payloadSize = payload ? payload->computeChainDataLength() : 0;

  // Construct ParsedFrame with reassembled data
  ParsedFrame result;
  result.metadata.descriptor = getDescriptor(state.originalType);
  result.metadata.streamId = state.streamId;
  result.metadata.flags_ = state.originalFlags & ~detail::kFollowsBit; // Clear follows
  result.metadata.payloadSize = payloadSize;
  result.metadata.payloadOffset = 0; // Payload starts at buffer beginning
  // TODO: Parse metadata size from first fragment's metadata length field
  result.metadata.metadataSize = 0;
  result.buffer = std::move(payload);

  return result;
}
```

### Acceptance Criteria

- [ ] Handler satisfies `InboundHandler` concept
- [ ] Non-fragmented frames pass through unchanged
- [ ] First fragment initializes `FragmentState` correctly
- [ ] Continuation fragments append to pending state
- [ ] Final fragment triggers reassembly and clears pending state
- [ ] `handlerRemoved()` clears all pending fragments
- [ ] Test accessors (`pendingCount`, `hasPendingFragment`) work

---

## Phase 3: Unit Tests

**Goal**: Test the core scenarios: basic cases, cancellation, and interleaving.

### File: `test/FrameDefragmentationHandlerTest.cpp`

### Test Categories

#### 3.1 Basic Cases

| Test | Description |
|------|-------------|
| `NonFragmentedFramePassesThrough` | Frame with `hasFollows=false` forwards unchanged, all fields preserved |
| `TwoFragmentsAssemble` | First fragment (`hasFollows=true`) + final fragment (`hasFollows=false`) → complete frame |
| `ThreeFragmentsAssemble` | First + middle + final fragments combine correctly |
| `PayloadDataCombinedInOrder` | Payload bytes from all fragments concatenated in arrival order |
| `OriginalFrameTypePreserved` | Reassembled frame has REQUEST_* type, not PAYLOAD |
| `MetadataSizePreserved` | `metadataSize` from first fragment preserved in result |
| `FollowsBitCleared` | Reassembled frame has `hasFollows() == false` |

#### 3.2 Cancellation

| Test | Description |
|------|-------------|
| `CancelWithPendingFragments` | CANCEL arrives while fragments pending → clears `FragmentState` for that stream |
| `CancelDoesNotAffectOtherStreams` | CANCEL for stream 1 does not affect pending fragments for stream 2 |
| `CancelForNonPendingStream` | CANCEL for stream with no pending fragments → no-op, passes through |
| `FragmentsAfterCancelStartFresh` | New first fragment after CANCEL starts new accumulation |

#### 3.3 Interleaving

| Test | Description |
|------|-------------|
| `TwoInterleavedStreams` | Stream 1 frag, stream 2 frag, stream 1 frag, stream 2 frag → both reassemble correctly |
| `InterleavedWithComplete` | Interleaved fragments mixed with non-fragmented frames → all handled correctly |
| `ManyInterleavedStreams` | 5 concurrent fragmenting streams, interleaved arrival → all reassemble correctly |

### Test Utilities

```cpp
// Helper to create ParsedFrame with specific properties
ParsedFrame makeFrame(
    FrameType type,
    uint32_t streamId,
    bool hasFollows,
    std::string_view data,
    std::string_view metadata = {});

// Helper to create CANCEL frame
ParsedFrame makeCancelFrame(uint32_t streamId);

// Mock context for handler testing
class MockContext {
 public:
  channel_pipeline::Result fireRead(channel_pipeline::TypeErasedBox&& msg);
  channel_pipeline::Result fireException(folly::exception_wrapper&& e);

  std::vector<ParsedFrame> receivedFrames;
  std::vector<folly::exception_wrapper> receivedExceptions;
};
```

### Acceptance Criteria

- [ ] All basic case tests pass
- [ ] All cancellation tests pass
- [ ] All interleaving tests pass
- [ ] Test utilities are reusable

---

## Phase 4: Microbenchmarks

**Goal**: Measure overhead to ensure handler doesn't regress performance.

### File: `test/FrameDefragmentationHandlerBench.cpp`

### Benchmark Scenarios

| Benchmark | Description | Key Metric |
|-----------|-------------|------------|
| `BM_PassthroughBaseline` | Forward frames without handler | Baseline ops/sec |
| `BM_PassthroughWithHandler` | Non-fragmented through handler | Overhead vs baseline |
| `BM_TwoFragmentReassembly` | Pairs of fragments | Per-reassembly cost |
| `BM_TenFragmentReassembly` | 10 fragments per frame | Accumulation overhead |
| `BM_InterleavedStreams_10` | 10 concurrent fragmenting streams | Map lookup cost |
| `BM_InterleavedStreams_100` | 100 concurrent streams | Scaling behavior |
| `BM_LargePayload_1MB` | 1MB payload in fragments | IOBufQueue efficiency |
| `BM_LargePayload_10MB` | 10MB payload | Memory/throughput |

### Benchmark Implementation Pattern

```cpp
#include <folly/Benchmark.h>
#include <thrift/lib/cpp2/fast_thrift/framing/reading/FrameDefragmentationHandler.h>

using namespace apache::thrift::fast_thrift::framing;

namespace {

class BenchContext {
 public:
  channel_pipeline::Result fireRead(channel_pipeline::TypeErasedBox&&) {
    return channel_pipeline::Result::Success;
  }
  channel_pipeline::Result fireException(folly::exception_wrapper&&) {
    return channel_pipeline::Result::Success;
  }
};

} // namespace

BENCHMARK(BM_PassthroughWithHandler, iters) {
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  auto frame = makeNonFragmentedFrame();

  for (size_t i = 0; i < iters; ++i) {
    channel_pipeline::TypeErasedBox box;
    box.emplace<ParsedFrame>(cloneFrame(frame));
    handler.onRead(ctx, std::move(box));
  }
}

BENCHMARK_RELATIVE(BM_TwoFragmentReassembly, iters) {
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  auto [first, last] = makeTwoFragments();

  for (size_t i = 0; i < iters; ++i) {
    channel_pipeline::TypeErasedBox box1, box2;
    box1.emplace<ParsedFrame>(cloneFrame(first));
    box2.emplace<ParsedFrame>(cloneFrame(last));

    handler.onRead(ctx, std::move(box1));
    handler.onRead(ctx, std::move(box2));
  }
}

// ... additional benchmarks
```

### Expected Metrics

| Scenario | Target | Notes |
|----------|--------|-------|
| Passthrough overhead | < 5% | Map lookup cost only |
| Two-fragment reassembly | < 1μs | Map insert + IOBuf clone |
| 10-fragment reassembly | < 5μs | Linear in fragment count |
| 100 concurrent streams | < 10% overhead | F14Map scales well |

### Acceptance Criteria

- [ ] All benchmarks compile and run
- [ ] Passthrough overhead < 5%
- [ ] Reassembly cost is linear in fragment count
- [ ] No memory leaks under sustained load
- [ ] Results documented

---

## Phase 5: BUCK Integration

**Goal**: Add build targets for new files.

### Updates to `reading/BUCK`

```python
cpp_library(
    name = "fragment_state",
    headers = ["FragmentState.h"],
    deps = [
        "//folly/io:iobuf",
        ":frame_type",
        ":parsed_frame",
    ],
)

cpp_library(
    name = "frame_defragmentation_handler",
    headers = ["FrameDefragmentationHandler.h"],
    deps = [
        "//folly/container:f14_hash",
        "//folly/io:iobuf",
        ":fragment_state",
        ":parsed_frame",
        "//thrift/lib/cpp2/fast_thrift/channel_pipeline:common",
        "//thrift/lib/cpp2/fast_thrift/channel_pipeline:type_erased_box",
    ],
)
```

### Updates to `reading/test/BUCK`

```python
cpp_unittest(
    name = "frame_defragmentation_handler_test",
    srcs = ["FrameDefragmentationHandlerTest.cpp"],
    deps = [
        "//thrift/lib/cpp2/fast_thrift/framing/reading:frame_defragmentation_handler",
        "//folly/portability:gtest",
    ],
)

cpp_binary(
    name = "frame_defragmentation_handler_bench",
    srcs = ["FrameDefragmentationHandlerBench.cpp"],
    deps = [
        "//thrift/lib/cpp2/fast_thrift/framing/reading:frame_defragmentation_handler",
        "//folly:benchmark",
    ],
)
```

---

## Implementation Order

1. **Phase 1**: `FragmentState.h` - Simple struct, no dependencies on handler
2. **Phase 2**: `FrameDefragmentationHandler.h` - Core handler logic
3. **Phase 3**: Unit tests - Verify correctness
4. **Phase 4**: Benchmarks - Verify performance
5. **Phase 5**: BUCK integration - Build system

## Design Decisions (Resolved)

1. **Metadata size**: Store `metadataSize` from first fragment in `FragmentState`. The first fragment contains metadata (if any) + partial data. Continuation fragments only have data. Preserve metadata size in reassembled frame.

2. **Memory limits**: Not implemented in v1. Keep it simple.

3. **Stream cancellation**: If CANCEL frame arrives for a stream with pending fragments, clear the `FragmentState` for that stream.

4. **Out-of-order handling**: Not an issue. TCP guarantees order and sender must honor stream order per RSocket spec.

---

## Success Criteria

- [ ] All unit tests pass
- [ ] Benchmarks show acceptable overhead
- [ ] Handler integrates into existing pipeline
- [ ] Code follows Meta C++ conventions
- [ ] Documentation is complete

---

# FrameFragmentationHandler (Outbound) - HOL Blocking Mitigation

## Overview

The outbound fragmentation handler addresses **Head-of-Line (HOL) blocking** in RSocket's multiplexed streams. Currently, fragmentation is only used for frames exceeding 16MB. This design enables fragmentation for **interleaving** - allowing small payloads to be sent between fragments of large payloads, reducing latency for concurrent streams.

### The Problem

In a single-threaded EventBase model, even fragmented payloads are sent sequentially:

```
Current behavior (HOL blocked):
  Stream A: 8MB ready → frag1, frag2, frag3... frag128 (all sequential)
  Stream B: 1KB ready → blocked until all 128 fragments complete
```

### The Solution

Batch frames during the EventBase tick, then round-robin across streams at flush:

```
Desired behavior (HOL mitigated):
  End of tick - flush phase:
    Stream B: 1KB (small, sent immediately)
    Stream A: frag1 (64KB)
    Stream C: frag1 (64KB)
    Stream A: frag2 (64KB)
    Stream C: frag2 (64KB)
    ... round-robin until drained
```

---

## Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Scheduling Algorithm | Round-Robin | Fixed 64KB fragments provide natural byte-fairness; simpler than WFQ/DRR |
| Map Type | `folly::F14NodeMap<uint32_t, PerStreamState>` | Iterator stability during round-robin while streams complete/start |
| Flush Trigger | `LoopCallback` self-scheduling | Handler schedules itself lazily when work arrives |
| Primary Limit | Max pending bytes | Correlates with memory pressure and latency; fairer than frame count |
| Secondary Limit | Max pending frames | Backstop for many-small-frames pathology |
| Small Frame Handling | Bypass fragmentation | Frames ≤ fragment size sent immediately, no queuing overhead |

---

## Core Algorithm

### EventBase Tick Lifecycle

```
1. Application calls write() for various streams
   ↓
2. Handler receives frames via onWrite()
   ↓
3. For each frame:
   - If size ≤ maxFragmentSize: add to immediateQueue (bypass fragmentation)
   - If size > maxFragmentSize: add to stream's fragment queue in F14NodeMap
   - Increment pendingBytes counter
   - If not scheduled: schedule self as LoopCallback
   ↓
4. If pendingBytes > maxPendingBytes OR pendingFrames > maxPendingFrames:
   - Force immediate flush (don't wait for LoopCallback)
   ↓
5. LoopCallback fires (or forced flush):
   - Send all frames in immediateQueue first
   - Round-robin through F14NodeMap entries:
     - Pop one fragment (≤64KB) from current stream
     - Advance to next stream
     - Repeat until all streams drained
   - Reset pendingBytes and pendingFrames counters
   - Unschedule self (will re-schedule on next write)
```

### Round-Robin Scheduling Detail

```
F14NodeMap<StreamId, PerStreamState>:
  Stream 5: [frag1, frag2, frag3, frag4]  ← 4 fragments pending
  Stream 7: [frag1, frag2]                 ← 2 fragments pending
  Stream 9: [frag1, frag2, frag3]          ← 3 fragments pending

Round 1: Send S5:frag1, S7:frag1, S9:frag1
Round 2: Send S5:frag2, S7:frag2, S9:frag2
Round 3: Send S5:frag3, (S7 done), S9:frag3
Round 4: Send S5:frag4, (S9 done)
         S5 done - all streams drained
```

### Stream Ordering Guarantee

**Critical invariant:** Fragments within a single stream MUST be sent in order.

The F14NodeMap groups fragments by stream ID. Each stream's queue is FIFO. Round-robin pops from the front of each queue, preserving per-stream order while interleaving across streams.

---

## LoopCallback Self-Scheduling

### Scheduling Invariant

The handler maintains a single boolean: `isScheduled_`

| State | Meaning |
|-------|---------|
| `isScheduled_ == false` | No pending work, not registered with EventBase |
| `isScheduled_ == true` | Pending work exists, callback registered for end of tick |

### Scheduling Logic

**On frame arrival (`onWrite`):**
```
if (!isScheduled_) {
  eventBase_->runInLoop(this);  // Schedule for end of tick
  isScheduled_ = true;
}
```

**On flush complete (`runLoopCallback`):**
```
// After draining all queues:
isScheduled_ = false;
// Will re-schedule on next onWrite()
```

### Why Self-Scheduling?

- **Lazy:** Only scheduled when work exists
- **Batching:** All writes within a tick are batched together
- **Single callback:** Avoids redundant scheduling per frame
- **Natural integration:** EventBase's LoopCallback is designed for this pattern

---

## Max Pending Bytes Threshold

### Why Bytes Over Frame Count

| Limit Type | Problem |
|------------|---------|
| Frame count | 10 × 1KB = 10KB buffered vs 10 × 1MB = 10MB buffered (same count, 1000x resource difference) |
| **Bytes** | Directly measures memory pressure, latency, and EventBase tick duration |

### Threshold Behavior

```
Thresholds:
  maxPendingBytes = 512KB (configurable)
  maxPendingFrames = 128 (backstop)

On each onWrite():
  pendingBytes_ += frame.size()
  pendingFrames_++

  if (pendingBytes_ > maxPendingBytes || pendingFrames_ > maxPendingFrames) {
    flushNow();  // Don't wait for LoopCallback
  } else if (!isScheduled_) {
    scheduleFlush();
  }
```

### Why Force Flush Mid-Tick?

If a tight loop produces frames faster than the EventBase can cycle:
- Without limit: unbounded memory growth
- With limit: periodic forced flushes keep memory bounded

The max pending bytes acts as a **high-water mark** triggering eager flush.

---

## PerStreamState Structure

Each entry in the F14NodeMap holds:

| Field | Type | Purpose |
|-------|------|---------|
| `streamId` | `uint32_t` | Stream identifier (key in map, also stored for convenience) |
| `pendingFragments` | Queue of IOBuf | Ordered fragments awaiting send |
| `currentOffset` | `size_t` | Offset into large payload being fragmented (lazy fragmentation) |
| `originalPayload` | `IOBuf*` | Pointer to original payload (if fragmenting on-the-fly) |
| `totalBytes` | `size_t` | Total bytes pending for this stream (for metrics) |

### Lazy vs Eager Fragmentation

| Approach | Description | Tradeoff |
|----------|-------------|----------|
| **Eager** | Split payload into N IOBuf fragments immediately on arrival | Simpler logic, but N allocations upfront |
| **Lazy** | Track offset, slice fragments on-demand during round-robin | Constant memory regardless of payload size, slightly more complex |

**Recommendation:** Lazy fragmentation - use cursor/offset tracking to avoid pre-allocating all fragments.

---

## Small Frame Bypass

Frames smaller than `maxFragmentSize` skip the fragmentation queue entirely:

```
onWrite(frame):
  if (frame.size() <= maxFragmentSize_) {
    immediateQueue_.push_back(std::move(frame));
  } else {
    addToStreamQueue(frame.streamId(), std::move(frame));
  }
```

During flush:
1. **First:** Send all frames in `immediateQueue_` (these are latency-sensitive small payloads)
2. **Then:** Round-robin through large-frame fragments

This ensures small payloads get minimal latency while large payloads are interleaved fairly.

---

## Configuration Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `maxFragmentSize` | 64KB | Fragment size for large payloads; also threshold for bypass |
| `maxPendingBytes` | 512KB | High-water mark triggering forced flush |
| `maxPendingFrames` | 128 | Backstop limit for many-small-frames case |
| `minSizeToFragment` | 1KB | Minimum payload size worth fragmenting (avoid overhead on tiny payloads) |

### Tuning Guidance

| Scenario | Adjustment |
|----------|------------|
| High-latency network | Larger `maxFragmentSize` (128KB+) to reduce header overhead |
| Low-latency requirements | Smaller `maxFragmentSize` (32KB) for finer interleaving |
| Memory-constrained | Lower `maxPendingBytes` (256KB) |
| High-throughput bulk transfers | Higher `maxPendingBytes` (1MB+) to amortize flush overhead |

---

## Integration with channel_pipeline

### Handler Position in Pipeline

```
Application
    ↓ write(payload)
FrameFragmentationHandler  ← NEW: batches, fragments, round-robins
    ↓ write(fragment)
FrameWriter                ← Serializes to wire format
    ↓ write(IOBuf)
Transport Adapter          ← TCP socket
```

### Backpressure Handling

The handler respects downstream backpressure:

```
During flush:
  for each fragment in round-robin:
    result = ctx.fireWrite(fragment);
    if (result == Result::Backpressure) {
      // Stop flushing, keep remaining fragments queued
      ctx.awaitWriteReady();
      return;
    }

onWriteReady():
  // Downstream ready - resume flushing
  ctx.cancelAwaitWriteReady();
  resumeFlush();
```

This integrates with the existing `WriteReadyHook` mechanism in channel_pipeline.

---

## Client vs Server Considerations

| Side | Typical Pattern | Fragmentation Benefit |
|------|-----------------|----------------------|
| **Server** | Large responses (query results) to many clients | High - prevents one large response from blocking others |
| **Client** | Small requests, occasional large uploads | Lower - but still helps for REQUEST_CHANNEL streaming |

Both sides use the same handler, but servers typically see more benefit due to response size asymmetry.

### SETUP Negotiation

The `max_frame_size` in RSocket SETUP can inform fragmentation:

- Client sends SETUP with `max_frame_size = 64KB`
- Server respects this and fragments responses > 64KB
- Ensures receiver can handle fragment size

---

## Interaction with REQUEST_N Flow Control

Fragmentation and flow control are **orthogonal**:

| Mechanism | What it controls |
|-----------|-----------------|
| **Fragmentation** | Interleaving of bytes on the wire (HOL mitigation) |
| **REQUEST_N** | Number of logical payloads sender can emit (rate limiting) |

A single REQUEST_N credit may result in multiple fragments. The sender fragments as needed to meet the credit, but cannot start a new logical payload without additional credit.

---

## File Structure (Outbound Handler)

```
thrift/lib/cpp2/fast_thrift/framing/writing/
├── PerStreamState.h                     # Per-stream fragment queue state
├── FrameFragmentationHandler.h          # Outbound handler implementation
├── test/
│   ├── FrameFragmentationHandlerTest.cpp    # Unit tests
│   └── FrameFragmentationHandlerBench.cpp   # Microbenchmarks
└── BUCK                                 # Build targets (update)
```

---

## Test Scenarios (Outbound Handler)

### Basic Cases

| Test | Description |
|------|-------------|
| `SmallFrameBypassesFragmentation` | Frame ≤ 64KB sent immediately, no queuing |
| `LargeFrameFragmented` | Frame > 64KB split into multiple fragments with FOLLOWS flag |
| `FragmentsPreserveStreamOrder` | Fragments for same stream sent in order |
| `RoundRobinAcrossStreams` | Multiple large streams interleave correctly |

### Flush Triggering

| Test | Description |
|------|-------------|
| `FlushOnLoopCallback` | Queued frames flush at end of EventBase tick |
| `FlushOnMaxPendingBytes` | Exceeding byte threshold forces immediate flush |
| `FlushOnMaxPendingFrames` | Exceeding frame count forces immediate flush |
| `SelfSchedulingOnlyOnce` | Multiple writes don't schedule multiple callbacks |

### Backpressure

| Test | Description |
|------|-------------|
| `BackpressurePausesFlushing` | Result::Backpressure stops flush mid-round-robin |
| `WriteReadyResumesFlushing` | onWriteReady() continues where flush left off |
| `BackpressurePreservesOrder` | Resumed flush maintains correct fragment order |

### Edge Cases

| Test | Description |
|------|-------------|
| `StreamCompleteMidFlush` | Stream's last fragment sent, removed from map |
| `NewStreamMidFlush` | New stream arriving during flush handled gracefully |
| `EmptyFlush` | LoopCallback with no pending work is no-op |
| `ZeroBytePayload` | Empty payload handled correctly (edge case) |

---

## Success Criteria (Outbound Handler)

- [ ] Small frames bypass fragmentation with minimal overhead
- [ ] Large frames fragment correctly with FOLLOWS flag
- [ ] Round-robin provides fair interleaving across streams
- [ ] LoopCallback self-scheduling works correctly
- [ ] Max pending bytes triggers forced flush
- [ ] Backpressure pauses and resumes flush correctly
- [ ] Per-stream fragment order preserved
- [ ] Benchmarks show acceptable overhead vs non-fragmenting baseline
