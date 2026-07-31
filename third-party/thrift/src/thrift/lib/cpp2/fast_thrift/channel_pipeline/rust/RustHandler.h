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

#pragma once

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/CallbackContext.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustMessageAdapter.h>
#include <thrift/lib/rust/channel_pipeline/src/ffi.rs.h>

namespace channel_pipeline_rust {

/**
 * C++ handler shim that delegates synchronously to a boxed Rust handler.
 *
 * Satisfies channel_pipeline handler concepts with zero virtual dispatch.
 *
 * ## Construction and ownership
 *
 * Construct with a `rust::Box<RustHandlerOpaque>` obtained from a Rust
 * factory function exposed through the CXX bridge:
 *
 *   // In your ffi.rs (exposed via extern "Rust"):
 *   fn rust_handler_new_my_handler() -> Box<RustHandlerOpaque>
 *
 *   // On the C++ side:
 *   rust::Box<RustHandlerOpaque> box = rust_handler_new_my_handler();
 *   RustHandler<Context> shim(std::move(box));
 *
 * Add the shim to a pipeline at build time:
 *
 *   PipelineBuilder<Head, Tail, Allocator>()
 *       .addHandler<RustHandler<Context>>(my_handler_tag, std::move(shim))
 *       .build();
 *
 * The `rust::Box<RustHandlerOpaque>` owns the Rust handler for the pipeline
 * lifetime. The box is destroyed during LIFO teardown when the C++ shim
 * destructs, dropping the inner `Box<dyn RustHandler>`.
 *
 * ## Synchronous scope
 *
 * This shim is synchronous only. The C++ `ContextHandle` and
 * `CoroContextHandle` APIs — which hand off the pipeline context to an
 * external thread or coroutine — are not accessible to Rust callbacks through
 * this bridge. All Rust handler methods must complete inline before the shim
 * returns to the pipeline.
 *
 * ## Result and backpressure mapping
 *
 * Rust `HandlerResult` discriminants map directly to C++ `Result`:
 * `Success=0`, `Backpressure=1`, `Error=2`. When Rust returns `Backpressure`,
 * the shim arms the corresponding intrusive readiness hook
 * (`ctx.awaitReadReady()` / `ctx.awaitWriteReady()`) before returning. The
 * hook is one-shot and cancelled by the shim before the next Rust ready
 * callback.
 *
 * ## Error and panic containment
 *
 * Rust panics are caught by `catch_unwind` in the CXX bridge layer:
 * - `on_read`/`on_write` panics → `HandlerResult::Error` → C++ `Result::Error`.
 * - Void callbacks (lifecycle, readiness, exception) swallow panics silently.
 * C++ exceptions thrown inside noexcept methods are caught before they escape.
 * After `on_exception`, the C++ shim always forwards the exception downstream
 * via `ctx.fireException(...)` regardless of what the Rust callback did.
 *
 * ## Supported message type
 *
 * The inbound `TypeErasedBox` is handed to Rust borrowed; the Rust handler
 * recovers the concrete message itself via `RustTypeErasedBox::take` (the
 * message type is the identity -- no numeric type id, no registry), or
 * forwards the box whole without ever inspecting it. Empty boxes are rejected
 * before invoking Rust, returning `Result::Error`.
 *
 * ## Composition
 *
 * RustHandler fits transparently in a compile-time pipeline:
 *   normal C++ handler → RustHandler<Context> → normal C++ handler
 */
template <typename Context>
class RustHandler {
 public:
  apache::thrift::fast_thrift::channel_pipeline::ReadReadyHook readReadyHook_;
  apache::thrift::fast_thrift::channel_pipeline::WriteReadyHook writeReadyHook_;

  RustHandler() : RustHandler(rust_handler_new_noop()) {}

  explicit RustHandler(rust::Box<RustHandlerOpaque> rustHandler)
      : rust_handler_(std::move(rustHandler)) {}

  ~RustHandler() = default;

  RustHandler(const RustHandler&) = delete;
  RustHandler& operator=(const RustHandler&) = delete;
  RustHandler(RustHandler&&) = default;
  RustHandler& operator=(RustHandler&&) = default;

  // HandlerLifecycle concept
  void handlerAdded(Context& ctx) noexcept {
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_handler_added(*rust_handler_, callbackContext);
    });
  }

  void handlerRemoved(Context& ctx) noexcept {
    ctx.cancelAwaitReadReady();
    ctx.cancelAwaitWriteReady();
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_handler_removed(*rust_handler_, callbackContext);
    });
  }

  // InboundHandler concept
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    using apache::thrift::fast_thrift::channel_pipeline::Result;
    try {
      if (msg.empty()) {
        return Result::Error;
      }
      CallbackContext callbackContext{
          static_cast<apache::thrift::fast_thrift::channel_pipeline::detail::
                          ContextImpl&>(ctx),
          msg};
      auto result = static_cast<Result>(
          rust_handler_on_read(*rust_handler_, callbackContext, msg));
      if (result == Result::Backpressure) {
        ctx.awaitReadReady();
      }
      return result;
    } catch (...) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }
  }

  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    using apache::thrift::fast_thrift::channel_pipeline::Result;
    try {
      if (msg.empty()) {
        return Result::Error;
      }
      CallbackContext callbackContext{
          static_cast<apache::thrift::fast_thrift::channel_pipeline::detail::
                          ContextImpl&>(ctx),
          msg};
      auto result = static_cast<Result>(
          rust_handler_on_write(*rust_handler_, callbackContext, msg));
      if (result == Result::Backpressure) {
        ctx.awaitWriteReady();
      }
      return result;
    } catch (...) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }
  }

  void onReadReady(Context& ctx) noexcept {
    ctx.cancelAwaitReadReady();
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_on_read_ready(*rust_handler_, callbackContext);
    });
  }
  void onException(
      Context& ctx, folly::exception_wrapper&& exception) noexcept {
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_on_exception(*rust_handler_, callbackContext);
    });
    ctx.fireException(std::move(exception));
  }
  void onPipelineActive(Context& ctx) noexcept {
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_on_pipeline_active(*rust_handler_, callbackContext);
    });
  }

  // OutboundHandler concept
  void onWriteReady(Context& ctx) noexcept {
    ctx.cancelAwaitWriteReady();
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_on_write_ready(*rust_handler_, callbackContext);
    });
  }
  void onPipelineInactive(Context& ctx) noexcept {
    ctx.cancelAwaitReadReady();
    ctx.cancelAwaitWriteReady();
    invokeWithContext(ctx, [this](CallbackContext& callbackContext) {
      rust_handler_on_pipeline_inactive(*rust_handler_, callbackContext);
    });
  }

 private:
  template <typename F>
  static void invokeWithContext(Context& ctx, F&& callback) noexcept {
    try {
      CallbackContext callbackContext{static_cast<
          apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&>(
          ctx)};
      std::forward<F>(callback)(callbackContext);
    } catch (...) {
    }
  }

  rust::Box<RustHandlerOpaque> rust_handler_;
};

} // namespace channel_pipeline_rust
