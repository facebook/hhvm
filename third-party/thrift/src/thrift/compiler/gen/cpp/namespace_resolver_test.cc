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

#include <thrift/compiler/gen/cpp/namespace_resolver.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace apache::thrift::compiler::gen::cpp {
namespace {

class NamespaceResolverTest : public ::testing::Test {
 protected:
  namespace_resolver namespaces_;
};

TEST_F(NamespaceResolverTest, gen_namespace_components_cpp2) {
  t_program p("path/to/program.thrift");
  p.set_namespace("cpp2", "foo.bar");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_THAT(
      namespace_resolver::gen_namespace_components(p),
      testing::ElementsAreArray({"foo", "bar"}));
}

TEST_F(NamespaceResolverTest, gen_namespace_components_cpp) {
  t_program p("path/to/program.thrift");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_THAT(
      namespace_resolver::gen_namespace_components(p),
      testing::ElementsAreArray({"baz", "foo", "cpp2"}));
}

TEST_F(NamespaceResolverTest, gen_namespace_components_none) {
  t_program p("path/to/program.thrift");
  EXPECT_THAT(
      namespace_resolver::gen_namespace_components(p),
      testing::ElementsAreArray({"cpp2"}));
}

TEST_F(NamespaceResolverTest, get_namespace_cpp2) {
  t_program p("path/to/program.thrift");
  p.set_namespace("cpp2", "foo.bar");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_EQ("::foo::bar", namespaces_.get_namespace(p));
}

TEST_F(NamespaceResolverTest, get_namespace_cpp) {
  t_program p("path/to/program.thrift");
  p.set_namespace("cpp", "baz.foo");
  EXPECT_EQ("::baz::foo::cpp2", namespaces_.get_namespace(p));
}

TEST_F(NamespaceResolverTest, get_namespace_none) {
  t_program p("path/to/program.thrift");
  EXPECT_EQ("::cpp2", namespaces_.get_namespace(p));
}

TEST_F(NamespaceResolverTest, get_namespace_from_package) {
  t_program p("path/to/program.thrift");
  p.set_package(t_package("foo.bar/path/to/program"));
  EXPECT_EQ("::foo::path::to::program", namespaces_.get_namespace(p));
}

TEST_F(NamespaceResolverTest, gen_namespaced_name) {
  t_program p("path/to/program.thrift");
  p.set_namespace("cpp2", "foo.bar");
  t_enum e1(&p, "MyEnum1");
  t_enum e2(&p, "MyEnum2");
  e2.set_annotation("cpp.name", "YourEnum");
  EXPECT_EQ("::foo::bar::MyEnum1", namespaces_.get_namespaced_name(e1));
  EXPECT_EQ("::foo::bar::YourEnum", namespaces_.get_namespaced_name(e2));
}

} // namespace
} // namespace apache::thrift::compiler::gen::cpp
