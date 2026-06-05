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

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ServerAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerCompositeAppAdapter — non-owning pipeline tail that routes
 * inbound requests across an ordered list of child adapters, dispatched by
 * method name.
 *
 * Children are borrowed. The caller owns each child through its concrete
 * `T::Ptr` and is responsible for keeping children alive at least until
 * the composite is destroyed. The composite holds raw pointers + a per-T
 * thunk pair (onRead + setPipeline) so storage is heterogeneous without
 * requiring children to share a base class — they only need to satisfy
 * the ServerAppAdapter concept.
 *
 * Routing is method-name only. addChild merges each child's method names
 * into a flat map (first-wins on duplicates). Unknown methods are
 * answered with a ResponseRpcErrorCode::UNKNOWN_METHOD framework error
 * fired through the composite's own pipeline reference.
 *
 * Satisfies TailEndpointHandler. The pipeline itself must be templated on
 * ThriftServerCompositeAppAdapter so the tail concept resolves.
 */
class ThriftServerCompositeAppAdapter final : public folly::DelayedDestruction {
 public:
  using Ptr = std::unique_ptr<
      ThriftServerCompositeAppAdapter,
      folly::DelayedDestruction::Destructor>;

  ThriftServerCompositeAppAdapter() = default;

  ThriftServerCompositeAppAdapter(const ThriftServerCompositeAppAdapter&) =
      delete;
  ThriftServerCompositeAppAdapter& operator=(
      const ThriftServerCompositeAppAdapter&) = delete;
  ThriftServerCompositeAppAdapter(ThriftServerCompositeAppAdapter&&) = delete;
  ThriftServerCompositeAppAdapter& operator=(
      ThriftServerCompositeAppAdapter&&) = delete;

  // Install a callback invoked once when the pipeline goes inactive (or, as
  // a fallback, when this adapter is destroyed). Used by the owning
  // ConnectionHandler to learn that the per-connection state is done so it
  // can erase its map entry.
  void setCloseCallback(std::function<void()> cb);

  // Register a child. Caller retains ownership.
  template <typename T>
    requires ServerInboundAppAdapter<T> && ServerComposableAppAdapter<T>
  void addChild(T* child) {
    DCHECK(child != nullptr);
    for (auto name : child->methodNames()) {
      auto [_, inserted] = methodMap_.try_emplace(
          std::string(name), Entry{child, &invokeOnRead<T>});
      if (!inserted) {
        warnDuplicateMethod(name);
      }
    }
    children_.push_back(ChildHook{child, &kLifecycleVTable<T>});
  }

  // === TailEndpointHandler ===
  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept;
  void onException(folly::exception_wrapper&& e) noexcept;
  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept;
  // Drops the pipeline reference and releases the DestructorGuard taken in
  // setPipeline. Must be called before the adapter is destroyed.
  void resetPipeline() noexcept;
  channel_pipeline::PipelineImpl* pipeline() const noexcept {
    return pipeline_;
  }
  void handlerAdded() noexcept;
  void handlerRemoved() noexcept;
  void onPipelineActive() noexcept;
  void onPipelineInactive() noexcept;
  // The single pipeline event this endpoint subscribes to: the inbound
  // ConnectionClosed emitted by ThriftServerConnectionCloseHandler.
  static constexpr std::array<ThriftServerEventType, 1> kSubscribedEvents{
      ThriftServerEventType::ConnectionClosed};

  // Handles the ConnectionClosed event from the pipeline's
  // ThriftServerConnectionCloseHandler — the edge that guarantees in-flight
  // handler callbacks have settled. Fires the user closeCallback. The
  // subscription means only ConnectionClosed reaches us; our own emitted
  // CloseConnection is never self-delivered. The pipeline's
  // onPipelineInactive is no longer the close trigger — see the cpp.
  void onEvent(
      ThriftServerEventType ev,
      const channel_pipeline::TypeErasedBox& evt) noexcept;
  void onWriteReady() noexcept;

  // Initiate connection close. Internally fires a
  // ThriftServerEventType::CloseConnection pipeline event; the
  // pipeline-resident ThriftServerConnectionCloseHandler picks it up and runs
  // the terminal state machine. The user closeCallback fires when the
  // connection has fully settled. No-op if the pipeline is not wired.
  void close() noexcept;

 protected:
  ~ThriftServerCompositeAppAdapter() override;

 private:
  // Per-child dispatch surfaces. Two thunk groups:
  //   - invokeOnRead<T>     : per-request, looked up by method name
  //   - kLifecycleVTable<T> : pipeline -> child wiring + lifecycle fan-out
  template <typename T>
  static channel_pipeline::Result invokeOnRead(
      void* p, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return static_cast<T*>(p)->onRead(std::move(msg));
  }

  struct LifecycleVTable {
    void (*setPipeline)(void*, channel_pipeline::PipelineImpl*) noexcept;
    void (*resetPipeline)(void*) noexcept;
    void (*onException)(void*, folly::exception_wrapper&&) noexcept;
    void (*handlerAdded)(void*) noexcept;
    void (*handlerRemoved)(void*) noexcept;
    void (*onPipelineActive)(void*) noexcept;
    void (*onPipelineInactive)(void*) noexcept;
    void (*onWriteReady)(void*) noexcept;
  };

  template <typename T>
  static constexpr LifecycleVTable kLifecycleVTable{
      +[](void* p, channel_pipeline::PipelineImpl* pipe) noexcept {
        static_cast<T*>(p)->setPipeline(pipe);
      },
      +[](void* p) noexcept { static_cast<T*>(p)->resetPipeline(); },
      +[](void* p, folly::exception_wrapper&& e) noexcept {
        static_cast<T*>(p)->onException(std::move(e));
      },
      +[](void* p) noexcept { static_cast<T*>(p)->handlerAdded(); },
      +[](void* p) noexcept { static_cast<T*>(p)->handlerRemoved(); },
      +[](void* p) noexcept { static_cast<T*>(p)->onPipelineActive(); },
      +[](void* p) noexcept { static_cast<T*>(p)->onPipelineInactive(); },
      +[](void* p) noexcept { static_cast<T*>(p)->onWriteReady(); },
  };

  void warnDuplicateMethod(std::string_view name) const;
  FOLLY_NOINLINE channel_pipeline::Result writeUnknownMethodError(
      uint32_t streamId, std::string_view methodName) noexcept;

  struct Entry {
    void* owner;
    channel_pipeline::Result (*invoke)(
        void*, channel_pipeline::TypeErasedBox&&) noexcept;
  };
  struct ChildHook {
    void* owner;
    const LifecycleVTable* vtable;
  };

  std::vector<ChildHook> children_;
  folly::F14FastMap<std::string, Entry> methodMap_;
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  // Keeps pipeline_ alive for the composite's lifetime so late writes
  // (writeUnknownMethodError, startDrain) and onPipelineInactive's EVB
  // hop cannot dereference a freed pipeline. Released by resetPipeline().
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  folly::Synchronized<std::function<void()>> closeCallback_;
};

} // namespace apache::thrift::fast_thrift::thrift
