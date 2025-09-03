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

#include <thrift/compiler/ast/scope_identifier.h>

using namespace apache::thrift::compiler::scope;
using source_range = apache::thrift::compiler::source_range;

identifier make_id(std::string_view name) {
  return identifier{name, source_range{}};
}

identifier::Pieces make_pieces(
    std::string_view first, std::string_view second, std::string_view third) {
  return identifier::Pieces{first, second, third};
}

std::pair<std::string_view, std::string_view> make_pieces(
    std::string_view first, std::string_view second) {
  return std::make_pair(first, second);
}

TEST(ScopeIdentifierTest, parse) {
  for (const std::string_view ident : {"foo", "foo.bar", "foo.bar.baz"}) {
    const identifier id{ident, source_range{}};
    ASSERT_EQ(ident, id.fmtDebug());
  }
}

TEST(ScopeIdentifierTest, has_scope) {
  const identifier id = make_id("foo");
  ASSERT_FALSE(id.has_scope());

  const identifier id2 = make_id("foo.bar");
  ASSERT_TRUE(id2.has_scope());
}

TEST(ScopeIdentifierTest, is_scoped_id) {
  const identifier id = make_id("foo");
  ASSERT_FALSE(id.is_scoped_id());

  const identifier id2 = make_id("foo.bar");
  ASSERT_TRUE(id2.is_scoped_id());

  const identifier id3 = make_id("foo.bar.baz");
  ASSERT_FALSE(id3.is_scoped_id());
}

TEST(ScopeIdentifierTest, split) {
  const identifier id = make_id("foo");
  const identifier id2 = make_id("foo.bar");
  const identifier id3 = make_id("foo.bar.baz");

  ASSERT_EQ(
      id.split(), make_pieces(identifier::UNUSED, "foo", identifier::UNUSED));
  ASSERT_EQ(id2.split(), make_pieces("foo", "bar", identifier::UNUSED));
  ASSERT_EQ(id3.split(), make_pieces("foo", "bar", "baz"));
}

TEST(ScopeIdentifierTest, get_base_name) {
  const identifier id = make_id("foo");
  const identifier id2 = make_id("foo.bar");
  const identifier id3 = make_id("foo.bar.baz");

  ASSERT_EQ(id.get_base_name(), "foo");
  ASSERT_EQ(id2.get_base_name(), "bar");
  ASSERT_EQ(id3.get_base_name(), "baz");
}

TEST(ScopeIdentifierTest, visit) {
  const identifier id = make_id("foo");
  const identifier id2 = make_id("foo.bar");
  const identifier id3 = make_id("foo.bar.baz");

  auto bad_visit = [](const auto&) { return 0; };

  ASSERT_EQ(id.visit([](const unscoped_id&) { return 1; }, bad_visit), 1);
  ASSERT_EQ(id2.visit([](const scoped_id&) { return 1; }, bad_visit), 1);
  ASSERT_EQ(id3.visit([](const enum_id&) { return 1; }, bad_visit), 1);
}

TEST(ScopeIdentifierTest, scope) {
  const identifier id = make_id("foo");
  const identifier id2 = make_id("foo.bar");
  const identifier id3 = make_id("foo.bar.baz");

  ASSERT_FALSE(id.has_scope());

  ASSERT_TRUE(id2.has_scope());
  ASSERT_EQ(id2.scope(), "foo");

  ASSERT_TRUE(id3.has_scope());
  ASSERT_EQ(id3.scope(), "foo");
}

TEST(ScopeIdentifierTest, unscope) {
  const identifier id2 = make_id("foo.bar");
  const identifier id3 = make_id("foo.bar.baz");

  ASSERT_EQ(id2.unscope(), make_pieces("bar", identifier::UNUSED));
  ASSERT_EQ(id3.unscope(), make_pieces("bar", "baz"));
}
