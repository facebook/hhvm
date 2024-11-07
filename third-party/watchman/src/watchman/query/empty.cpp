/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/FileResult.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"

#include <memory>

namespace watchman {

class QueryContextBase;

class ExistsExpr : public QueryExpr {
 public:
  EvaluateResult evaluate(QueryContextBase*, FileResult* file) override {
    return file->exists();
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref&) {
    return std::make_unique<ExistsExpr>();
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // `exists` doesn't constrain the path.
    return std::nullopt;
  }

  ReturnOnlyFiles listOnlyFiles() const override {
    return ReturnOnlyFiles::Unrelated;
  }

  SimpleSuffixType evaluateSimpleSuffix() const override {
    return SimpleSuffixType::Excluded;
  }

  std::vector<std::string> getSuffixQueryGlobPatterns() const override {
    return std::vector<std::string>{};
  }
};
W_TERM_PARSER(exists, ExistsExpr::parse);

class EmptyExpr : public QueryExpr {
 public:
  EvaluateResult evaluate(QueryContextBase*, FileResult* file) override {
    auto exists = file->exists();
    auto stat = file->stat();
    auto size = file->size();

    if (!exists.has_value()) {
      return std::nullopt;
    }
    if (!exists.value()) {
      return false;
    }

    if (!stat.has_value()) {
      return std::nullopt;
    }

    if (!size.has_value()) {
      return std::nullopt;
    }

    if (stat->isDir() || stat->isFile()) {
      return size.value() == 0;
    }

    return false;
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref&) {
    return std::make_unique<EmptyExpr>();
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // `empty` doesn't constrain the path.
    return std::nullopt;
  }

  ReturnOnlyFiles listOnlyFiles() const override {
    return ReturnOnlyFiles::Unrelated;
  }

  SimpleSuffixType evaluateSimpleSuffix() const override {
    return SimpleSuffixType::Excluded;
  }

  std::vector<std::string> getSuffixQueryGlobPatterns() const override {
    return std::vector<std::string>{};
  }
};
W_TERM_PARSER(empty, EmptyExpr::parse);

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
