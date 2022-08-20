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

#include <memory>
#include <vector>
#include <thrift/lib/cpp/concurrency/SFQThreadManager.h>

#include <glog/logging.h>
#include <folly/executors/MeteredExecutor.h>
#include <folly/experimental/FunctionScheduler.h>
#include <folly/hash/Hash.h>

#include <thrift/lib/cpp/concurrency/SFQThreadManager.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>

namespace apache {
namespace thrift {
namespace concurrency {

static constexpr uint64_t kDefaultTenantId{0};

SFQThreadManager::SFQThreadManager(SFQThreadManagerConfig config)
    : ThreadManagerExecutorAdapter(config.getExecutors()), config_(config) {
  if (config_.getPerturbInterval().count() > 0) {
    initPerturbation();
  }

  initQueues();
}

SFQThreadManager::~SFQThreadManager() {
  perturbationSchedule_.shutdown();
}

size_t SFQThreadManager::getTaskCount(ExecutionScope es) {
  auto pri = es.getPriority();
  auto tenantId = es.getTenantId().value_or(kDefaultTenantId);
  return getMeteredExecutor(pri, tenantId)->pendingTasks();
}

void SFQThreadManager::initQueues() {
  // We make fair queues to be used on UPSTREAM sources for each priority.
  for (size_t pri = 0; pri < PRIORITY::N_PRIORITIES; ++pri) {
    fqs_[pri].resize(config_.getNumFairQueuesForUpstream());
    for (uint32_t ii = 0; ii < config_.getNumFairQueuesForUpstream(); ++ii) {
      auto keepalive = ThreadManagerExecutorAdapter::getKeepAlive(
          ExecutionScope(static_cast<PRIORITY>(pri)), Source::UPSTREAM);
      fqs_[pri][ii] = std::make_unique<folly::MeteredExecutor>(keepalive);
    }
  }
}

[[nodiscard]] ThreadManager::KeepAlive<> SFQThreadManager::getKeepAlive(
    ExecutionScope es, Source source) const {
  // We only use the metered executor fair queuing for upstream sources. Bypass
  // the FQs if it's any other source.
  if (source != Source::UPSTREAM) {
    return ThreadManagerExecutorAdapter::getKeepAlive(std::move(es), source);
  }

  const size_t pri = es.getPriority();
  auto* mx =
      getMeteredExecutor(pri, es.getTenantId().value_or(kDefaultTenantId));
  return getKeepAliveToken(mx);
}

} // namespace concurrency
} // namespace thrift
} // namespace apache
