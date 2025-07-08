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

#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/ServerRequestData.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/IOWorkerContext.h>
#include <thrift/lib/cpp2/server/IResourcePoolAcceptor.h>
#include <thrift/lib/cpp2/server/RequestCompletionCallback.h>

namespace apache::thrift {

namespace detail {
class ServerRequestHelper;
}

// The ServerRequest is used to hold all the information about a request that we
// need to save when queueing it in order to execute the request later.
//
// In the thrift server this is constructed on the stack on the IO thread and
// only moved into allocated storage if the resource pool that will execute it
// has a queue (request pile associated with it).
//
// We provide various methods to accommodate read only access as well as moving
// out portions of the data.
//
// We keep the accessible interface of a ServerRequest and a const ServerRequest
// as narrow as possible as this type is used in several customization points.
class ServerRequest {
 public:
  // Eventually we won't need a default ctor once there is no path that doesn't
  // use the ServerRequest and resource pools.
  ServerRequest() : serializedRequest_(std::unique_ptr<folly::IOBuf>{}) {}

  ServerRequest(
      ResponseChannelRequest::UniquePtr&& request,
      SerializedCompressedRequest&& serializedRequest,
      Cpp2RequestContext* ctx,
      protocol::PROTOCOL_TYPES protocol,
      std::shared_ptr<folly::RequestContext> follyRequestContext,
      AsyncProcessor* asyncProcessor,
      const AsyncProcessor::MethodMetadata* methodMetadata)
      : request_(std::move(request)),
        serializedRequest_(std::move(serializedRequest)),
        ctx_(ctx),
        protocol_(protocol),
        follyRequestContext_(std::move(follyRequestContext)),
        asyncProcessor_(asyncProcessor),
        methodMetadata_(methodMetadata) {}

  ServerRequest(const ServerRequest&) = delete;

  ServerRequest& operator=(const ServerRequest&) = delete;

  ServerRequest(ServerRequest&&) = default;

  ServerRequest& operator=(ServerRequest&&) = default;

  // in most cases, the completion callback should be done
  // on the thread where HandlerCallback destructor is run
  // e.g. on CPU thread.
  // This short-cut could make the callback run on different threads
  // e.g. on IO thread pool, which is ok.
  ~ServerRequest() {
    if (callbacks_.notifyRequestPile_) {
      callbacks_.notifyRequestPile_->onRequestFinished(requestData_);
    }

    if (callbacks_.notifyConcurrencyController_) {
      callbacks_.notifyConcurrencyController_->onRequestFinished(requestData_);
    }
  }

  // The public accessors are available to user code that receives the
  // ServerRequest through various customization points.

  const AsyncProcessor::MethodMetadata* methodMetadata() const {
    return methodMetadata_;
  }

  const Cpp2RequestContext* requestContext() const { return ctx_; }

  Cpp2RequestContext* requestContext() { return ctx_; }

  // TODO: T108089128 We should change this to return a ResponseChannelRequest
  // once we change downstream code to accept that instead of the
  // ResponseChannelRequest::UniquePtr&.
  const ResponseChannelRequest::UniquePtr& request() const { return request_; }

  ResponseChannelRequest::UniquePtr& request() { return request_; }

  const ServerRequestData& requestData() const { return requestData_; }

  ServerRequestData& requestData() { return requestData_; }

  const std::shared_ptr<folly::RequestContext>& follyRequestContext() const {
    return follyRequestContext_;
  }

  // Set this if the request pile should be notified (via
  // RequestPileInterfaceo::onRequestFinished) when the request is completed.
  void setRequestPileNotification(RequestCompletionCallback* requestPile) {
    DCHECK(callbacks_.notifyRequestPile_ == nullptr);
    callbacks_.notifyRequestPile_ = requestPile;
  }

  // Set this if the concurrency controller should be notified (via
  // ConcurrencyControllerInterface::onRequestFinished) when the request is
  // completed.
  void setConcurrencyControllerNotification(
      RequestCompletionCallback* concurrencyController) {
    DCHECK(callbacks_.notifyConcurrencyController_ == nullptr);
    callbacks_.notifyConcurrencyController_ = concurrencyController;
  }

 protected:
  using InternalPriority = int8_t;

  // The protected accessors are for use only by the thrift server
  // implementation itself. They are accessed using
  // detail::ServerRequestHelper.

  friend class detail::ServerRequestHelper;

  static AsyncProcessor* asyncProcessor(const ServerRequest& sr) {
    return sr.asyncProcessor_;
  }

  static SerializedCompressedRequest& compressedRequest(ServerRequest& sr) {
    return sr.serializedRequest_;
  }

  static SerializedCompressedRequest compressedRequest(ServerRequest&& sr) {
    return std::move(sr.serializedRequest_);
  }

  static ResponseChannelRequest::UniquePtr request(ServerRequest&& sr) {
    return std::move(sr.request_);
  }

  static folly::EventBase* eventBase(ServerRequest& sr) {
    return sr.ctx_->getConnectionContext()
        ->getWorkerContext()
        ->getWorkerEventBase();
  }

  // The executor is only available once the request has been assigned to
  // a resource pool.
  static folly::Executor::KeepAlive<> executor(ServerRequest& sr) {
    return sr.executor_ ? sr.executor_ : eventBase(sr);
  }

  // Only available once the request has been assigned to
  // a resource pool.
  static IResourcePoolAcceptor* resourcePool(ServerRequest& sr) {
    return sr.resourcePool_ ? sr.resourcePool_ : nullptr;
  }

  static InternalPriority internalPriority(const ServerRequest& sr) {
    return sr.priority_;
  }

  static void setExecutor(
      ServerRequest& sr, folly::Executor::KeepAlive<> executor) {
    sr.executor_ = std::move(executor);
  }

  static void setResourcePool(ServerRequest& sr, IResourcePoolAcceptor* rp) {
    sr.resourcePool_ = rp;
  }

  static void setInternalPriority(
      ServerRequest& sr, InternalPriority priority) {
    sr.priority_ = priority;
  }

  static protocol::PROTOCOL_TYPES protocol(ServerRequest& sr) {
    return sr.protocol_;
  }

  static RequestCompletionCallback* moveRequestPileNotification(
      ServerRequest& sr) {
    return std::exchange(sr.callbacks_.notifyRequestPile_, nullptr);
  }

  static RequestCompletionCallback* moveConcurrencyControllerNotification(
      ServerRequest& sr) {
    return std::exchange(sr.callbacks_.notifyConcurrencyController_, nullptr);
  }

  static intptr_t& queueObserverPayload(ServerRequest& sr) {
    return sr.queueObserverPayload_;
  }

 private:
  ResponseChannelRequest::UniquePtr request_;
  SerializedCompressedRequest serializedRequest_;
  folly::Executor::KeepAlive<> executor_{};
  Cpp2RequestContext* ctx_;
  protocol::PROTOCOL_TYPES protocol_;
  std::shared_ptr<folly::RequestContext> follyRequestContext_;
  AsyncProcessor* asyncProcessor_;
  const AsyncProcessor::MethodMetadata* methodMetadata_;
  ServerRequestData requestData_;
  intptr_t queueObserverPayload_;
  IResourcePoolAcceptor* resourcePool_{nullptr};
  InternalPriority priority_{folly::Executor::LO_PRI};
  /**
   * Small struct to hold callback pointers that need to be handled specially.
   * These callbacks are invoked during ServerRequest destruction. We only want
   * them invoked once, when the last "moved-to" object is destroyed. In the
   * move constructor and operator, swap and clear them from the "moved-from"
   * object.
   */
  struct SwapAndNullOnMove {
    SwapAndNullOnMove() = default;
    SwapAndNullOnMove(SwapAndNullOnMove&& other) noexcept {
      notifyRequestPile_ = std::exchange(other.notifyRequestPile_, nullptr);
      notifyConcurrencyController_ =
          std::exchange(other.notifyConcurrencyController_, nullptr);
    }

    SwapAndNullOnMove& operator=(SwapAndNullOnMove&& other) noexcept {
      notifyRequestPile_ = std::exchange(other.notifyRequestPile_, nullptr);
      notifyConcurrencyController_ =
          std::exchange(other.notifyConcurrencyController_, nullptr);

      return *this;
    }

    SwapAndNullOnMove(const SwapAndNullOnMove&) = delete;
    SwapAndNullOnMove& operator=(const SwapAndNullOnMove&) = delete;
    ~SwapAndNullOnMove() = default;

    RequestCompletionCallback* notifyRequestPile_{nullptr};
    RequestCompletionCallback* notifyConcurrencyController_{nullptr};
  } callbacks_;
};

} // namespace apache::thrift
