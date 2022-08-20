/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/ProtocolMethodsTest_types.h>

using namespace apache::thrift::test;

TEST(ProtocolMethodsTest, roundtrip) {
  MyStruct before;
  before.num()->value = 42;
  before.str()->value = "foo";

  const auto after = apache::thrift::CompactSerializer::deserialize<MyStruct>(
      apache::thrift::CompactSerializer::serialize<std::string>(before));
  EXPECT_EQ(after, before);
}
