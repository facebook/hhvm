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

#include <thrift/compiler/test/compiler.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <fmt/ostream.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <re2/re2.h>

#include <thrift/compiler/compiler.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler {
// Add newlines when printing diagnostics to improve readability.
void PrintTo(const diagnostic& d, std::ostream* os) {
  fmt::print(*os, "\n{}\n", d);
}

// Avoid order dependence when comparing diagnostics on one line.
bool operator<(const diagnostic& diag1, const diagnostic& diag2) {
  // @lint-ignore-every CLANGTIDY facebook-hte-MissingBraces
  if (diag1.file() != diag2.file())
    return diag1.file() < diag2.file();
  if (diag1.lineno() != diag2.lineno())
    return diag1.lineno() < diag2.lineno();
  if (diag1.level() != diag2.level())
    return diag1.level() < diag2.level();
  if (diag1.message() != diag2.message())
    return diag1.message() < diag2.message();
  if (diag1.name() != diag2.name())
    return diag1.name() < diag2.name();
  return false;
}
} // namespace apache::thrift::compiler

namespace apache::thrift::compiler::test {

struct temp_dir {
  boost::filesystem::path dir;

  temp_dir() {
    dir = boost::filesystem::temp_directory_path() /
        boost::filesystem::unique_path();
    boost::filesystem::create_directory(dir);
  }

  ~temp_dir() { boost::filesystem::remove_all(dir); }
  std::string string() const { return dir.string(); }
};

std::vector<diagnostic> extract_expected_diagnostics(
    const std::string_view& source, const std::string& file_name) {
  std::vector<diagnostic> result;
  int line_number = 1;
  for (auto it = source.begin(); it != source.end();) {
    auto line_end = std::find(it, source.end(), '\n');

    re2::StringPiece line(&*it, line_end - it);
    re2::StringPiece type_match, message_match, line_num_match;
    re2::RE2 diagnostic_pattern(
        "#\\s*expected-(?P<type>error|warning)@?(?P<linenum>[+-]?\\d+)?:\\s*(?P<message>.*)$");

    if (re2::RE2::PartialMatch(
            line,
            diagnostic_pattern,
            &type_match,
            &line_num_match,
            &message_match)) {
      diagnostic_level level = type_match.as_string() == "error"
          ? diagnostic_level::error
          : diagnostic_level::warning;
      auto ln_no_str = line_num_match.as_string();
      int line_num = line_number;
      if (!ln_no_str.empty()) {
        line_num = (ln_no_str.at(0) == '+' || ln_no_str.at(0) == '-')
            ? line_num + std::stoi(ln_no_str)
            : std::stoi(ln_no_str);
      }
      result.emplace_back(
          level, message_match.as_string(), file_name, line_num);
    }

    if (line_end == source.end()) {
      break;
    }
    it = line_end + 1;
    line_number++;
  }
  return result;
}

void check_compile(
    const std::map<std::string, std::string>& name_contents_map,
    const std::string& file_name,
    std::vector<std::string> args) {
  source_manager smgr;
  temp_dir tmp_dir;
  std::vector<diagnostic> expected_diagnostics;

  for (const auto& entry : name_contents_map) {
    smgr.add_virtual_file(entry.first, entry.second);
    std::vector<diagnostic> extracted_diagnostics =
        extract_expected_diagnostics(entry.second, entry.first);
    expected_diagnostics.insert(
        expected_diagnostics.end(),
        extracted_diagnostics.begin(),
        extracted_diagnostics.end());
  }
  compile_retcode expected_ret_code =
      std::any_of(
          expected_diagnostics.begin(),
          expected_diagnostics.end(),
          [](const auto& diag) {
            return diag.level() == diagnostic_level::error;
          })
      ? compile_retcode::failure
      : compile_retcode::success;

  args.insert(args.begin(), "thrift_binary"); // Ignored argument
  args.emplace_back("--gen"); // generator arg
  args.emplace_back("mstch_cpp2"); // target language
  args.emplace_back("-o"); // output directory
  args.emplace_back(tmp_dir.string() + "/");
  if (auto* includes = std::getenv("IMPLICIT_INCLUDES")) {
    args.emplace_back("-I"); // include directory
    args.emplace_back(includes);
  }
  args.emplace_back(file_name); // input file name

  compile_result result = compile(args, smgr);
  EXPECT_EQ(expected_ret_code, result.retcode);
  std::vector<diagnostic> result_diagnostics = result.detail.diagnostics();
  std::sort(result_diagnostics.begin(), result_diagnostics.end());
  std::sort(expected_diagnostics.begin(), expected_diagnostics.end());

  EXPECT_THAT(result_diagnostics, ::testing::ContainerEq(expected_diagnostics));
}

void check_compile(const std::string& source, std::vector<std::string> args) {
  const std::string TEST_FILE_NAME = "test.thrift";
  std::map<std::string, std::string> file_contents_map{
      {TEST_FILE_NAME, source}};
  check_compile(file_contents_map, TEST_FILE_NAME, std::move(args));
}

} // namespace apache::thrift::compiler::test
