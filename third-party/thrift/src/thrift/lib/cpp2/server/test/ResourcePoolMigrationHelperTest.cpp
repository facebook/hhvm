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
