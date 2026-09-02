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

#include <thrift/lib/cpp2/server/DiscriminantLoadSheddingOSS.h>

#include <algorithm>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <glog/logging.h>

#include <folly/Indestructible.h>
#include <folly/Synchronized.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/logging/xlog.h>
#include <folly/system/HardwareConcurrency.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/RequestExpirationDelegate.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/TokenBucketConcurrencyController.h>

namespace apache::thrift {

namespace {

using DLS = DiscriminantLoadSheddingOSS;

/*
 * Entries are never erased, so a ThriftServer address reused after the server
 * is destroyed will report the previous server's Setup. Acceptable here only
 * because setup() is a process-lifetime, startup-only call.
 */
folly::Synchronized<folly::F14FastMap<ThriftServer*, DLS::Setup>>&
serverToDlsSetupMap() {
  static folly::Indestructible<
      folly::Synchronized<folly::F14FastMap<ThriftServer*, DLS::Setup>>>
      map;
  return *map;
}

bool methodIsInteraction(
    const std::pair<
        std::string,
        std::shared_ptr<const AsyncProcessorFactory::MethodMetadata>>&
        methodMetadataEntry) {
  return methodMetadataEntry.second->interactionType ==
      AsyncProcessorFactory::MethodMetadata::InteractionType::INTERACTION_V1;
}

/*
 * Returns true iff the interface provided to the ThriftServer contains any
 * interaction methods. Returns false if the interface is not set yet.
 */
bool serviceHasInteraction(ThriftServer* server) {
  if (server->getProcessorFactory() == nullptr) {
    LOG(WARNING)
        << "Service interface is not set yet, automatic prioritization of requests "
        << "for existing Thrift interactions will not work. If the service doesn't "
        << "use Thrift interactions, this is ok.";
    return false;
  }

  auto methodMetadataVariant =
      server->getProcessorFactory()->createMethodMetadata();
  auto* methodMetadataMap =
      std::get_if<AsyncProcessorFactory::MethodMetadataMap>(
          &methodMetadataVariant);
  if (methodMetadataMap == nullptr) {
    return false;
  }
  return std::any_of(
      methodMetadataMap->begin(),
      methodMetadataMap->end(),
      methodIsInteraction);
}

uint64_t getRequestBytes(const ServerRequest& request) {
  return request.requestContext()
      ? request.requestContext()->getWiredRequestBytes()
      : 0;
}

size_t getShedderPriority(const ServerRequest& request) {
  return request.requestData().bucket ? request.requestData().bucket->first : 0;
}

std::unique_ptr<ConcurrencyControllerInterface>
makeDefaultConcurrencyController(
    bool useParallelConcurrencyController,
    RequestPileInterface& requestPile,
    folly::Executor& executor) {
  if (useParallelConcurrencyController) {
    return std::make_unique<ParallelConcurrencyController>(
        requestPile, executor);
  }
  return std::make_unique<TokenBucketConcurrencyController>(
      requestPile, executor);
}

std::shared_ptr<folly::Executor> makeCpuExecutor(
    const DLS::Setup& setup, const ThriftServer& server) {
  const size_t numCpuThreads = setup.cpuThreadPoolSize > 0
      ? setup.cpuThreadPoolSize
      : folly::available_concurrency();
  auto queue = setup.useThrottledLifoSem
      ? folly::CPUThreadPoolExecutor::makeThrottledLifoSemQueue(
            std::chrono::microseconds{100})
      : folly::CPUThreadPoolExecutor::makeDefaultQueue();

  return std::make_shared<folly::CPUThreadPoolExecutor>(
      numCpuThreads,
      std::move(queue),
      std::make_shared<folly::NamedThreadFactory>(
          server.getCPUWorkerThreadName()));
}

RoundRobinRequestPile::Options::PileSelectionFunction makePileSelectionFunction(
    DLS::Setup& setup) {
  auto getBucketForTenant = setup.getBucketforTenant
      ? std::move(*setup.getBucketforTenant)
      : DLS::GetBucketForTenantFunctions::useShardedBucket(setup.bucketsCount);

  return [prioritiesCount = setup.prioritiesCount,
          getRequestPriority = std::move(setup.getRequestPriority),
          getTenant = std::move(setup.getTenant),
          getBucketForTenant = std::move(getBucketForTenant)](
             const ServerRequest& request) mutable
             -> std::pair<
                 RoundRobinRequestPile::Priority,
                 RoundRobinRequestPile::Bucket> {
    const auto priority = std::clamp<DLS::Priority>(
        getRequestPriority(request), 0, prioritiesCount - 1);
    const auto bucket = getBucketForTenant(getTenant(request));
    return {
        RoundRobinRequestPile::Priority(priority),
        RoundRobinRequestPile::Bucket(bucket)};
  };
}

RoundRobinRequestPile::Options makePileOptions(DLS::Setup& setup) {
  RoundRobinRequestPile::Options options;
  options.setNumPriorities(setup.prioritiesCount);

  if (setup.bucketSizePerPriority.empty()) {
    options.setNumMaxRequests(setup.bucketSize);
  } else {
    DCHECK_EQ(setup.bucketSizePerPriority.size(), setup.prioritiesCount)
        << "bucketSizePerPriority size must match prioritiesCount";
    options.setNumMaxRequestsPerPriority(
        std::vector<uint32_t>(
            setup.bucketSizePerPriority.begin(),
            setup.bucketSizePerPriority.end()));
  }

  for (uint32_t i = 0; i < setup.prioritiesCount; i++) {
    options.setNumBucketsPerPriority(i, setup.bucketsCount);
  }

  options.setPileSelectionFunction(makePileSelectionFunction(setup));
  return options;
}

/*
 * Thresholds are supplied per logical priority, so they must be expanded
 * before addInternalPriorities() doubles the pile's priority count.
 */
std::shared_ptr<DLS::QueueLoadShedder> makeQueueLoadShedder(
    DLS::QueueLoadShedder::Options options, bool hasInteractions) {
  if (hasInteractions) {
    if (!options.byteThresholds.empty()) {
      options.byteThresholds =
          DLS::QueueLoadShedder::expandForInternalPriorities(
              options.byteThresholds);
    }
    if (!options.countThresholds.empty()) {
      options.countThresholds =
          DLS::QueueLoadShedder::expandForInternalPriorities(
              options.countThresholds);
    }
  }
  return std::make_shared<DLS::QueueLoadShedder>(std::move(options));
}

void enableExpireRequestsOnDequeue(
    RequestPileInterface& requestPile,
    ConcurrencyControllerInterface& concurrencyController) {
  auto* pile = dynamic_cast<RoundRobinRequestPile*>(&requestPile);
  if (pile == nullptr) {
    LOG(FATAL)
        << "Couldn't enable expireRequestsOnDequeue because RequestPile is not RoundRobinRequestPile";
  }

  auto* delegate =
      dynamic_cast<RequestExpirationDelegate*>(&concurrencyController);
  if (delegate == nullptr) {
    LOG(FATAL)
        << "Couldn't enable expireRequestsOnDequeue because ConcurrencyController is not RequestExpirationDelegate";
  }

  pile->setRequestExpirationDelegate(delegate);
}

} // namespace

std::optional<ServerRequestRejection>
DiscriminantLoadSheddingOSS::QueueLoadShedder::checkEnqueue(
    const ServerRequest& request) {
  const auto priority = getShedderPriority(request);

  const auto shed =
      [&](const std::vector<uint64_t>& thresholds,
          uint64_t current,
          const char* dimension) -> std::optional<ServerRequestRejection> {
    if (thresholds.empty() || priority >= thresholds.size()) {
      return std::nullopt;
    }
    const auto threshold = thresholds[priority];
    if (threshold == 0 || current < threshold) {
      return std::nullopt;
    }
    return ServerRequestRejection(AppOverloadedException(
        "QueueLoadShedder",
        fmt::format(
            "shed priority {} ({}: {} >= threshold {})",
            priority,
            dimension,
            current,
            threshold)));
  };

  if (auto rejection = shed(
          opts_.byteThresholds,
          totalQueuedBytes_.load(std::memory_order_relaxed),
          "bytes")) {
    return rejection;
  }
  if (auto rejection = shed(
          opts_.countThresholds,
          totalQueuedRequests_.load(std::memory_order_relaxed),
          "count")) {
    return rejection;
  }

  if (!opts_.byteThresholds.empty()) {
    totalQueuedBytes_.fetch_add(
        getRequestBytes(request), std::memory_order_relaxed);
  }
  if (!opts_.countThresholds.empty()) {
    totalQueuedRequests_.fetch_add(1, std::memory_order_relaxed);
  }
  return std::nullopt;
}

void DiscriminantLoadSheddingOSS::QueueLoadShedder::onDequeue(
    const ServerRequest& request) {
  if (!opts_.byteThresholds.empty()) {
    totalQueuedBytes_.fetch_sub(
        getRequestBytes(request), std::memory_order_relaxed);
  }
  if (!opts_.countThresholds.empty()) {
    totalQueuedRequests_.fetch_sub(1, std::memory_order_relaxed);
  }
}

/*static*/ std::vector<uint64_t>
DiscriminantLoadSheddingOSS::QueueLoadShedder::expandForInternalPriorities(
    const std::vector<uint64_t>& thresholds) {
  std::vector<uint64_t> result;
  result.reserve(thresholds.size() * 2);
  for (auto threshold : thresholds) {
    result.push_back(threshold);
    result.push_back(threshold);
  }
  return result;
}

/*static*/ void DiscriminantLoadSheddingOSS::setup(
    Setup&& setup,
    ThriftServer* server,
    std::shared_ptr<folly::Executor> executor) {
  DCHECK(server->useResourcePools()) << "Server must use Resource Pools";
  DCHECK(server->runtimeResourcePoolsChecks())
      << "Server must pass Resource Pools' runtime checks";
  DCHECK(server->resourcePoolSet().empty())
      << "No Resource Pools must be configured just yet";
  DCHECK(
      setup.bucketSizePerPriority.empty() ||
      setup.bucketSize == Setup{}.bucketSize)
      << "bucketSizePerPriority and bucketSize cannot both be set. "
      << "Use bucketSizePerPriority for per-priority limits, "
      << "or bucketSize for a uniform limit.";

  if (FLAGS_thrift_server_enforces_qps_limit) {
    LOG(INFO) << "Setting --thrift_server_enforces_qps_limit=false at runtime";
    FLAGS_thrift_server_enforces_qps_limit = false;
  }

  server->setMaxQps(setup.qpsLimit);

  if (!executor) {
    executor = makeCpuExecutor(setup, *server);
  }

  const bool hasInteractions = serviceHasInteraction(server);
  auto options = makePileOptions(setup);

  std::shared_ptr<QueueLoadShedder> queueLoadShedder;
  if (setup.queueLoadShedderOptions) {
    queueLoadShedder = makeQueueLoadShedder(
        std::move(*setup.queueLoadShedderOptions), hasInteractions);
    options.setPreEnqueueFilter(
        [queueLoadShedder](const ServerRequest& request) {
          return queueLoadShedder->checkEnqueue(request);
        });
  }

  if (hasInteractions) {
    options = options.addInternalPriorities();
  }

  std::unique_ptr<RequestPileInterface> requestPile = [&] {
    auto pile = std::make_unique<RoundRobinRequestPile>(std::move(options));
    if (queueLoadShedder) {
      pile->setDequeueObserver(
          [queueLoadShedder](const ServerRequest& request) {
            queueLoadShedder->onDequeue(request);
          });
    }
    return pile;
  }();
  if (setup.requestPileWrapper) {
    requestPile = (*setup.requestPileWrapper)(std::move(requestPile));
  }

  auto concurrencyController = setup.concurrencyControllerFactory
      ? (*setup.concurrencyControllerFactory)(*requestPile, *executor)
      : makeDefaultConcurrencyController(
            setup.useParallelConcurrencyController, *requestPile, *executor);

  if (setup.expireRequestsOnDequeue) {
    enableExpireRequestsOnDequeue(*requestPile, *concurrencyController);
  }

  server->resourcePoolSet().setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  server->resourcePoolSet().setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(requestPile),
      std::move(executor),
      std::move(concurrencyController));

  serverToDlsSetupMap().wlock()->insert({server, std::move(setup)});
}

/*static*/ DiscriminantLoadSheddingOSS::GetRequestPriorityFunction
DiscriminantLoadSheddingOSS::GetRequestPriorityFunctions::useThriftPriority() {
  return [](const ServerRequest& request) -> Priority {
    return Priority(request.requestContext()->getCallPriority());
  };
}

/*static*/ DiscriminantLoadSheddingOSS::GetTenantFunction
DiscriminantLoadSheddingOSS::GetTenantFunctions::hashClientId() {
  return [](const ServerRequest& request) -> Tenant {
    const std::hash<std::string> hasher;
    const auto* clientId = request.requestContext()->clientId();
    return hasher(clientId ? *clientId : "");
  };
}

/*static*/ DiscriminantLoadSheddingOSS::GetBucketForTenantFunction
DiscriminantLoadSheddingOSS::GetBucketForTenantFunctions::useShardedBucket(
    size_t bucketsCount) {
  return
      [bucketsCount](Tenant tenant) -> Bucket { return tenant % bucketsCount; };
}

DiscriminantLoadSheddingOSS::BucketManager::BucketManager(
    size_t numBuckets, size_t fallbackBucket)
    : numBuckets_{numBuckets}, fallbackBucket_{fallbackBucket} {
  for (size_t i = 0; i < numBuckets; i++) {
    availableBuckets_.push(i);
  }
}

size_t DiscriminantLoadSheddingOSS::BucketManager::getBucketForTenant(
    Tenant tenant) {
  {
    const std::shared_lock lock{mutex_};
    if (auto it = tenantToBucket_.find(tenant); it != tenantToBucket_.end()) {
      return it->second;
    }
  }

  const std::lock_guard lock{mutex_};

  // Another thread may have assigned this tenant while the lock was upgraded.
  if (auto it = tenantToBucket_.find(tenant); it != tenantToBucket_.end()) {
    return it->second;
  }

  auto bucket = fallbackBucket_;
  if (!availableBuckets_.empty()) {
    bucket = availableBuckets_.front();
    availableBuckets_.pop();
  }
  tenantToBucket_.insert({tenant, bucket});

  if (bucket == fallbackBucket_) {
    XLOG_EVERY_MS(ERR, 10'000) << "No available bucket for tenant " << tenant;
  }

  return bucket;
}

void DiscriminantLoadSheddingOSS::BucketManager::evictTenant(Tenant tenant) {
  const std::lock_guard lock{mutex_};
  if (auto it = tenantToBucket_.find(tenant); it != tenantToBucket_.end()) {
    availableBuckets_.push(it->second);
    tenantToBucket_.erase(it);
  }
}

std::string DiscriminantLoadSheddingOSS::Setup::describe() const {
  return fmt::format(
      "{{qpsLimit={}, prioritiesCount={}, bucketsCount={}, bucketSize={}, bucketSizePerPriority={}, cpuThreadPoolSize={}}}",
      qpsLimit,
      prioritiesCount,
      bucketsCount,
      bucketSize,
      fmt::format("{{{}}}", fmt::join(bucketSizePerPriority, ",")),
      cpuThreadPoolSize);
}

/*static*/ std::optional<DiscriminantLoadSheddingOSS::Setup>
DiscriminantLoadSheddingOSS::getDlsServerSetup(ThriftServer* server) {
  DCHECK(server != nullptr) << "Server is nullptr";

  auto map = serverToDlsSetupMap().rlock();
  if (auto it = map->find(server); it != map->end()) {
    return it->second;
  }
  return std::nullopt;
}

} // namespace apache::thrift
