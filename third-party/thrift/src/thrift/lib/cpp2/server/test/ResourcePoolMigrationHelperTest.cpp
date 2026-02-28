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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

#include <thrift/lib/cpp2/server/ResourcePoolMigrationHelper.h>

using namespace ::apache::thrift;
using namespace ::testing;

TEST(ResourcePoolMigrationHelperTest, TestAllGflagAndThriftFlagCombos) {
  for (auto disable_value : {true, false}) {
    for (auto enable_value : {true, false}) {
      gflags::FlagSaver flagSaver;
      FLAGS_thrift_disable_resource_pool_migration = disable_value;
      THRIFT_FLAG_SET_MOCK(enable_resource_pool_migration, enable_value);

      EXPECT_EQ(
          (disable_value == false) && (enable_value == true),
          ResourcePoolMigrationHelper::isMigrationEnabled());
    }
  }
}

TEST(ResourcePoolMigrationHelperTest, EnabledMigrationDisablesOptOutFlags) {
  gflags::FlagSaver flagSaver;
  FLAGS_thrift_disable_resource_pool_migration = false;
  THRIFT_FLAG_SET_MOCK(enable_resource_pool_migration, true);

  FLAGS_thrift_experimental_use_resource_pools = false;
  FLAGS_thrift_disable_resource_pools = true;
  THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, false);

  // This call should set the opt-out flags to non-opt-out values.
  EXPECT_EQ(true, ResourcePoolMigrationHelper::isMigrationEnabled());

  EXPECT_EQ(true, FLAGS_thrift_experimental_use_resource_pools);
  EXPECT_EQ(false, FLAGS_thrift_disable_resource_pools);
  EXPECT_EQ(true, THRIFT_FLAG(experimental_use_resource_pools));

  EXPECT_EQ(true, useResourcePoolsFlagsSet());
}

TEST(
    ResourcePoolMigrationHelperTest,
    MigrationIsDisabledAndOptOutFlagsUnchangedIfUseResoursePoolsFlagsSetIsFalse) {
  gflags::FlagSaver flagSaver;

  // Normally, these flag settings would enable the migration, but not if we
  // lose the race to calling `useResourcePoolsFlagsSet()` and it returns
  // `false`.
  FLAGS_thrift_disable_resource_pool_migration = false;
  THRIFT_FLAG_SET_MOCK(enable_resource_pool_migration, true);

  FLAGS_thrift_experimental_use_resource_pools = false;
  FLAGS_thrift_disable_resource_pools = true;
  THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, false);

  // Simulate another caller beating us in the race to calling
  // `useResourcePoolsFlagsSet()` and it returned `false`.
  EXPECT_EQ(false, useResourcePoolsFlagsSet());

  // This call should not modify the opt-out flags.
  EXPECT_EQ(false, ResourcePoolMigrationHelper::isMigrationEnabled());

  EXPECT_EQ(false, FLAGS_thrift_experimental_use_resource_pools);
  EXPECT_EQ(true, FLAGS_thrift_disable_resource_pools);
  EXPECT_EQ(false, THRIFT_FLAG(experimental_use_resource_pools));
}
