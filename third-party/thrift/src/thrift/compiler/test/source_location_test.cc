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

#include <fstream>
#include <string_view>

#include <boost/filesystem/operations.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/ScopeGuard.h>
#include <thrift/compiler/source_location.h>

using namespace apache::thrift::compiler;
namespace fs = boost::filesystem;

TEST(SourceLocationTest, get_file) {
  auto sm = source_manager();
  auto text = std::string("test");
  auto file = fs::temp_directory_path() / fs::unique_path();
  const auto& file_name = file.string();
  std::ofstream(file_name) << text;
  auto source = sm.get_file(file_name);
  auto loc = sm.resolve_location(source->start);
  EXPECT_EQ(loc.file_name(), file_name);
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);
  EXPECT_EQ(std::string_view(text.c_str(), text.size() + 1), source->text);
}

TEST(SourceLocationTest, get_file_error) {
  auto sm = source_manager();
  EXPECT_FALSE(sm.get_file("nonexistent"));
}

TEST(SourceLocationTest, add_virtual_file) {
  auto sm = source_manager();
  auto text = std::string("test");
  auto source = sm.add_virtual_file("path/to/file", text);
  auto loc = sm.resolve_location(source.start);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);
  EXPECT_EQ(std::string_view(text.c_str(), text.size() + 1), source.text);
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

TEST(SourceLocationTest, get_text_range) {
  source_manager sm;
  auto source = sm.add_virtual_file("path/to/file", R"(First line
Second    Line)");
  EXPECT_EQ(sm.get_text_range({source.start, source.start + 5}), "First");
  EXPECT_EQ(
      sm.get_text_range({source.start + 5, source.start + 17}),
      " line\nSecond");
}

TEST(SourceLocationTest, stable_file_name) {
  auto sm = source_manager();
  auto source = sm.add_virtual_file("f1", "");
  auto file_name = sm.resolve_location(source.start).file_name();
  sm.add_virtual_file("f2", "");
  // Check that the pointer to file name hasn't changed.
  EXPECT_EQ(file_name, sm.resolve_location(source.start).file_name());
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

  auto loc = sm.resolve_location(source.start);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);

  loc = sm.resolve_location(source.start + 4);
  EXPECT_STREQ(loc.file_name(), "path/to/file");
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 5);

  loc = sm.resolve_location(source.start + 6);
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
  auto loc = sm.resolve_location(source->start);
  EXPECT_EQ(loc.file_name(), file_name);
  EXPECT_EQ(loc.line(), 1);
  EXPECT_EQ(loc.column(), 1);
  EXPECT_EQ(std::string_view(text.c_str(), text.size() + 1), source->text);
}

TEST(SourceLocationTest, ignore_file_system) {
  auto text = std::string("test");
  auto file = fs::temp_directory_path() / fs::unique_path();
  const auto& file_name = file.string();
  std::ofstream(file_name) << text;
  auto guard = folly::makeGuard([&] { fs::remove(file); });

  auto sm = source_manager(nullptr /* backend */);
  EXPECT_FALSE(sm.get_file(file_name));
  sm.add_virtual_file(file_name, text);

  // sm.get_file(file_name)->text has an extra \0 at the end. Adding `data()` to
  // avoid this issue.
  EXPECT_EQ(text, sm.get_file(file_name)->text.data());
}
