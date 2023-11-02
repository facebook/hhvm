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

#include <thrift/compiler/ast/name_index.h>

#include <memory>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>

namespace apache::thrift::compiler {
namespace {

class NameIndexTest : public ::testing::Test {};

TEST_F(NameIndexTest, FindPutReplace) {
  t_program program("/path/to/file.thrift");
  name_index<t_struct> index;
  EXPECT_EQ(index.contains("s"), false);
  EXPECT_EQ(index.contains("t"), false);
  EXPECT_EQ(index.find("s"), nullptr);

  // Can't use string temporaries.
  // Uncomment to see expected compiler error.
  // index.put("hi", nullptr);

  auto s1 = std::make_unique<t_struct>(&program, "s");
  EXPECT_EQ(index.put(*s1), nullptr);
  EXPECT_EQ(index.contains("s"), true);
  EXPECT_EQ(index.contains("t"), false);
  EXPECT_EQ(index.find("s"), s1.get());

  auto s2 = std::make_unique<t_struct>(&program, "s");
  EXPECT_EQ(index.put(*s2), s1.get());
  s1.reset(); // Safe to delete once replaced in the index.
  EXPECT_EQ(index.contains("s"), true);
  EXPECT_EQ(index.contains("t"), false);
  EXPECT_EQ(index.find("s"), s2.get());

  index.clear();
  s2.reset(); // Safe to delete once cleared from the index.
  EXPECT_EQ(index.contains("s"), false);
  EXPECT_EQ(index.contains("t"), false);
  EXPECT_EQ(index.find("s"), nullptr);
}

TEST_F(NameIndexTest, PutAll) {
  t_program program("/path/to/file.thrift");
  auto s1 = std::make_unique<t_struct>(&program, "s1");
  auto s2a = std::make_unique<t_struct>(&program, "s2");
  auto s2b = std::make_unique<t_struct>(&program, "s2");
  auto s3 = std::make_unique<t_struct>(&program, "s3");

  name_index<t_struct> index1;
  name_index<t_struct> index2;

  index1.put(*s1);
  index1.put(*s2a);
  index2.put(*s2b);
  index2.put(*s3);
  index1.put_all(index2);

  EXPECT_EQ(index1.find("s1"), s1.get());
  EXPECT_EQ(index1.find("s2"), s2b.get());
  EXPECT_EQ(index1.find("s3"), s3.get());

  std::vector<const t_struct*> nodes;
  index1.for_each([&](std::string_view name, const t_struct& node) {
    EXPECT_EQ(name, node.name());
    nodes.emplace_back(&node);
  });
  EXPECT_THAT(
      nodes, ::testing::UnorderedElementsAre(s1.get(), s2b.get(), s3.get()));
}

} // namespace
} // namespace apache::thrift::compiler
