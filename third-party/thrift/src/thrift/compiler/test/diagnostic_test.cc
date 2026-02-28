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

#include <thrift/compiler/diagnostic.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace apache::thrift::compiler;

TEST(DiagnosticTest, str) {
  EXPECT_EQ(
      diagnostic(diagnostic_level::debug, "m", "f", 1).str(), "[DEBUG:f:1] m");
  EXPECT_EQ(
      diagnostic(diagnostic_level::error, "m", "f", 0).str(), "[ERROR:f] m");
  EXPECT_EQ(
      diagnostic(diagnostic_level::info, "m", "f", 1).str(), "[INFO:f:1] m");
  EXPECT_EQ(
      diagnostic(diagnostic_level::warning, "m", "").str(), "[WARNING:] m");
}

class DiagnosticsEngineTest : public ::testing::Test {
 public:
  source_manager source_mgr;
  diagnostic_results results;
  diagnostics_engine diags;
  source_view src;

  DiagnosticsEngineTest()
      : diags(source_mgr, results),
        src(source_mgr.add_virtual_file("path/to/file.thrift", "")) {}
};

TEST_F(DiagnosticsEngineTest, keep_debug) {
  // Not reported by default.
  diags.report(src.start, diagnostic_level::debug, "hi");
  EXPECT_THAT(results.diagnostics(), testing::IsEmpty());

  diags.params().debug = true;
  diags.report(src.start, diagnostic_level::debug, "hi");
  EXPECT_THAT(
      results.diagnostics(),
      testing::ElementsAre(
          diagnostic(diagnostic_level::debug, "hi", "path/to/file.thrift", 1)));
}

TEST_F(DiagnosticsEngineTest, keep_info) {
  // Not reported by default.
  diags.report(src.start, diagnostic_level::info, "hi");
  EXPECT_THAT(results.diagnostics(), ::testing::IsEmpty());

  diags.params().info = true;
  diags.report(src.start, diagnostic_level::info, "hi");
  EXPECT_THAT(
      results.diagnostics(),
      ::testing::ElementsAre(
          diagnostic(diagnostic_level::info, "hi", "path/to/file.thrift", 1)));
}

TEST_F(DiagnosticsEngineTest, warning_level) {
  // Strict not reported by default.
  diags.warning(src.start, "hi");
  diags.warning_legacy_strict(src.start, "bye");
  EXPECT_THAT(
      results.diagnostics(),
      ::testing::ElementsAre(diagnostic(
          diagnostic_level::warning, "hi", "path/to/file.thrift", 1)));
  results = {};

  // Not reported.
  diags.params().warn_level = 0;
  diags.warning(src.start, "hi");
  diags.warning_legacy_strict(src.start, "bye");
  EXPECT_THAT(results.diagnostics(), ::testing::IsEmpty());

  // Both reported.
  diags.params().warn_level = 2;
  diags.warning(src.start, "hi");
  diags.warning_legacy_strict(src.start, "bye");
  EXPECT_THAT(
      results.diagnostics(),
      ::testing::ElementsAre(
          diagnostic(diagnostic_level::warning, "hi", "path/to/file.thrift", 1),
          diagnostic(
              diagnostic_level::warning, "bye", "path/to/file.thrift", 1)));
}

TEST(DiagnosticResultsTest, empty) {
  diagnostic_results results;
  EXPECT_FALSE(results.has_error());
  for (int i = 0; i <= static_cast<int>(diagnostic_level::debug); ++i) {
    EXPECT_EQ(results.count(static_cast<diagnostic_level>(i)), 0);
  }
}

TEST(DiagnosticResultsTest, count) {
  diagnostic_results results;
  for (int i = 0; i <= static_cast<int>(diagnostic_level::debug); ++i) {
    auto level = static_cast<diagnostic_level>(i);
    for (int j = 0; j <= i; ++j) {
      results.add({level, "hi", "file"});
    }
  }
  for (int i = 0; i <= static_cast<int>(diagnostic_level::debug); ++i) {
    EXPECT_EQ(results.count(static_cast<diagnostic_level>(i)), i + 1);
  }
}
