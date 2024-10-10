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

#include <string>
#include <vector>

#include <folly/portability/GTest.h>

#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/lib/py3/util.h>

namespace apache::thrift::compiler {

TEST(GenerateCommon, SplitNamespace) {
  const std::vector<std::string> namespaces{
      "",
      "this",
      "this.is",
      "this.is.valid",
  };

  const std::vector<std::vector<std::string>> expected{
      {},
      {"this"},
      {"this", "is"},
      {"this", "is", "valid"},
  };

  std::vector<std::vector<std::string>> splits;
  splits.reserve(namespaces.size());
  for (const auto& ns : namespaces) {
    splits.push_back(split_namespace(ns));
  }

  EXPECT_EQ(expected, splits);
}

TEST(GenerateCommon, StripCppCommentsAndNewlines) {
  std::vector<std::string> validCases{
      {},
      {"no comments"},
      {"two\nlines"},
      {"/*foo*/"},
      {"one/*foo*/comment"},
      {"this/*foo*/has/*bar*/three/*baz*/comments"},
      {"three/*before*/\nlines\n/*after*/with/*mid\ndle*/comments"},
      {"//foo\na//bar\nlot/*\n//baz*/\ncomments\n//foo"},
  };

  const std::vector<std::string> expected{
      {},
      {"no comments"},
      {"two lines"},
      {""},
      {"onecomment"},
      {"thishasthreecomments"},
      {"three lines withcomments"},
      {" a lot comments "},
  };

  for (auto& s : validCases) {
    strip_cpp_comments_and_newlines(s);
  }

  EXPECT_EQ(expected, validCases);

  std::string unpaired{"unpaired/*foo*/comments/*baz* /"};
  EXPECT_THROW(strip_cpp_comments_and_newlines(unpaired), std::runtime_error);
}

} // namespace apache::thrift::compiler
