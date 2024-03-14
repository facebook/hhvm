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
#include <chrono>
#include <fmt/core.h>
#include <folly/IntrusiveList.h>
#include <folly/SocketAddress.h>
#include <folly/executors/QueueObserver.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/Request.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/transport/core/RequestStateMachine.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>

namespace apache {
namespace thrift {

class Cpp2RequestContext;
class ResponseChannelRequest;
class RequestsRegistry;

namespace detail {

// Returns the current "tick" of the Thrift server -- a monotonically
// increasing counter that effectively determines the size of the time
// interval for each bucket in the RecentRequestCounter.
THRIFT_PLUGGABLE_FUNC_DECLARE(uint64_t, getCurrentServerTick);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<folly::WorkerProvider>,
    createIOWorkerProvider,
    folly::Executor*,
    RequestsRegistry*);
} // namespace detail

// Helper class to track recently received request counts
class RecentRequestCounter {
 public:
  static inline constexpr uint64_t kBuckets = 512ul;

  struct RequestsCount {
    int32_t arrivalCount;
    int32_t activeCount;
    int32_t overloadCount;
  };
  using Values = std::array<RequestsCount, kBuckets>;

  void increment();
  void decrement();
  Values get() const;

  /**
   * Returns the request sum for the last x ticks.

   * Here are two common approaches for calculating request counts:
   *
   * Inclusive Approach: Include the current tick in the
   * calculation of the last x ticks, and thus consider the time range from
   * the current tick to the previous x-1 ticks. This means you would
   * include the current tick in the calculation.
   *
   * Exclusive Approach: Exclude the current tick and only
   * consider the previous x ticks, and thus exclude the current tick
   * from the calculation. In this case, you would only consider the ticks
   * preceding the current tick.
   *
   * Here it uses EXCLUSIVE approach to calculate request counts for the last x
   * ticks.
   *
   * @param ticksLookback number (x) of ticks to look back for request counts.
   */
  using ArrivalSum = uint64_t;
  using OverloadSum = uint64_t;
  std::tuple<ArrivalSum, OverloadSum> getSumRequestCountsLastXTicks(
      uint64_t ticksLookback) const;

  void incrementOverloadCount();

 private:
  uint64_t getCurrentBucket() const;

  mutable uint64_t currentBucket_{};
  mutable uint64_t lastTick_{};
  mutable Values counts_{};
  mutable uint64_t currActiveCount_{};
};

/**
 * Stores a list of request stubs in memory.
 *
 * Each IO worker stores a single RequestsRegistry instance as its
 * member, so that it can intercept and insert request data into the registry.
 *
 * Note that read operations to the request list should be always executed in
 * the same thread as write operations to avoid race condition, which means
 * most of the time reads should be issued to the event base which the
 * corresponding registry belongs to, as a task.
 */
class RequestsRegistry {
 public:
  /**
   * A small piece of information associated with those thrift requests that
   * we are tracking in the registry. The stub lives alongside the request
   * in the same chunk of memory.
   * Requests Registry is just a fancy list of such DebugStubs.
   *
   * DebugStub tracks request payload to its corresponding thrift
   * request. Handles to the payloads can be optionally released by its
   * parent request registry, indicating the payload memory has been reclaimed
   * to control memory usage. DebugStub should be unlinked from lists
   * only during:
   *   1. Destruction of DebugStub.
   *   2. Memory collection from RequestsRegistry.
   */
  class DebugStub {
    friend class RequestsRegistry;
    friend class Cpp2Worker;

   public:
    DebugStub(
        RequestsRegistry& reqRegistry,
        ResponseChannelRequest& req,
        const Cpp2RequestContext& reqContext,
        std::shared_ptr<folly::RequestContext> rctx,
        protocol::PROTOCOL_TYPES protoId,
        rocket::Payload&& debugPayload,
        RequestStateMachine& stateMachine)
        : req_(&req),
          reqContext_(&reqContext),
          rctx_(std::move(rctx)),
          protoId_(protoId),
          payload_(std::move(debugPayload)),
          registry_(&reqRegistry),
          rootRequestContextId_(rctx_->getRootId()),
          stateMachine_(stateMachine) {
      reqRegistry.registerStub(*this);
    }

    /**
     * DebugStub objects are oblivious to memory collection, but they should
     * notify their owner registry when unlinking themselves.
     */
    ~DebugStub() {
      if (payload_.hasData()) {
        DCHECK(activeRequestsPayloadHook_.is_linked());
        registry_->onStubPayloadUnlinked(*this);
      }
    }

    const ResponseChannelRequest* getRequest() const { return req_; }

    const Cpp2RequestContext* getCpp2RequestContext() const {
      return reqContext_;
    }

    std::chrono::steady_clock::time_point getTimestamp() const {
      return stateMachine_.started();
    }

    std::chrono::steady_clock::time_point getFinished() const {
      return finished_;
    }

    intptr_t getRootRequestContextId() const { return rootRequestContextId_; }

    std::shared_ptr<folly::RequestContext> getRequestContext() const {
      return rctx_;
    }

    const std::string& getMethodName() const;
    const folly::SocketAddress* getLocalAddress() const;
    const folly::SocketAddress* getPeerAddress() const;

    /**
     * Clones the payload buffer to data accessors. If the buffer is already
     * released by memory collection, returns an empty unique_ptr.
     * Since RequestsRegistry doesn'y provide synchronization by default,
     * this should be called from the IO worker which also owns the same
     * RequestsRegistry.
     */
    rocket::Payload clonePayload() const { return payload_.clone(); }

    protocol::PROTOCOL_TYPES getProtoId() const { return protoId_; }

    bool getStartedProcessing() const {
      return stateMachine_.getStartedProcessing();
    }

   private:
    uint64_t getPayloadSize() const { return payload_.dataSize(); }
    void releasePayload() { payload_ = rocket::Payload(); }

    void prepareAsFinished();

    void incRef() noexcept;
    void decRef() noexcept;

    std::string methodNameIfFinished_;
    folly::SocketAddress peerAddressIfFinished_;
    folly::SocketAddress localAddressIfFinished_;
    ResponseChannelRequest* req_;
    const Cpp2RequestContext* reqContext_;
    std::shared_ptr<folly::RequestContext> rctx_;
    const protocol::PROTOCOL_TYPES protoId_;
    rocket::Payload payload_;
    std::chrono::steady_clock::time_point finished_{
        std::chrono::steady_clock::duration::zero()};
    RequestsRegistry* registry_;
    const intptr_t rootRequestContextId_;
    folly::IntrusiveListHook activeRequestsPayloadHook_;
    folly::IntrusiveListHook activeRequestsRegistryHook_;
    size_t refCount_{1};
    RequestStateMachine& stateMachine_;
  };

  class Deleter {
   public:
    Deleter(DebugStub* stub = nullptr) : stub_(stub) {}
    template <typename U>
    /* implicit */ Deleter(std::default_delete<U>&&) : stub_(nullptr) {}

    template <typename T>
    void operator()(T* p) {
      if (!stub_) {
        delete p;
      } else {
        stub_->registry_->moveToFinishedList(*stub_);
        p->~T();
        // We release ownership over the stub, but it still may be held alive
        // by reqFinishedList_
        stub_->decRef();
      }
    }

    template <typename U>
    Deleter& operator=(std::default_delete<U>&&) {
      stub_ = nullptr;
      return *this;
    }

   private:
    DebugStub* stub_;
  };

  template <typename T, typename... Args>
  static std::unique_ptr<T, Deleter> makeRequest(Args&&... args) {
    static_assert(std::is_base_of<ResponseChannelRequest, T>::value, "");
    auto offset = sizeof(std::aligned_storage_t<sizeof(DebugStub), alignof(T)>);
    DebugStub* pStub = reinterpret_cast<DebugStub*>(malloc(offset + sizeof(T)));
    T* pT = reinterpret_cast<T*>(reinterpret_cast<char*>(pStub) + offset);
    new (pT) T(pStub, std::forward<Args>(args)...);
    return std::unique_ptr<T, Deleter>(pT, pStub);
  }

  intptr_t genRootId();
  static bool isThriftRootId(intptr_t) noexcept;
  static std::string getRequestId(intptr_t rootid);

  using ActiveRequestDebugStubList =
      folly::IntrusiveList<DebugStub, &DebugStub::activeRequestsRegistryHook_>;
  using ActiveRequestPayloadList =
      folly::IntrusiveList<DebugStub, &DebugStub::activeRequestsPayloadHook_>;

  RequestsRegistry(
      uint64_t requestPayloadMem,
      uint64_t totalPayloadMem,
      uint16_t finishedRequestsLimit);
  ~RequestsRegistry();

  const ActiveRequestDebugStubList& getActive() { return reqActiveList_; }

  const ActiveRequestDebugStubList& getFinished() { return reqFinishedList_; }

  void registerStub(DebugStub& req);

  const RecentRequestCounter& getRequestCounter() const {
    return requestCounter_;
  }
  RecentRequestCounter& getRequestCounter() { return requestCounter_; }

 private:
  void moveToFinishedList(DebugStub& stub);

  void evictStubPayloads() {
    while (payloadMemoryUsage_ > payloadMemoryLimitTotal_) {
      auto& stub = nextStubToEvict();

      onStubPayloadUnlinked(stub);
      reqPayloadList_.erase(reqPayloadList_.iterator_to(stub));
      stub.releasePayload();
    }
  }
  DebugStub& nextStubToEvict() { return reqPayloadList_.front(); }
  void onStubPayloadUnlinked(const DebugStub& stub) {
    uint64_t payloadSize = stub.getPayloadSize();
    DCHECK(payloadMemoryUsage_ >= payloadSize);
    payloadMemoryUsage_ -= payloadSize;
  }
  uint32_t registryId_;
  uint64_t nextLocalId_{0};
  uint64_t payloadMemoryLimitPerRequest_;
  uint64_t payloadMemoryLimitTotal_;
  uint64_t payloadMemoryUsage_{0};
  ActiveRequestDebugStubList reqActiveList_;
  ActiveRequestPayloadList reqPayloadList_;
  ActiveRequestDebugStubList reqFinishedList_;
  uint16_t finishedRequestsCount_{0};
  uint16_t finishedRequestsLimit_;
  RecentRequestCounter requestCounter_;
};

} // namespace thrift
} // namespace apache
