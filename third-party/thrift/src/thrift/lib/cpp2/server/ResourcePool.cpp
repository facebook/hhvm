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

#include <chrono>
#include <stdexcept>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/MeteredExecutor.h>
#include <folly/executors/ThreadPoolExecutor.h>
#include <folly/executors/VirtualExecutor.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>

using namespace std::chrono_literals;

namespace apache::thrift {
namespace {
std::string describeExecutor(std::shared_ptr<folly::Executor> executor) {
  if (auto ex = executor.get()) {
    auto& exref = *ex;
    auto executorName = folly::demangle(typeid(exref)).toStdString();
    if (auto* cpuThreadPoolExecutor =
            dynamic_cast<folly::CPUThreadPoolExecutor*>(executor.get())) {
      return fmt::format(
          "{{{} numThreads={}}}",
          executorName,
          cpuThreadPoolExecutor->numThreads());
    }
    return fmt::format("{{{}}}", executorName);
  } else {
    return "None";
  }
}

serverdbginfo::ExecutorDbgInfo getExecutorDbgInfo(folly::Executor* executor) {
  serverdbginfo::ExecutorDbgInfo executorDbgInfo;
  executorDbgInfo.name() = folly::demangle(typeid(*executor)).toStdString();

  if (auto* threadPoolExecutor =
          dynamic_cast<folly::ThreadPoolExecutor*>(executor)) {
    executorDbgInfo.threadsCount() = threadPoolExecutor->numThreads();
  }

  return executorDbgInfo;
}
} // namespace

// ResourcePool

ResourcePool::ResourcePool(
    std::unique_ptr<RequestPileInterface>&& requestPile,
    std::shared_ptr<folly::Executor> executor,
    std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
    std::string_view name,
    bool joinExecutorOnStop)
    : requestPile_(std::move(requestPile)),
      executor_(executor),
      concurrencyController_(std::move(concurrencyController)),
      name_(name),
      joinExecutorOnStop_(joinExecutorOnStop) {
  DCHECK(isAsyncPool() || isDeferredPool() || isSyncPool());
}

void ResourcePool::stop() {
  if (concurrencyController_) {
    concurrencyController_->stop();
  }
  if (executor_ && joinExecutorOnStop_) {
    if (auto threadPoolExecutor =
            dynamic_cast<folly::ThreadPoolExecutor*>(executor_.get())) {
      threadPoolExecutor->join();
    } else if (dynamic_cast<folly::VirtualExecutor*>(executor_.get())) {
      // since we're dealing with an executor wrapper it's sufficient to just
      // release the pointer
      executor_.reset();
    } else if (dynamic_cast<folly::MeteredExecutor*>(executor_.get())) {
      // since we're dealing with an executor wrapper it's sufficient to just
      // release the pointer
      executor_.reset();
    } else if (
        auto threadManager =
            dynamic_cast<concurrency::ThreadManager*>(executor_.get())) {
      threadManager->join();
    } else {
      // This ResourcePool is using an executor type that is not known to
      // this method. As such, it is the responsibility of the user to ensure
      // that the executor is stopped, but this ResourcePool was created with
      // joinExecutorOnStop = true.
      auto& exe = *executor_.get();
      XLOG_N_PER_MS(WARNING, 10, 1min)
          << "ResourcePool \"" << name_
          << "\" could not join executor threads. Executor type: "
          << folly::demangle(typeid(exe)).toStdString();
    }
  }
}

ResourcePool::~ResourcePool() {}

bool ResourcePool::isAsyncPool() {
  return requestPile_ != nullptr && executor_ != nullptr &&
      concurrencyController_ != nullptr;
}

bool ResourcePool::isDeferredPool() {
  return requestPile_ == nullptr && executor_ != nullptr &&
      concurrencyController_ == nullptr;
}

bool ResourcePool::isSyncPool() {
  return requestPile_ == nullptr && executor_ == nullptr &&
      concurrencyController_ == nullptr;
}

std::optional<ServerRequestRejection> ResourcePool::acceptAsync(
    ServerRequest&& request) {
  // Enqueue the request to be scheduled for proecessing by the concurrency
  // controller.
  auto maybeRejection = requestPile_->enqueue(std::move(request));
  if (maybeRejection) {
    return maybeRejection;
  }
  concurrencyController_->onEnqueued();
  return {std::nullopt};
}

std::optional<ServerRequestRejection> ResourcePool::acceptDeferred(
    ServerRequest&& request) {
  // Deferr the request to the executor.
  executor_->add([request = std::move(request)]() mutable {
    if (!request.request()->isOneway() &&
        !request.request()->getShouldStartProcessing()) {
      auto eb = detail::ServerRequestHelper::eventBase(request);
      HandlerCallbackBase::releaseRequest(
          detail::ServerRequestHelper::request(std::move(request)), eb);
    } else {
      request.requestData().setRequestExecutionBegin();
      AsyncProcessorHelper::executeRequest(std::move(request));
      request.requestData().setRequestExecutionEnd();
    }
  });
  return {std::nullopt};
}

std::optional<ServerRequestRejection> ResourcePool::acceptSync(
    ServerRequest&& request) {
  // Trigger processing of request and check for queue timeouts.
  if (!request.request()->getShouldStartProcessing()) {
    auto eb = detail::ServerRequestHelper::eventBase(request);
    HandlerCallbackBase::releaseRequest(
        detail::ServerRequestHelper::request(std::move(request)), eb);
    return {std::nullopt};
  }

  // This pool is sync, just now we execute the request inline.
  AsyncProcessorHelper::executeRequest(std::move(request));
  return {std::nullopt};
}

std::optional<ServerRequestRejection> ResourcePool::accept(
    ServerRequest&& request) {
  if (isAsyncPool()) {
    return acceptAsync(std::move(request));
  } else if (isDeferredPool()) {
    return acceptDeferred(std::move(request));
  } else {
    DCHECK(isSyncPool());
    return acceptSync(std::move(request));
  }
}

std::string ResourcePool::describe() const {
  return fmt::format(
      "{{ResourcePool name={}, requestPile={}, concurrencyController={}, executor={}}}",
      name_,
      requestPile_ ? requestPile_->describe() : "None",
      concurrencyController_ ? concurrencyController_->describe() : "None",
      describeExecutor(executor_));
}

serverdbginfo::ResourcePoolDbgInfo ResourcePool::getDbgInfo() const {
  serverdbginfo::ResourcePoolDbgInfo info;
  info.name() = name_;

  if (requestPile_) {
    info.requestPileDbgInfo() = requestPile_->getDbgInfo();
  }

  if (concurrencyController_) {
    info.concurrencyControllerDbgInfo() = concurrencyController_->getDbgInfo();
  }

  if (auto executor = executor_.get()) {
    info.executorDbgInfo() = getExecutorDbgInfo(executor);
  }

  return info;
}
} // namespace apache::thrift
