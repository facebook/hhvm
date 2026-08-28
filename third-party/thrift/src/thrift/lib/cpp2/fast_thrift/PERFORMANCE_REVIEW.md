# Fast Thrift Performance Review

## Scope

This review covers the native Fast Thrift and Rocket request/response paths under `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift` after the landed stack ending in [D117535169](https://www.internalfb.com/diff/D117535169).

The stack already addressed:

- Reusing transport read-buffer capacity in D117535164.
- Sizing metadata buffers to the serialized payload in D117535165.
- Removing parser and frame-writer-local duplicate `IOBuf` chain walks in D117535166.
- Naturally aligning `ThriftPayloadVariant` and server pipeline messages in D117535169.
- Direction-aware pipeline wiring was separately implemented and benchmarked, then reverted because it showed no reliable improvement.

Those items are not open findings below.

## Current Performance Findings

### 1. `DirectStreamMap` ordered erasure performs quadratic work

**Location:** `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h:188-205`

**Frequency:** Once per terminal stream removal on client and server Rocket stream tables.

`StreamIdIndex` maps normal same-parity RSocket stream IDs into consecutive natural slots. `eraseSlot()` nevertheless scans forward until the first empty slot, even when no entry is displaced and no backshift is possible.

For `N` naturally placed streams erased in stream-ID order, the implementation performs exactly:

```text
N * (N - 1) / 2
```

unproductive live-slot inspections:

- 10 streams: 45 inspections.
- 100 streams: 4,950 inspections.
- 1,000 streams: 499,500 inspections.

**Recommended change:** Maintain an exact map-level count of entries stored away from their natural slot. When that count is zero, clear the erased slot in O(1). Retain the existing backshift algorithm when displaced entries exist. Update the count during insertion, backshift, growth, clear, and move operations.

**Validation:** Add a non-allocating value type to the existing map benchmark so `erase()` is isolated from `FakeState` allocation, then run:

```bash
timeout 300s buck2 run fbcode//thrift/lib/cpp2/fast_thrift/frame/read/test:direct_stream_map_bench -- --bm_mode=adaptive --bm_max_secs=20 --bm_regex='Interleaved_(10|100|1000)Streams'
```

Collision, wraparound, growth, duplicate-insert, and move-only-value tests must continue to pass.

### 2. Opening a server stream probes the map twice

**Location:** `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h:221-226`

**Frequency:** Once per new inbound Rocket stream.

The handler performs `find(streamId)` and then `emplace(streamId, ...)`. Both operations begin at the same natural slot and repeat the same probe sequence.

Current code:

```cpp
if (contexts.streams.find(streamId) != contexts.streams.end()) {
  return channel_pipeline::Result::Error;
}
contexts.streams.emplace(
    streamId, RocketStreamContext{.streamType = streamType});
```

Use the insertion result directly:

```cpp
auto [it, inserted] = contexts.streams.emplace(
    streamId, RocketStreamContext{.streamType = streamType});
if (!inserted) {
  return channel_pipeline::Result::Error;
}
```

This removes one complete lookup per opened stream. The absolute saving is likely small for naturally placed IDs, so validate it independently from the erase optimization.

### 3. Batching and queued fragmentation still repeat full chain walks

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/handler/IntervalBatchingFrameHandler.h:115-120,337`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h:143-164,260-276`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/PerStreamState.h:46-60,84-96`
- `/data/users/rroeser/fbsource/fbcode/folly/io/IOBufQueue.cpp:153-162`

**Frequency:** Once per outbound physical server frame in interval batching, and multiple times for each logical frame entering the fragmentation queue.

The interval batcher explicitly calls `computeChainDataLength()` and then appends the same frame to an `IOBufQueue` configured with `cacheChainLength()`. `IOBufQueue::append()` consequently walks the chain again. The handler already maintains `totalBytesBuffered_` and never consumes the queue's cached length.

Replace:

```cpp
folly::IOBufQueue bufferedWritesQueue_{
    folly::IOBufQueue::cacheChainLength()};
```

with:

```cpp
folly::IOBufQueue bufferedWritesQueue_;
```

The queued-fragmentation path is worse: the fragmenter computes `dataSize`, `PerStreamState::enqueue()` recomputes it, and appending to `PendingFrame::dataQueue` computes it again to maintain the queue cache.

Pass the already-known size into `enqueue()`. If the queue cache is removed, retain a `remainingBytes` field and decrement it when fragments are split. This preserves O(1) scheduler accounting without repeatedly traversing payload nodes.

These costs grow with chain-node count and are therefore more important for fragmented messages and large batches than for common one-node serializer output.

**Validation:** Extend the existing batching and fragmentation benchmarks with 1-, 8-, and 32-node chains:

```bash
timeout 300s buck2 run fbcode//thrift/lib/cpp2/fast_thrift/frame/write/handler/bench:batching_benchmark -- --bm_mode=adaptive --bm_max_secs=20
timeout 300s buck2 run fbcode//thrift/lib/cpp2/fast_thrift/frame/write/handler/bench:frame_fragmentation_handler_bench -- --bm_mode=adaptive --bm_max_secs=20
```

### 4. Native client requests allocate two independent control objects

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.cpp:44-59`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h:34-90`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/client/common/ThriftRequestContext.h:32-43`

**Frequency:** Two unconditional heap allocations per submitted native request, excluding request data and serialized metadata buffers.

Every native request independently allocates:

- `RequestRpcMetadata`.
- `client::ThriftRequestContext`.

The request context already has a stable heap address and survives through timeout inspection, checksum mutation, metadata serialization, response correlation, and cleanup. The metadata is needed only until initial-frame serialization.

**Recommended change:** Embed `RequestRpcMetadata` in the native request context and use a native-client-only outbound payload with a passive non-owning metadata pointer. Keep the shared owning payload unchanged for compatibility and decoded inbound paths.

This removes one allocation/free pair but enlarges the request-context allocation and retains metadata storage for the RPC lifetime. Measure allocator size class, allocations per RPC, peak memory at high concurrency, and timeout/cancellation retention before accepting it.

### 5. Generated server dispatch allocates one callback per request

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/server/util/FastHandlerCallback.h:511-516`
- `/data/users/rroeser/fbsource/xplat/thrift/compiler/generate/templates/cpp2/service_tcc/fast_process_and_return.whisker:35-53`

**Frequency:** Once per valid generated native request, whether the method executes inline or on a CPU executor.

`makeFastHandlerCallback()` unconditionally performs:

```cpp
return detail::CallbackPtr<Cb>(new Cb(...));
```

The callback is uniquely owned and eventually destroyed on the connection EventBase. For CPU-offloaded success, response delivery and callback deletion may also enqueue separate EventBase callbacks.

**Recommended direction:** Measure allocator attribution first. If material, use an EventBase-owned fixed-size pool or slab for callback objects and recycle them on the existing EventBase-affine destruction path. Preserve the current unique-ownership and non-atomic lifetime model.

### 6. Successful server responses allocate metadata separately

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h:144-174`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/common/ThriftResponsePayloads.h:58-82`

**Frequency:** One `ResponseRpcMetadata` allocation per successful response and per declared-exception response.

The originating `ThriftRequestContext` already travels through outbound handlers until metadata serialization, but `ThriftInitialResponsePayload` separately owns a heap-allocated metadata object. Offloaded handlers can allocate this object on a CPU worker and release it after EventBase-side serialization.

An allocation-fusion design must preserve typed metadata until the checksum handler has mutated it and must not make decoded client payloads non-owning. A server-specific owned-or-borrowed payload is safer than globally replacing the current `unique_ptr`.

Do not implement this before measuring `sizeof(ThriftRequestContext)`, `sizeof(ResponseRpcMetadata)`, allocator size-class changes, and high-concurrency memory retention.

## Correctness Defects Found During The Review

These are not performance optimizations, but they affect whether performance results represent valid behavior.

### Production handler order bypasses request-response validation

`/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerConnectionFactory.cpp:477-481` installs `RocketServerRequestResponseHandler` before `RocketServerStreamStateHandler` for inbound traversal.

`/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h:103-110` filters on `request.streamType`, but `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h:228-230` stamps that field later. New native requests therefore bypass request-response-specific validation. Existing integration and channel-server pipelines use the opposite order.

Add a production-shape integration test and install stream-state before request-response if that test confirms the intended dependency.

### Native coroutine cancellation can retain dangling references

`/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/gen/fast_client_h.h:63-87` gives the transport a callback capturing coroutine-frame locals by reference. Cancellation posts the baton but does not detach or invalidate the retained transport callback. `/data/users/rroeser/fbsource/fbcode/folly/coro/Task.h:858-872` destroys the completed child frame during `await_resume()`.

A late response or transport failure can therefore invoke a callback that references the destroyed `baton` and `response`; cancellation and transport completion can also race to write `response`.

Use independently owned, idempotent completion state shared by cancellation and transport completion. Add ASAN coverage for cancellation followed by late response, timeout, connection failure, and not-open rejection, plus TSAN coverage for cancellation racing completion.

## Explicitly Not Retained As Findings

- Transport read-buffer allocation, metadata over-allocation, parser-local duplicate chain traversal, writer-local duplicate metadata traversal, and packed server messages were fixed by the landed D117535164-D117535169 stack.
- Direction-mismatch pipeline skipping was implemented and benchmarked, then reverted because it showed no reliable improvement.
- `TypeErasedBox::take<T>()` typed destruction, `ComposedFrame` cache-line size, remaining packed client envelopes, and optional client-stat slot size require optimized assembly or compiler-derived layout evidence before being presented as defects.
- No request-path lock, explicit atomic, or demonstrated false-sharing defect was found in EventBase-local stream, framing, batching, or pipeline state.
