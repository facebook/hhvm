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

#include <string>
#include <vector>
#include <re2/re2.h>

#include <folly/portability/GTest.h>

#include <thrift/compiler/compiler.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler::test {

using apache::thrift::compiler::diagnostic;
using apache::thrift::compiler::diagnostic_level;
using apache::thrift::compiler::diagnostic_results;
using apache::thrift::compiler::parse_and_mutate_program;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

std::vector<diagnostic> extract_expected_diagnostics(
    const std::string_view& source, const std::string& file_name) {
  std::vector<diagnostic> result;
  int line_number = 1;
  for (auto it = source.begin(); it != source.end();) {
    auto line_end = std::find(it, source.end(), '\n');

    re2::StringPiece input(&*it, line_end - it);
    re2::StringPiece type_match, message_match;
    re2::RE2 diagnostic_pattern(
        "#\\s*expected-(?P<type>error|warning):\\s*(?P<message>.*)$");

    if (re2::RE2::PartialMatch(
            input, diagnostic_pattern, &type_match, &message_match)) {
      diagnostic_level level = type_match.as_string() == "error"
          ? diagnostic_level::error
          : diagnostic_level::warning;
      result.emplace_back(
          level, message_match.as_string(), file_name, line_number);
    }

    if (line_end == source.end()) {
      break;
    }
    it = line_end + 1;
    line_number++;
  }
  return result;
}

void check_compile(const std::string& source) {
  const std::string TEST_FILE_NAME = "test.thrift";
  source_manager smgr;
  smgr.add_virtual_file(TEST_FILE_NAME, source);

  std::vector<diagnostic> extracted_diagnostics =
      extract_expected_diagnostics(source, TEST_FILE_NAME);
  std::pair<std::unique_ptr<t_program_bundle>, diagnostic_results>
      parsed_results = parse_and_mutate_program(smgr, TEST_FILE_NAME, {});
  auto diagnostics = parsed_results.second.diagnostics();
  std::sort(
      diagnostics.begin(),
      diagnostics.end(),
      [](const diagnostic& diag1, const diagnostic& diag2) {
        return diag1.lineno() < diag2.lineno();
      });

  EXPECT_EQ(diagnostics, extracted_diagnostics);
}

} // namespace apache::thrift::compiler::test
