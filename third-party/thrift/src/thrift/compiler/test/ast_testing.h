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

#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler::test {

class BaseProgramTest : public testing::Test {
 protected:
  t_program program_{"path/to/program.thrift"};
  // TODO(afuller): Add source manager, and upddate helpers to set locations.
  // source_manager mgr_;

  template <typename T>
  T& create_def(const std::string& name, fmt::string_view uri = {}) {
    return program_.add_def(std::make_unique<T>(&program_, name), uri);
  }

  t_struct& create_annotation(const std::string& name, fmt::string_view uri) {
    return create_def<t_struct>(name, uri);
  }

  t_struct& create_annotation(const std::string& name) {
    return create_annotation(name, "facebook.com/thrift/annotation/" + name);
  }

  std::unique_ptr<t_const> create_const(const t_struct& type) {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->set_ttype(type);
    return std::make_unique<t_const>(&program_, type, "", std::move(value));
  }

  void add_annot(const t_struct& type, t_named& node) {
    node.add_structured_annotation(create_const(type));
  }
};

// TODO(afuller): Use to factor out code from moar tests!!
class BaseVisitorTest : public BaseProgramTest {
 protected:
  BaseVisitorTest() : BaseProgramTest{} {}
};

} // namespace apache::thrift::compiler::test
