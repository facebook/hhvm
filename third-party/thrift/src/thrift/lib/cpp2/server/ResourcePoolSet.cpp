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

#include <folly/Executor.h>
#include <folly/executors/ThreadPoolExecutor.h>
#include <thrift/lib/cpp2/server/ResourcePoolSet.h>

namespace apache::thrift {
void ResourcePoolSet::setResourcePool(
    const ResourcePoolHandle& handle,
    std::unique_ptr<RequestPileInterface>&& requestPile,
    std::shared_ptr<folly::Executor> executor,
    std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
    std::optional<concurrency::PRIORITY> priorityHint_deprecated) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (locked_) {
    throw std::logic_error("Cannot setResourcePool() after lock()");
  }

  resourcePools_.resize(std::max(resourcePools_.size(), handle.index() + 1));
  if (resourcePools_.at(handle.index())) {
    LOG(ERROR) << "Cannot overwrite resourcePool:" << handle.name();
    throw std::invalid_argument("Cannot overwrite resourcePool");
  }
  std::unique_ptr<ResourcePool> pool{new ResourcePool{
      std::move(requestPile),
      executor,
      std::move(concurrencyController),
      handle.name()}};
  resourcePools_.at(handle.index()) = std::move(pool);

  priorities_.resize(std::max(priorities_.size(), handle.index() + 1));
  priorities_.at(handle.index()) = priorityHint_deprecated;
}

ResourcePoolHandle ResourcePoolSet::addResourcePool(
    std::string_view poolName,
    std::unique_ptr<RequestPileInterface>&& requestPile,
    std::shared_ptr<folly::Executor> executor,
    std::unique_ptr<ConcurrencyControllerInterface>&& concurrencyController,
    std::optional<concurrency::PRIORITY> priorityHint_deprecated) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (locked_) {
    throw std::logic_error("Cannot addResourcePool() after lock()");
  }

  std::unique_ptr<ResourcePool> pool{new ResourcePool{
      std::move(requestPile),
      executor,
      std::move(concurrencyController),
      poolName}};
  // Ensure that any default slots have been initialized (with empty unique_ptr
  // if necessary).
  resourcePools_.resize(std::max(
      resourcePools_.size(), ResourcePoolHandle::kMaxReservedIndex + 1));
  resourcePools_.emplace_back(std::move(pool));

  priorities_.resize(
      std::max(priorities_.size(), ResourcePoolHandle::kMaxReservedIndex + 1));
  priorities_.emplace_back(priorityHint_deprecated);

  return ResourcePoolHandle::makeHandle(poolName, resourcePools_.size() - 1);
}

void ResourcePoolSet::setPoolSelectionFunc(PoolSelectionFunction func) {
  poolSelectionFunction_ = std::move(func);
}

SelectPoolResult ResourcePoolSet::selectResourcePool(
    const ServerRequest& serverRequest) {
  return poolSelectionFunction_(serverRequest);
}

void ResourcePoolSet::lock() {
  std::lock_guard<std::mutex> lock(mutex_);
  locked_ = true;
  // Whilst we still have ThreadManager we may lock an empty ResourcePoolSet.
  // Eventually we should make that a fatal error.
  if (!empty()) {
    calculatePriorityMapping();
  }
}

size_t ResourcePoolSet::numQueued() const {
  if (!locked_) {
    return 0;
  }
  size_t sum = 0;
  for (auto& pool : resourcePools_) {
    if (auto rp = pool->requestPile()) {
      sum += rp.value().get().requestCount();
    }
    if (pool->executor()) {
      if (auto* tm = dynamic_cast<concurrency::ThreadManager*>(
              &pool->executor()->get())) {
        sum += tm->pendingUpstreamTaskCount();
      }
    }
  }
  return sum;
}

size_t ResourcePoolSet::numInExecution() const {
  if (!locked_) {
    return 0;
  }
  size_t sum = 0;
  for (auto& pool : resourcePools_) {
    if (auto cc = pool->concurrencyController()) {
      sum += cc.value().get().requestCount();
    }
  }
  return sum;
}

size_t ResourcePoolSet::numPendingDeque() const {
  if (!locked_) {
    return 0;
  }
  size_t sum = 0;
  for (auto& pool : resourcePools_) {
    if (auto cc = pool->concurrencyController()) {
      sum += cc.value().get().numPendingDequeRequest();
    }
  }
  return sum;
}

std::optional<ResourcePoolHandle> ResourcePoolSet::findResourcePool(
    std::string_view poolName) const {
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);
  for (std::size_t i = 0; i < resourcePools_.size(); ++i) {
    if (resourcePools_.at(i) && resourcePools_.at(i)->name() == poolName) {
      if (i == ResourcePoolHandle::kDefaultSyncIndex) {
        return ResourcePoolHandle::defaultSync();
      } else if (i == ResourcePoolHandle::kDefaultAsyncIndex) {
        return ResourcePoolHandle::defaultAsync();
      }
      return ResourcePoolHandle::makeHandle(poolName, i);
    }
  }
  return std::nullopt;
}

bool ResourcePoolSet::hasResourcePool(const ResourcePoolHandle& handle) const {
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);
  if (handle.index() >= resourcePools_.size()) {
    return false;
  }
  return static_cast<bool>(resourcePools_[handle.index()]);
}

ResourcePool& ResourcePoolSet::resourcePool(
    const ResourcePoolHandle& handle) const {
  folly::annotate_ignore_thread_sanitizer_guard g(__FILE__, __LINE__);
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);
  DCHECK_LT(handle.index(), resourcePools_.size());
  DCHECK(resourcePools_[handle.index()]);
  return *resourcePools_[handle.index()];
}

ResourcePool& ResourcePoolSet::resourcePoolByPriority_deprecated(
    concurrency::PRIORITY priority) const {
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);
  DCHECK_LT(poolByPriority_[priority], resourcePools_.size());
  DCHECK(resourcePools_[poolByPriority_[priority]]);
  return *resourcePools_[poolByPriority_[priority]];
}

bool ResourcePoolSet::empty() const {
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);
  return resourcePools_.empty();
}

std::size_t ResourcePoolSet::workerCount() const {
  if (!locked_) {
    return 0;
  }

  std::size_t workers = 0;
  for (std::size_t i = 0; i < resourcePools_.size(); ++i) {
    if (resourcePools_[i]->executor()) {
      if (auto* tpe = dynamic_cast<folly::ThreadPoolExecutor*>(
              &resourcePools_[i]->executor()->get())) {
        // Return the configured number of threads not the dynamic number.
        workers += tpe->numThreads();
      } else if (
          auto* tm = dynamic_cast<concurrency::ThreadManager*>(
              &resourcePools_[i]->executor()->get())) {
        workers += tm->workerCount();
      }
    }
  }

  return workers;
}

std::size_t ResourcePoolSet::idleWorkerCount() const {
  if (!locked_) {
    return 0;
  }

  std::size_t idle = 0;
  for (std::size_t i = 0; i < resourcePools_.size(); ++i) {
    if (resourcePools_[i]->executor()) {
      if (auto* tpe = dynamic_cast<folly::ThreadPoolExecutor*>(
              &resourcePools_[i]->executor()->get())) {
        auto poolStats = tpe->getPoolStats();
        // PoolStats idleThreadCount correctly reflects threads which are
        // configured in the executor but not running as idle.
        idle += poolStats.idleThreadCount;
      } else if (
          auto* tm = dynamic_cast<concurrency::ThreadManager*>(
              &resourcePools_[i]->executor()->get())) {
        idle += tm->idleWorkerCount();
      }
    }
  }

  return idle;
}

void ResourcePoolSet::stopAndJoin() {
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);

  for (auto& resourcePool : resourcePools_) {
    if (resourcePool) {
      resourcePool->stop();
    }
  }
}

std::string ResourcePoolSet::describe() const {
  std::string result;
  auto guard = locked_ ? std::unique_lock<std::mutex>()
                       : std::unique_lock<std::mutex>(mutex_);

  auto resourcePoolsString = [this]() -> std::string {
    if (resourcePools_.size() == 0) {
      return "None";
    }

    std::string result;
    for (std::size_t i = 0; i < resourcePools_.size(); i++) {
      if (resourcePools_.at(i)) {
        auto& rp = resourcePools_.at(i);
        result += fmt::format(
            "{{{} {} priority={}}} ",
            i,
            rp->describe(),
            priorities_[i] ? std::to_string(priorities_[i].value()) : "None");
      } else {
        result += fmt::format("{{{} None}}", i);
      }
    }
    return result;
  };

  auto priorityMapString = [this]() -> std::string {
    if (resourcePools_.size() == 0) {
      return "None";
    }

    std::string result;
    for (auto i = 0; i < concurrency::N_PRIORITIES; ++i) {
      result +=
          fmt::format(" {}={}", i, resourcePools_[poolByPriority_[i]]->name());
    }
    return result;
  };

  return fmt::format(
      "{{ResourcePoolSet resourcePools={}, locked={}, priorityMap={}}}",
      resourcePoolsString(),
      locked_,
      priorityMapString());
}

std::vector<std::string> ResourcePoolSet::poolsDescriptions() const {
  std::vector<std::string> result;
  for (auto& pool : resourcePools_) {
    result.push_back(pool ? pool->describe() : "null");
  }
  return result;
}

void ResourcePoolSet::calculatePriorityMapping() {
  // Calculate the best resource pool to return for each concurrency::PRIORITY
  // value

  // Fill with sentinel.
  auto sentinel = resourcePools_.size();
  std::fill_n(std::begin(poolByPriority_), poolByPriority_.size(), sentinel);

  // First put any resource pool with a priority hint into the approriate slot.
  DCHECK_EQ(priorities_.size(), resourcePools_.size());
  for (std::size_t i = 0; i < resourcePools_.size(); ++i) {
    if (priorities_[i]) {
      poolByPriority_[priorities_[i].value()] = i;
    }
  }

  // Check that NORMAL is filled - if not fill it with default async
  if (poolByPriority_[concurrency::NORMAL] == sentinel) {
    if (hasResourcePool(ResourcePoolHandle::defaultAsync())) {
      poolByPriority_[concurrency::NORMAL] =
          ResourcePoolHandle::defaultAsync().index();
    }
  }

  // Then fill out the rest of the slots moving away from NORMAL, copying the
  // nearest assigned value into empty slots.
  for (std::size_t i = concurrency::NORMAL; i < (concurrency::N_PRIORITIES - 1);
       ++i) {
    if (poolByPriority_[i + 1] == sentinel) {
      poolByPriority_[i + 1] = poolByPriority_[i];
    }
  }

  for (std::size_t i = concurrency::NORMAL; i > 0; i--) {
    if (poolByPriority_[i - 1] == sentinel) {
      poolByPriority_[i - 1] = poolByPriority_[i];
    }
  }
}

} // namespace apache::thrift
