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

#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/Flags.h>

THRIFT_FLAG_DECLARE_bool(experimental_use_resource_pools);
FOLLY_GFLAGS_DECLARE_bool(thrift_experimental_use_resource_pools);
FOLLY_GFLAGS_DECLARE_bool(thrift_disable_resource_pools);

THRIFT_FLAG_DECLARE_bool(allow_resource_pools_for_wildcards);
FOLLY_GFLAGS_DECLARE_bool(thrift_allow_resource_pools_for_wildcards);

THRIFT_FLAG_DECLARE_bool(allow_set_thread_manager_resource_pools);
THRIFT_FLAG_DECLARE_bool(enable_resource_pools_for_interaction);

THRIFT_FLAG_DECLARE_bool(allow_wildcard_process_via_execute_request);
THRIFT_FLAG_DECLARE_bool(allow_resource_pools_set_thread_manager_from_executor);

// Use TokenBucketConcurrencyController as a standard concurrency controller in
// ThriftServer
FOLLY_GFLAGS_DECLARE_bool(thrift_use_token_bucket_concurrency_controller);

// Enforce QPS limit in ThriftServer::checkOverload
FOLLY_GFLAGS_DECLARE_bool(thrift_server_enforces_qps_limit);

namespace apache::thrift {

bool useResourcePoolsFlagsSet();
bool allowResourcePoolsForWildcards();

// This is a temporary solution during resource pools roll out. Once
// roll out is complete there will be no tests that require this and it
// will be removed.
#define THRIFT_OMIT_TEST_WITH_RESOURCE_POOLS()        \
  do {                                                \
    if (apache::thrift::useResourcePoolsFlagsSet()) { \
      return;                                         \
    }                                                 \
  } while (false)

#define THRIFT_CO_OMIT_TEST_WITH_RESOURCE_POOLS()     \
  do {                                                \
    if (apache::thrift::useResourcePoolsFlagsSet()) { \
      co_return;                                      \
    }                                                 \
  } while (false)

} // namespace apache::thrift
