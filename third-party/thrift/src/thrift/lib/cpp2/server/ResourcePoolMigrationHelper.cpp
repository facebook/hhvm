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
#include <thrift/lib/cpp2/server/ResourcePoolMigrationHelper.h>

FOLLY_GFLAGS_DEFINE_bool(
    thrift_disable_resource_pool_migration,
    false,
    "Disable the thrift resource pool migration behavior on process startup.");
THRIFT_FLAG_DEFINE_bool(enable_resource_pool_migration, false);

namespace apache::thrift {

bool ResourcePoolMigrationHelper::isMigrationEnabled() {
  const auto migration_is_enabled =
      !FLAGS_thrift_disable_resource_pool_migration &&
      THRIFT_FLAG(enable_resource_pool_migration);

  XLOG_IF(WARNING, migration_is_enabled)
      << "This service was recently migrated onto resource pools. Migration can "
         "be rolled back by setting the gflag killswitch "
         "(`--thrift_disable_resource_pool_migration`) or unsetting thrift flag "
         "(`enable_resource_pool_migration`). The process must be restarted for "
         "either flag change to take effect.";

  return migration_is_enabled;
}

} // namespace apache::thrift
