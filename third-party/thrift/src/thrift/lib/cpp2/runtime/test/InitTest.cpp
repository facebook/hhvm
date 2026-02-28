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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <stdexcept>

#include <thrift/lib/cpp2/runtime/Init.h>
#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>

TEST(InitTest, Basic) {
  EXPECT_EQ(
      apache::thrift::runtime::getGlobalLegacyClientEventHandlers().size(), 0);

  auto eventHandler1 =
      std::make_shared<apache::thrift::test::TrackingTProcessorEventHandler>();
  auto eventHandler2 =
      std::make_shared<apache::thrift::test::TrackingTProcessorEventHandler>();

  EXPECT_FALSE(apache::thrift::runtime::wasInitialized());

  {
    apache::thrift::runtime::InitOptions options;
    options.legacyClientEventHandlers.push_back(eventHandler1);
    options.legacyClientEventHandlers.push_back(eventHandler2);
    apache::thrift::runtime::init(std::move(options));
    EXPECT_TRUE(apache::thrift::runtime::wasInitialized());
  }
  auto globalEventHandlers =
      apache::thrift::runtime::getGlobalLegacyClientEventHandlers();
  EXPECT_EQ(globalEventHandlers.size(), 2);
  EXPECT_EQ(globalEventHandlers[0].get(), eventHandler1.get());
  EXPECT_EQ(globalEventHandlers[1].get(), eventHandler2.get());

  // Second call to init should be an error
  {
    apache::thrift::runtime::InitOptions options;
    options.legacyClientEventHandlers.push_back(
        std::make_shared<
            apache::thrift::test::TrackingTProcessorEventHandler>());
    EXPECT_THROW(
        apache::thrift::runtime::init(std::move(options)), std::logic_error);
    EXPECT_TRUE(apache::thrift::runtime::wasInitialized());
  }
  globalEventHandlers =
      apache::thrift::runtime::getGlobalLegacyClientEventHandlers();
  EXPECT_EQ(globalEventHandlers.size(), 2);
  EXPECT_EQ(globalEventHandlers[0].get(), eventHandler1.get());
  EXPECT_EQ(globalEventHandlers[1].get(), eventHandler2.get());
}
