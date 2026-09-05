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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include <folly/Executor.h>
#include <folly/SharedMutex.h>
#include <folly/container/F14Map.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

namespace apache::thrift {

/**
 * Priority- and tenant-aware overload protection for a Thrift server.
 *
 * DLS partitions the server's request queue along two axes: a caller-supplied
 * priority, and a "bucket" derived from the request's tenant (its caller
 * identity). Every (priority, bucket) pair gets its own queue with its own
 * depth limit, which buys two properties:
 *
 *   - Isolation: a single misbehaving tenant can only fill its own bucket, so
 *     it cannot starve the rest of the callers.
 *   - Graceful degradation: when the server is saturated, low-priority traffic
 *     is shed while high-priority traffic still gets through.
 *
 * `setup()` wires this up by installing a RoundRobinRequestPile plus a
 * concurrency controller as the server's default async resource pool, so it
 * must be called before the server configures any resource pools of its own.
 *
 * This is a self-contained reference copy of Meta's internal
 * DiscriminantLoadShedding, kept for illustration. It depends only on fbthrift
 * and folly, emits no metrics, and has no callers.
 */
struct DiscriminantLoadSheddingOSS {
  using Priority = uint32_t;
  using Tenant = uint64_t;
  using Bucket = size_t;

  using GetRequestPriorityFunction =
      std::function<Priority(const ServerRequest&)>;
  using GetTenantFunction = std::function<Tenant(const ServerRequest&)>;
  using GetBucketForTenantFunction = std::function<Bucket(Tenant)>;
  using GetRequestPileWrapperFunction =
      std::function<std::unique_ptr<RequestPileInterface>(
          std::unique_ptr<RequestPileInterface>)>;
  using GetConcurrencyControllerFunction =
      std::function<std::unique_ptr<ConcurrencyControllerInterface>(
          RequestPileInterface&, folly::Executor&)>;

  /**
   * Sheds the lowest-priority traffic first as the server's total queue grows.
   *
   * Where the request pile's per-bucket limits isolate tenants from each
   * other, this applies a global budget across all of them: each priority gets
   * its own threshold on the TOTAL queued value, so giving lower priorities
   * lower thresholds sheds them while headroom still remains for the rest.
   *
   * With byte thresholds {100MB, 80MB, 60MB, 40MB, 20MB}, at 25MB queued only
   * the lowest priority is shed; at 45MB the two lowest are; at 100MB
   * everything is.
   */
  class QueueLoadShedder {
   public:
    struct Options {
      // byteThresholds[pri]: shed priority `pri` once the total queued byte
      // count reaches this value. 0 disables the byte limit for that priority;
      // an empty vector disables byte-based shedding entirely. Should be
      // non-increasing, so that higher priorities survive longer.
      std::vector<uint64_t> byteThresholds;

      // The same, applied to the total number of queued requests.
      std::vector<uint64_t> countThresholds;
    };

    explicit QueueLoadShedder(Options opts) : opts_{std::move(opts)} {}

    std::optional<ServerRequestRejection> checkEnqueue(
        const ServerRequest& request);
    void onDequeue(const ServerRequest& request);

    uint64_t totalQueuedBytes() const {
      return totalQueuedBytes_.load(std::memory_order_relaxed);
    }

    uint64_t totalQueuedRequests() const {
      return totalQueuedRequests_.load(std::memory_order_relaxed);
    }

    // RoundRobinRequestPile::Options::addInternalPriorities() doubles the
    // pile's priority count, so threshold vectors must be doubled to match.
    static std::vector<uint64_t> expandForInternalPriorities(
        const std::vector<uint64_t>& thresholds);

   private:
    Options opts_;
    std::atomic<uint64_t> totalQueuedBytes_{0};
    std::atomic<uint64_t> totalQueuedRequests_{0};
  };

  struct Setup {
    int32_t qpsLimit = 0;
    uint32_t prioritiesCount = 5;
    size_t bucketsCount = 100;
    size_t bucketSize = 10000;
    size_t cpuThreadPoolSize = 0;

    // Per-priority per-bucket limits. When set, overrides bucketSize for
    // each priority. The vector size must equal prioritiesCount.
    // Lower index = higher priority. 0 means no limit for that priority.
    // Example: {0, 0, 1000, 500, 100} means P0-P1 unlimited, P2=1000,
    // P3=500, P4=100.
    std::vector<size_t> bucketSizePerPriority;

    GetRequestPriorityFunction getRequestPriority;
    GetTenantFunction getTenant;
    std::optional<GetBucketForTenantFunction> getBucketforTenant{std::nullopt};
    std::optional<GetRequestPileWrapperFunction> requestPileWrapper{
        std::nullopt};

    std::optional<GetConcurrencyControllerFunction>
        concurrencyControllerFactory{std::nullopt};

    std::string describe() const;

    bool useThrottledLifoSem{false};
    bool useParallelConcurrencyController{false};

    // If this flag is set, RoundRobinRequestPile will be configured with
    // RoundRobinRequestPile::Options::expireRequestsOnDequeue set to true.
    bool expireRequestsOnDequeue{false};

    // Optional priority-based queue load shedding. When set, a QueueLoadShedder
    // is created and wired into the pile's PreEnqueueFilter and
    // DequeueObserver. Thresholds are specified for logical priorities (N) and
    // automatically expanded for internal priorities (2N) if the service uses
    // Thrift interactions.
    std::optional<QueueLoadShedder::Options> queueLoadShedderOptions;
  };

  struct GetRequestPriorityFunctions {
    static GetRequestPriorityFunction useThriftPriority();
  };

  struct GetTenantFunctions {
    static GetTenantFunction hashClientId();
  };

  struct GetBucketForTenantFunctions {
    static GetBucketForTenantFunction useShardedBucket(size_t bucketsCount);
  };

  static void setup(
      Setup&& setup,
      ThriftServer* server,
      std::shared_ptr<folly::Executor> executor = nullptr);

  /**
   * Assigns tenants to buckets on a first-come-first-served basis, recycling a
   * bucket once its tenant is evicted. Tenants that arrive after every bucket
   * is taken share `fallbackBucket`, and so lose isolation from each other.
   *
   * Prefer this over hashing when the tenant set is small and known to fit;
   * unlike hashing it never collides two tenants while a bucket is still free.
   */
  class BucketManager {
   public:
    explicit BucketManager(size_t numBuckets, size_t fallbackBucket = 0);

    size_t getBucketForTenant(Tenant tenant);
    void evictTenant(Tenant tenant);

    size_t getNumBuckets() const { return numBuckets_; }

    size_t getNumAvailableBuckets() const { return availableBuckets_.size(); }

    size_t getFallbackBucket() const { return fallbackBucket_; }

   private:
    const size_t numBuckets_;
    const size_t fallbackBucket_;

    folly::F14NodeMap<Tenant, size_t> tenantToBucket_;
    std::queue<size_t> availableBuckets_;
    folly::SharedMutex mutex_;
  };

  static std::optional<Setup> getDlsServerSetup(ThriftServer* server);
};

} // namespace apache::thrift
