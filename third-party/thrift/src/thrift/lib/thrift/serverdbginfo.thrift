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

namespace cpp2 apache.thrift.serverdbginfo
namespace php ThriftServerDbgInfo
namespace py3 apache.thrift.serverdbginfo
namespace go thrift.lib.thrift.serverdbginfo
namespace java.swift com.facebook.thrift.serverdbginfo

struct RequestPileDbgInfo {
  // The name of the requests pile indicating its type and behavior.
  1: string name;

  // The number of `priorities` to which requests can be assigned.
  2: i32 prioritiesCount;

  // The number of 'buckets' for each `priority`.
  3: list<i64> bucketsPerPriority;

  // The maximum number of queued requests in each `bucket`.
  4: i64 perBucketRequestLimit;

  // The maximum number of queued requests across all `priorities` and `buckets`.
  5: i64 totalRequestsLimit;

  // The number of queued requests across all `priorities` and `buckets`.
  6: i64 queuedRequestsCount;
}

struct ConcurrencyControllerDbgInfo {
  // The name of the concurrency controller.
  1: string name;

  // The maximum number of requests processed in the same time.
  2: optional i32 concurrencyLimit;

  // The maximum QPS processed in the same time.
  3: optional i32 qpsLimit;

  // The total number of requests being processed.
  4: i64 totalProcessedRequests;
}

struct ExecutorDbgInfo {
  // The name of the executor.
  1: string name;

  // The number of threads used by the executor
  2: optional i32 threadsCount;
}

struct ResourcePoolDbgInfo {
  // The name of the resource pool
  1: string name;

  // The debug information about request pile.
  2: optional RequestPileDbgInfo requestPileDbgInfo;

  // The debug information about concurrency controller.
  3: optional ConcurrencyControllerDbgInfo concurrencyControllerDbgInfo;

  // The debug information about executor.
  4: optional ExecutorDbgInfo executorDbgInfo;
}

struct CPUConcurrencyControllerDbgInfo {
  // Indicates whether CPU concurrency controller is enabled, disabled or in dry run
  1: string mode;

  // The CPU concurrency enforcement method
  2: string method;

  // The CPU target
  3: i16 cpuTarget;

  // The source of CPU load metrics.
  4: string cpuLoadSource;

  // The period indicating how often concurrency limit can be updated.
  5: i64 refreshPeriodMs;

  // The multiplier by which request limit may be increased.
  6: double additiveMultiplier;

  // The multiplier by which request limit may be decreased.
  7: double decreaseMultiplier;

  // The distance to concurrency limit required to adjust the request limit.
  8: double increaseDistanceRatio;

  // Indicates whether concurrency limit should be adjusted based on overload errors.
  9: bool bumpOnError;

  // The time required after overload errors to estimate concurrency.
  10: i64 refractoryPeriodMs;

  // The initial estimate factor for concurrency estimation.
  11: double initialEstimateFactor;

  // The initial percentile estimate factor for concurrency estimation.
  12: double initialEstimatePercentile;

  // The initial estimate sample size.
  13: i64 collectionSampleSize;

  // The upper bound concurrency limit.
  14: i64 concurrencyUpperBound;

  // The lower bound concurrency limit.
  15: i64 concurrencyLowerBound;

  // The current CPU load, as seen by CPU Concurrency Controller.
  16: i16 cpuLoad;
}

struct ResourcePoolsDbgInfo {
  // Indicates whether resource pools are enabled on the server.
  1: bool enabled;

  // The list of all the resource pools configured on the server.
  2: optional list<ResourcePoolDbgInfo> resourcePools;
}
