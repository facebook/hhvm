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
#include <thrift/lib/cpp2/server/PerturbSource.h>

using namespace ::testing;
using namespace apache::thrift;

TEST(ThriftServerPerturbSourceTest, BasicTest) {
  auto perturbSource =
      std::make_shared<PerturbSource>(std::chrono::milliseconds(10));
  auto now = perturbSource->perturbedId(1);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  EXPECT_NE(now, perturbSource->perturbedId(1));
}
