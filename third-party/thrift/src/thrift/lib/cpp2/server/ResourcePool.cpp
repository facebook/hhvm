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

#include <stdexcept>
#include <folly/VirtualExecutor.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadPoolExecutor.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>

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
    std::string_view name)
    : requestPile_(std::move(requestPile)),
      executor_(executor),
      concurrencyController_(std::move(concurrencyController)),
      name_(name) {
  // Current preconditions - either we have all three of these or none of them
  if (requestPile_ && concurrencyController_ && executor_) {
    // This is an async pool - that's allowed.
  } else {
    // This is a sync/eb pool.
    DCHECK(!requestPile_ && !concurrencyController && !executor_);
  }
}

void ResourcePool::stop() {
  if (concurrencyController_) {
    concurrencyController_->stop();
  }
  if (executor_) {
    if (auto threadPoolExecutor =
            dynamic_cast<folly::ThreadPoolExecutor*>(executor_.get())) {
      threadPoolExecutor->join();
    } else if (
        auto virtualExecutor =
            dynamic_cast<folly::VirtualExecutor*>(executor_.get())) {
      executor_.reset();
    } else if (
        auto threadManager =
            dynamic_cast<concurrency::ThreadManager*>(executor_.get())) {
      threadManager->join();
    } else {
      auto& exe = *executor_.get();
      LOG(WARNING) << "Could not join executor threads:"
                   << folly::demangle(typeid(exe)).toStdString();
    }
  }
}

ResourcePool::~ResourcePool() {}

std::optional<ServerRequestRejection> ResourcePool::accept(
    ServerRequest&& request) {
  if (requestPile_) {
    // This pool is async, enqueue it on the requestPile
    auto maybeRejection = requestPile_->enqueue(std::move(request));
    if (maybeRejection) {
      return maybeRejection;
    }
    concurrencyController_->onEnqueued();
    return {std::nullopt};
  } else {
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
