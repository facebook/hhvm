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

#include <stdexcept>
#include <string>
#include <vector>
#include <re2/re2.h>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

#include <thrift/compiler/compiler.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/source_location.h>

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

void check_compile(const std::string& source, std::vector<std::string> args) {
  const std::string TEST_FILE_NAME = "test.thrift";
  source_manager smgr;
  temp_dir tmp_dir;
  smgr.add_virtual_file(TEST_FILE_NAME, source);

  std::vector<diagnostic> extracted_diagnostics =
      extract_expected_diagnostics(source, TEST_FILE_NAME);
  compile_retcode expected_ret_code = compile_retcode::success;
  for (const auto& diag : extracted_diagnostics) {
    if (diag.level() == diagnostic_level::error) {
      expected_ret_code = compile_retcode::failure;
      break;
    }
  }

  args.insert(args.begin(), "thrift_binary"); // Ignored argument
  args.emplace_back("--gen"); // generator arg
  args.emplace_back("mstch_cpp2"); // target language
  args.emplace_back("-o"); // output directory
  args.emplace_back(tmp_dir.string() + "/");
  if (auto* includes = std::getenv("IMPLICIT_INCLUDES")) {
    args.emplace_back("-I"); // include directory
    args.emplace_back(includes);
  }
  args.emplace_back(TEST_FILE_NAME); // input file name

  compile_result result = compile(args, smgr);
  EXPECT_EQ(expected_ret_code, result.retcode);

  auto diagnostics = result.detail.diagnostics();
  std::sort(
      diagnostics.begin(),
      diagnostics.end(),
      [](const diagnostic& diag1, const diagnostic& diag2) {
        return diag1.lineno() < diag2.lineno();
      });
  EXPECT_EQ(diagnostics, extracted_diagnostics);
}

void check_compile(const std::string& source) {
  std::vector<std::string> args;
  check_compile(source, args);
}

} // namespace apache::thrift::compiler::test
