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
#include <thrift/compiler/test/test_utils.h>

#include <algorithm>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include <fmt/ostream.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <re2/re2.h>

#include <thrift/compiler/compiler.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/sema/sema_context.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler {
// Avoid order dependence when comparing diagnostics on one line.
bool operator<(const diagnostic& diag1, const diagnostic& diag2) {
  // @lint-ignore-every CLANGTIDY readability-braces-around-statements
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
  std::filesystem::path dir;

  temp_dir() {
    using res_t = std::random_device::result_type;
    constexpr auto hex = "0123456789abcdef";
    char buf[16] = {};
    std::random_device rng;
    for (size_t i = 0; i < sizeof(buf); i += 2 * sizeof(res_t)) {
      const auto res = rng();
      for (size_t j = 0; j < 2 * sizeof(res_t); ++j) {
        buf[i + j] = hex[(res >> (4 * j)) & 0xf];
      }
    }
    const auto unique = std::string_view(buf, sizeof(buf));
    dir = std::filesystem::temp_directory_path() / unique;
    std::filesystem::create_directory(dir);
  }

  ~temp_dir() { std::filesystem::remove_all(dir); }
  std::string string() const { return dir.string(); }
};

std::vector<diagnostic> extract_expected_diagnostics(
    const std::string_view& source, const std::string& file_name) {
  std::vector<diagnostic> result;
  int line_number = 1;
  for (auto it = source.begin(); it != source.end();) {
    auto line_end = std::find(it, source.end(), '\n');

    re2::StringPiece line(&*it, line_end - it);
    re2::StringPiece type_match, original_match, replacement_match,
        message_match, line_num_match, name_match, column_match;
    // Match the following:
    // #\\s*expected-(?P<type>error|warning)
    // match the text #expected- and then either error or warning for error
    // severity
    //
    // @?(?P<linenum>[+-]?\\d+)?
    // match @ and then a line number, optionaly can be + or - lines to be an
    // offset to the current line
    //
    // \\s*(?:#original\\[(?P<original>.*?)\\])?\\s*(?:#replacement\\[(?P<replacement>.*?)\\])?
    // match an optional original and replacement text in the form
    // #original[text here]#replacement[text here]
    //
    // @?(?P<column>\\d+)?
    // match an optional column number, no offset here just the number
    //
    // :\\s*(?P<message>.*?)
    // after a colon, match any characters as the message, until a [ character
    // is hit for the next match group
    //
    // \\s*(?:\\[(?P<name>.*)\\])?$
    // match a name, in the format [name_here] then end match
    //
    // Note that this is just a format to match the comments in
    // compiler_test.cc, the actual format of the lint messages is different,
    // for that see diagnostic.cc::format
    std::string diagnostic_severity_and_line(
        "#\\s*expected-(?P<type>error|warning)@?(?P<linenum>[+-]?\\d+)?");
    std::string diagnostic_fixit(
        R"(\s*(?:#original\[(?P<original>.*?)\])?\s*(?:#replacement\[(?P<replacement>.*?)\])?@?(?P<column>\d+)?)");
    std::string diagnostic_message_and_name(
        R"(:\s*(?P<message>.*?)\s*(?:\[(?P<name>.*)\])?$)");

    if (re2::RE2::PartialMatch(
            line,
            diagnostic_severity_and_line + diagnostic_fixit +
                diagnostic_message_and_name,
            &type_match,
            &line_num_match,
            &original_match,
            &replacement_match,
            &column_match,
            &message_match,
            &name_match)) {
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
      auto name = name_match.as_string();
      if ((!original_match.empty() || !replacement_match.empty()) &&
          !column_match.empty()) {
        auto col_str = column_match.as_string();
        int col_num = std::stoi(col_str);

        // Both the original string or the replacement could have newlines,
        // which need to be unescaped in order for the equality match to work
        auto replacement = replacement_match.as_string();
        re2::RE2::GlobalReplace(&replacement, "\\\\n", "\n");
        auto original = original_match.as_string();
        re2::RE2::GlobalReplace(&original, "\\\\n", "\n");

        result.emplace_back(
            level,
            message_match.as_string(),
            file_name,
            line_num,
            name,
            fixit(original, replacement, line_num, col_num));
      } else {
        result.emplace_back(
            level, message_match.as_string(), file_name, line_num, name);
      }
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
    std::vector<std::string> args,
    check_compile_options options) {
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

  gen_params gparams;
  gparams.targets.emplace_back("mstch_cpp2");
  gparams.out_path = tmp_dir.string() + "/";

  parsing_params pparams;
  if (options.add_standard_includes) {
    if (auto* includes = std::getenv("IMPLICIT_INCLUDES")) {
      pparams.incl_searchpath.emplace_back(includes);
    }
  }
  pparams.use_global_resolution = options.use_global_resolution;

  diagnostic_params dparams;
  sema_params sparams;

  args.emplace(args.begin(), ""); // Dummy binary name
  args.emplace_back(""); // Dummy file name

  parse_args(args, pparams, gparams, dparams, sparams);

  compile_result result =
      compile_with_options(pparams, gparams, dparams, sparams, file_name, smgr);

  EXPECT_EQ(expected_ret_code, result.retcode);
  std::vector<diagnostic> result_diagnostics = result.detail.diagnostics();
  std::sort(result_diagnostics.begin(), result_diagnostics.end());
  std::sort(expected_diagnostics.begin(), expected_diagnostics.end());

  EXPECT_THAT(result_diagnostics, ::testing::ContainerEq(expected_diagnostics));
}

void check_compile(
    const std::string& source,
    std::vector<std::string> args,
    check_compile_options options) {
  const std::string test_file_name = "test.thrift";
  std::map<std::string, std::string> file_contents_map{
      {test_file_name, source}};
  check_compile(file_contents_map, test_file_name, std::move(args), options);
}

} // namespace apache::thrift::compiler::test
