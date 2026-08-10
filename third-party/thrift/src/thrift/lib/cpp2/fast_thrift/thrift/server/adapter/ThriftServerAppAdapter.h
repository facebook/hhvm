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

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
#include <folly/Portability.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerAppAdapter — base class for generated fast server handlers.
 *
 * The generated <Service>AppAdapter extends this class, inheriting pipeline
 * integration, request dispatch, and the single writeResponse write entry
 * point. Payload construction lives in ResponsePayloads.h — callers build a
 * ThriftServerResponseMessage there, then hand it to writeResponse().
 *
 * Satisfies TailEndpointHandler concept (onRead / onException) and the
 * ThriftAppAdapter concept consumed by ThriftServerCompositeAppAdapter.
 *
 * The adapter holds no terminal-phase state of its own. Close coordination
 * (drain, in-flight reap, deferred closeCallback) is owned by the
 * pipeline-resident ThriftServerConnectionCloseHandler; the adapter just
 * emits the CloseConnection event from close() and fires the user
 * closeCallback in response to the inbound ConnectionClosed event.
 *
 * Lifetime: upper-layer owners must hold the adapter Ptr until
 * closeCallback fires. After that, the adapter may persist independently
 * of the pipeline until in-flight FastHandlerCallbacks drain — each one
 * holds a DestructorGuard on the adapter, so the underlying object stays
 * valid even if the upper-layer Ptr is dropped. Once the pipeline is
 * gone (signaled to the adapter via ConnectionClosed), pipelineActive_
 * is false and straggler writeResponse calls are silently dropped.
 *
 * Threading: when a CPU executor is attached, generated method bodies run
 * off the connection's EventBase. Two refcounts reachable from a request
 * are deliberately non-atomic — this adapter's DelayedDestruction
 * guardCount_, and the ThriftConnContext refcount held via
 * ThriftRequestContext. Both are per-connection, so concurrent requests
 * would contend on them.
 *
 * The invariant that keeps that sound: *both are mutated only on this
 * adapter's EventBase.* Three things uphold it, and all three must survive
 * refactoring together —
 *   1. FastHandlerCallback is constructed on the EventBase, before any
 *      offload, so acquiring the adapter guard happens there.
 *   2. FastHandlerCallback releases on the EventBase, hopping if needed,
 *      which covers both its adapter guard and its request context.
 *   3. Moving a request context never touches the refcount, so building a
 *      response off-EventBase is safe.
 * Everything else off-EventBase must confine itself to writeResponse,
 * which does its own hop.
 */
class ThriftServerAppAdapter : public folly::DelayedDestruction {
 public:
  // Canonical owning-pointer alias. Adapters are folly::DelayedDestruction
  // objects, so they can't be deleted with plain delete. Every site that
  // owns an adapter (FastThriftServer's per-connection state, factory
  // returns, composite children) should use this alias.
  using Ptr = std::
      unique_ptr<ThriftServerAppAdapter, folly::DelayedDestruction::Destructor>;

  // Per-method handler signature for SINGLE_REQUEST_SINGLE_RESPONSE RPCs.
  // The dispatcher pre-extracts the components codegen actually needs from
  // the inbound frame so the handler doesn't have to reach back into
  // ThriftServerRequestMessage / ParsedFrame.
  //   streamId       — Rocket stream id; echo into the response so the client
  //                    can match it to the in-flight request.
  //   data           — request payload IOBuf, ready to feed into a protocol
  //                    reader for argument deserialization.
  //   protocol       — wire protocol id (Binary / Compact / ...) for the
  //                    codec.
  //   requestContext — per-request context stamped by
  //                    ThriftServerRequestContextHandler; ownership moves into
  //                    the FastHandlerCallback so it outlives async handlers.
  using RequestResponseProcessFn = void (*)(
      ThriftServerAppAdapter*,
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::ProtocolId protocol,
      std::unique_ptr<ThriftRequestContext> requestContext) noexcept;

  ThriftServerAppAdapter() = default;
  ThriftServerAppAdapter(ThriftServerAppAdapter&&) = delete;
  ThriftServerAppAdapter& operator=(ThriftServerAppAdapter&&) = delete;

  void setCloseCallback(std::function<void()> cb);

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept;

  // Attach the executor that generated dispatchers offload method bodies to.
  // When unset, dispatch stays inline on the connection's EventBase.
  //
  // Must be called before the connection starts reading — the executor is
  // read without synchronization on the dispatch path.
  void setCPUExecutor(folly::Executor::KeepAlive<> executor) noexcept {
    cpuExecutor_ = std::move(executor);
  }

  // Drops the pipeline reference and releases the DestructorGuard taken in
  // setPipeline. Must be called before the adapter is destroyed.
  //
  // EventBase-only: it mutates guardCount_ and clears evb_, both of which
  // in-flight off-EventBase work depends on. See the threading note above.
  void resetPipeline() noexcept;

  channel_pipeline::PipelineImpl* pipeline() const noexcept {
    return pipeline_;
  }

  // === TailEndpointHandler interface ===

  channel_pipeline::Result onRead(
      channel_pipeline::detail::ContextImpl&,
      channel_pipeline::TypeErasedBox&& msg) noexcept;

  void onException(folly::exception_wrapper&& e) noexcept;

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

  // The single pipeline event this endpoint subscribes to: the inbound
  // ConnectionClosed emitted by ThriftServerConnectionCloseHandler.
  static constexpr channel_pipeline::Subscriptions<
      ThriftServerEventType::ConnectionClosed>
      kSubscribedEvents{};

  // Handles the ConnectionClosed event from the pipeline's
  // ThriftServerConnectionCloseHandler — the canonical "pipeline is done"
  // edge. Clears pipelineActive_, releases pipelineGuard_ (so the pipeline
  // can die independently of any straggler FastHandlerCallbacks still holding
  // a DG on the adapter), and fires the user closeCallback. The subscription
  // means only ConnectionClosed reaches us; our own emitted CloseConnection
  // is never self-delivered.
  void onEvent(
      ThriftServerEventType ev,
      const channel_pipeline::TypeErasedBox& evt) noexcept;
  void onWriteReady() noexcept {}

  // Write entry point for callers that may be off the EventBase.
  //
  // `adapterGuard` must be a guard the caller already owns — typically the
  // one a FastHandlerCallback took at construction time. It is *moved*, never
  // constructed here: constructing a DestructorGuard bumps this adapter's
  // non-atomic guardCount_, which is only safe on the EventBase, whereas
  // moving one just transfers a pointer. Ownership rides into the hop so the
  // adapter cannot be destroyed before the write runs, and the guard is
  // released on the EventBase.
  //
  // Build the message via the free functions in
  // thrift/server/util/ResponsePayloads.h (makeResponseMessage,
  // makeErrorMessage, makeFrameworkErrorMessage, makeUnknownExceptionMessage,
  // makeSuccessResponseMessage<>, makeDeclaredExceptionMessage<>, ...).
  void writeResponse(
      ThriftServerResponseMessage&& message,
      folly::DelayedDestruction::DestructorGuard&& adapterGuard) noexcept;

  // Write entry point for callers already on the EventBase, which therefore
  // need no guard to survive a hop. DCHECKs the thread.
  void writeResponse(ThriftServerResponseMessage&& message) noexcept;

  // Initiate connection close. Internally fires a
  // ThriftServerEventType::CloseConnection pipeline event; the
  // pipeline-resident ThriftServerConnectionCloseHandler picks it up and runs
  // the terminal state machine (drain timeout → reap → force-close on stuck
  // handler callbacks). The user closeCallback fires when the connection has
  // fully settled. No-op if the pipeline is not yet wired.
  void close() noexcept;

  // Methods registered via addMethodHandler — consumed by the composite
  // to build its name -> owner routing map.
  std::vector<std::string_view> methodNames() const noexcept;

 protected:
  void addMethodHandler(
      std::string_view name, RequestResponseProcessFn handler);

  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  // Keeps pipeline_ alive while pipelineActive_ is true. Released on
  // ConnectionClosed (or in resetPipeline) so the pipeline can die
  // independently of any straggler FastHandlerCallbacks. Straggler
  // writes are gated by pipelineActive_ and never touch pipeline_
  // after release.
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  // EVB-only. Set true in setPipeline(), cleared on ConnectionClosed
  // (and on adapter destruction via resetPipeline). When false,
  // writeResponseOnEventBase drops without touching pipeline_.
  bool pipelineActive_{false};

  ~ThriftServerAppAdapter() override;

  folly::EventBase* getEventBase() const { return evb_.get(); }

  // Executor for generated dispatchers to offload to, or null when method
  // bodies should run inline on the EventBase.
  folly::Executor* cpuExecutor() const noexcept { return cpuExecutor_.get(); }

 private:
  void handleRequestResponse(ThriftServerRequestMessage&& request) noexcept;
  FOLLY_NOINLINE void handleWrongRpcKind(
      uint32_t streamId, apache::thrift::RpcKind kind) noexcept;
  FOLLY_NOINLINE void handleUnknownMethod(
      uint32_t streamId, std::string_view methodName) noexcept;
  FOLLY_NOINLINE void handleMissingPipeline() noexcept;

  // Must be called on evb_.
  void writeResponseOnEventBase(ThriftServerResponseMessage&& message) noexcept;

  folly::Executor::KeepAlive<folly::EventBase> evb_{};
  // Null unless the server was configured with a CPU executor. Written once
  // at wiring time, read on every dispatch.
  folly::Executor::KeepAlive<> cpuExecutor_{};
  folly::F14FastMap<std::string, RequestResponseProcessFn> dispatch_;
  folly::Synchronized<std::function<void()>> closeCallback_;

  void fireCloseCallback() noexcept;
};

} // namespace apache::thrift::fast_thrift::thrift
