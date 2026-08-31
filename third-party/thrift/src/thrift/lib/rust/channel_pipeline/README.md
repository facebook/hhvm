# channel_pipeline (Rust)

Write a [`fast_thrift` channel pipeline](../../cpp2/fast_thrift/channel_pipeline/README.md)
handler in Rust, synchronously or as a coroutine, and drop it into a C++ pipeline
that keeps its static dispatch and pays nothing if it does not use one.

> **Status: experimental prototype.** Built and owned by the Thrift team (oncall
> `thrift`); the Rust runtime itself is community-supported (oncall
> `rust_thrift`). The API may change or be removed without notice. Do not depend
> on it in production.

## Why this exists

`channel_pipeline` is the C++ handler chain that a `fast_thrift` connection runs
its bytes through — framing, headers, protocol, application dispatch. Adding a
stage normally means writing C++. This crate lets that stage be Rust while the
rest of the pipeline stays native: same EventBase, same zero-copy `IOBuf`, same
`TypeErasedBox` message, no serialization hop and no thread hop at the boundary.

The design constraint that shapes everything else: **the pipeline is
single-threaded.** Every callback arrives on the pipeline's EventBase thread, and
every pipeline operation must originate there. The Rust API makes that
structural rather than documented — [`CallbackContext`] is `!Send`, `!Sync`, and
lifetime-bound to the callback frame, so code that would violate the invariant
does not compile.

## The 30-second version

```rust
use channel_pipeline::{BytesPtr, CallbackContext, HandlerResult, RustHandler, RustTypeErasedBox};

pub struct CountingHandler {
    reads: u64,
}

impl RustHandler for CountingHandler {
    fn on_read(
        &mut self,
        ctx: &mut CallbackContext<'_>,
        mut msg: RustTypeErasedBox<'_>,
    ) -> HandlerResult {
        self.reads += 1;
        let bytes = msg.take::<BytesPtr>(); // recover the concrete message
        ctx.fire_read(bytes) // forward downstream
    }
}
```

Opaque inline C++ messages can be inspected without taking them out of the
pipeline box. Implement `BorrowedMessageAdapter`, borrow the callback-scoped
view with `msg.borrow::<Adapter>()`, drop the view, and call
`ctx.forward_read(msg)` or `ctx.forward_write(msg)` to forward the original box
unchanged. This path performs no message allocation or copy.

Expose a factory from the handler's own CXX bridge:

```rust
#[cxx::bridge(namespace = "my_crate")]
mod ffi {
    unsafe extern "C++" {
        include!("thrift/lib/rust/channel_pipeline/src/ffi.rs.h");
        #[namespace = "channel_pipeline_rust"]
        type RustHandlerOpaque = channel_pipeline::RustHandlerOpaque;
    }

    extern "Rust" {
        fn new_counting_handler() -> Box<RustHandlerOpaque>;
    }
}

pub fn new_counting_handler() -> Box<channel_pipeline::RustHandlerOpaque> {
    channel_pipeline::box_handler(CountingHandler { reads: 0 })
}
```

```cpp
channel_pipeline_rust::RustHandler<Context> shim(
    my_crate::new_counting_handler());
```

The alias reuses the owning bridge's allocation and drop glue. See
[`rust/RustHandler.h`](../../cpp2/fast_thrift/channel_pipeline/rust/RustHandler.h)
for the C++ shim.

## Ways a handler can answer

A callback owes the pipeline an answer. How quickly it gives one determines which
tool to reach for — and the cost ladder below is real, measured, and worth
knowing before you pick.

**1. Answer now.** Return a [`HandlerResult`] (`Success`, `Backpressure`,
`Error`). This is the cheap path: forwarding is zero-allocation and never touches
the EventBase queue.

**2. Answer later, from anywhere.** `ctx.context_handle()` captures the exact
pipeline position into a move-only, two-word `ContextHandle` that is safe to send
to another thread. Consuming it with `fire_read`, `fire_write`, or
`fire_exception` resumes the pipeline from that position — inline when called on
the EventBase, enqueued when called from a worker. It is RAII: a dropped handle
releases the native guard instead of leaking the pipeline, and delivery is
suppressed after the pipeline closes.

```rust
let handle = ctx.context_handle();
std::thread::spawn(move || {
    let answer = do_blocking_work();
    handle.fire_read(answer); // resumes at the captured position
});
HandlerResult::Success
```

**3. Answer as a coroutine.** `ctx.spawn(future, complete)` runs a
`Send + 'static` Rust future on the pipeline's own EventBase. The first poll runs
inline in the current callback, so a future that is already ready never leaves
the callback frame; later wakes schedule further polls back onto that same
EventBase. The task owns a `ContextHandle`, so the pipeline and its EventBase
stay alive until the task finishes or is cancelled.

The [`CoroReadHandle`], [`CoroWriteHandle`], and [`CoroExceptionHandle`] adapters
wrap this up so a handler body can just be an `async` closure:

```rust
let mut read = CoroReadHandle::<BytesPtr, _>::new(|message: BytesPtr| async move {
    transform(message).await
});
```

**4. Defer the current read.** Use
`ctx.spawn_deferred_read(msg, future, complete)` when async work needs the
original erased message after suspension; the completion gets a `DeferredRead`
instead of a `ContextHandle`.

For move-only native state, implement `OwnedMessageAdapter`: call `take_owned`, then restore before `forward_read`. On
a deferred result, call `DeferredRead::restore_owned` followed by `resume`; to reply instead, consume it with
`DeferredRead::fire_write` through `OutboundMessageAdapter`. Unlike `spawn`, `spawn_deferred_read` accepts
EventBase-confined futures without a `Send` bound.

## Futures are polled where they live

This is the invariant most likely to be "optimized" back into a bug, so it is
worth stating plainly.

A future may be self-referential once polled: an `async` block's state machine
can hold interior references into its own frame across an `await`. `Pin`
therefore requires the address used for the first poll to be the address used
for every later poll and for the eventual drop.

`event_base.rs` allocates the task cell and moves the future into it **before**
the future is polled at all. The inline first poll already observes the future's
final location, and the future is never relocated — whether it completes inline
or suspends.

The tempting alternative is to poll the future on the stack and only allocate if
it comes back `Pending`, which makes an already-ready future free. That is
unsound: it relocates a future that has already been polled, which is undefined
behavior for any `async` block holding a reference across an `await`. It also
happens to pass naive tests, because trivial futures hold nothing across their
awaits. `future_is_polled_at_a_stable_address` in `event_base.rs` exists to fail
if anyone reintroduces it.

What that costs, and why it is a good trade:

- One allocation per `spawn` — a single 64-byte cell holding the task state,
  scheduler, future, completion, and payload inline. Readiness is only knowable
  after a poll, and the poll may only happen in the future's final home, so a
  future that completes on its first poll still allocates its cell.
- Nothing else allocates. Wakers are borrowed and their clones are refcount
  bumps, so polling, waking, completion, and scheduling add no allocation.
- The cost is **per task, not per suspension**. An `async fn` state machine
  flattens every await point and every nested async call into that one cell, so a
  coroutine that suspends ten times across five layers is still one allocation —
  unlike a `folly::coro` chain, where each `Task` gets a frame unless HALO elides
  it.

An atomic state word serializes publication, polling, completion, and
cancellation: wakes arriving before, during, or concurrently with payload
installation coalesce into exactly one scheduled poll, and wakes after completion
are dropped. Both the inline and scheduled polls are wrapped in `catch_unwind`,
so a panicking future cannot unwind across the FFI boundary.

## Cost ladder (opt-clang-lto, medians)

Micro-benchmark pipeline (noop head → Rust handler → noop tail), 100,000
iterations × 10 repetitions:

| Path | Median | Allocation |
|------|--------|-----------|
| Native C++ handler (baseline) | 2.726 ns | 0 |
| Synchronous Rust handler, read | 15.344 ns | 0 |
| Synchronous Rust handler, write | 14.971 ns | 0 |
| Coroutine, future ready on first poll (read) | 32.814 ns | 64 B |
| Coroutine, future ready on first poll (write) | 32.421 ns | 64 B |
| `ContextHandle` resume, EventBase-local | 63.256 ns | 0 |
| Coroutine that actually suspends | 75.521 ns | 64 B + enqueue |

Two things worth noticing. A coroutine whose future is ready on the first poll is
*cheaper* than the `ContextHandle` path it would otherwise use, because it
forwards inline instead of building and firing through a handle — the ergonomic
option is not the slow option. And plain `opt` without LTO roughly doubles the
Rust forwarding figures, so compare like with like.

At the application level these differences are noise: 15 ns costs 1% of a core
only at ~670k spawns/sec/core, against a Thrift request path measured in
microseconds. Reach for the synchronous tier when a handler is genuinely hot
enough to care.

Run it yourself:

```bash
buck2 run @//mode/opt-clang-lto \
    fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench
```

The harness fails the run if the synchronous path allocates at all, if the
inline-ready coroutine path touches the EventBase queue, or if a spawn exceeds a
256-byte-per-call budget — the numbers above are enforced, not aspirational.

## Messages: the type is the identity

There is no numeric type id and no registry. A message type becomes usable from
Rust by specializing `RustMessageAdapter<T>` in C++ and implementing
[`RustMessageAdapter`] in Rust. `BytesPtr` (a zero-copy
`unique_ptr<folly::IOBuf>`) is the production adapter.

`TypeErasedBox` never crosses the FFI boundary as a value. The shim hands Rust a
*borrowed* box, and the handler either recovers the concrete message with
`take::<T>()` — a zero-copy relocate out of the inline storage — or forwards the
whole box downstream with `forward_read` / `forward_write` without ever naming
the type. Forward what you do not understand.

`take` carries no runtime tag: dev builds check it against the box's `type_info`
and panic on mismatch; release builds compile the check out, so a wrong type
reinterprets the bytes. This matches C++ `take<T>()` semantics.

## Panics stop at the boundary

Every Rust callback is wrapped in `catch_unwind` before returning to C++. Panics
on the data path (`on_read`, `on_write`) become `HandlerResult::Error` and reach
C++ as `Result::Error`. Void callbacks — lifecycle, readiness, `on_exception` —
swallow the panic and return normally; after `on_exception` the shim always fires
the exception downstream regardless. No Rust unwind ever crosses into C++.

## Scope

Available: `on_read`, `on_write`, the lifecycle callbacks (`handler_added`,
`on_pipeline_active`, `on_pipeline_inactive`, `handler_removed`), one-shot
readiness hooks (`await_read_ready` / `await_write_ready` and their
`cancel_*` / `is_awaiting_*` companions), buffer allocation helpers
(`allocate`, `copy_from_slice`, `clone_chain`, `clone_one`, `coalesced_copy`),
`close` / `is_closed`, `handler_id`, plus the `ContextHandle`, `DeferredRead`,
and coroutine continuations above.

Not available:

- **Folly's `CoroContextHandle`** — Rust drives its own futures and never awaits
  a `folly::coro::Task`. Use `spawn` or the coro adapters instead.
- **Typed events (`fireEvent`)** — the native write-completion chain and
  close-signal channel. Not exposed because no Rust consumer needs them yet;
  adding them means an append-only event enum with a `Count` sentinel and
  per-event intrusive dispatch, preserving the `!Send` context and the zero-cost
  `NoEvent` path.

## Source map

| File | Contents |
|------|----------|
| `src/lib.rs` | Crate root, public re-exports, full API documentation |
| `src/handler.rs` | `RustHandler` trait, `HandlerResult`, `NoopHandler` |
| `src/context.rs` | `CallbackContext`, `ContextHandle`, `PipelineError`, `spawn` |
| `src/coro_handler.rs` | `Coro{Read,Write,Exception}Handle` async adapters |
| `src/event_base.rs` | Future-to-EventBase task bridge; the Pin contract lives here |
| `src/adapter.rs` | `RustMessageAdapter` trait and the `BytesPtr` adapter |
| `src/erased.rs` | `RustTypeErasedBox`, `ErasedCheck` |
| `src/ffi.rs` | CXX bridge declarations, each with a `SAFETY` contract |
| `src/integration_test.rs` | End-to-end tests against a real C++ pipeline |
| `src/bench.rs` | Benchmark driver for the C++ harness |

The C++ half lives in
[`thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/`](../../cpp2/fast_thrift/channel_pipeline/rust/):
`RustHandler.h` (the shim), `CallbackContext.{h,cpp}` (the context view and
EventBase enqueue), `RustMessageAdapter.h`, and the test and benchmark harnesses.

## Build, test, benchmark

```bash
# Library
buck2 build fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline

# Unit tests, end-to-end tests against a real C++ pipeline, and the C++ shim tests
buck2 test fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline-unittest \
           fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_integration_test \
           fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:adapter_test

# Benchmark (see the cost ladder above)
buck2 run @//mode/opt-clang-lto \
    fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench
```

Before submitting, run `arc lint -a -e extra`. The `-e extra` is not optional:
it enables clippy and unused-dependency checks that CI enforces. This crate is
`#![deny(warnings)]`, so a warning is a build failure.

Source of truth is `fbcode/thrift/lib/rust/channel_pipeline/`; dirsync mirrors it
to `xplat/thrift/lib/rust/channel_pipeline/` on commit. Edit the fbcode copy only.

## Zero cost when unused

Rust support is entirely opt-in. The native `pipeline_impl` and
`channel_pipeline` targets carry no Rust or CXX link edge — only a pipeline that
puts `RustHandler<Context>` in its builder pays for the FFI and adapter. The C++
shim is a template that satisfies the pipeline's handler concepts statically, so
the pipeline itself keeps its non-virtual dispatch; the one dynamic call is on the
Rust side, through the `Box<dyn RustHandler>` inside `RustHandlerOpaque`.

All cross-language symbols live in the `channel_pipeline_rust` C++ namespace, and
the bridge exchanges only opaque types: raw `TypeErasedBox`, `ContextImpl`, and
EventBase layouts never cross the boundary.

[`CallbackContext`]: src/context.rs
[`DeferredRead`]: src/context.rs
[`HandlerResult`]: src/handler.rs
[`RustMessageAdapter`]: src/adapter.rs
[`CoroReadHandle`]: src/coro_handler.rs
[`CoroWriteHandle`]: src/coro_handler.rs
[`CoroExceptionHandle`]: src/coro_handler.rs
