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

#include <thrift/compiler/lib/schematizer.h>

#include <unordered_map>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_struct.h>

namespace apache::thrift::compiler {
namespace {
// Converts map const val to c++ map and flattens definition mixin
std::unordered_map<std::string, t_const_value*> flatten_map(
    const t_const_value& val) {
  std::unordered_map<std::string, t_const_value*> map;
  for (const auto& pair : val.get_map()) {
    map[pair.first->get_string()] = pair.second;
  }
  for (const auto& pair : map.at("definition")->get_map()) {
    map[pair.first->get_string()] = pair.second;
  }
  return map;
}
} // namespace

TEST(SchematizerTest, Structured) {
  t_struct s(nullptr, "Struct");
  s.set_uri("path/to/Struct");

  auto schema = schematizer::gen_schema(s);
  EXPECT_EQ(schema->get_type(), t_const_value::CV_MAP);
  auto map = flatten_map(*schema);
  const auto& fields = map.at("fields")->get_list();

  EXPECT_EQ(map.at("name")->get_string(), "Struct");
  EXPECT_EQ(map.at("uri")->get_string(), "path/to/Struct");
  EXPECT_EQ(fields.size(), 0);
}

} // namespace apache::thrift::compiler
