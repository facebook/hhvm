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

//! Synchronous Rust handler bridge for the `fast_thrift` channel pipeline.
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
//! `channel_pipeline` framework. It lets you write a
//! pipeline handler in Rust that participates synchronously in a C++ pipeline.
//! All callbacks arrive on the pipeline's EventBase thread with a borrowed,
//! non-escapable [`CallbackContext`] that is structurally `!Send`/`!Sync`.
//!
//! # Synchronous-only scope
//!
//! This bridge exposes **only the synchronous subset** of the pipeline API:
//! `on_read`, `on_write`, lifecycle callbacks (`handler_added`,
//! `on_pipeline_active`, …), one-shot readiness hooks (`await_read_ready`,
//! `await_write_ready`), and `close`. The C++ `ContextHandle`
//! (EventBase-enqueue) and `CoroContextHandle` (coroutine) APIs — which hand
//! off a pipeline context to an external thread or coroutine — are **not**
//! exposed through this bridge. All Rust handler code must complete inline and
//! return a [`HandlerResult`] before control passes back to C++.
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
//! On the C++ side, expose a factory function via the CXX bridge, wrap the
//! returned `rust::Box<RustHandlerOpaque>` in a `RustHandler<Context>` shim,
//! and add it to a `PipelineBuilder`. See
//! `thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustHandler.h` for the
//! C++ shim that satisfies handler concepts with zero virtual dispatch.
//!
//! # Public API
//!
//! | Item | Role |
//! |------|------|
//! | [`RustHandler`] | Trait to implement for your handler |
//! | [`CallbackContext`] | Borrowed pipeline context — `!Send`, `!Sync`, non-escapable |
//! | [`BytesPtr`] | Zero-copy `unique_ptr<folly::IOBuf>` adapter |
//! | [`HandlerResult`] | FFI-stable return value (`Success`, `Backpressure`, `Error`) |
//! | [`RustMessageAdapter`] | Trait describing how a message type crosses the FFI boundary |
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
//! | `rust_read_pipeline_ns` | 13.525 ns |
//! | `rust_write_pipeline_ns` | 13.262 ns |
//! | Allocations on forwarding path | 0 bytes |
//! | EventBase enqueues on forwarding path | 0 |
//!
//! These figures reflect the micro-benchmark pipeline. Production pipelines
//! with additional handlers will differ. Allocation and enqueue absence is
//! verified via jemalloc allocation counting in the benchmark harness.
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
//! Any type implementing [`RustMessageAdapter`] can flow through a Rust
//! handler; the message type itself is the identity, so there is no numeric
//! type id and no central registry. A future Rust handler at the framing layer
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
//! synchronous Rust handler consumer uses them. A future extension would add
//! an append-only event enum with a `Count` sentinel, static
//! `kSubscribedEvents`, and per-event intrusive dispatch — preserving
//! `CallbackContext` `!Send` semantics and the `NoEvent` zero-cost path.

#![deny(warnings)]

pub mod adapter;
pub mod context;
pub mod erased;
pub mod ffi;
pub mod handler;

#[cfg(test)]
mod context_test;

pub use adapter::BytesPtr;
pub use adapter::RustMessageAdapter;
pub use context::CallbackContext;
pub use context::ContextHandle;
pub use context::PipelineError;
pub use erased::ErasedCheck;
pub use erased::RustTypeErasedBox;
pub use handler::HandlerResult;
pub use handler::NoopHandler;
pub use handler::RustHandler;
