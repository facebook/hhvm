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

#include <thrift/lib/cpp2/server/ThriftServerConfig.h>

using namespace ::testing;
using namespace ::apache::thrift;

TEST(ThriftServerConfigTest, UpdateConfigValueHavingDefault) {
  ThriftServerConfig config;

  // Zero should be an impossible value
  EXPECT_NE(config.getNumIOWorkerThreads(), 0);

  config.setDefaultNumIOWorkerThreads(0);
  EXPECT_EQ(config.getNumIOWorkerThreads(), 0);

  // No baseline value should be present
  EXPECT_FALSE(config.getBaselineNumIOWorkerThreads().has_value());

  // Set a baseline value and ensure its return by the getter
  config.setNumIOWorkerThreads(1, AttributeSource::BASELINE);
  ASSERT_TRUE(config.getBaselineNumIOWorkerThreads().has_value());
  EXPECT_EQ(config.getNumIOWorkerThreads(), 1);

  // Reset the baseline, the getter should return the default
  config.resetNumIOWorkerThreads(AttributeSource::BASELINE);
  EXPECT_EQ(config.getNumIOWorkerThreads(), 0);
}
