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

#include <folly/executors/ManualExecutor.h>
#include <folly/portability/GTest.h>
#include <wangle/client/persistence/PersistentCacheCommon.h>

using namespace testing;

namespace wangle {

class PersistentCacheConfigTest : public Test {};

TEST_F(PersistentCacheConfigTest, ConfigBuilderCanBuild) {
  auto executor = std::make_shared<folly::ManualExecutor>();
  auto config = PersistentCacheConfig::Builder()
                    .setCapacity(135)
                    .setSyncInterval(std::chrono::seconds(200))
                    .setSyncRetries(246)
                    .setInlinePersistenceLoading(false)
                    .setExecutor(executor)
                    .build();
  EXPECT_EQ(135, config.capacity);
  EXPECT_EQ(std::chrono::seconds(200), config.syncInterval);
  EXPECT_EQ(246, config.nSyncRetries);
  EXPECT_FALSE(config.inlinePersistenceLoading);
  EXPECT_EQ(executor, config.executor);
}

} // namespace wangle
