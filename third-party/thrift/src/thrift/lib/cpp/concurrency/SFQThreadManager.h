/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>

#include <glog/logging.h>
#include <folly/executors/MeteredExecutor.h>
#include <folly/experimental/FunctionScheduler.h>
#include <folly/hash/Hash.h>

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>

namespace apache {
namespace thrift {
namespace concurrency {

class SFQThreadManagerConfig {
 public:
  SFQThreadManagerConfig() = default;

  SFQThreadManagerConfig& setPerturbInterval(std::chrono::milliseconds p) {
    perturb_ = p;
    return *this;
  }
  std::chrono::milliseconds getPerturbInterval() const { return perturb_; }

  // Currently only supporting fair queuing for upstream source.
  SFQThreadManagerConfig& setNumFairQueuesForUpstream(size_t numQueues) {
    numQueues_ = numQueues;
    return *this;
  }
  size_t getNumFairQueuesForUpstream() const { return numQueues_; }

  SFQThreadManagerConfig& setExecutors(
      std::array<std::shared_ptr<folly::Executor>, N_PRIORITIES> executors) {
    executors_ = std::move(executors);
    return *this;
  }
  std::array<std::shared_ptr<folly::Executor>, N_PRIORITIES>& getExecutors() {
    return executors_;
  }

 private:
  std::chrono::milliseconds perturb_{std::chrono::seconds(30)};
  size_t numQueues_{1};
  std::array<std::shared_ptr<folly::Executor>, N_PRIORITIES> executors_;
};

/**
 * Stochastic fair queue thread manager (SFQTM)
 * ============================================
 *
 * This TM variant aims to address fairness concerns in multi-tenant systems by
 * fronting the CPU executor' UPSTREAM queue with a configurable number of
 * sub-queues used to isolate different request "tenants". Usage of the SFQTM
 * relies on users to identify different request tenants via encoding the data
 * in the priority provided to getKeepAlive() calls for UPSTREAM sources.
 *
 * Fairness in concurrency
 * -----------------------
 * Fairness in the SFQTM context means that each tenant is allowed the same
 * number of tasks to reside on the internal executor queue. As an example,
 * let's say we have 2 tenants (A and B). When the internal executor's threads
 * are busy running tasks and begins queuing tasks, its queue will contain at
 * most a single task from A and a single task from B. As tasks from each tenant
 * are scheduled on a thread, a new task for that tenant is pulled from the
 * tenant's queue and placed in the internal executor's queue.
 *
 * Stochastic fairness
 * -------------------
 * For practical reasons related to memory utilization and performance, multiple
 * tenants can potentially map to a single queue. When instantiating the
 * SFQTM, we allocate a fixed number of queues and hash the tenant ID onto this
 * fixed set of queues. This means that there is potential for multiple tenants
 * to map onto the same queue, resulting in a shared request concurrency
 * entitlement between them and unfairness. We mitigate this via perturbing the
 * hash function periodically via 'perturb'. This ensures that even if there is
 * a collision between two tenants, the condition will not be persistent. This
 * perturbation may result in request reordering for a tenant under heavy load.
 *
 *   Notes
 *   ~~~~~
 *   ** One may disable perturbing of the tenant hash by setting the period to
 *   0 seconds.
 *
 *   ** If unspecified in the execution scope, tenant ID is assumed to be
 *   zero.
 */
class SFQThreadManager : public ThreadManagerExecutorAdapter {
 public:
  explicit SFQThreadManager(SFQThreadManagerConfig config);
  ~SFQThreadManager() override;

  [[nodiscard]] KeepAlive<> getKeepAlive(
      ExecutionScope es, Source source) const override;

  [[noreturn]] void add(
      std::shared_ptr<Runnable>,
      int64_t,
      int64_t,
      ThreadManager::Source) noexcept override {
    LOG(FATAL)
        << "add*() is unsupported in SFQThreadManager, use getKeepAlive()";
  }

  // Return size of tenantQueue for a given priority and tenantId
  size_t getTaskCount(ExecutionScope es);

 private:
  using ExecutorPtr = std::unique_ptr<folly::DefaultKeepAliveExecutor>;

  void initPerturbation() {
    perturbationSchedule_.addFunction(
        [this]() mutable {
          perturbVal_.fetch_add(1, std::memory_order_relaxed);
        },
        config_.getPerturbInterval(),
        "sfq_perturb");
    perturbationSchedule_.start();
  }

  uint64_t perturbId(uint64_t tenant, size_t val) const {
    return folly::hash::hash_combine(tenant, val);
  }

  // Returns a metered executor associated with a tenant ID.
  //
  // Since we use a stochastic fair queue, we have a fixed number of metered
  // queues and it's possible for multiple tenant IDs to collide on the same
  // metered queue which manifests in unfairness for those tenants. We
  // mitigate the issue by hashing the tenant IDs here with a periodically
  // changing value so that it is unlikely to persist beyond the current
  // period.
  folly::MeteredExecutor* getMeteredExecutor(
      size_t pri, uint64_t tenantId) const {
    const size_t p = perturbVal_.load(std::memory_order_relaxed);
    const uint64_t perturbedTenantId = perturbId(tenantId, p);
    CHECK_LT(pri, fqs_.size());
    const size_t perturbedIdx = perturbedTenantId % fqs_[pri].size();
    return fqs_[pri][perturbedIdx].get();
  }

  // Set up the metered executors to act as fair queues.
  void initQueues();

  SFQThreadManagerConfig config_;
  using MeteredExVec = std::vector<std::unique_ptr<folly::MeteredExecutor>>;
  std::array<MeteredExVec, PRIORITY::N_PRIORITIES> fqs_;
  std::atomic<size_t> perturbVal_{0};
  folly::FunctionScheduler perturbationSchedule_;
};

} // namespace concurrency
} // namespace thrift
} // namespace apache
