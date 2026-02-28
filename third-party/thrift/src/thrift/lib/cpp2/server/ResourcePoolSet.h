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
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#include <folly/Executor.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>

namespace apache::thrift {
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

  ~ResourcePoolSet() { stopAndJoin(); }

  // Set a specific resource pool by handle. This fails if the ResourcePool
  // already exists. This can only be called before lock() is called. This is
  // primarily used to set the defaultAsync or defaultSync resource pools.
  void setResourcePool(
      const ResourcePoolHandle& handle,
      std::unique_ptr<RequestPileInterface>&& requestPile,
      std::shared_ptr<folly::Executor> executor,
      std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
      std::optional<concurrency::PRIORITY> priorityHint = std::nullopt,
      bool joinExecutorOnStop = true);

  // Add a ResourcePool to the set by name and return a ResourcePoolHandle that
  // can be used to obtain it. This can only be called before lock() is called.
  ResourcePoolHandle addResourcePool(
      std::string_view poolName,
      std::unique_ptr<RequestPileInterface>&& requestPile,
      std::shared_ptr<folly::Executor> executor,
      std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
      std::optional<concurrency::PRIORITY> priorityHint = std::nullopt,
      bool joinExecutorOnStop = true);

  // Set a ResourcePool selection function that returns a ResourcePoolHandle or
  // reject based on the request
  void setPoolSelectionFunc(PoolSelectionFunction func);

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

  std::vector<std::string> poolsDescriptions() const;

  template <typename F>
  void forEachResourcePool(F&& f) const {
    for (auto& resourcePool : resourcePools_) {
      std::forward<F>(f)(resourcePool.get());
    }
  }

 private:
  void calculatePriorityMapping();

  std::vector<std::unique_ptr<ResourcePool>> resourcePools_;
  mutable std::mutex mutex_;
  std::atomic<bool> locked_{false};
  std::vector<std::optional<concurrency::PRIORITY>> priorities_;
  std::array<std::size_t, concurrency::N_PRIORITIES> poolByPriority_;
  PoolSelectionFunction poolSelectionFunction_{
      [](const ServerRequest&) -> SelectPoolResult {
        return SelectPoolResult{};
      }};
};

} // namespace apache::thrift
