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

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <thrift/compiler/detail/system.h>

namespace apache::thrift::compiler {

struct TestCase {
  TestCase(std::string base_path, std::string path, std::string expected_path)
      : base_path{std::move(base_path)},
        path{std::move(path)},
        expected_path{std::move(expected_path)} {}

  std::filesystem::path base_path;
  std::filesystem::path path;
  std::filesystem::path expected_path;
};

TEST(SystemTest, MakeAbsPath) {
  const std::vector<TestCase> kCasesForWindows{
      {"C:/i/am/base/path",
       "C:/i/am\\\\absolute/path",
       R"(\\?\C:\i\am\absolute\path)"},
      {"C:/i/am\\base/path", "append/me", R"(\\?\C:\i\am\base\path\append\me)"},
      {"C:/i/am/base/path", "", R"(\\?\C:\i\am\base\path)"},
      {"C:/i/am/a/veeeeery/long/path/Lorem\\ipsum/dolor/sit/amet/consectetur/adipiscing/elit/sed/do/eiusmod/tempor/Lorem/ipsum/dolor/sit/amet/consectetur/adipiscing/elit/sed/do/eiusmod/tempor/Lorem/ipsum/dolor/sit/amet/consectetur/adipiscing/elit/sed/do/eiusmod/tempor/ipsum",
       R"(\append\me\\please)",
       R"(\\?\C:\i\am\a\veeeeery\long\path\Lorem\ipsum\dolor\sit\amet\consectetur\adipiscing\elit\sed\do\eiusmod\tempor\Lorem\ipsum\dolor\sit\amet\consectetur\adipiscing\elit\sed\do\eiusmod\tempor\Lorem\ipsum\dolor\sit\amet\consectetur\adipiscing\elit\sed\do\eiusmod\tempor\ipsum\append\me\please)"},
      {"C:/i/am\\base/./path",
       "append/./me",
       R"(\\?\C:\i\am\base\path\append\me)"},
      {"C:/i/am/base/path",
       R"(\append/../me\\please)",
       R"(\\?\C:\i\am\base\path\me\please)"},
  };
  const std::vector<TestCase> kCasesForOther{
      {"/i/am/base/path", "/i/am/absolute/path", "/i/am/absolute/path"},
      {"/i/am/base/path", "append/me", "/i/am/base/path/append/me"},
      {"/i/am/base/path", "", "/i/am/base/path"},
  };

  const auto& kCases =
      detail::platform_is_windows() ? kCasesForWindows : kCasesForOther;
  for (const auto& testCase : kCases) {
    auto actual_path = detail::make_abs_path(testCase.base_path, testCase.path);

    EXPECT_EQ(testCase.expected_path, actual_path);
    EXPECT_EQ(true, actual_path.is_absolute());
  }
}

} // namespace apache::thrift::compiler
