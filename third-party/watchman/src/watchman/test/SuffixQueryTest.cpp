/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/test/TestUtils.h>
#include "watchman/query/GlobTree.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"
#include "watchman/thirdparty/jansson/jansson.h"

using namespace watchman;
using namespace testing;

namespace {

std::optional<json_ref> parse_json(std::string expression_json) {
  json_error_t err{};
  auto expression = json_loads(expression_json.c_str(), JSON_DECODE_ANY, &err);
  if (!expression.has_value()) {
    ADD_FAILURE() << "JSON parse error in fixture: " << err.text << " at "
                  << err.source << ":" << err.line << ":" << err.column;
    return std::nullopt;
  }
  return expression;
}

std::optional<SimpleSuffixType> expr_evaluate_simple_suffix(
    std::string expression_json) {
  json_error_t err{};
  auto expression = parse_json(expression_json);
  if (!expression.has_value()) {
    return std::nullopt;
  }
  Query query;
  // Disable automatic parsing of "match" as "imatch", "name" as "iname", etc.
  auto expr = watchman::parseQueryExpr(&query, *expression);
  return expr->evaluateSimpleSuffix();
}

std::optional<std::vector<std::string>> expr_get_suffix_glob(
    std::string expression_json) {
  json_error_t err{};
  auto expression = parse_json(expression_json);
  if (!expression.has_value()) {
    return std::nullopt;
  }
  Query query;
  // Disable automatic parsing of "match" as "imatch", "name" as "iname", etc.
  auto expr = watchman::parseQueryExpr(&query, *expression);
  auto rv = expr->getSuffixQueryGlobPatterns();
  std::sort(rv.begin(), rv.end());
  return rv;
}

} // namespace

TEST(SuffixQueryTest, false) {
  EXPECT_THAT(
      expr_evaluate_simple_suffix(R"( ["false"] )"),
      Optional(SimpleSuffixType::Excluded));
}

TEST(SuffixQueryTest, false_glob) {
  EXPECT_THAT(
      expr_get_suffix_glob(R"( ["false"] )"),
      Optional(std::vector<std::string>{}));
}

TEST(SuffixQueryTest, type_d) {
  EXPECT_THAT(
      expr_evaluate_simple_suffix(R"( ["type", "d"] )"),
      Optional(SimpleSuffixType::Excluded));
}

TEST(SuffixQueryTest, type_d_glob) {
  EXPECT_THAT(
      expr_get_suffix_glob(R"( ["type", "d"] )"),
      Optional(std::vector<std::string>{}));
}

TEST(SuffixQueryTest, type_f) {
  EXPECT_THAT(
      expr_evaluate_simple_suffix(R"( ["type", "f"] )"),
      Optional(SimpleSuffixType::Type));
}

TEST(SuffixQueryTest, type_f_glob) {
  EXPECT_THAT(
      expr_get_suffix_glob(R"( ["type", "f"] )"),
      Optional(std::vector<std::string>{}));
}
TEST(SuffixQueryTest, suffix) {
  EXPECT_THAT(
      expr_evaluate_simple_suffix(R"( ["suffix", ["a", "f"]] )"),
      Optional(SimpleSuffixType::Suffix));
}

TEST(SuffixQueryTest, suffix_glob) {
  EXPECT_THAT(
      expr_get_suffix_glob(R"( ["suffix", ["a", "f"]] )"),
      Optional(std::vector<std::string>{"**/*.a", "**/*.f"}));
}

TEST(SuffixQueryTest, allof_excl) {
  EXPECT_THAT(
      expr_evaluate_simple_suffix(R"( ["allof", ["type", "f"], ["exists"]] )"),
      Optional(SimpleSuffixType::Excluded));
}

TEST(SuffixQueryTest, allof_excl_glob) {
  EXPECT_THAT(
      expr_get_suffix_glob(R"( ["allof", ["type", "f"], ["exists"]] )"),
      Optional(std::vector<std::string>{}));
}

TEST(SuffixQueryTest, allof_yes) {
  EXPECT_THAT(
      expr_evaluate_simple_suffix(
          R"( ["allof", ["type", "f"], ["suffix", ["a"]]] )"),
      Optional(SimpleSuffixType::IsSimpleSuffix));
}

TEST(SuffixQueryTest, allof_yes_glob) {
  EXPECT_THAT(
      expr_get_suffix_glob(R"( ["allof", ["type", "f"], ["suffix", ["a"]]] )"),
      Optional(std::vector<std::string>{"**/*.a"}));
}
