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

#include <cstddef>
#include <cstdint>
#include <memory>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

namespace channel_pipeline_rust {

/**
 * Callback-scoped view of the live C++ context and message container.
 *
 * Both references originate from RustHandler's current stack frame and remain
 * valid for exactly one synchronous Rust callback. Rust receives only a pinned
 * mutable borrow, cannot retain it in the safe public API, and calls it only on
 * the invoking pipeline thread. The exclusive borrow covers both references.
 *
 * Methods are noexcept, retain nothing, and may forward the original box once.
 *
 * The Rust handler now receives the borrowed `TypeErasedBox` and recovers the
 * concrete message itself via `RustTypeErasedBox::take`, which leaves the box
 * empty. `fireRead`/`fireWrite` therefore restore the returned message into
 * that same empty box via `RustMessageAdapter<BytesPtr>::tryRestore`,
 * preserving the same-box fast path. `forwardRead`/`forwardWrite` move the box
 * on untouched for handlers that never recover the type.
 *
 * Phase 3 additions:
 * - handlerId(): stable FNV-64 identity set at build time. Never 0 for a valid
 *   installed handler; 0 is the error sentinel for exception paths.
 * - allocate(size): delegates to PipelineImpl::allocate (pipeline allocator or
 *   IOBuf::create fallback). noexcept, returns null on failure (None in Rust).
 *   Zero and boundary sizes valid and produce an empty chain when underlying
 *   allocator honors them.
 * - copyBuffer(data,size): deep copy via IOBuf::copyBuffer, cost allocation +
 *   memcpy, ignores pipeline allocator, zero-length returns empty IOBuf, null
 *   data with nonzero size returns null (None in Rust).
 * - cloneBufferChain / cloneOne: shallow share (refcount increment), cheap, no
 *   payload copy.
 * - coalescedCopy: deep copy of whole chain into single contiguous IOBuf, cost
 *   allocation + memcpy of chainDataLength. Use only when contiguous needed.
 * - close(): idempotent terminal close, clears ready lists + event lists and
 *   fires handlerRemoved reverse; noexcept.
 * - isClosed(): true after close or when owning pipeline pointer null.
 */
class CallbackContext final {
 public:
  CallbackContext(
      apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&
          context,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          message) noexcept
      : context_{context}, message_{&message} {}

  explicit CallbackContext(
      apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&
          context) noexcept
      : context_{context} {}

  CallbackContext(const CallbackContext&) = delete;
  CallbackContext& operator=(const CallbackContext&) = delete;

  int32_t fireRead(std::unique_ptr<folly::IOBuf> message) noexcept;
  int32_t fireWrite(std::unique_ptr<folly::IOBuf> message) noexcept;

  // Forward the message downstream UNCHANGED, without recovering its type.
  // This is the "forward what you don't understand" (Netty pass-through)
  // primitive: the whole TypeErasedBox is moved on, so the handler need not
  // know the concrete type. One-shot, guarded by `forwarded_`, like fireRead.
  int32_t forwardRead() noexcept;
  int32_t forwardWrite() noexcept;

  void awaitReadReady() noexcept;
  void cancelReadReady() noexcept;
  bool isAwaitingReadReady() const noexcept;
  void awaitWriteReady() noexcept;
  void cancelWriteReady() noexcept;
  bool isAwaitingWriteReady() const noexcept;
  uint64_t handlerId() const noexcept;
  std::unique_ptr<folly::IOBuf> allocate(size_t size) noexcept;
  std::unique_ptr<folly::IOBuf> copyBuffer(
      const uint8_t* data, size_t size) noexcept;
  std::unique_ptr<folly::IOBuf> cloneBufferChain(
      const folly::IOBuf& buffer) noexcept;
  std::unique_ptr<folly::IOBuf> cloneOne(const folly::IOBuf& buffer) noexcept;
  std::unique_ptr<folly::IOBuf> coalescedCopy(
      const folly::IOBuf& buffer) noexcept;
  void close() noexcept;
  bool isClosed() const noexcept;

 private:
  apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl& context_;
  apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox* message_{
      nullptr};
  bool forwarded_{false};
};

} // namespace channel_pipeline_rust
