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

#include <thrift/compiler/generate/json.h>
#include <thrift/compiler/test/parser_test_helpers.h>

#include <sstream>
#include <folly/portability/GTest.h>

using namespace apache::thrift::compiler;

TEST(JsonTest, json_quote_ascii_string) {
  auto actual = json_quote_ascii("the\bquick\"brown\nfox\001jumps\201over");
  EXPECT_EQ("\"the\\bquick\\\"brown\\nfox\\u0001jumps\\u0081over\"", actual);
}

TEST(JsonTest, json_quote_ascii_stream) {
  std::ostringstream actual;
  json_quote_ascii(actual, "the\bquick\"brown\nfox\001jumps\201over");
  EXPECT_EQ(
      "\"the\\bquick\\\"brown\\nfox\\u0001jumps\\u0081over\"", actual.str());
}

TEST(JsonTest, to_json_t_const_value) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(source_mgr, R"(
    enum MyEnum {
      FIRST = 1;
      SECOND = 2;
    }
    struct Nested {
      1: i32 nested_int;
      2: MyEnum nested_enum;
    }
    struct MyAnnotation {
      1: bool my_bool;
      2: i64 my_int;
      3: string my_string;
      4: double my_double;
      5: list<float> my_list;
      6: Nested my_nested;
    }
    @MyAnnotation{
      my_bool = true,
      my_int = 1,
      my_string = "hello",
      my_double = 9.9,
      my_list = [0.1, -0.2],
      my_nested = Nested{
        nested_int = 0,
        nested_enum = MyEnum.SECOND,
      },
    }
    struct MyStruct{}
  )");

  const std::vector<t_structured*>& structs = program->structs_and_unions();
  EXPECT_EQ(structs.size(), 3);

  const t_structured* my_struct = structs[2];
  auto annotations = my_struct->structured_annotations();
  EXPECT_EQ(annotations.size(), 1);

  std::string to_json_result = to_json(annotations.at(0).value());
  EXPECT_EQ(
      "{\"my_bool\": true, \"my_int\": 1, \"my_string\": \"hello\", "
      "\"my_double\": 9.9, \"my_list\": [0.1, -0.2], "
      "\"my_nested\": {\"nested_int\": 0, \"nested_enum\": 2}}",
      to_json_result);
}
