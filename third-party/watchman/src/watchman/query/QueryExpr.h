/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include "watchman/Clock.h"
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
};

} // namespace watchman
