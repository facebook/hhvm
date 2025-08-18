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

// Either of these flags can be used to disable the resource pool migration, but
// they have slightly different intents.
//
// - The gflag is to be used by service owners as a kill switch to disable
//   resource pools without getting approval from the thrift team.
// - The thrift flag is to be used by the thrift team to slowly roll out
//   resource pools onto individual services.
//
// **RESTART IS REQUIRED** for these flag changes to take effect.
FOLLY_GFLAGS_DECLARE_bool(thrift_disable_resource_pool_migration);
THRIFT_FLAG_DECLARE_bool(enable_resource_pool_migration);

namespace apache::thrift {

class ResourcePoolMigrationHelper {
 public:
  /**
   * Returns true if the resource pool migration is enabled, and false
   * otherwise.
   *
   * Services that are being migrated to resource pools should check if the
   * migration is enabled. If so, the service should use resource pools.
   * Otherwise, the service should fall back to its old behavior.
   */
  static bool isMigrationEnabled();
};

} // namespace apache::thrift
