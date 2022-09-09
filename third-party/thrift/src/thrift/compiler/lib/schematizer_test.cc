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
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_map.h>
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
  if (auto def = map.find("definition"); def != map.end()) {
    for (const auto& pair : def->second->get_map()) {
      map[pair.first->get_string()] = pair.second;
    }
  }
  return map;
}
} // namespace

TEST(SchematizerTest, Structured) {
  t_struct s(nullptr, "Struct");
  s.set_uri("path/to/Struct");
  s.create_field(t_base_type::t_i16(), "i16", 1);
  s.create_field(s, "Struct", 2).set_qualifier(t_field_qualifier::optional);
  t_map tmap(t_base_type::t_string(), t_base_type::t_double());
  s.create_field(tmap, "Map", 3);

  auto schema = schematizer::gen_schema(s);
  EXPECT_EQ(schema->get_type(), t_const_value::CV_MAP);
  auto map = flatten_map(*schema);
  const auto& fields = map.at("fields")->get_list();

  EXPECT_EQ(map.at("name")->get_string(), "Struct");
  EXPECT_EQ(map.at("uri")->get_string(), "path/to/Struct");
  EXPECT_EQ(fields.size(), 3);

  auto field1 = flatten_map(*fields.at(0));
  EXPECT_EQ(field1.at("id")->get_integer(), 1);
  EXPECT_EQ(field1.at("qualifier")->get_integer(), 3);
  EXPECT_EQ(field1.at("name")->get_string(), "i16");
  auto type1 = flatten_map(*field1.at("type"));
  EXPECT_EQ(type1.at("name")->get_map().at(0).first->get_string(), "i16Type");
  EXPECT_FALSE(type1.count("params"));

  auto field2 = flatten_map(*fields.at(1));
  EXPECT_EQ(field2.at("id")->get_integer(), 2);
  EXPECT_EQ(field2.at("qualifier")->get_integer(), 1);
  EXPECT_EQ(field2.at("name")->get_string(), "Struct");
  auto type2 = flatten_map(*field2.at("type"));
  EXPECT_EQ(
      type2.at("name")->get_map().at(0).first->get_string(), "structType");
  EXPECT_EQ(
      type2.at("name")
          ->get_map()
          .at(0)
          .second->get_map()
          .at(0)
          .second->get_string(),
      "path/to/Struct");
  EXPECT_FALSE(type2.count("params"));

  auto field3 = flatten_map(*fields.at(2));
  EXPECT_EQ(field3.at("name")->get_string(), "Map");
  auto type3 = flatten_map(*field3.at("type"));
  EXPECT_EQ(type3.at("name")->get_map().at(0).first->get_string(), "mapType");
  auto params3 = type3.at("params")->get_list();
  EXPECT_EQ(params3.size(), 2);
  EXPECT_EQ(
      flatten_map(*params3.at(0))
          .at("name")
          ->get_map()
          .at(0)
          .first->get_string(),
      "stringType");
  EXPECT_EQ(
      flatten_map(*params3.at(1))
          .at("name")
          ->get_map()
          .at(0)
          .first->get_string(),
      "doubleType");
}

} // namespace apache::thrift::compiler
