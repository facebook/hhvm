/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <vector>
#include "watchman/Clock.h"
#include "watchman/fs/FileDescriptor.h"
#include "watchman/watchman_string.h"

namespace watchman {

using EvaluateResult = std::optional<bool>;
class FileResult;

class QueryContextBase {
 public:
  // root number, ticks at start of query execution
  ClockSpec clockAtStartOfQuery;
  uint32_t lastAgeOutTickValueAtStartOfQuery;

  virtual ~QueryContextBase() = default;

  /**
   * Returns the wholename of this query's current file.

   * Note: The wholename is lazily computed and the returned reference is valid
   * until the next file is set.
   */
  virtual const w_string& getWholeName() = 0;
};

/**
 * Describes how terms are being aggregated.
 */
enum AggregateOp {
  AnyOf,
  AllOf,
};

/**
 * Describes which part of a simple suffix expression
 */
enum SimpleSuffixType { Excluded, Suffix, IsSimpleSuffix, Type };

class QueryExpr {
 public:
  virtual ~QueryExpr() = default;
  virtual EvaluateResult evaluate(QueryContextBase* ctx, FileResult* file) = 0;

  // If OTHER can be aggregated with THIS, returns a new expression instance
  // representing the combined state.  Op provides information on the containing
  // query and can be used to determine how aggregation is done.
  // returns nullptr if no aggregation was performed.
  virtual std::unique_ptr<QueryExpr> aggregate(
      const QueryExpr* /*other*/,
      const AggregateOp /*op*/) const {
    return nullptr;
  }

  /**
   * Returns a set of glob expressions that form an upper bound on the results
   * of this expression. This SHOULD be a tight upper bound that restricts the
   * paths to a small set of prefixes (small relative to the size of the
   * project).
   *
   * The patterns are intended to be evaluated by wildmatch (or a compatible
   * globber) with the WM_PATHNAME flag set, and optionally WM_CASEFOLD
   * (depending on the CaseSensitivity parameter). Note that this may differ
   * from how the `glob` generator and any `match` terms are configured in the
   * current query.
   *
   * WARNING: nullopt and an empty vector do NOT mean the same thing:
   * - nullopt signifies no upper bound (the expression cannot be safely
   * approximated by a glob)
   * - an empty vector signifies that the expression cannot match any path.
   */
  virtual std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const = 0;

  virtual std::vector<std::string> getSuffixQueryGlobPatterns() const = 0;

  enum ReturnOnlyFiles { No, Yes, Unrelated };

  /**
   * Returns whether this expression only returns files.
   * Used to determine if Eden can use the faster server-based
   * method to handle this query.
   */
  virtual ReturnOnlyFiles listOnlyFiles() const = 0;

  /**
   * Returns whether this expression is a simple suffix expression, or a part
   * of a simple suffix expression. A simple suffix expression is an allof
   * expression that contains a single suffix expresssion containing one or more
   * suffixes, and a type expresssion that wants files only. The intention for
   * this is to allow watchman to more accurately determine what arguments to
   * pass to eden's globFiles API.
   */
  virtual SimpleSuffixType evaluateSimpleSuffix() const = 0;
};

} // namespace watchman
