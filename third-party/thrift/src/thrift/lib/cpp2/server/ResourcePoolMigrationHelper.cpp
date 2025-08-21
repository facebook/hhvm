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

#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

#include <thrift/lib/cpp2/server/ResourcePoolMigrationHelper.h>

FOLLY_GFLAGS_DEFINE_bool(
    thrift_disable_resource_pool_migration,
    false,
    "Disable the thrift resource pool migration behavior on process startup.");
THRIFT_FLAG_DEFINE_bool(enable_resource_pool_migration, false);

namespace apache::thrift {

bool ResourcePoolMigrationHelper::isMigrationEnabled() {
  if (FLAGS_thrift_disable_resource_pool_migration ||
      !THRIFT_FLAG(enable_resource_pool_migration)) {
    XLOG(WARNING)
        << "This service will soon be migrated onto resource pools. Migration can "
           "be rolled back by setting the gflag killswitch "
           "(`--thrift_disable_resource_pool_migration`) or unsetting thrift flag "
           "(`enable_resource_pool_migration`). The process must be restarted for "
           "either flag change to take effect.";
    return false;
  }

  // Override all the flags that normally disable resource pools.

  const auto original_FLAGS_thrift_experimental_use_resource_pools =
      FLAGS_thrift_experimental_use_resource_pools;
  if (!FLAGS_thrift_experimental_use_resource_pools) {
    XLOG(WARNING)
        << "--thrift_experimental_use_resource_pools gflag is unset. This flag is "
           "being set as this build rule is being migrated onto resource pools "
           "and is no longer allowed to opt out.";
    FLAGS_thrift_experimental_use_resource_pools = true;
  }

  const auto original_FLAGS_thrift_disable_resource_pools =
      FLAGS_thrift_disable_resource_pools;
  if (FLAGS_thrift_disable_resource_pools) {
    XLOG(WARNING)
        << "--thrift_disable_resource_pools gflag is set. This flag is being "
           "unset as this build rule is being migrated onto resource pools "
           "and is no longer allowed to opt out.";
    FLAGS_thrift_disable_resource_pools = false;
  }

  const auto original_THRIFT_FLAG_experimental_use_resource_pools =
      THRIFT_FLAG(experimental_use_resource_pools);
  if (!THRIFT_FLAG(experimental_use_resource_pools)) {
    XLOG(WARNING)
        << "experimental_use_resource_pools thrift flag is unset. This flag is "
           "being set as this build rule is being migrated onto resource pools "
           "and is no longer allowed to opt out.";
    THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, true);
  }

  // The `useResourcePoolFlagsSet()` function (wisely) checks the above flags
  // just the first time its called and never changes its return value for the
  // lifetime of the process even if the flags have changed. If this function
  // was already called and returned false it's not safe for us to proceed with
  // the migration since we may violate a server state invariant.
  if (!useResourcePoolsFlagsSet()) {
    FLAGS_thrift_experimental_use_resource_pools =
        original_FLAGS_thrift_experimental_use_resource_pools;
    FLAGS_thrift_disable_resource_pools =
        original_FLAGS_thrift_disable_resource_pools;
    THRIFT_FLAG_SET_MOCK(
        experimental_use_resource_pools,
        original_THRIFT_FLAG_experimental_use_resource_pools);
    XLOG(ERR)
        << "`useResourcePoolsFlagSet()` was called before we changed the above "
           "flags and already claimed that resource pools are not enabled. It is "
           "not safe to move forward with migration as it may violate a server "
           "state invariant. These flag changes are now reverted. Fix this race "
           "so that migration can proceed.";
    return false;
  }

  XLOG(WARNING)
      << "This service was recently migrated onto resource pools. Migration can "
         "be rolled back by setting the gflag killswitch "
         "(`--thrift_disable_resource_pool_migration`) or unsetting thrift flag "
         "(`enable_resource_pool_migration`). The process must be restarted for "
         "either flag change to take effect.";

  return true;
}

} // namespace apache::thrift
