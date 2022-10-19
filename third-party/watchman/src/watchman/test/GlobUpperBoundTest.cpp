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
std::optional<std::vector<std::string>> expr_to_upper_bound(
    std::string expression_json,
    CaseSensitivity caseSensitivity) {
  json_error_t err{};
  auto expression = json_loads(expression_json.c_str(), JSON_DECODE_ANY, &err);
  if (!expression.has_value()) {
    ADD_FAILURE() << "JSON parse error in fixture: " << err.text << " at "
                  << err.source << ":" << err.line << ":" << err.column;
    return std::nullopt;
  }
  Query query;
  // Disable automatic parsing of "match" as "imatch", "name" as "iname", etc.
  query.case_sensitive = CaseSensitivity::CaseSensitive;
  auto expr = watchman::parseQueryExpr(&query, *expression);
  return expr->computeGlobUpperBound(caseSensitivity);
}

class CaseInvariantGlobUpperBoundTest
    : public testing::TestWithParam<CaseSensitivity> {
 protected:
  CaseSensitivity caseSensitivity_ = GetParam();
};
} // namespace

TEST_P(CaseInvariantGlobUpperBoundTest, term_false) {
  // "false" is bounded by an empty set of globs
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["false"] )", caseSensitivity_),
      Optional(IsEmpty()));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_true) {
  // "true" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["true"] )", caseSensitivity_), Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_since) {
  // "since" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["since", "c:0:0"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_exists) {
  // "exists" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["exists"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_empty) {
  // "empty" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["empty"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_size) {
  // "size" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["size", "eq", 1024] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_suffix) {
  // "suffix" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["suffix", "foo"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_type) {
  // "type" is unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["type", "f"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_name_basename) {
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["name", "Foo", "basename"] )", caseSensitivity_),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["iname", "Foo", "basename"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST(GlobUpperBoundTest, term_name_wholename) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", "Foo", "wholename"] )", CaseSensitivity::CaseSensitive),
      Optional(UnorderedElementsAre("Foo")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", "Foo", "wholename"] )", CaseSensitivity::Unknown),
      Optional(UnorderedElementsAre("Foo")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", "Foo", "wholename"] )",
          CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo")));
}

TEST(GlobUpperBoundTest, term_name_wholename_multi) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", ["Foo", "foo"], "wholename"] )",
          CaseSensitivity::CaseSensitive),
      Optional(UnorderedElementsAre("Foo", "foo")));

  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", ["Foo", "foo"], "wholename"] )",
          CaseSensitivity::Unknown),
      Optional(UnorderedElementsAre("Foo", "foo")));

  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", ["Foo", "foo"], "wholename"] )",
          CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_name_escape) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", "a/b?*", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre(R"(a/b\?\*)")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_name_multi_escape) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", ["*", "a?[]"], "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre(R"(\*)", R"(a\?\[])")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_name_normalize) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", "a\\b", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre("a/b")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_name_multi_normalize) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["name", ["a\\b", "c\\d"], "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre("a/b", "c/d")));
}

TEST(GlobUpperBoundTest, term_iname_wholename) {
  // iname can be bounded with a case-insensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["iname", "Foo", "wholename"] )",
          CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo")));

  // iname cannot be bounded with a case-sensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["iname", "foo", "wholename"] )", CaseSensitivity::CaseSensitive),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["iname", "foo", "wholename"] )", CaseSensitivity::Unknown),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_basename) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/**/bar", "basename"] )", caseSensitivity_),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["imatch", "foo/**/bar", "basename"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_wholename) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/*/bar", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre("foo/*/bar")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_trim_doublestar) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/**/bar", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre("foo/**")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/**/bar/**/baz", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre("foo/**")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "**/foo", "wholename"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_escaped_doublestar) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/\\**/bar", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre(R"(foo/\**/bar)")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/*\\*/bar", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre(R"(foo/*\*/bar)")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/[**]/bar", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre("foo/[**]/bar")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_dotfiles) {
  // The bounding globs are always evaluated with the equivalent of
  // `"includedotfiles": true`. It's safe to widen a glob omitting dotfiles to a
  // glob including dotfiles.
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/*", "wholename", {"includedotfiles": true}] )",
          caseSensitivity_),
      Optional(UnorderedElementsAre("foo/*")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo/*", "wholename", {"includedotfiles": false}] )",
          caseSensitivity_),
      Optional(UnorderedElementsAre("foo/*")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_noescape) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo\\*", "wholename", {"noescape": true}] )",
          caseSensitivity_),
      Optional(UnorderedElementsAre(R"(foo\\*)")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_match_escape) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["match", "foo\\\\*", "wholename"] )", caseSensitivity_),
      Optional(UnorderedElementsAre(R"(foo\\*)")));
}

TEST(GlobUpperBoundTest, term_imatch_wholename) {
  // imatch can be bounded with a case-insensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["imatch", "Foo/*/Bar", "wholename"] )",
          CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo/*/bar")));

  // imatch cannot be bounded with a case-sensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["imatch", "foo/*/bar", "wholename"] )",
          CaseSensitivity::CaseSensitive),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["imatch", "foo/*/bar", "wholename"] )",
          CaseSensitivity::Unknown),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_pcre_basename) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["pcre", "^Foo[0-9]", "basename"] )", caseSensitivity_),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["ipcre", "^Foo[0-9]", "basename"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_pcre_wholename) {
  // NOTE: In principle, with sufficiently advanced analysis, we could translate
  // this regex into a useful glob upper bound. But currently we don't.
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["pcre", "^Foo[0-9]", "wholename"] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST(GlobUpperBoundTest, term_ipcre_wholename) {
  // NOTE: In principle, with sufficiently advanced analysis, we could translate
  // this regex into a useful glob upper bound. But currently we don't.
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["ipcre", "^Foo[0-9]", "wholename"] )",
          CaseSensitivity::CaseInSensitive),
      Eq(std::nullopt));

  // ipcre can never be bounded with a case-sensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["ipcre", "^foo[0-9]", "wholename"] )",
          CaseSensitivity::CaseSensitive),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["ipcre", "^foo[0-9]", "wholename"] )", CaseSensitivity::Unknown),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_dirname_root_unbounded) {
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["dirname", ""] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_dirname_root_exact_depth) {
  // NOTE: In principle, we could bound this with "*/*/*/*", but currently we
  // don't.
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["dirname", "", ["depth", "eq", 3]] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST(GlobUpperBoundTest, term_dirname_prefix) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["dirname", "Foo/Bar"] )", CaseSensitivity::CaseSensitive),
      Optional(UnorderedElementsAre("Foo/Bar/**")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["dirname", "Foo/Bar"] )", CaseSensitivity::Unknown),
      Optional(UnorderedElementsAre("Foo/Bar/**")));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["dirname", "Foo/Bar"] )", CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo/bar/**")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_dirname_escape) {
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["dirname", "*[?"] )", caseSensitivity_),
      Optional(UnorderedElementsAre(R"(\*\[\?/**)")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_dirname_prefix_depth_zero) {
  // NOTE: In principle, we could bound this with "foo/bar/*", but currently we
  // don't.
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["dirname", "foo/bar", ["depth", "eq", 0]] )", caseSensitivity_),
      Optional(UnorderedElementsAre("foo/bar/**")));
}

TEST(GlobUpperBoundTest, term_idirname) {
  // idirname can be bounded with a case-insensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["idirname", "Foo/Bar"] )", CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo/bar/**")));

  // idirname cannot be bounded with a case-sensitive glob
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["idirname", "foo/bar"] )", CaseSensitivity::CaseSensitive),
      Eq(std::nullopt));
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["idirname", "foo/bar"] )", CaseSensitivity::Unknown),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_not) {
  // Negating a bounded expression --> unbounded
  EXPECT_THAT(
      expr_to_upper_bound(
          R"( ["not", ["dirname", "foo/bar"]] )", caseSensitivity_),
      Eq(std::nullopt));
  // Negating an unbounded expression --> also unbounded
  EXPECT_THAT(
      expr_to_upper_bound(R"( ["not", ["type", "f"]] )", caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_allof_nested_anyof) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "allof",
            ["anyof",
              ["dirname", "foo/bar"],
              ["match", "baz/**", "wholename"]
            ],
            ["type", "f"],
            ["match", "*.js"]
          ])",
          caseSensitivity_),
      Optional(UnorderedElementsAre("foo/bar/**", "baz/**")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_allof_prefers_fewer_globs) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "allof",
            ["name", ["foo/bar/baz", "foo/bar/quux"]],
            ["dirname", "foo/bar"]
          ])",
          caseSensitivity_),
      Optional(UnorderedElementsAre("foo/bar/**")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_anyof_unbounded) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "anyof",
            ["dirname", "foo/bar"],
            ["type", "f"]
          ])",
          caseSensitivity_),
      Eq(std::nullopt));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_anyof_nested_allof) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "anyof",
            ["allof",
              ["dirname", "foo"],
              ["type", "f"]
            ],
            ["allof",
              ["match", "foo/bar/baz/**", "wholename"],
              ["type", "d"]
            ]
          ])",
          caseSensitivity_),
      Optional(UnorderedElementsAre("foo/**", "foo/bar/baz/**")));
}

TEST(GlobUpperBoundTest, term_anyof_dedupes) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "anyof",
            ["dirname", "foo"],
            ["dirname", "Bar"],
            ["match", "Foo/**", "wholename"],
            ["match", "bar/**", "wholename"]
          ])",
          CaseSensitivity::CaseSensitive),
      Optional(UnorderedElementsAre("foo/**", "Foo/**", "bar/**", "Bar/**")));

  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "anyof",
            ["dirname", "foo"],
            ["dirname", "Bar"],
            ["match", "Foo/**", "wholename"],
            ["match", "bar/**", "wholename"]
          ])",
          CaseSensitivity::Unknown),
      Optional(UnorderedElementsAre("foo/**", "Foo/**", "bar/**", "Bar/**")));

  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "anyof",
            ["dirname", "foo"],
            ["dirname", "Bar"],
            ["match", "Foo/**", "wholename"],
            ["match", "bar/**", "wholename"]
          ])",
          CaseSensitivity::CaseInSensitive),
      Optional(UnorderedElementsAre("foo/**", "bar/**")));
}

TEST_P(CaseInvariantGlobUpperBoundTest, term_anyof_dedupes_expensive_globs) {
  EXPECT_THAT(
      expr_to_upper_bound(
          R"([
            "anyof",
            ["match", "foo/**/bar", "wholename"],
            ["match", "foo/**/baz/quux", "wholename"],
            ["match", "foo/**/more/*", "wholename"]
          ])",
          caseSensitivity_),
      Optional(UnorderedElementsAre("foo/**")));
}

INSTANTIATE_TEST_CASE_P(
    GlobUpperBoundTest,
    CaseInvariantGlobUpperBoundTest,
    testing::Values(
        CaseSensitivity::CaseSensitive,
        CaseSensitivity::CaseInSensitive,
        CaseSensitivity::Unknown),
    [](const auto& info) {
      switch (info.param) {
        case CaseSensitivity::CaseSensitive:
          return "CaseSensitive";
        case CaseSensitivity::CaseInSensitive:
          return "CaseInSensitive";
        case CaseSensitivity::Unknown:
          return "Unknown";
        default:
          return "-";
      }
    });
