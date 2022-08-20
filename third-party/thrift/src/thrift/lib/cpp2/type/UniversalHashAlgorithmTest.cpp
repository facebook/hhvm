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

#include <thrift/lib/cpp2/type/UniversalHashAlgorithm.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/thrift/gen-cpp2/standard_constants.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::type {
namespace {

TEST(UniversalHashAlgorithmTest, Conversions) {
  EXPECT_EQ(
      static_cast<UniversalHashAlgorithm>(UniversalHashAlgorithmEnum::Sha2_256),
      UniversalHashAlgorithm::Sha2_256);
}

TEST(UniversalHashAlgorithmTest, ConstConversions) {
  static_assert(
      kDefaultHashBytes == standard_constants::defaultTypeHashBytes());
  static_assert(kMinHashBytes == standard_constants::minTypeHashBytes());
}

} // namespace
} // namespace apache::thrift::type
