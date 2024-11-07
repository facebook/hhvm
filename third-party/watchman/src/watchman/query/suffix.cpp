/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"

#include <memory>
#include <unordered_set>

using namespace watchman;

class SuffixExpr : public QueryExpr {
  std::unordered_set<w_string> suffixSet_;

 public:
  explicit SuffixExpr(std::unordered_set<w_string>&& suffixSet)
      : suffixSet_(std::move(suffixSet)) {}

  EvaluateResult evaluate(QueryContextBase*, FileResult* file) override {
    if (suffixSet_.size() < 3) {
      // For small suffix sets, benchmarks indicated that iteration provides
      // better performance since no suffix allocation is necessary.
      for (auto const& suffix : suffixSet_) {
        if (file->baseName().hasSuffix(suffix)) {
          return true;
        }
      }
      return false;
    }
    auto suffix = file->baseName().asLowerCaseSuffix();
    return suffix && (suffixSet_.find(*suffix) != suffixSet_.end());
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref& term) {
    std::unordered_set<w_string> suffixSet;

    if (!term.isArray()) {
      throw QueryParseError{"Expected array for 'suffix' term"};
    }

    if (json_array_size(term) > 2) {
      throw QueryParseError("Invalid number of arguments for 'suffix' term");
    }

    const auto& suffix = term.at(1);

    // Suffix match supports array or single suffix string
    if (suffix.isArray()) {
      suffixSet.reserve(json_array_size(suffix));
      for (const auto& ele : suffix.array()) {
        if (!ele.isString()) {
          throw QueryParseError(
              "Argument 2 to 'suffix' must be either a string or an array of string");
        }
        suffixSet.insert(json_to_w_string(ele).piece().asLowerCase());
      }
    } else if (suffix.isString()) {
      suffixSet.insert(json_to_w_string(suffix).piece().asLowerCase());
    } else {
      throw QueryParseError(
          "Argument 2 to 'suffix' must be either a string or an array of string");
    }
    return std::make_unique<SuffixExpr>(std::move(suffixSet));
  }

  std::unique_ptr<QueryExpr> aggregate(
      const QueryExpr* other,
      const AggregateOp op) const override {
    if (op != AggregateOp::AnyOf) {
      return nullptr;
    }
    const SuffixExpr* otherExpr = dynamic_cast<const SuffixExpr*>(other);
    if (otherExpr == nullptr) {
      return nullptr;
    }
    std::unordered_set<w_string> suffixSet;
    suffixSet.reserve(suffixSet_.size() + otherExpr->suffixSet_.size());
    suffixSet.insert(
        otherExpr->suffixSet_.begin(), otherExpr->suffixSet_.end());
    suffixSet.insert(suffixSet_.begin(), suffixSet_.end());
    return std::make_unique<SuffixExpr>(std::move(suffixSet));
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // We mostly care about prefix bounds that help us skip fetching information
    // about entire subtrees of the root. `suffix` doesn't help there so treat
    // it as unbounded.
    return std::nullopt;
  }

  ReturnOnlyFiles listOnlyFiles() const override {
    return ReturnOnlyFiles::Unrelated;
  }

  SimpleSuffixType evaluateSimpleSuffix() const override {
    return SimpleSuffixType::Suffix;
  }

  std::vector<std::string> getSuffixQueryGlobPatterns() const override {
    std::vector<std::string> patterns;
    for (const auto& suffix : suffixSet_) {
      patterns.push_back("**/*." + suffix.string());
    }

    return patterns;
  }
};
W_TERM_PARSER(suffix, SuffixExpr::parse);
W_CAP_REG("suffix-set")

/* vim:ts=2:sw=2:et:
 */
