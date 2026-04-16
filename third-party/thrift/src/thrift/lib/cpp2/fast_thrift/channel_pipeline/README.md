# Channel Pipeline Framework

## Overview

Channel Pipeline is the backbone of **FastTransport** — a modular, composable
framework for building high-performance Thrift transports in C++.

The goal is to allow developers to create a **performant but composable** layer
for transport implementation. It has the following goal: **being performant and reliable**
while still be relatively easy to use.

The design makes it possible to create a Thrift Rocket transport that is
composable but can achieve higher performance than monolithic implementations.

---

## Design Principles

### Zero Virtualization

Uses **concepts and templates** instead of virtual methods.

**External Polymorphism**: Handler classes don't inherit from a base class or
implement virtual functions. Instead, polymorphic dispatch is achieved through
function pointers stored alongside type-erased handler objects. When
`addNextInbound<H>()`/`addNextOutbound<H>()`/`addNextDuplex<H>()` is called,
the compiler generates type-specific function pointers that bridge between
type-erased storage and the concrete handler's methods. Each method enforces
the corresponding handler concept at compile time via `static_assert`.

This enables:
- Compile-time dispatch (no vtable lookups)
- Compiler can inline handler calls
- Handlers are plain classes with no inheritance overhead
- Third-party types can be used without modification
- "Pit of success" — default usage is performant

### Inlining Optimizations

For maximum performance on hot paths, the framework uses `__attribute__((flatten))`
on pipeline methods like `fireRead`, `fireWrite`, etc. This hints the compiler to
inline all function calls made from within these methods.

**For handler implementers**: If you want to ensure your handler methods get inlined
into the pipeline's hot path, you can mark them with `FOLLY_ALWAYS_INLINE`:

```cpp
#include <folly/lang/Hint.h>

class MyCodec {
 public:
  FOLLY_ALWAYS_INLINE Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    auto& bytes = msg.get<BytesPtr>();
    auto decoded = decode(bytes);
    return ctx.fireRead(eraseAndBox(std::move(decoded)));
  }

  FOLLY_ALWAYS_INLINE Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    auto& message = msg.get<Message>();
    auto encoded = encode(message);
    return ctx.fireWrite(eraseAndBox(std::move(encoded)));
  }
};
```

This is optional but recommended for performance-critical handlers. Combined with
LTO (Link-Time Optimization), which Meta uses, the compiler can often inline handler
calls even through function pointers, achieving near-zero overhead dispatch.

### Near-Zero Allocations

The pipeline framework avoids allocations as much as possible:
- Object pooling
- Arena allocation
- Bump allocation
- `TypeErasedBox`: 56-byte inline storage (no heap fallback)

### Type Erasure Without Runtime Cost

`TypeErasedBox` provides type erasure without the overhead of `std::any`:
- **Zero-cost wrapper** over SmallBuffer (64 bytes total = one L1 cache line)
- **No runtime overhead** in release builds (same size as underlying SmallBuffer)
- **Compile-time size enforcement** via `static_assert` (won't compile if type > 56 bytes)
- **Debug mode exceptions**: Throws `TypeErasedBoxTypeMismatch` with demangled type names
- **For types > 56 bytes**: Wrap in `unique_ptr`:
  ```cpp
  erase_and_box(std::make_unique<LargeType>(...))
  ```

Handlers only need to satisfy concept requirements (`InboundHandler`,
`OutboundHandler`, etc.) — the pipeline can store and invoke heterogeneous
handler types without requiring them to share a common interface.

### Asynchronous Message Passing

Async code inside pipelines is handled through message passing.

### Compile-Time Finite State Machines

Logical transitions are managed by compile-time FSMs:
- Known, valid set of transitions
- Reduces bugs from invalid state transitions
- Compile-time verification where possible

### Lock-Free Architecture

FastTransport will not use locks in the fast path:
- No mutexes in hot path
- Single-threaded design eliminates need for synchronization

### Single-Threaded

All code is assumed to be accessed by a single thread:
- No internal synchronization
- Thread safety is the caller's responsibility
- Simplifies reasoning about correctness

### Back-Pressure as First Class

Back-pressure is a first-class feature for reliability:
- Can pause processing when overloaded
- Supports load-shedding
- Graceful degradation under load

### Explicit Lifecycle

Object lifecycle is explicit and well-understood:
- Objects tied to a context
- Clear ownership model (Pipeline → Context → Handler)
- `DelayedDestruction` for safe async cleanup

### Exceptions as Messages

Exceptions should not be thrown but handled as messages passed through the
pipeline:
- `fireException(e)` propagates errors through handlers
- Handlers can intercept, transform, or forward exceptions
- No stack unwinding through the pipeline

---

## Folly Dependencies

Channel Pipeline is built on top of **folly**, Meta's foundational C++ library:

| Component | Usage |
|-----------|-------|
| `folly::EventBase` | Event loop for async I/O — accessed via `ctx.eventBase()` |
| `folly::IOBuf` | Zero-copy buffer chains — `BytesPtr = std::unique_ptr<folly::IOBuf>` |
| `folly::DelayedDestructionBase` | Safe async cleanup for Pipeline and Context |
| `folly::exception_wrapper` | Type-erased exception propagation |
| `folly::coro::Task` | Coroutine support for async operations |

Future transport integration will use:
- `folly::AsyncSocket` — socket abstraction
- `folly::AsyncTransport` — transport interface with `ReadCallback`/`WriteCallback`

---

## Threading Model

Channel Pipeline follows a **single-threaded model**:

```
┌─────────────────────────────────────────────────────────┐
│                     EventBase Thread                     │
│                                                         │
│  ┌─────────┐    ┌──────────┐    ┌─────────────────┐    │
│  │ Socket  │───►│ Pipeline │───►│   Application   │    │
│  │  I/O    │◄───│          │◄───│                 │    │
│  └─────────┘    └──────────┘    └─────────────────┘    │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Rules

| Rule | Description |
|------|-------------|
| **All operations on one thread** | Pipeline construction and all message processing must occur on the owning EventBase thread |
| **No cross-thread access** | Never call pipeline methods from another thread without explicit synchronization |
| **No internal locks** | The pipeline has no mutexes — thread safety is the caller's responsibility |
| **Use runInEventBaseThread** | To interact with a pipeline from another thread, schedule work on its EventBase |

### Example: Cross-Thread Access

```cpp
// WRONG: Direct call from another thread
otherThread.spawn([pipeline] {
    pipeline->fireRead(msg);  // UNDEFINED BEHAVIOR
});

// CORRECT: Schedule on the pipeline's EventBase
otherThread.spawn([pipeline, evb = pipeline->eventBase()] {
    evb->runInEventBaseThread([pipeline, msg = std::move(msg)]() mutable {
        pipeline->fireRead(std::move(msg));  // Safe
    });
});
```

---

## Error and Backpressure Handling

All data-path methods return `Result`. Callers must handle all three cases:

### Handling Result at Transport Layer

```cpp
void TransportAdapter::onDataReceived(BytesPtr bytes) {
    Result result = pipeline_->fireRead(eraseAndBox(std::move(bytes)));

    switch (result) {
        case Result::Success:
            // Continue reading
            break;

        case Result::Backpressure:
            // Stop reading until pipeline signals ready
            transport_->pauseRead();
            break;

        case Result::Error:
            // Handle error — close connection, log, etc.
            transport_->close();
            break;
    }
}
```

### Handling Result in Handlers

```cpp
Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    // Process the message
    auto processed = process(std::move(msg));

    // Propagate to next handler and return its result
    Result result = ctx.fireRead(eraseAndBox(std::move(processed)));

    // Optionally react to downstream backpressure
    // Note: Backpressure means the message was accepted, but we should slow down.
    // Handlers may choose to respect this signal or ignore it.
    if (result == Result::Backpressure) {
        // Could buffer future messages, propagate signal, or ignore
    }

    return result;  // Always propagate result upstream
}
```

### Result Propagation

```
Transport ──► Handler A ──► Handler B ──► Handler C ──► App
    │              │              │              │        │
    │              │              │              │        │
    ◄──────────────◄──────────────◄──────────────◄────────┘
                        Result propagates back
```

- Results flow **backwards** through the pipeline
- Each handler can inspect and react to downstream results
- Backpressure signals that the operation was accepted but the sender should slow down
- Handlers may choose to respect backpressure or ignore it
- Errors propagate back to allow cleanup at each layer

---

## Robustness Goals

The architecture is designed to create a robust, reliable system:

| Goal | How |
|------|-----|
| **Predictable under load** | Graceful degradation via back-pressure, not catastrophic failure |
| **Fewer classes of bugs** | Lock-free, single-threaded, FSMs, explicit resource management |
| **Easier to reason about** | Composable handlers, clear lifecycle, testable in isolation |

---

## Quick Start

### 1. Define Handler Tags

```cpp
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>

HANDLER_TAG(codec);
HANDLER_TAG(compression);
HANDLER_TAG(thrift_processor);
```

### 2. Implement a Handler

Handlers are plain classes — no base class, no virtual methods. They just
need to satisfy concept requirements:

```cpp
class MyCodec {
public:
    // === Lifecycle ===

    void handlerAdded(Context& ctx) noexcept {
        // Cache references to other handlers for hot path
        processorCtx_ = ctx.pipeline()->context(thrift_processor_tag);
    }

    void handlerRemoved(Context& ctx) noexcept {
        processorCtx_ = nullptr;
    }

    // === Inbound (transport → application) ===

    Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
        auto& bytes = msg.get<BytesPtr>();
        auto decoded = decode(bytes);
        return ctx.fireRead(eraseAndBox(std::move(decoded)));
    }

    void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
        ctx.fireException(std::move(e));
    }

    void onPipelineActivated(Context& ctx) noexcept {}

    // === Outbound (application → transport) ===

    Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
        auto& message = msg.get<Message>();
        auto encoded = encode(message);
        return ctx.fireWrite(eraseAndBox(std::move(encoded)));
    }

    void onWriteReady(Context& ctx) noexcept {}
    void onPipelineDeactivated(Context& ctx) noexcept {}

private:
    Context* processorCtx_ = nullptr;
};
```

### 3. Build a Pipeline

```cpp
// Construction API is implementation-defined
auto pipeline = PipelineBuilder()
    .addNextDuplex(codec_tag, std::make_unique<MyCodec>())
    .addNextDuplex(compression_tag, std::make_unique<CompressionHandler>())
    .addNextInbound(thrift_processor_tag, std::make_unique<ThriftProcessor>())
    .build();
```

### 4. Process Messages

```cpp
// Inbound: data from transport
pipeline->fireRead(eraseAndBox(std::move(bytes)));

// Outbound: data to transport
pipeline->fireWrite(eraseAndBox(std::move(response)));

// Fire to specific handler
pipeline->sendRead(codec_tag, eraseAndBox(data));
```

---

## Core Concepts

### Pipeline

The container that owns all handlers and contexts.

- Satisfies `Pipeline<P, Ctx>` concept
- Requires `folly::DelayedDestructionBase`
- Static after construction (no dynamic add/remove)
- To change pipeline, rebuild entirely

| Method | Description |
|--------|-------------|
| `fireRead(msg)` | Fire inbound from head |
| `fireWrite(msg)` | Fire outbound from tail |
| `sendRead(id, msg)` | Fire to specific handler |
| `sendWrite(id, msg)` | Fire to specific handler |
| `context(id)` | Get context by ID (nullptr if not found) |
| `close()` | Shutdown pipeline |

### Context

A handler's view into the pipeline.

- Satisfies `ContextApi<Ctx, P>` concept
- Requires `folly::DelayedDestructionBase`
- One context per handler

| Method | Description |
|--------|-------------|
| `fireRead(msg)` | Fire to next inbound handler |
| `fireWrite(msg)` | Fire to next outbound handler |
| `fireException(e)` | Propagate exception as message |
| `pipeline()` | Access owning pipeline |
| `handlerId()` | This handler's ID |
| `allocate(size)` | Allocate buffer using pipeline's allocator |
| `eventBase()` | Get the EventBase this pipeline runs on |
| `close()` | Close pipeline |

### Handlers

| Concept | Direction | Methods |
|---------|-----------|--------|
| `InboundHandler` | Transport → App | `onRead`, `onException`, `onPipelineActivated` |
| `OutboundHandler` | App → Transport | `onWrite`, `onPipelineDeactivated`, `onWriteReady` |
| `HandlerLifecycle` | Setup/Teardown | `handlerAdded`, `handlerRemoved` |

### Handler IDs

Compile-time integer IDs for zero-overhead lookup:

```cpp
HANDLER_TAG(decoder);  // "decoder"_hid → uint64_t at compile time

auto* ctx = pipeline->context(decoder_tag);  // Fast integer lookup
```

### Channel

The top-level abstraction that ties together a transport and pipeline.

- Satisfies `Channel<C, P>` concept
- Requires `folly::DelayedDestructionBase`
- Hides transport details from handlers

| Method | Description |
|--------|-------------|
| `pipeline()` | Access the pipeline |
| `isActive()` | Check if connection is active |
| `close()` | Close the connection |

### Transport Adapters

Concepts for data flow between transport and pipeline:

| Concept | Direction | Purpose |
|---------|-----------|---------|
| `InboundTransportHandler` | Network → Pipeline | Receives bytes from transport (`onRead`, `onError`, `onClose`) |
| `OutboundTransportHandler` | Pipeline → Network | Sends bytes to transport (`write`), backpressure control (`pauseRead`, `resumeRead`) |

### App Adapters

Concepts for data flow between pipeline and application:

| Concept | Direction | Purpose |
|---------|-----------|---------|
| `ClientInboundAppAdapter` | Pipeline → App | Receives decoded messages (`onRead`) |
| `ClientOutboundAppAdapter` | App → Pipeline | Sends messages to pipeline (`write`) |

### Endpoint Handlers

The pipeline endpoints use specialized handler concepts with fixed data flow direction:

| Concept | Method | Direction | Purpose |
|---------|--------|-----------|---------|
| `HeadEndpointHandler` | `onWrite()` | Outbound | Transport-side endpoint, sends data to network |
| `TailEndpointHandler` | `onRead()` | Inbound | Application-side endpoint, receives data from pipeline |

**Pipeline Structure:**
```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Pipeline                                        │
│                                                                              │
│  ┌────────────────────┐                      ┌─────────────────────┐        │
│  │  HeadEndpointHandler│  ◄── fireWrite() ── │  TailEndpointHandler │        │
│  │  (TransportHandler) │  ── fireRead() ──►  │     (AppAdapter)     │        │
│  └────────────────────┘                      └─────────────────────┘        │
│         onWrite()                                  onRead()                  │
│                                                    onException()             │
└─────────────────────────────────────────────────────────────────────────────┘
```

**HeadEndpointHandler** (transport side):
```cpp
template <typename H>
concept HeadEndpointHandler = requires(H& h, TypeErasedBox&& msg) {
  { h.onWrite(std::move(msg)) } noexcept -> std::same_as<Result>;
  // Lifecycle methods
  { h.handlerAdded() } noexcept;
  { h.handlerRemoved() } noexcept;
  { h.onPipelineActive() } noexcept;
  { h.onPipelineInactive() } noexcept;
};
```

**TailEndpointHandler** (application side):
```cpp
template <typename T>
concept TailEndpointHandler = requires(T& t, TypeErasedBox&& msg, folly::exception_wrapper&& e) {
  { t.onRead(std::move(msg)) } noexcept -> std::same_as<Result>;
  { t.onException(std::move(e)) } noexcept -> std::same_as<void>;
  // Lifecycle methods
  { t.handlerAdded() } noexcept;
  { t.handlerRemoved() } noexcept;
  { t.onPipelineActive() } noexcept;
  { t.onPipelineInactive() } noexcept;
};
```

**Example Usage with PipelineBuilder:**
```cpp
// Head = TransportHandler (sends requests via onWrite)
// Tail = AppAdapter (receives responses via onRead)
auto pipeline = PipelineBuilder<TransportHandler, AppAdapter, Allocator>()
    .setEventBase(evb)
    .setHead(&transportHandler)
    .setTail(&appAdapter)
    .setAllocator(&allocator)
    .addNextInbound<CodecHandler>(codec_tag)
    .addNextDuplex<FrameHandler>(frame_tag)
    .build();
```

**Deprecated:** The old `EndpointHandler` concept (using `onMessage()`) is deprecated. New code should use `HeadEndpointHandler` or `TailEndpointHandler` with explicit direction.

---

## Message Types

Messages flow through the pipeline via `TypeErasedBox`:

```cpp
// Type erasure without runtime cost
TypeErasedBox box = erase_and_box(std::move(myMessage));

// Retrieve typed value
auto& msg = box.get<MyMessage>();
```

- **≤64 bytes**: Stored inline (no heap allocation)
- **>64 bytes**: Heap allocated
- **Move-only**: No copying
- **No runtime type checking in release**: Relies on compile-time safety

---

## Buffer Allocation

Handlers allocate buffers through the context, not directly:

```cpp
Result on_write(Context& ctx, TypeErasedBox&& msg) noexcept {
    auto buffer = ctx.allocate(1024);  // Uses pipeline's allocator
    encode_into(msg, buffer);
    return ctx.fire_write(erase_and_box(std::move(buffer)));
}
```

### Why Context-Based Allocation?

| Benefit | Description |
|---------|-------------|
| **Swappable** | Pipeline chooses the allocator (pooled, arena, simple) |
| **No code changes** | Switch allocators without modifying handlers |
| **Testable** | Inject mock allocators for testing |
| **Optimizable** | Use specialized allocators per-pipeline (e.g., arena for short-lived connections) |

### BufferAllocator Concept

```cpp
template <typename B>
concept BufferAllocator = requires(B b, size_t size) {
  { b.allocate(size) } -> std::same_as<BytesPtr>;
};
```

Implementations provide the allocator to the pipeline; handlers just call `ctx.allocate(size)`.

---

## Ownership Model

```
Pipeline (owns) ──► Context (owns) ──► Handler
```

- Pipeline owns all Contexts
- Contexts own their Handlers
- Handlers accessed only through Contexts
- `DelayedDestruction` ensures safe async cleanup
- Static pipelines: immutable after construction, rebuild to change

---

## Data Flow

```
┌─────────────┐    ┌─────────────────────────────────────┐    ┌─────────────┐
│  Transport  │───►│              Pipeline               │───►│ Application │
│   (Socket)  │    │  [Head] ─► [Handler] ─► [Tail]      │    │   (Thrift)  │
│             │◄───│                                     │◄───│             │
└─────────────┘    └─────────────────────────────────────┘    └─────────────┘
       │                         │                                   │
       │   InboundTransportHandler                    TailEndpointHandler
       │   (on_read, on_error)                        (onRead)
       │                         │                                   │
       │   HeadEndpointHandler                        ClientOutboundAppAdapter
       │   (onWrite)                                  (write)
```

**Inbound flow** (Network → Application):
1. Transport receives bytes from socket
2. Calls `InboundTransportHandler.on_read(bytes)`
3. Pipeline handlers decode, transform, pass through
4. Pipeline tail receives via `TailEndpointHandler.onRead(msg)`

**Outbound flow** (Application → Network):
1. Application calls `ClientOutboundAppAdapter.write(msg)`
2. Pipeline handlers encode, transform, pass through
3. Pipeline head calls `HeadEndpointHandler.onWrite(bytes)`
4. Transport sends bytes to socket

---

## Backpressure

Backpressure is propagated through **return values**, not listeners or callbacks.

### Result Type

Handler methods return `Result` to signal the outcome of an operation:

| Value | Meaning |
|-------|---------|
| `Result::Success` | Operation accepted, continue processing |
| `Result::Backpressure` | Operation accepted, but slow down (see below) |
| `Result::Error` | Operation failed, stop processing |

### Backpressure Semantics

`Result::Backpressure` is a **soft signal**, not an error:
- The current operation **was accepted** and will be processed
- It is an explicit signal telling the caller to **slow down**
- Handlers **may or may not** choose to respect this signal
- Unlike errors, backpressure does not indicate failure or require retry

### Methods That Return Result

| Concept | Method | Returns |
|---------|--------|--------|
| `InboundHandler` | `onRead(ctx, msg)` | `Result` |
| `OutboundHandler` | `onWrite(ctx, msg)` | `Result` |
| `HeadEndpointHandler` | `onWrite(msg)` | `Result` |
| `TailEndpointHandler` | `onRead(msg)` | `Result` |
| `ClientOutboundAppAdapter` | `write(msg)` | `Result` |

### Methods That Return void

Exception handling methods return `void` because exceptions represent terminal
failures that must always be handled/propagated — they are not flow control signals:

| Concept | Method | Returns |
|---------|--------|--------|
| `InboundHandler` | `onException(ctx, e)` | `void` |
| `ContextApi` | `fireException(e)` | `void` |

### Basic Example

```cpp
Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    if (buffer_full()) {
        return Result::Backpressure;  // Tell upstream to slow down
    }

    auto result = process(std::move(msg));
    return ctx.fireRead(std::move(result));  // Propagate downstream result
}
```

### Transport Integration

When backpressure propagates to the transport layer:

```cpp
// OutboundTransportHandler controls read flow
void pauseRead() noexcept;   // Stop reading from socket
void resumeRead() noexcept;  // Resume reading from socket
```

### Why Return Values?

- **Simpler than Netty**: No listener registration or future callbacks
- **Synchronous propagation**: Backpressure signal is immediate
- **Compile-time enforced**: Cannot forget to handle Result
- **Zero overhead**: No allocations for signaling

---

## Explicit Backpressure API

While return values signal backpressure, handlers often need to **buffer messages**
and **retry when ready**. The explicit backpressure API provides this capability
through intrusive list hooks and ready callbacks.

### The Problem

When a handler receives `Result::Backpressure` from downstream, it knows the
operation was accepted but should slow down. The handler has several choices:

1. **Ignore the signal** — Continue at current rate (may cause further backpressure)
2. **Propagate upstream** — Let the caller decide how to slow down
3. **Buffer future messages** — Absorb backpressure and retry when ready

Option 3 requires a mechanism to notify the handler when it can resume sending.

### Solution: Write Ready Hooks

Handlers that need write ready notifications embed a hook member:

```cpp
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>

class BufferingHandler {
 public:
  // Embed hook as public member — detected at compile time
  WriteReadyHook write_ready_hook_;

  // ... handler methods
};
```

The pipeline automatically detects this hook via `requires` expressions when
the handler is added, enabling O(1) registration/unregistration.

### Context API for Write Ready Notifications

| Method | Description |
|--------|-------------|
| `ctx.awaitWriteReady()` | Register for `onWriteReady()` callback |
| `ctx.cancelAwaitWriteReady()` | Unregister from write ready list |

### Handler Callback Method

| Method | Called When |
|--------|-------------|
| `onWriteReady(ctx)` | Transport becomes writable (socket buffer drained) |

### Read Backpressure

Read backpressure is handled at the transport level via TCP flow control.
When a handler returns `Result::Backpressure` from `onRead()`, the transport
adapter should call `pauseRead()` on the socket. There is no handler-to-handler
read backpressure signaling — this follows the Netty model where read flow is
controlled by `setAutoRead(false)` at the transport level.

### Complete Example: Write Buffering Handler

```cpp
class WriteBufferingHandler {
 public:
  // Hook for write ready notifications
  WriteReadyHook write_ready_hook_;

  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    // If we have pending messages, buffer this one too
    if (!pending_.empty()) {
      pending_.push_back(std::move(msg));
      return Result::Success;  // Absorbed into buffer
    }

    // Try to send immediately
    Result result = ctx.fireWrite(std::move(msg));

    if (result == Result::Backpressure) {
      // Message was accepted, but downstream signals to slow down.
      // Buffer future messages until backpressure clears.
      // Note: The current message was already accepted and will be processed.

      // Register for notification when we should resume sending
      ctx.awaitWriteReady();

      // Tell upstream we're handling flow control
      return Result::Success;
    }

    return result;
  }

  void onWriteReady(Context& ctx) noexcept {
    // Drain buffered messages
    while (!pending_.empty()) {
      auto msg = std::move(pending_.front());
      pending_.pop_front();

      Result result = ctx.fireWrite(std::move(msg));

      if (result == Result::Backpressure) {
        // Message accepted but still signaling to slow down — wait before sending more
        return;  // Stay registered for next callback
      }
    }

    // All drained — unregister
    ctx.cancelAwaitWriteReady();
  }

  // Required handler methods
  void handlerAdded(Context&) noexcept {}
  void handlerRemoved(Context&) noexcept { pending_.clear(); }
  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
  void onPipelineDeactivated(Context&) noexcept {}

 private:
  std::deque<TypeErasedBox> pending_;
};
```

### How It Works Internally

```
┌─────────────────────────────────────────────────────────────────┐
│                         PipelineImpl                            │
│                                                                 │
│  ┌─────────────────┐                                           │
│  │ write_ready_list│                                           │
│  │  (IntrusiveList)│                                           │
│  └────────┬────────┘                                           │
│           │                                                     │
│           ▼                                                     │
│  ┌────────────────┐     ┌────────────────┐                     │
│  │ Handler A hook │────►│ Handler C hook │────► (end)          │
│  │ (handlerIndex) │     │ (handlerIndex) │                     │
│  └────────────────┘     └────────────────┘                     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

1. Handler calls ctx.awaitWriteReady()
2. Hook is linked into write_ready_list (O(1) intrusive list insert)
3. Transport signals pipeline.onWriteReady()
4. Pipeline walks list, calls onWriteReady() for each registered handler
5. Handler calls ctx.cancelAwaitWriteReady() when done (O(1) unlink)
```

### Performance Characteristics

| Operation | Complexity |
|-----------|------------|
| `awaitWriteReady()` | O(1) |
| `cancelAwaitWriteReady()` | O(1) |
| `onWriteReady()` notification | O(m) where m = registered handlers |
| Handler lookup by ID (`sendRead`, `sendWrite`) | O(1) average |

The O(1) operations are achieved through:
- **Intrusive lists**: Hooks are embedded in handlers, no allocation on register
- **Handler index in hooks**: Each hook stores its handler's index for direct lookup
- **Hash table for IDs**: Handler IDs map to indices via open-addressing hash table

### Design Principles

| Principle | Implementation |
|-----------|----------------|
| **Explicit over implicit** | Handlers choose their backpressure strategy |
| **Zero allocation** | Intrusive lists, no heap allocation on register |
| **O(1) operations** | Hash table lookups, intrusive list link/unlink |
| **Compile-time detection** | Hooks detected via `requires` expressions |
| **Handler autonomy** | Each handler decides: drop, error, or buffer |

---

## Handler Lifecycle

```
┌─────────────────────────────────────────────────────┐
│ 1. handlerAdded(ctx)                               │
│    └─ Called in order handlers are added            │
│    └─ Safe to cache other contexts                  │
├─────────────────────────────────────────────────────┤
│ 2. Pipeline active                                  │
│    └─ Process messages                              │
│    └─ Cached pointers are VALID                     │
├─────────────────────────────────────────────────────┤
│ 3. handlerRemoved(ctx)                              │
│    └─ Called in REVERSE order (LIFO)                │
│    └─ Clear cached pointers                         │
│    └─ Do NOT fire events here (undefined behavior)  │
└─────────────────────────────────────────────────────┘
```

### Caching Other Handlers

```cpp
class MyHandler {
    Context* otherCtx_ = nullptr;

    void handlerAdded(Context& ctx) noexcept {
        otherCtx_ = ctx.pipeline()->context(other_tag);
    }

    void handlerRemoved(Context& ctx) noexcept {
        otherCtx_ = nullptr;
    }

    Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
        // Use cached pointer — no lookup
        return otherCtx_->fireRead(std::move(msg));
    }
};
```

### Lifecycle Guarantees

| Guarantee | Description |
|-----------|-------------|
| `handlerAdded()` order | Called in order handlers are added |
| `handlerRemoved()` order | Called in **reverse order** (LIFO) |
| `ctx.pipeline()` | Never null between added/removed |
| `pipeline->context(id)` | Returns nullptr if handler not found |
| Firing during `handlerRemoved()` | Undefined behavior |
| DelayedDestruction | Objects survive until call stack unwinds |

---

## Handler Patterns

### Handlers as Actors

Each handler in the pipeline acts like an **actor**:

- **Owns its own state** — stored as member variables
- **Receives messages** — via `onRead()` / `onWrite()`
- **Fires messages** — via `ctx.fireRead()` / `ctx.fireWrite()`
- **Single-threaded** — no locks needed

```
┌──────────────────────────────────────────────────────────────────┐
│                          Pipeline                                 │
│                                                                   │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐   │
│  │ Handler  │───▶│ Handler  │───▶│ Handler  │───▶│ Handler  │   │
│  │  (state) │◀───│  (state) │◀───│  (state) │◀───│  (state) │   │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘   │
│                                                                   │
└──────────────────────────────────────────────────────────────────┘
```

This design avoids the need for per-pipeline user context storage. If a handler
needs connection-level state, it stores it as member variables. If another handler
needs that state, it can look up the handler via `ctx.pipeline()->context(id)`.

### Example: Handler with Connection State

```cpp
class AuthHandler {
 public:
  // Connection-level state — just member variables
  void handlerAdded(Context& ctx) noexcept {}
  void handlerRemoved(Context& ctx) noexcept {}

  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    if (!authenticated_) {
      auto authResult = tryAuthenticate(msg);
      if (!authResult) {
        failedAttempts_++;
        if (failedAttempts_ >= kMaxAttempts) {
          ctx.close();
          return Result::Error;
        }
        return Result::Success;  // Consume failed auth attempt
      }
      user_ = std::move(*authResult);
      authenticated_ = true;
    }
    return ctx.fireRead(std::move(msg));
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }
```

```cpp
  // === OutboundHandler ===

  void onPipelineDeactivated(Context& ctx) noexcept {}
  void onWriteReady(Context& ctx) noexcept {}

  // Public accessor for other handlers that need user info
  const UserInfo* user() const { return authenticated_ ? &user_ : nullptr; }
  bool isAuthenticated() const { return authenticated_; }

 private:
  static constexpr int kMaxAttempts = 3;

  bool authenticated_{false};
  int failedAttempts_{0};
  UserInfo user_;
};
```

### Example: Handler Accessing Another Handler's State

```cpp
HANDLER_TAG(auth);
HANDLER_TAG(business);

class BusinessHandler {
 public:
  void handlerAdded(Context& ctx) noexcept {
    // Cache pointer to auth handler for hot path
    if (auto* authCtx = ctx.pipeline()->context(auth_tag)) {
      // Get handler pointer through the node (implementation-specific)
      authHandler_ = getHandler<AuthHandler>(authCtx);
    }
  }

  void handlerRemoved(Context& ctx) noexcept {
    authHandler_ = nullptr;
  }

  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    // Access auth handler's state
    if (authHandler_ && authHandler_->isAuthenticated()) {
      const auto* user = authHandler_->user();
      // Use user info for authorization, logging, etc.
      return processRequest(ctx, std::move(msg), user);
    }
    return Result::Error;  // Not authenticated
  }

  // ... other handler methods

 private:
  AuthHandler* authHandler_{nullptr};
};
```

### Stateful Handlers with Phase Transitions

Pipelines are **static after construction** — handlers cannot be dynamically added
or removed. For protocols that require different behavior across phases (e.g., setup,
negotiation, steady-state), use **internal state transitions** instead.

This pattern is nearly zero-cost: after the initial phase completes, the branch
predictor learns the pattern and the check becomes ~1 cycle.

### Example: Two-Phase Setup Handler

```cpp
class SetupHandler {
 public:
  void handlerAdded(Context& ctx) noexcept {}
  void handlerRemoved(Context& ctx) noexcept {}

  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    if (FOLLY_UNLIKELY(!setupComplete_)) {
      // First message(s): process setup frame
      auto result = processSetup(msg);
      if (result == SetupResult::Complete) {
        setupComplete_ = true;
        negotiatedVersion_ = result.version;
        maxFrameSize_ = result.maxFrameSize;
      }
      return Result::Success;  // Consume setup frame
    }

    // After setup: passthrough (nearly free — predicted branch)
    return ctx.fireRead(std::move(msg));
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  void onPipelineDeactivated(Context& ctx) noexcept {}
  void onWriteReady(Context& ctx) noexcept {}

  // Accessors for negotiated parameters
  int negotiatedVersion() const { return negotiatedVersion_; }
  size_t maxFrameSize() const { return maxFrameSize_; }

 private:
  bool setupComplete_{false};
  int negotiatedVersion_{0};
  size_t maxFrameSize_{0};
};
```

### Example: Multi-Phase Protocol State Machine

For protocols with multiple phases (e.g., RSocket: setup → resume → ready):

```cpp
class RSocketHandler {
 public:
  void handlerAdded(Context& ctx) noexcept {}
  void handlerRemoved(Context& ctx) noexcept {}

  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    switch (phase_) {
      case Phase::Ready:
        // Hot path — put common case first for better codegen
        return handleRequest(ctx, std::move(msg));

      case Phase::AwaitingSetup:
        return handleSetup(ctx, std::move(msg));

      case Phase::AwaitingResume:
        return handleResume(ctx, std::move(msg));
    }
    folly::assume_unreachable();
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    // Writes may also need phase-aware handling
    if (FOLLY_UNLIKELY(phase_ != Phase::Ready)) {
      // Buffer or reject writes during setup
      return Result::Error;
    }
    return ctx.fireWrite(std::move(msg));
  }

  void onPipelineDeactivated(Context& ctx) noexcept {}
  void onWriteReady(Context& ctx) noexcept {}

 private:
  enum class Phase { AwaitingSetup, AwaitingResume, Ready };

  Result handleSetup(Context& ctx, TypeErasedBox&& msg) noexcept {
    auto& frame = msg.get<SetupFrame>();

    // Validate setup
    if (!isValidSetup(frame)) {
      sendError(ctx, ErrorCode::InvalidSetup);
      ctx.close();
      return Result::Error;
    }

    // Store connection parameters
    keepaliveInterval_ = frame.keepaliveInterval;
    resumeToken_ = frame.resumeToken;

    if (frame.resumeToken.has_value()) {
      phase_ = Phase::AwaitingResume;
    } else {
      phase_ = Phase::Ready;
    }

    return Result::Success;
  }

  Result handleResume(Context& ctx, TypeErasedBox&& msg) noexcept {
    auto& frame = msg.get<ResumeFrame>();

    if (!canResume(frame.token, frame.lastReceivedPosition)) {
      sendError(ctx, ErrorCode::RejectResume);
      ctx.close();
      return Result::Error;
    }

    // Restore state and transition to ready
    restoreState(frame);
    phase_ = Phase::Ready;

    return Result::Success;
  }

  Result handleRequest(Context& ctx, TypeErasedBox&& msg) noexcept {
    // Steady-state request processing
    return ctx.fireRead(std::move(msg));
  }

  Phase phase_{Phase::AwaitingSetup};
  std::chrono::milliseconds keepaliveInterval_{0};
  std::optional<std::string> resumeToken_;
};
```

### Performance: Why State Machines Are Nearly Free

After the initial phase completes:

| Operation | Cost |
|-----------|------|
| Branch check (`phase_ == Ready`) | ~1 cycle (predicted) |
| Mispredicted branch (rare) | ~10-20 cycles |
| Actual handler removal (if supported) | 0 cycles |

The ~1 cycle overhead is **noise** compared to:
- Network I/O: microseconds
- Serialization: hundreds of cycles
- Memory allocation: hundreds of cycles

**Tip**: Use `FOLLY_UNLIKELY()` for setup phase checks to hint the compiler:

```cpp
if (FOLLY_UNLIKELY(!ready_)) {
  return handleSetup(ctx, std::move(msg));
}
return handleRequest(ctx, std::move(msg));  // Hot path
```

### Why Not Dynamic Pipeline Modification?

Unlike Netty, this framework does not support runtime `addHandler()` / `removeHandler()`.

| Netty Approach | This Framework |
|----------------|----------------|
| Linked list of handlers | Contiguous vector (cache-friendly) |
| Dynamic add/remove at runtime | Static after construction |
| Handler becomes no-op after remove | Handler transitions to passthrough state |

**Tradeoffs**:

| Consideration | Static Pipeline | Dynamic Pipeline |
|---------------|-----------------|------------------|
| Cache locality | ✅ Contiguous storage | ❌ Linked list indirection |
| Runtime overhead | ✅ Predicted branch (~1 cycle) | ⚠️ No overhead after remove, but remove is O(n) |
| Complexity | ✅ Simple index-based dispatch | ❌ Index invalidation, ref counting |
| Flexibility | ⚠️ State machine in handler | ✅ True handler removal |

For most use cases, the state machine pattern is simpler and has equivalent
performance to removing a handler entirely.

---

## Code Standards

All code follows:
- Thrift C++ Coding Guidelines
- C++ Core Guidelines naming conventions
