/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Errors.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"

#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>

using namespace watchman;

/* Basic boolean and compound expressions */

class NotExpr : public QueryExpr {
  std::unique_ptr<QueryExpr> expr;

 public:
  explicit NotExpr(std::unique_ptr<QueryExpr> other_expr)
      : expr(std::move(other_expr)) {}

  EvaluateResult evaluate(QueryContextBase* ctx, FileResult* file) override {
    auto res = expr->evaluate(ctx, file);
    if (!res.has_value()) {
      return res;
    }
    return !*res;
  }

  static std::unique_ptr<QueryExpr> parse(Query* query, const json_ref& term) {
    /* rigidly require ["not", expr] */
    if (!term.isArray() || json_array_size(term) != 2) {
      throw QueryParseError("must use [\"not\", expr]");
    }

    const auto& other = term.at(1);
    auto other_expr = parseQueryExpr(query, other);
    return std::make_unique<NotExpr>(std::move(other_expr));
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // We can't negate globs, so return an unbounded result regardless of what
    // the inner expression is.
    return std::nullopt;
  }

  /**
   * Inverts the result of the subexpression.
   * Expressions that are unrelated stay unrelated.
   */
  ReturnOnlyFiles listOnlyFiles() const override {
    auto subExprValue = expr->listOnlyFiles();
    if (subExprValue == ReturnOnlyFiles::Yes) {
      return ReturnOnlyFiles::No;
    } else if (subExprValue == ReturnOnlyFiles::No) {
      return ReturnOnlyFiles::Yes;
    }
    return ReturnOnlyFiles::Unrelated;
  }

  SimpleSuffixType evaluateSimpleSuffix() const override {
    return SimpleSuffixType::Excluded;
  }

  std::vector<std::string> getSuffixQueryGlobPatterns() const override {
    return std::vector<std::string>{};
  }
};

W_TERM_PARSER(not, NotExpr::parse);

class TrueExpr : public QueryExpr {
 public:
  EvaluateResult evaluate(QueryContextBase*, FileResult*) override {
    return true;
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref&) {
    return std::make_unique<TrueExpr>();
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // We will match every path --> unbounded.
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

W_TERM_PARSER(true, TrueExpr::parse);

class FalseExpr : public QueryExpr {
 public:
  EvaluateResult evaluate(QueryContextBase*, FileResult*) override {
    return false;
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref&) {
    return std::make_unique<FalseExpr>();
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // We will not match any path --> bounded by an empty list of globs.
    return std::vector<std::string>{};
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

W_TERM_PARSER(false, FalseExpr::parse);

class ListExpr : public QueryExpr {
  bool allof;
  std::vector<std::unique_ptr<QueryExpr>> exprs;

 public:
  ListExpr(bool isAll, std::vector<std::unique_ptr<QueryExpr>> exprs)
      : allof(isAll), exprs(std::move(exprs)) {}

  EvaluateResult evaluate(QueryContextBase* ctx, FileResult* file) override {
    bool needData = false;

    for (auto& expr : exprs) {
      auto res = expr->evaluate(ctx, file);

      if (!res.has_value()) {
        needData = true;
      } else if (!*res) {
        if (allof) {
          // Return NoMatch even if we have needData set, as allof
          // requires that all terms match and this one doesn't,
          // so we can avoid loading the data for prior terms
          // in this list
          return false;
        }
      } else {
        // Matched

        if (!allof) {
          // Similar to the condition above, we can short circuit loading
          // other data if this one matches for the anyof case.
          return true;
        }
      }
    }

    if (needData) {
      // We're not sure yet
      return std::nullopt;
    }
    return allof;
  }

  static std::unique_ptr<QueryExpr>
  parse(Query* query, const json_ref& term, bool allof) {
    std::vector<std::unique_ptr<QueryExpr>> list;

    /* don't allow "allof" on its own */
    if (!term.isArray() || json_array_size(term) < 2) {
      if (allof) {
        throw QueryParseError("must use [\"allof\", expr...]");
      }
      throw QueryParseError("must use [\"anyof\", expr...]");
    }

    auto n = json_array_size(term) - 1;
    list.reserve(n);

    for (size_t i = 0; i < n; i++) {
      const auto& exp = term.at(i + 1);

      auto op = allof ? AggregateOp::AllOf : AggregateOp::AnyOf;
      auto parsed = parseQueryExpr(query, exp);
      if (list.empty()) {
        list.emplace_back(std::move(parsed));
      } else {
        // Try to aggregate with previous expression
        auto aggExpr = list.back().get()->aggregate(parsed.get(), op);
        if (aggExpr) {
          list.back() = std::move(aggExpr);
        } else {
          list.emplace_back(std::move(parsed));
        }
      }
    }

    return std::make_unique<ListExpr>(allof, std::move(list));
  }

  static std::unique_ptr<QueryExpr> parseAllOf(
      Query* query,
      const json_ref& term) {
    return parse(query, term, true);
  }
  static std::unique_ptr<QueryExpr> parseAnyOf(
      Query* query,
      const json_ref& term) {
    return parse(query, term, false);
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity caseSensitive) const override {
    if (allof) {
      // Heuristic: The fewer patterns needed to describe the upper bound, the
      // tighter we assume the bound will be. This is suboptimal, as it doesn't
      // consider the tree structure.
      //
      // The success case for this heuristic is eliminating dead "anyof"
      // branches:
      // {"foo/**"} vs {"foo/**", "bar/**"}
      //   --> selects {"foo/**"}, which is narrower
      //
      // The failure cases include:
      // {"foo/**"} vs {"foo/bar/**", "foo/baz/**"}
      //   --> selects {"foo/**"} despite the alternative being narrower
      // {"foo/**"} vs {"foo/bar/**"}
      //   --> selects {"foo/**"} despite the alternative being narrower
      // {"foo/**"} vs {"bar/**"}
      //   --> selects {"foo/**"} despite the optimal result being {}
      std::optional<std::vector<std::string>> minUpperBound;
      for (auto& expr : exprs) {
        auto elemUpperBound = expr->computeGlobUpperBound(caseSensitive);
        if (!elemUpperBound.has_value()) {
          continue;
        }
        if (!minUpperBound.has_value() ||
            minUpperBound->size() > elemUpperBound->size()) {
          minUpperBound = std::move(elemUpperBound);
        }
        if (minUpperBound.has_value() && minUpperBound->size() <= 1) {
          break;
        }
      }
      return minUpperBound;
    }

    /* anyof */

    // If all expressions in the list are bounded, return the union of all
    // upper bounds. If any expression is unbounded, return an unbounded
    // result.
    //
    // TODO: Consider the tree structure here as well, e.g. the union
    // of {"foo/**"} and {"foo/bar/**"} would just be {"foo/**"}.
    std::unordered_set<std::string> unionOfUpperBounds;
    for (auto& expr : exprs) {
      auto elemUpperBound = expr->computeGlobUpperBound(caseSensitive);
      if (!elemUpperBound.has_value()) {
        return std::nullopt;
      }
      unionOfUpperBounds.insert(elemUpperBound->begin(), elemUpperBound->end());
    }

    return std::vector<std::string>(
        unionOfUpperBounds.begin(), unionOfUpperBounds.end());
  }
  /**
   * Combines the results of the subexpressions.
   * For allof, the result needs to satisfy each subexpression.
   * - If any subexpression requests non-file data, the whole expression must
   *   also contain non-file data. Therefore we return No if any subexpression
   *   requests non-file data.
   * - If any subexpression requests only file data, the whole expression must
   *   also contain only file data. Therefore we return Yes if one subexpression
   *   requests only file data. and there is no subexpression requesting
   *    non-file data.
   * - We return No if there are both Yes and No subexpressions because
   *   we want expressions that exclude groups of types like
   *   ["not", ["allof", ["type", "l"], ["type", "d"]]]
   *   to return Yes if we are excluding directories.
   * For anyof, the result can satisfy any subexpression.
   * - If any subexpression requests non-file data, the whole expression could
   *   also contain non-file data. Therefore we return No if any subexpression
   *   requests non-file data.
   * - The only way for the whole expression to contain only file data is if
   *   each subexpression contains only file data. Therefore we return Yes only
   * if each subexpression is Yes.
   */
  ReturnOnlyFiles listOnlyFiles() const override {
    ReturnOnlyFiles result = ReturnOnlyFiles::Unrelated;
    if (allof) {
      for (auto& subExpr : exprs) {
        auto value = subExpr->listOnlyFiles();
        if (value == ReturnOnlyFiles::Yes) {
          result = ReturnOnlyFiles::Yes;
        } else if (value == ReturnOnlyFiles::No) {
          return ReturnOnlyFiles::No;
        }
      }
    } else {
      bool allYes = exprs.empty()
          ? false
          : exprs[0]->listOnlyFiles() == ReturnOnlyFiles::Yes;
      for (auto& subExpr : exprs) {
        auto value = subExpr->listOnlyFiles();
        if (value == ReturnOnlyFiles::Yes) {
          // Keep checking the remaining subexprs
        } else if (value == ReturnOnlyFiles::No) {
          return ReturnOnlyFiles::No;
        } else {
          allYes = false;
        }
      }
      if (allYes) {
        return ReturnOnlyFiles::Yes;
      }
    }
    return result;
  }

  SimpleSuffixType evaluateSimpleSuffix() const override {
    if (allof) {
      std::vector<SimpleSuffixType> types;
      for (auto& subExpr : exprs) {
        types.push_back(subExpr->evaluateSimpleSuffix());
      }
      if (types.size() == 2) {
        if ((types[0] == SimpleSuffixType::Type &&
             types[1] == SimpleSuffixType::Suffix) ||
            (types[1] == SimpleSuffixType::Type &&
             types[0] == SimpleSuffixType::Suffix)) {
          return SimpleSuffixType::IsSimpleSuffix;
        }
      }
    }
    return SimpleSuffixType::Excluded;
  }

  std::vector<std::string> getSuffixQueryGlobPatterns() const override {
    if (allof) {
      for (auto& subExpr : exprs) {
        if (subExpr->evaluateSimpleSuffix() == SimpleSuffixType::Suffix) {
          return subExpr->getSuffixQueryGlobPatterns();
        }
      }
    }
    return std::vector<std::string>{};
  }
};

W_TERM_PARSER(anyof, ListExpr::parseAnyOf);
W_TERM_PARSER(allof, ListExpr::parseAllOf);

/* vim:ts=2:sw=2:et:
 */
