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

#include <folly/GLog.h>
#include <folly/synchronization/RelaxedAtomic.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

THRIFT_FLAG_DEFINE_bool(experimental_use_resource_pools, false);
FOLLY_GFLAGS_DEFINE_bool(
    thrift_experimental_use_resource_pools,
    false,
    "Use experimental resource pools");
FOLLY_GFLAGS_DEFINE_bool(
    thrift_disable_resource_pools, false, "Disable use of resource pools");

THRIFT_FLAG_DEFINE_bool(allow_resource_pools_for_wildcards, true);
FOLLY_GFLAGS_DEFINE_bool(
    thrift_allow_resource_pools_for_wildcards,
    true,
    "Allow resource pools for wildcard processors");

THRIFT_FLAG_DEFINE_bool(allow_set_thread_manager_resource_pools, false);
THRIFT_FLAG_DEFINE_bool(enable_resource_pools_for_interaction, false);

THRIFT_FLAG_DEFINE_bool(allow_wildcard_process_via_execute_request, true);
THRIFT_FLAG_DEFINE_bool(
    allow_resource_pools_set_thread_manager_from_executor, true);

FOLLY_GFLAGS_DEFINE_bool(
    thrift_use_token_bucket_concurrency_controller,
    false,
    "Use TokenBucketConcurrencyController as a standard concurrency controller in ThriftServer");

FOLLY_GFLAGS_DEFINE_bool(
    thrift_server_enforces_qps_limit,
    true,
    "Enforce QPS limit in ThriftServer::checkOverload");

namespace apache::thrift {

bool useResourcePoolsFlagsSet() {
  static bool gFlagEnable = FLAGS_thrift_experimental_use_resource_pools;
  static bool gFlagDisable = FLAGS_thrift_disable_resource_pools;
  static bool thriftFlag = THRIFT_FLAG(experimental_use_resource_pools);
  return (gFlagEnable || thriftFlag) && !gFlagDisable;
}

bool allowResourcePoolsForWildcards() {
  static bool gFlagAllow = FLAGS_thrift_allow_resource_pools_for_wildcards;
  static bool thriftFlag = THRIFT_FLAG(allow_resource_pools_for_wildcards);
  return gFlagAllow && thriftFlag;
}

} // namespace apache::thrift
