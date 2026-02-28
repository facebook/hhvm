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

#include <thrift/compiler/generate/templates.h>
#include <thrift/compiler/whisker/parser.h>

#include <fmt/std.h>
#include <gtest/gtest.h>

// Test which validates all the templates we bundle into the compiler via
// templates.h can be validly parsed by Whisker
namespace {

namespace compiler = apache::thrift::compiler;

std::vector<std::string> all_template_names() {
  std::vector<std::string> names;
  for (const auto& [name, _] : compiler::create_templates_by_path()) {
    names.push_back(name);
  }
  return names;
}

} // namespace

class WhiskerTemplateTest : public testing::TestWithParam<std::string> {};

TEST_P(WhiskerTemplateTest, WhiskerTemplateIsValid) {
  std::string template_name = GetParam();
  whisker::source_manager source_manager{
      std::make_unique<compiler::in_memory_source_manager_backend>(
          compiler::create_templates_by_path())};
  std::optional<whisker::source> source_view =
      source_manager.get_file(template_name);
  ASSERT_TRUE(source_view.has_value());

  std::vector<whisker::diagnostic> diagnostics;
  whisker::diagnostics_engine diag{
      source_manager,
      [&diagnostics](whisker::diagnostic d) {
        diagnostics.push_back(std::move(d));
      },
      whisker::diagnostic_params::only_errors()};
  std::optional<whisker::ast::root> result =
      whisker::parse(source_view.value(), diag);
  EXPECT_FALSE(diag.has_errors()) << fmt::format(
      "Template {} has errors:\n"
      "  {}",
      template_name,
      fmt::join(diagnostics, "\n  "));
  EXPECT_TRUE(result.has_value())
      << fmt::format("Template {} failed to parse", template_name);
}

// Use the template name as the test parameter, so the test name/parameter is
// human readable (i.e. the path relative to compiler/generate/templates)
INSTANTIATE_TEST_CASE_P(
    WhiskerTemplateTest,
    WhiskerTemplateTest,
    testing::ValuesIn(all_template_names()));
