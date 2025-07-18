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

#include <folly/Executor.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp2/async/AsyncProcessorFactory.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessorFunc.h>
#include <thrift/lib/cpp2/async/processor/RequestParams.h>
#include <thrift/lib/cpp2/async/processor/ServiceHandlerBase.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift {

/**
 * Base-class for generated service handlers. While AsyncProcessorFactory and
 * ServiceHandlerBase are separate layers of abstraction, generated code reuse
 * the same object for both.
 */
class ServerInterface : public virtual AsyncProcessorFactory,
                        public virtual ServiceHandlerBase {
 public:
  ServerInterface() = default;
  ServerInterface(const ServerInterface&) = delete;
  ServerInterface& operator=(const ServerInterface&) = delete;

  std::string_view getName() const {
    return nameOverride_ ? *nameOverride_ : getGeneratedName();
  }
  virtual std::string_view getGeneratedName() const = 0;

  [[deprecated("Replaced by getRequestContext")]] Cpp2RequestContext*
  getConnectionContext() const {
    return requestParams_.requestContext_;
  }

  Cpp2RequestContext* getRequestContext() const {
    return requestParams_.requestContext_;
  }

  [[deprecated("Replaced by setRequestContext")]] void setConnectionContext(
      Cpp2RequestContext* c) {
    requestParams_.requestContext_ = c;
  }

  void setRequestContext(Cpp2RequestContext* c) {
    requestParams_.requestContext_ = c;
  }

  void setThreadManager(concurrency::ThreadManager* tm) {
    requestParams_.threadManager_ = tm;
  }

  // For cases where caller only needs the folly::Executor* interface.
  // These calls can be replaced with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor")]] folly::Executor* getThreadManager() {
    return getHandlerExecutor();
  }

  // For cases where the caller needs the ThreadManager interface. Caller
  // needs to be refactored to replace these calls with getHandlerExecutor.
  [[deprecated("Use getHandlerExecutor")]] concurrency::ThreadManager*
  getThreadManager_deprecated() {
    return requestParams_.threadManager_;
  }

  void setHandlerExecutor(folly::Executor* executor) {
    requestParams_.handlerExecutor_ = executor;
  }

  folly::Executor* getHandlerExecutor() {
    return requestParams_.handlerExecutor_ ? requestParams_.handlerExecutor_
                                           : requestParams_.threadManager_;
  }

  folly::Executor::KeepAlive<> getBlockingThreadManager() {
    if (requestParams_.threadManager_) {
      return BlockingThreadManager::create(requestParams_.threadManager_);
    } else {
      return BlockingThreadManager::create(requestParams_.handlerExecutor_);
    }
  }

  static folly::Executor::KeepAlive<> getBlockingThreadManager(
      concurrency::ThreadManager* threadManager) {
    return BlockingThreadManager::create(threadManager);
  }

  static folly::Executor::KeepAlive<> getBlockingThreadManager(
      folly::Executor* executor) {
    return BlockingThreadManager::create(executor);
  }

  void setEventBase(folly::EventBase* eb);

  folly::EventBase* getEventBase() { return requestParams_.eventBase_; }

  void clearRequestParams() { requestParams_ = RequestParams(); }

  virtual concurrency::PRIORITY getRequestPriority(
      Cpp2RequestContext* ctx, concurrency::PRIORITY prio);
  // TODO: replace with getRequestExecutionScope.
  concurrency::PRIORITY getRequestPriority(Cpp2RequestContext* ctx) {
    return getRequestPriority(ctx, concurrency::NORMAL);
  }

  virtual concurrency::ThreadManager::ExecutionScope getRequestExecutionScope(
      Cpp2RequestContext* ctx, concurrency::PRIORITY defaultPriority) {
    concurrency::ThreadManager::ExecutionScope es(
        getRequestPriority(ctx, defaultPriority));
    return es;
  }
  concurrency::ThreadManager::ExecutionScope getRequestExecutionScope(
      Cpp2RequestContext* ctx) {
    return getRequestExecutionScope(ctx, concurrency::NORMAL);
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override {
    return {this};
  }

  /**
   * The concrete instance of MethodMetadata that generated AsyncProcessors
   * expect will be passed to them. Therefore, generated service handlers will
   * also create instances of these for entries in
   * AsyncProcessorFactory::createMethodMetadata.
   */
  template <typename Processor>
  struct GeneratedMethodMetadata final
      : public AsyncProcessorFactory::MethodMetadata {
    GeneratedMethodMetadata(
        AsyncProcessorFunc::ProcessFuncs<Processor> funcs,
        ExecutorType executor,
        InteractionType interaction,
        RpcKind rpcKind,
        concurrency::PRIORITY priority,
        const std::optional<std::string>& interactionName,
        const bool createsInteraction)
        : MethodMetadata(
              executor,
              interaction,
              rpcKind,
              priority,
              interactionName,
              createsInteraction),
          processFuncs(funcs) {}

    AsyncProcessorFunc::ProcessFuncs<Processor> processFuncs;
  };

 protected:
  folly::Executor::KeepAlive<> getInternalKeepAlive();

 private:
  class BlockingThreadManager : public folly::Executor {
   public:
    static folly::Executor::KeepAlive<> create(
        concurrency::ThreadManager* executor) {
      return makeKeepAlive(new BlockingThreadManager(executor));
    }
    static folly::Executor::KeepAlive<> create(folly::Executor* executor) {
      return makeKeepAlive(new BlockingThreadManager(executor));
    }

    void add(folly::Func f) override;

   private:
    explicit BlockingThreadManager(concurrency::ThreadManager* threadManager)
        : threadManagerKa_(folly::getKeepAliveToken(threadManager)) {}
    explicit BlockingThreadManager(folly::Executor* executor)
        : executorKa_(folly::getKeepAliveToken(executor)) {}

    bool keepAliveAcquire() noexcept override;
    void keepAliveRelease() noexcept override;

    static constexpr std::chrono::seconds kTimeout{30};
    std::atomic<size_t> keepAliveCount_{1};
    folly::Executor::KeepAlive<concurrency::ThreadManager> threadManagerKa_;
    folly::Executor::KeepAlive<folly::Executor> executorKa_;
  };

  /**
   * This variable is only used for sync calls when in a threadpool it
   * is threadlocal, because the threadpool will probably be
   * processing multiple requests simultaneously, and we don't want to
   * mix up the connection contexts.
   *
   * This threadlocal trick doesn't work for async requests, because
   * multiple async calls can be running on the same thread.  Instead,
   * use the callback->getConnectionContext() method.  This reqCtx_
   * will be NULL for async calls.
   */
  static thread_local RequestParams requestParams_;

  std::optional<std::string> nameOverride_;

 protected:
  /**
   * If set, getName will return this name instead of getGeneratedName.
   *
   * NOTE: This method will be removed soon. Do not call it directly.
   */
  void setNameOverride(std::string name) { nameOverride_ = std::move(name); }
};

} // namespace apache::thrift
