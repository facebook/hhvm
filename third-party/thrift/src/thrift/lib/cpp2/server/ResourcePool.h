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

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <folly/Executor.h>
#include <folly/Synchronized.h>

#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>

namespace apache::thrift {

// The ResourcePool is a single resource for executing requests. It comprises
// a request pile, an executor and a concurrency controller.
//
// The currently supported configurations are all three (for async request
// processing) or none (for synchronous request processing - denoted by thread =
// 'eb' in thrift idl).
//
// The request pile and concurrency controller are dedicated to a resource pool
// but the executor may be shared amongst multiple resource pools if desired.
class ResourcePool {
 public:
  // It is prefereble that the executor used in a resource pool has at least
  // this many priorities so that we can deprioritize tasks that start new
  // requests (compared to tasks that are continuations of existing requests).
  static constexpr unsigned int kPreferredExecutorNumPriorities = 2;

  ~ResourcePool();

  // Access to the request pile if it exists.
  std::optional<std::reference_wrapper<RequestPileInterface>> requestPile() {
    auto result = requestPile_.get();
    if (result) {
      return std::ref(*result);
    }
    return std::nullopt;
  }

  // Access to executor if it exists.
  std::optional<std::reference_wrapper<folly::Executor>> executor() {
    auto result = executor_.get();
    if (result) {
      return std::ref(*result);
    }
    return std::nullopt;
  }

  // Access tp executor as shared pointer if it exists.
  std::optional<std::shared_ptr<folly::Executor>> sharedPtrExecutor() {
    if (executor_) {
      return executor_;
    }
    return std::nullopt;
  }

  // Access to concurrency controller if it exists.
  std::optional<std::reference_wrapper<ConcurrencyControllerInterface>>
  concurrencyController() {
    auto result = concurrencyController_.get();
    if (result) {
      return std::ref(*result);
    }
    return std::nullopt;
  }

  // Get the name of this pool.
  std::string_view name() { return name_; }

  // Accept a request into the resource pool. If an empty optional is returned
  // the request is consumed (moved from), otherwise it is left intact.
  //
  // Once a request has been accepted the only outcomes should be
  // timeout/expired or executed.
  std::optional<ServerRequestRejection> accept(ServerRequest&& request);

  // Stop the resource pool. Prevent the concurrency controller from scheduling
  // new requests and join the threads in the executor to ensure all requests
  // have completed.
  void stop();

  std::string describe() const;

 private:
  friend class ResourcePoolSet;
  ResourcePool(
      std::unique_ptr<RequestPileInterface>&& requestPile,
      std::shared_ptr<folly::Executor> executor,
      std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
      std::string_view name);

  std::unique_ptr<RequestPileInterface> requestPile_;
  std::shared_ptr<folly::Executor> executor_;
  std::unique_ptr<ConcurrencyControllerInterface> concurrencyController_;
  std::string name_;
};

// A set of ResourcePools - typically used by a service to select different
// resource pools but can also be used elsewhere, for example, resources to run
// requests on special connections.
//
// A newly created ResourcePoolSet is writable and readable with synchronization
// provided by the ResourcePoolSet for access during setup. Once lock() is
// called it becomes read only (no synchronization is performed and write
// operations are prohibited).
class ResourcePoolSet {
 public:
  using PoolSelectionFunction =
      folly::Function<SelectPoolResult(const ServerRequest&)>;

  // Set a specific resource pool by handle. This fails if the ResourcePool
  // already exists. This can only be called before lock() is called. This is
  // primarily used to set the defaultAsync or defaultSync resource pools.
  void setResourcePool(
      const ResourcePoolHandle& handle,
      std::unique_ptr<RequestPileInterface>&& requestPile,
      std::shared_ptr<folly::Executor> executor,
      std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
      std::optional<concurrency::PRIORITY> priorityHint = std::nullopt);

  // Add a ResourcePool to the set by name and return a ResourcePoolHandle that
  // can be used to obtain it. This can only be called before lock() is called.
  ResourcePoolHandle addResourcePool(
      std::string_view poolName,
      std::unique_ptr<RequestPileInterface>&& requestPile,
      std::shared_ptr<folly::Executor> executor,
      std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
      std::optional<concurrency::PRIORITY> priorityHint = std::nullopt);

  // Set a ResourcePool selection function that returns a ResourcePoolHandle or
  // reject based on the request
  void setPoolSelectionFunc(PoolSelectionFunction func);

  // Returns a ResourcePool based on the request, it will return a nullptr if
  // the request was rejected.
  SelectPoolResult selectResourcePool(const ServerRequest& serverRequest);

  // Lock the ResourcePoolSet and make it read only. The thrift server
  // infrastructure will call this at the correct time during setup.
  void lock();

  // Number of requests that are currently sitting in the RequestPile
  // This is an estimate and should not be called frequently.
  size_t numQueued() const;

  // Number of requests that are being executed in the executor
  // This is an estimate and should not be called frequently.
  size_t numInExecution() const;

  size_t numPendingDeque() const;

  // Returns the handle of the ResourcePool with the supplied name if it
  // exists. This should be called once and the results cached by the caller.
  std::optional<ResourcePoolHandle> findResourcePool(
      std::string_view poolName) const;

  // Returns whether or not a ResourcePool specified by the handle exists in
  // this ResourcePoolSet
  bool hasResourcePool(const ResourcePoolHandle& handle) const;

  // Get the resource pool specified by the handle. This is an unchecked access
  // so should only be done using a ResourcePoolHandle you know to be valid for
  // the ResourcePoolSet.
  ResourcePool& resourcePool(const ResourcePoolHandle& handle) const;

  // This is provided to aid migration. It should not be used in new code.
  [[deprecated("Use resourcePool instead")]] ResourcePool&
  resourcePoolByPriority_deprecated(concurrency::PRIORITY priority) const;

  // Returns true if the ResourcePoolSet is empty.
  bool empty() const;

  // Returns the total number of threads available in this ResourcePoolSet.
  // This is inteded for reporting statistics and should not be called
  // frequently.
  std::size_t workerCount() const;

  // Returns the number of of idle threads in this ResourcePoolSet.
  // This is inteded for reporting statistics and should not be called
  // frequently.
  std::size_t idleWorkerCount() const;

  // Stop all the concurrency controllers in this ResourcePoolSet and join
  // all the executors before returning. This unlocks the ResourcePoolSet. This
  // is called by the thrift server during shutdown.
  void stopAndJoin();

  std::size_t size() const { return resourcePools_.size(); }

  // Returns a string with a human readable description of the setup of this
  // object. Do not encode any assumptions about the format of the string
  // returned.
  std::string describe() const;

 private:
  void calculatePriorityMapping();

  std::vector<std::unique_ptr<ResourcePool>> resourcePools_;
  mutable std::mutex mutex_;
  bool locked_{false};
  std::vector<std::optional<concurrency::PRIORITY>> priorities_;
  std::array<std::size_t, concurrency::N_PRIORITIES> poolByPriority_;
  PoolSelectionFunction poolSelectionFunction_{
      [](const ServerRequest&) -> SelectPoolResult {
        return SelectPoolResult{};
      }};
};

} // namespace apache::thrift
