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

#include <memory>
#include <string>
#include <vector>

#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {
namespace {

/**
 * Inherit from t_type to access protected declarations
 */
class t_type_fake : public t_type {
 public:
  t_type_fake() : t_type() {}

  explicit t_type_fake(t_program* program) : t_type(program) {}

  /**
   * Override abstract functions with fake implementations
   */
  std::string get_full_name() const override { return full_name_; }

  type get_type_value() const override { return {}; }

  /**
   * full_name_ setter to have granular control over get_full_name() for tests
   */
  void set_full_name(std::string full_name) {
    full_name_ = std::move(full_name);
  }

 private:
  std::string full_name_;
};

TEST(TType, GetScopedNameNoNameOrProgram) {
  t_type_fake fake_ttype;
  EXPECT_EQ("", fake_ttype.get_scoped_name());
}

TEST(TType, GetScopedNameNoProgram) {
  const std::string ttype_name = "fake";

  t_type_fake fake_ttype;
  fake_ttype.set_name(ttype_name);

  EXPECT_EQ(ttype_name, fake_ttype.get_scoped_name());
}

TEST(TType, GetScopedName) {
  const std::string ttype_name = "fake";
  const std::string base_file_name = "ttypetest";
  const std::string file_path = "/this/is/a/path/" + base_file_name + ".thrift";
  std::unique_ptr<t_program> named_program(new t_program(file_path));

  t_type_fake fake_ttype(named_program.get());
  fake_ttype.set_name(ttype_name);

  EXPECT_EQ(base_file_name + "." + ttype_name, fake_ttype.get_scoped_name());
}

TEST(TType, GetTypeId) {
  const std::vector<std::string> full_names{
      " ", " fake", "prefix fake", " filename.fake", "prefix filename.fake"};

  const std::vector<uint64_t> expect{
      719194018156337312u,
      13248906732611438720u,
      14547348474893505952u,
      11756729886077015840u,
      18229475703801871904u,
  };

  std::vector<uint64_t> hashes;
  for (const auto& full_name : full_names) {
    t_type_fake fake_ttype;
    fake_ttype.set_full_name(full_name);
    hashes.push_back(fake_ttype.get_type_id());
  }

  EXPECT_EQ(expect, hashes);
}

TEST(TType, TypeName) {
  EXPECT_EQ(t_type::type_name(t_type::type::t_void), "void");
  EXPECT_EQ(t_type::type_name(t_type::type::t_string), "string");
  EXPECT_EQ(t_type::type_name(t_type::type::t_bool), "bool");
  EXPECT_EQ(t_type::type_name(t_type::type::t_byte), "byte");
  EXPECT_EQ(t_type::type_name(t_type::type::t_i16), "i16");
  EXPECT_EQ(t_type::type_name(t_type::type::t_i32), "i32");
  EXPECT_EQ(t_type::type_name(t_type::type::t_i64), "i64");
  EXPECT_EQ(t_type::type_name(t_type::type::t_double), "double");
  EXPECT_EQ(t_type::type_name(t_type::type::t_enum), "enum");
  EXPECT_EQ(t_type::type_name(t_type::type::t_list), "list");
  EXPECT_EQ(t_type::type_name(t_type::type::t_set), "set");
  EXPECT_EQ(t_type::type_name(t_type::type::t_map), "map");
  EXPECT_EQ(t_type::type_name(t_type::type::t_struct), "struct");
  EXPECT_EQ(t_type::type_name(t_type::type::t_service), "service");
  EXPECT_EQ(t_type::type_name(t_type::type::t_program), "program");
  EXPECT_EQ(t_type::type_name(t_type::type::t_float), "float");
  EXPECT_EQ(t_type::type_name(t_type::type::t_stream), "stream");
  EXPECT_EQ(t_type::type_name(t_type::type::t_binary), "binary");
}

} // namespace
} // namespace apache::thrift::compiler
