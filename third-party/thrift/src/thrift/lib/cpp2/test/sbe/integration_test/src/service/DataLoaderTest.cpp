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

#define SBE_ENABLE_PRECEDENCE_CHECKS 1
#include <gtest/gtest.h>
#include <folly/String.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/DataLoader.h>

using namespace facebook::sbe::test;
using apache::thrift::sbe::MessageWrapper;

const auto kPath =
    "thrift/lib/cpp2/test/sbe/integration_test/resources/test_data.txt";

TEST(DataLoaderTest, TestLoaderConstructor) {
  DataLoader loader;
  std::vector<std::string> lines;
  loader.loadLines(kPath, lines);
  std::cout << "First Line: " << lines.front() << std::endl;
  EXPECT_EQ(lines.size(), 20000);
}

TEST(DataLoaderTest, TestLoadMap) {
  auto map = folly::F14FastMap<std::string, Customer>();
  DataLoader loader;
  loader.loadIntoMap(kPath, map);
  EXPECT_EQ(map.size(), 20000);

  // Row for testing
  // 34,B0Cbb4e7B655D5A,James,Rush,Schmidt Group,West Herbert,Saint Kitts and
  // Nevis,(368)251-7910x824,488.747.4904x403,brandychang@neal-adams.net,2021-12-18,http://montoya.info/
  const auto& customer = map["B0Cbb4e7B655D5A"];

  EXPECT_EQ(customer.index, 34);
  EXPECT_EQ(customer.customerId, std::string("B0Cbb4e7B655D5A"));
}
