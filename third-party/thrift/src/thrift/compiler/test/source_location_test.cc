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

#include <folly/experimental/TestUtil.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/source_location.h>

using namespace apache::thrift::compiler;

TEST(SourceLocationTest, get_file) {
  auto sm = source_manager();
  auto text = std::string("test");
  auto file = folly::test::TemporaryFile();
  auto file_name = file.path().string();
  std::ofstream(file_name) << text;
  auto source = sm.get_file(file_name);
  auto loc = resolved_location(source->start, sm);
  EXPECT_EQ(loc.file_name(), file_name);
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);
  EXPECT_EQ(fmt::string_view(text.c_str(), text.size() + 1), source->text);
}

TEST(SourceLocationTest, get_file_error) {
  auto sm = source_manager();
  EXPECT_FALSE(sm.get_file("nonexistent"));
}

TEST(SourceLocationTest, add_virtual_file) {
  auto sm = source_manager();
  auto text = std::string("test");
  auto source = sm.add_virtual_file("path/to/file", text);
  auto loc = resolved_location(source.start, sm);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);
  EXPECT_EQ(fmt::string_view(text.c_str(), text.size() + 1), source.text);
}

TEST(SourceLocationTest, get_source_start) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("path/to/file", "test");
  auto loc = source.start + 2;
  EXPECT_NE(loc, source.start);
  EXPECT_EQ(sm.get_source_start(loc), source.start);
}

TEST(SourceLocationTest, get_text) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("path/to/file", "test");
  auto text = source.text.data();
  EXPECT_EQ(sm.get_text(source.start), text);
  EXPECT_EQ(sm.get_text(source.start + 2), text + 2);
}

TEST(SourceLocationTest, stable_file_name) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("f1", "");
  auto file_name = resolved_location(source.start, sm).file_name();
  sm.add_virtual_file("f2", "");
  // Check that the pointer to file name hasn't changed.
  EXPECT_EQ(file_name, resolved_location(source.start, sm).file_name());
}

TEST(SourceLocationTest, compare) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("path/to/file", "test");
  auto loc = source.start;
  EXPECT_TRUE(source.start == loc);
  EXPECT_FALSE(source.start != loc);
  loc = loc + 1;
  EXPECT_FALSE(source.start == loc);
  EXPECT_TRUE(source.start != loc);
}

TEST(SourceLocationTest, offset) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("path/to/file", "test");
  auto loc = source.start;
  EXPECT_EQ(loc.offset(), 0);
  loc = loc + 2;
  EXPECT_EQ(loc.offset(), 2);
  EXPECT_EQ(source_location().offset(), 0);
}

TEST(SourceLocationTest, multi_line) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("path/to/file", "line1\nline2");

  auto loc = resolved_location(source.start, sm);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);

  loc = resolved_location(source.start + 4, sm);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 5);

  loc = resolved_location(source.start + 6, sm);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 2);
  EXPECT_EQ(loc.column(), 1);
}

TEST(SourceLocationTest, get_cached_virtual_file) {
  auto sm = source_manager();
  auto text = std::string("test");
  auto file_name = std::string("virtual_file");
  sm.add_virtual_file(file_name, text);
  auto source = sm.get_file(file_name);
  auto loc = resolved_location(source->start, sm);
  EXPECT_EQ(loc.file_name(), file_name);
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);
  EXPECT_EQ(fmt::string_view(text.c_str(), text.size() + 1), source->text);
}
