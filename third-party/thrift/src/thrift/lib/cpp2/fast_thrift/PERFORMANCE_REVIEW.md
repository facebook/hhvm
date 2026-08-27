# fast_thrift Performance Review

**Scope:** Production code under `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift`

**Revision reviewed:** `578339624294635d41acac2d5de97666929a96a0`

This review examines the production request/response path for avoidable allocations, cache pressure, indirect dispatch, repeated buffer traversal, and excess hot-structure size. Nanosecond estimates below are order-of-magnitude estimates, not measured results. Computed structure sizes should be confirmed with compiler `static_assert`s before implementation.

## Hot Path

The server request path is:

```text
epoll readable
  -> AsyncSocket
  -> TransportHandler
  -> Rocket pipeline
  -> RocketServerAppAdapter
  -> ThriftServerTransportAdapter
  -> Thrift pipeline
  -> ThriftServerAppAdapter
  -> generated method dispatch
```

The response returns through the Thrift pipeline, Rocket pipeline, frame encoder and batching handler, and finally `TransportHandler::onWrite()`.

Connections and their pipelines are owned by one `folly::EventBase` IO thread. Optional generated method execution may move to a CPU executor, after which responses return to the connection's EventBase.

## Findings

### 1. Critical: A fresh 64 KB `IOBuf` is allocated for every socket read event

**Location:** `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h:134-152`

**Frequency:** Once per readable socket event, commonly once per request for non-pipelined request/response traffic.

`getReadBuffer()` appends a newly allocated buffer when the read queue lacks tailroom. `readDataAvailable()` then passes `readBufQueue_.move()` into the pipeline, emptying the queue. The next read consequently has zero tailroom and allocates another buffer.

The default maximum buffer size is 65,536 bytes, and the production server factory uses that default. `SimpleBufferAllocator::allocate()` delegates directly to `folly::IOBuf::create()`, so the buffer is not pooled or reused.

At this size, the allocation may require both a 64 KB data allocation and a separate `IOBuf` metadata allocation. Downstream frame parsing may then copy small reads while packing the queue, undermining the intended zero-copy handoff.

**Estimated impact:** Approximately 150-400 ns per read event, plus 64 KB allocation-class churn for requests that are often only a few hundred bytes.

**Recommended fix:** Retain a connection-level read buffer and pass a shared view of only the newly written bytes downstream. Allocate a replacement only when the retained buffer has insufficient tailroom. Reset the retained buffer during drain and immediate-close paths.

Illustrative shape:

```cpp
channel_pipeline::BytesPtr readBuf_;

void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
  DCHECK(pipeline_);
  if (!readBuf_ || readBuf_->tailroom() < minBufferSize_) {
    readBuf_ = pipeline_->allocate(maxBufferSize_);
  }
  *bufReturn = readBuf_->writableTail();
  *lenReturn = readBuf_->tailroom();
}

void readDataAvailable(size_t len) noexcept override {
  folly::DelayedDestruction::DestructorGuard guard(this);
  DCHECK(readBuf_);
  DCHECK(pipeline_);

  auto chunk = readBuf_->cloneOne();
  chunk->trimStart(chunk->length());
  chunk->append(len);
  readBuf_->append(len);

  handleReadResult(
      pipeline_->fireRead(TypeErasedBox(std::move(chunk))));
}
```

This design requires careful validation of shared-buffer write safety and lifecycle behavior.

### 2. Completed: Non-participating pipeline handler evaluation

**Status:** Completed. Direction-aware wiring was implemented and correctness-tested, then reverted after before/after measurement showed no reliable performance improvement with the existing benchmarks. No phase #2 code changes remain.

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/HandlerNode.h:304-341`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.cpp:117-166`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h:100-115`

**Frequency:** Every message, in both directions.

Handlers that do not participate in one direction receive passthrough thunks. Pipeline wiring connects every adjacent node without skipping these non-participating handlers.

On the representative server request/response path, this adds approximately eight useless indirect calls per round trip:

- Rocket inbound: batching, frame-length encoder, and fragmentation handlers.
- Rocket outbound: frame defragmentation and frame-length parser handlers.
- Thrift outbound: request-context, connection-context, and request-header handlers.

Each hop performs an indirect call and dependent loads for the next function, handler, and context. It also consumes indirect branch-predictor capacity and prevents inlining across the boundary.

**Estimated impact:** Approximately 25-40 ns per request/response round trip.

**Recommended fix:** Record whether each `HandlerNode` participates in reads and writes, then wire `ContextImpl` directly to the next participating node. Keep passthrough thunks available for indexed entry points that require them.

```cpp
struct HandlerNode {
  // Existing fields...
  bool participatesInRead{false};
  bool participatesInWrite{false};
};
```

Set these flags in `makeHandlerNode()` when the corresponding concepts are satisfied. During pipeline construction, scan to the next read participant and previous write participant rather than wiring unconditional adjacency. Apply the same logic to cached pipeline entry points. Leave the exception chain unchanged because it is cold.

### 3. Completed: Metadata allocation uses the computed serialized size

**Status:** Completed. Request and response metadata serializers retain the required 16-byte frame headroom while removing the unrelated 1 KB minimum capacity. Focused coverage verifies exact-size allocation for request Binary metadata and response Binary/Compact metadata.

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/common/RequestMetadata.h:44-50`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/common/ResponseMetadata.h:42-49`

**Frequency:** Once per request and once per response.

Both paths call `serializedSizeZC()` and then discard the exact sizing benefit by enforcing a 1,024-byte minimum allocation. Typical metadata is roughly 30 bytes, and the required framing headroom is 16 bytes.

This also contradicts the nearby documentation stating that serialization allocates an exactly sized buffer.

**Estimated impact:** The allocation count is unchanged, but a typical metadata buffer lands near a 1,280-byte allocator class instead of approximately 160 bytes. That is roughly eight times the allocation and resident footprint per metadata object, increasing cache, TLB, and allocator-bin pressure.

**Recommended fix:** Allocate the computed serialized size plus required headroom.

```cpp
const size_t serializedSize = metadata.serializedSizeZC(&writer);
auto buffer = folly::IOBuf::create(
    kMetadataHeadroomBytes + serializedSize);
buffer->advance(kMetadataHeadroomBytes);
```

If a minimum is necessary for allocator behavior, use a substantially smaller documented floor and validate it with allocation profiling.

### 4. Moderate: Frame lengths are repeatedly recomputed by walking `IOBuf` chains

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h:148-182`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h:89-136`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.cpp:108-182`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h:97`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/handler/IntervalBatchingFrameHandler.h:115`

**Frequency:** Multiple times per frame.

Examples include:

- `tryParseFrame()` walks the chain for a minimum-size check, then `parseFrame()` immediately walks it again.
- `FrameLengthParserHandler` maintains a separate `size_` by walking incoming chains even though `IOBufQueue` already caches chain length.
- `FrameWriter` uses `computeChainDataLength()` as an emptiness predicate.
- Metadata length is computed for a boolean and then computed again for serialization.
- Adjacent outbound handlers walk the same frame chain independently.

**Estimated impact:** Approximately 5-15 ns per round trip for typical short chains, with higher cost for fragmented payloads.

**Recommended fixes:**

- Compute the total frame length once in `tryParseFrame()` and pass it to `parseFrame()`.
- Replace `FrameLengthParserHandler::size_` with `IOBufQueue::chainLength()`.
- Use `empty()` rather than `computeChainDataLength() > 0` for predicates.
- Cache metadata length for the duration of frame serialization.
- Propagate known outbound frame lengths between adjacent handlers where practical.

### 5. Moderate: `TypeErasedBox::take<T>()` destroys through type erasure despite knowing `T`

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h:248-252`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/dynamic/detail/SmallBuffer.h:117-124`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/dynamic/detail/SmallBuffer.h:195-200`

**Frequency:** Every typed extraction and numerous box moves across the request/response pipeline.

`TypeErasedBox::take<T>()` knows the concrete type, moves it out, and then calls the type-erased `reset()`. That loads the operations table and invokes `ops_->destroy` indirectly. The concrete destructor target is already statically known.

A round trip crosses many `TypeErasedBox` boundaries, so a small cost at each boundary compounds.

**Estimated impact:** Approximately 25-50 ns per round trip, assuming 2-4 ns across roughly a dozen affected transitions.

**Recommended fix:** Add a typed reset operation to `SmallBuffer` and use it from `TypeErasedBox::take<T>()`.

```cpp
template <typename T>
void resetAs() noexcept {
  if constexpr (SupportsNonTrivial) {
    if (ops_ != &kEmptyOps) {
      std::destroy_at(std::launder(reinterpret_cast<T*>(inline_)));
    }
    ops_ = &kEmptyOps;
  } else {
    reset();
  }
}
```

```cpp
template <typename T>
T take() noexcept {
  T result = std::move(Base::as<T>());
  Base::template resetAs<T>();
  return result;
}
```

This crosses into the shared dynamic `SmallBuffer` implementation and therefore requires validation with that component's owners.

### 6. Moderate: `ComposedFrame` appears to spill into a second cache line

**Location:** `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h:56-85`

**Frequency:** At least once per response, followed by moves through the Rocket pipeline.

The computed size is 72 bytes. A normal PAYLOAD frame uses roughly the first 30 bytes, while approximately 40 bytes of fields are specific to ERROR, REQUEST_N, REQUEST_STREAM, EXT, KEEPALIVE, and SETUP frames.

Those cold fields are nevertheless default-initialized and moved with every request/response frame. A 72-byte object also crosses a 64-byte cache-line boundary.

**Estimated impact:** Approximately 10-20 ns per response from excess initialization, copying, and cache footprint.

**Recommended fix:** First reorder fields so the type fits in 64 bytes and places PAYLOAD fields together. Confirm the size with:

```cpp
static_assert(sizeof(ComposedFrame) == 64);
```

A union for frame-type-specific fields could reduce the type further, but field reordering is the safer initial change. All designated initializers must be updated to match declaration order.

### 7. Low to moderate: Packed request messages misalign fields without needing the saved bytes

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h:46-72`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h:73`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h:323`

**Frequency:** Every server request and response message.

`#pragma pack(1)` appears intended to keep messages inside `TypeErasedBox`'s 120-byte inline budget. The request message is estimated at approximately 34 bytes packed and 40 bytes naturally aligned, leaving ample room either way.

Packing therefore saves about six bytes while forcing misaligned scalar accesses and preventing efficient naturally aligned moves.

**Estimated impact:** Small on x86-64, but this is likely a pure loss on a per-request type and may matter more on other architectures.

**Recommended fix:** Remove the packing directives and document the actual invariant with compiler-checked size limits.

```cpp
static_assert(
    sizeof(ThriftServerRequestMessage) <= 120,
    "request message must fit TypeErasedBox inline storage");
static_assert(
    sizeof(ThriftServerResponseMessage) <= 120,
    "response message must fit TypeErasedBox inline storage");
```

### 8. Low: Two smaller write-path allocation wastes

**Locations:**

- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.cpp:156`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.cpp:479`
- `/data/users/rroeser/fbsource/fbcode/thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h:24-39`

The non-headroom frame-writing fallback allocates an `IOBuf` for a header only 6-13 bytes long. The normal request/response path generally uses metadata headroom, so this is not the steady-state cost, but the fallback can use stack-backed header storage instead of a heap allocation.

Separately, server response serialization reserves 128 bytes of headroom on the response data buffer. On the request/response path, the frame header is written into metadata-buffer headroom and the data buffer is only chained behind it. The 128 bytes on the data buffer are therefore allocated but unused for every response.

**Recommended fixes:**

- Use stack-backed header storage for the non-headroom fallback.
- Remove or reduce response data headroom when metadata owns the frame header.

## Areas Checked Without Findings

- **Metrics and statistics:** Per-thread shards use plain counters and cache-line alignment. Handlers are omitted when statistics are disabled, so there is no unnecessary atomic or allocation cost.
- **Method dispatch:** The `folly::F14FastMap<std::string, fn>` lookup accepts `std::string_view` through heterogeneous hashing and does not allocate a temporary string.
- **Core dispatch structure:** `ContextImpl` is 128 bytes and cache-line aligned. Its read/write dispatch fields are placed in the first cache line, and `fireRead()`/`fireWrite()` are aggressively inlined.
- **Pipeline maps:** The handler lookup map is used by indexed control operations, not the normal request/response traversal.
- **Event subscriptions:** The event system is opt-in and compiles out under `NoEvent`; non-subscribers do not add per-request list traversal.
- **Backpressure:** Intrusive lists avoid allocation, and no-backpressure specializations leave the paths inactive.
- **EventBase scheduling:** Response writes only hop to the EventBase when invoked off-thread. On-thread writes are direct.
- **Message forwarding:** Most Rocket handlers inspect messages in place with `get<T>()` and forward the same box by move; they do not repeatedly unbox and rebox payloads.
- **Write batching:** Frames are chained without packing and emitted through batched writes.

## Recommended Order

1. Reuse transport read buffers.
2. ~~Skip non-participating handlers during pipeline wiring.~~ Completed; evaluated and reverted because no reliable benchmark improvement was observed.
3. ~~Correct request and response metadata buffer sizing.~~ Completed; the 16-byte frame headroom remains reserved.
4. Remove redundant `IOBuf` chain-length walks.
5. Add typed destruction for `TypeErasedBox::take<T>()`.
6. Reduce `ComposedFrame` to one cache line.
7. Remove unnecessary packed message layouts.
8. Eliminate small fallback and unused-headroom allocations.

## Targeted Validation

- Count 65,536-byte `folly::IOBuf::create()` calls under steady request/response load before and after read-buffer reuse.
- Use jemalloc allocation profiling to rank the transport and metadata allocation sites by allocated bytes and allocation count.
- Count retired indirect branches before and after skipping passthrough handlers; expect approximately eight fewer indirect calls per round trip.
- Inspect `FrameParser`, `FrameWriter`, and `TypeErasedBox::take()` with instruction-level profiling before prioritizing their smaller changes.
- Add compiler `static_assert`s to confirm the computed `ComposedFrame` and naturally aligned message sizes.
- Validate one change at a time with fixed offered load, reporting per-core QPS, p50/p99 latency, RSS, and allocator statistics.
