/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! Rust handler bridge for the `fast_thrift` channel pipeline.
//!
//! # Status: experimental prototype
//!
//! This is an **experimental, prototype** Rust integration into
//! `channel_pipeline`, built and owned by the **Thrift team** (oncall:
//! `thrift`). It is a research/experiment effort, not a supported product:
//! the API may change or be removed without notice. Do not depend on it in
//! production.
//!
//! # Overview
//!
//! This crate provides a pay-for-use, opt-in Rust integration for the
//! `channel_pipeline` framework. It lets you write a pipeline handler in Rust
//! that participates in a C++ pipeline, either synchronously or as a coroutine.
//! All callbacks arrive on the pipeline's EventBase thread with a borrowed,
//! non-escapable [`CallbackContext`] that is structurally `!Send`/`!Sync`.
//!
//! # Scope
//!
//! The synchronous subset is `on_read`, `on_write`, lifecycle callbacks
//! (`handler_added`, `on_pipeline_active`, …), one-shot readiness hooks
//! (`await_read_ready`, `await_write_ready`), and `close`. A synchronous
//! callback returns a [`HandlerResult`] before control passes back to C++.
//!
//! Beyond that, a callback can defer its answer:
//!
//! - [`CallbackContext::context_handle`] captures the exact pipeline position
//!   into a move-only, two-word [`ContextHandle`] that is safe to send to
//!   another thread. Its consuming `fire_read`, `fire_write`, and
//!   `fire_exception` resume the pipeline from that position, executing inline
//!   when called on the EventBase and enqueueing otherwise. RAII destruction
//!   releases the native guard, so a dropped handle cannot leak the pipeline.
//! - [`CallbackContext::spawn`] starts a `Send + 'static` Rust future on the
//!   pipeline's own EventBase. The first poll runs inline in the current
//!   callback, so an already-ready future never leaves the callback frame;
//!   later wakes schedule further polls back onto that same EventBase. The task
//!   owns a [`ContextHandle`], which retains the pipeline until the task
//!   completes or is cancelled.
//! - [`CallbackContext::defer_read`] moves the intact inbound erased message
//!   and its continuation into a one-word [`DeferredRead`] token. Typed opaque
//!   views can be borrowed on the owning EventBase, and consuming `resume`
//!   forwards the original box without copying the message payload.
//! - [`CoroReadHandle`], [`CoroWriteHandle`], and [`CoroExceptionHandle`] wrap
//!   an `async` handler body so the message or error round-trips through a
//!   future and resumes the pipeline on completion.
//!
//! Still **not** exposed: Folly's `CoroContextHandle` — Rust drives its own
//! futures and never awaits a `folly::coro::Task` — and typed events
//! (`fireEvent`), covered below.
//!
//! # Futures are polled where they live
//!
//! A future may be self-referential once polled, so `Pin` requires the address
//! used for the first poll to be the address used for every later poll and for
//! the drop. The task cell is allocated and the future moved into it *before*
//! the future is polled at all, so the inline first poll already observes the
//! future's final location and the future is never relocated.
//!
//! That costs one allocation per [`CallbackContext::spawn`], since readiness is
//! only knowable after a poll and the poll may not happen anywhere but the
//! future's final home — a future completing on its first poll still allocates
//! its cell. The cell is a single 64-byte allocation holding the task state,
//! scheduler, future, completion, and payload inline; wakers are borrowed and
//! their clones are refcount bumps, so polling, waking, completion, and
//! scheduling allocate nothing further. Because an `async fn` state machine
//! flattens every await point and nested async call into that one cell, the
//! cost is per task rather than per suspension.
//!
//! # Quick start
//!
//! ```rust,ignore
//! use channel_pipeline::{BytesPtr, CallbackContext, HandlerResult, RustHandler, RustTypeErasedBox};
//!
//! pub struct MyHandler;
//!
//! impl RustHandler for MyHandler {
//!     fn on_read(&mut self, ctx: &mut CallbackContext<'_>, mut msg: RustTypeErasedBox<'_>) -> HandlerResult {
//!         let m = msg.take::<BytesPtr>(); // recover the concrete type (dev-checked)
//!         // Inspect or transform `m`, then forward downstream.
//!         ctx.fire_read(m)
//!     }
//! }
//! ```
//!
//! A handler body may instead be an `async` closure over the message, driven on
//! the pipeline's EventBase:
//!
//! ```rust,ignore
//! use channel_pipeline::{BytesPtr, CoroReadHandle};
//!
//! // Completion resumes the read from the exact captured pipeline position.
//! let mut read = CoroReadHandle::<BytesPtr, _>::new(|message: BytesPtr| async move {
//!     transform(message).await
//! });
//! ```
//!
//! To reach C++, expose a direct factory from the handler's own CXX bridge. The
//! bridge imports [`RustHandlerOpaque`] as an `extern "C++"` alias, and
//! [`box_handler`] constructs the box:
//!
//! ```rust,ignore
//! #[cxx::bridge(namespace = "my_crate")]
//! mod ffi {
//!     unsafe extern "C++" {
//!         include!("thrift/lib/rust/channel_pipeline/src/ffi.rs.h");
//!         #[namespace = "channel_pipeline_rust"]
//!         type RustHandlerOpaque = channel_pipeline::RustHandlerOpaque;
//!     }
//!
//!     extern "Rust" {
//!         fn new_my_handler() -> Box<RustHandlerOpaque>;
//!     }
//! }
//!
//! pub fn new_my_handler() -> Box<channel_pipeline::RustHandlerOpaque> {
//!     channel_pipeline::box_handler(MyHandler)
//! }
//! ```
//!
//! ```cpp
//! channel_pipeline_rust::RustHandler<Context> shim(
//!     my_crate::new_my_handler());
//! ```
//!
//! # Public API
//!
//! | Item | Role |
//! |------|------|
//! | [`RustHandler`] | Trait to implement for your handler |
//! | [`box_handler`] | Type-erases a handler for a downstream CXX factory |
//! | [`CallbackContext`] | Borrowed pipeline context — `!Send`, `!Sync`, non-escapable |
//! | [`ContextHandle`] | Move-only captured pipeline position; consuming `fire_read`/`fire_write`/`fire_exception` |
//! | [`DeferredRead`] | Move-only suspended inbound message plus its exact pipeline continuation |
//! | [`RustTypeErasedBox`] | Borrowed, type-erased message box; recover the value with `take::<T>()` |
//! | [`BytesPtr`] | Zero-copy `unique_ptr<folly::IOBuf>` adapter |
//! | [`PipelineError`] | Owned Rust error converted to `folly::exception_wrapper` |
//! | [`HandlerResult`] | FFI-stable return value (`Success`, `Backpressure`, `Error`) |
//! | [`RustMessageAdapter`] | Trait describing how a message type crosses the FFI boundary |
//! | [`BorrowedMessageAdapter`] | Callback-scoped view of an opaque inline C++ message without taking it from the box |
//! | [`OwnedMessageAdapter`] | Move owned state out of an inline C++ message and restore it later |
//! | [`CoroReadHandle`] / [`CoroWriteHandle`] / [`CoroExceptionHandle`] | Adapters from a pipeline callback to an `async` handler body |
//! | [`ContextReadMessage`] / [`ContextWriteMessage`] | Message traits describing how to resume a captured continuation |
//! | [`ErasedCheck`] | Dev-build type check for erased message recovery |
//! | [`NoopHandler`] | Pass-through handler for testing and composition |
//!
//! # Opt-in: zero cost for native pipelines
//!
//! Rust support is entirely opt-in. Pipelines that do not add a
//! `RustHandler<Context>` pay no dependency, code-size, or runtime cost. The
//! native `pipeline_impl` C++ target has no CXX or Rust link edge. Only
//! pipelines that explicitly include the shim incur the FFI and adapter costs.
//!
//! # Measured performance (opt-clang-lto)
//!
//! Benchmarked under `opt-clang-lto` on a micro-benchmark pipeline
//! (noop head → `RustHandler<NoopHandler>` → noop tail), 100,000 iterations ×
//! 10 repetitions (run `fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline_bench`):
//!
//! | Metric | Median |
//! |--------|--------|
//! | `native_read_pipeline_ns` (C++ baseline) | 2.726 ns |
//! | `rust_read_pipeline_ns` | 15.344 ns |
//! | `rust_write_pipeline_ns` | 14.971 ns |
//! | `rust_context_handle_read_ns` | 63.256 ns |
//! | `rust_ready_coro_read_ns` (future ready on first poll) | 32.814 ns |
//! | `rust_ready_coro_write_ns` | 32.421 ns |
//! | `rust_pending_coro_submit_ns` (future suspends) | 75.521 ns |
//! | Allocations on the synchronous forwarding path | 0 bytes |
//! | Allocations per spawned coroutine | 64 bytes (one task cell) |
//! | EventBase enqueues on forwarding and inline-ready coro paths | 0 |
//!
//! A coroutine whose future is ready on the first poll is cheaper than the
//! [`ContextHandle`] path it would otherwise use, because it forwards inline
//! instead of building and firing through a handle.
//!
//! These figures reflect the micro-benchmark pipeline. Production pipelines
//! with additional handlers will differ. Allocation and enqueue counts are
//! verified via jemalloc allocation counting in the benchmark harness, which
//! fails the run if the synchronous path allocates at all or a spawn exceeds a
//! 256-byte-per-call budget.
//!
//! # FFI and memory safety
//!
//! All `unsafe extern "C++"` declarations in `ffi.rs` carry explicit `SAFETY`
//! contracts. Raw pointers do not cross the public Rust API surface. The
//! `dispatch` helper wraps every data-path Rust callback in `catch_unwind`;
//! panics map to `HandlerResult::Error` (returned to C++ as `Result::Error`).
//! Void lifecycle callbacks are wrapped in `contain_with_context` and swallow
//! panics without producing a result. After `on_exception`, the C++ shim
//! always fires the exception downstream.
//!
//! # Dependency and link isolation
//!
//! - Rust crate: `fbcode//thrift/lib/rust/channel_pipeline:channel_pipeline`
//!   (depends only on `cxx` and `folly/rust/iobuf`)
//! - C++ shim: `fbcode//thrift/lib/cpp2/fast_thrift/channel_pipeline/rust:rust_handler`
//!   (depends on `pipeline_impl` + `callback_context`)
//!
//! Only pipelines that include `RustHandler<Context>` in their builder acquire
//! the Rust/CXX link edge.
//!
//! # Bridge namespace
//!
//! All cross-language symbols live in the `channel_pipeline_rust` C++ namespace.
//! The CXX bridge exchanges only opaque types — raw `TypeErasedBox`,
//! `ContextImpl`, and EventBase layouts are never exposed across the boundary.
//!
//! # Message adapter extension
//!
//! An owned type implementing [`RustMessageAdapter`] can flow through a Rust
//! handler. An opaque inline C++ type can instead implement
//! [`BorrowedMessageAdapter`] and forward the original box unchanged after the
//! view is dropped. The message type itself is the identity, so there is no
//! numeric type id and no central registry. A future Rust handler at the framing layer
//! that needs `ParsedFrame`/`ComposedFrame` should use an opaque
//! `UniquePtr<ParsedFrame>` boxed via CXX methods (never mirror the C++ layout)
//! or serialize via `cxx-thrift-utils`. The single-message-type-per-layer
//! invariant is preserved: adapters convert at the handler boundary;
//! `TypeErasedBox` itself never crosses the FFI boundary.
//!
//! # Typed events — not exposed
//!
//! Native C++ pipelines use typed events for write-completion correlation
//! (`TransportWriteComplete` → … → `RocketWriteComplete`) and connection-close
//! signaling. These are not exposed to Rust handlers today because no concrete
//! Rust handler consumer uses them. A future extension would add
//! an append-only event enum with a `Count` sentinel, static
//! `kSubscribedEvents`, and per-event intrusive dispatch — preserving
//! `CallbackContext` `!Send` semantics and the `NoEvent` zero-cost path.

#![deny(warnings)]

pub mod adapter;
pub mod context;
mod coro_handler;
pub mod erased;
mod event_base;
pub mod ffi;
pub mod handler;

#[cfg(test)]
mod context_test;

pub use adapter::BytesPtr;
pub use adapter::RustMessageAdapter;
pub use context::CallbackContext;
pub use context::ContextHandle;
pub use context::DeferredRead;
pub use context::OutboundMessageAdapter;
pub use context::PipelineError;
pub use coro_handler::ContextReadMessage;
pub use coro_handler::ContextWriteMessage;
pub use coro_handler::CoroExceptionHandle;
pub use coro_handler::CoroReadHandle;
pub use coro_handler::CoroWriteHandle;
pub use erased::BorrowedMessageAdapter;
pub use erased::ErasedCheck;
pub use erased::OwnedMessageAdapter;
pub use erased::RustTypeErasedBox;
pub use erased::StableOwnedMessageAdapter;
pub use ffi::FfiCallbackContext;
pub use ffi::FfiTypeErasedBox;
pub use ffi::RustHandlerOpaque;
pub use ffi::box_handler;
pub use handler::HandlerResult;
pub use handler::NoopHandler;
pub use handler::RustHandler;
