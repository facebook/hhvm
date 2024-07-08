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
std::optional<QueryExpr::ReturnOnlyFiles> expr_return_only_files(
    std::string expression_json) {
  json_error_t err{};
  auto expression = json_loads(expression_json.c_str(), JSON_DECODE_ANY, &err);
  if (!expression.has_value()) {
    ADD_FAILURE() << "JSON parse error in fixture: " << err.text << " at "
                  << err.source << ":" << err.line << ":" << err.column;
    return std::nullopt;
  }
  Query query;
  // Disable automatic parsing of "match" as "imatch", "name" as "iname", etc.
  auto expr = watchman::parseQueryExpr(&query, *expression);
  return expr->listOnlyFiles();
}

} // namespace

TEST(ReturnOnlyFilesTest, false) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["false"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, true) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["true"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, type_d) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["type", "d"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::No));
}

TEST(ReturnOnlyFilesTest, type_f) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["type", "f"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Yes));
}

TEST(ReturnOnlyFilesTest, type_l) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["type", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Yes));
}

TEST(ReturnOnlyFilesTest, dirname) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["dirname", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, idirname) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["idirname", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, empty) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["empty"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, exists) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["exists"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, match) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["match", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, imatch) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["imatch", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, name) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["name", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, iname) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["iname", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, pcre) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["pcre", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, ipcre) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["ipcre", "l"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, since) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["since", "c:0:0"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, size) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["size", "eq", 0] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, suffix) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["suffix", "txt"] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, allof_yes) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["allof", ["type", "f"], ["exists"]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Yes));
}

TEST(ReturnOnlyFilesTest, allof_no) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["allof", ["type", "d"], ["exists"]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::No));
}

TEST(ReturnOnlyFilesTest, allof_unrelated) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["allof", ["false"], ["exists"]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}

TEST(ReturnOnlyFilesTest, allof_yesno) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["allof", ["type", "d"], ["type", "f"]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::No));
}

TEST(ReturnOnlyFilesTest, not_allof_yesno) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["not",
        ["allof", ["type", "d"], ["type", "f"]]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Yes));
}

TEST(ReturnOnlyFilesTest, anyof_no) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["allof", ["type", "d"], ["exists"]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::No));
}

TEST(ReturnOnlyFilesTest, anyof_yes) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["anyof", ["type", "f"], ["type", "l"]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Yes));
}

TEST(ReturnOnlyFilesTest, anyof_unrelated) {
  EXPECT_THAT(
      expr_return_only_files(R"( ["not",
        ["anyof", ["exists"], ["true"]]] )"),
      Optional(QueryExpr::ReturnOnlyFiles::Unrelated));
}
