/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/String.h>

#include <string>
#include <string_view>
#include <vector>

namespace facebook::common::mysql_client {

class QueryArgument;
class InternalConnection;
class Query;

// QueryRenderer handles the actual work of rendering a Query's format string
// and parameters into a final SQL string. It is templated on the output string
// type, supporting both folly::fbstring and std::string without requiring
// callers to do post-hoc conversion.
//
// It is also templated on a `Validate` flag that gates only the
// format-string-structure checks — the dangerous-character scan, the
// unknown/incomplete specifier checks, and the parameter-count checks. When
// true (the default, used for legacy queries) those run and throw on violation;
// when false (used for compile-time-checked queries, where Query::checked
// already proved the format string is well-formed and has the right argument
// count) they are elided via `if constexpr`, leaving a smaller, faster hot
// path.
//
// Per-argument value-type checks (e.g. a string passed to %d) always run, in
// BOTH modes. Compile-time checking cannot see through a type-erased argument
// (a QueryArgument or folly::dynamic, which Query::checked accepts for any
// specifier and validates at render time), so render time is the only place
// such a mismatch can be caught. Both instantiations share the exact same
// rendering logic and produce identical output; the only difference is whether
// the redundant format-structure checks run.
//
// Template instantiations are provided for folly::fbstring and std::string
// (both Validate values) in QueryRenderer.cpp. The extern template declarations
// below prevent implicit instantiation in other translation units.
template <typename StringType, bool Validate = true>
class QueryRenderer {
 public:
  enum class EscapeMode { None, Simple, Full };

  // Render a query format string with the given parameters.
  // conn is only used when escapeMode is Full.
  // maxSize of SIZE_MAX means no truncation. When truncation occurs,
  // truncationIndicator is appended (its length is accounted for within
  // maxSize).
  static StringType render(
      std::string_view queryText,
      bool unsafeQuery,
      const std::vector<QueryArgument>& params,
      EscapeMode escapeMode,
      const InternalConnection* conn = nullptr,
      size_t maxSize = SIZE_MAX,
      std::string_view truncationIndicator = "...");

 private:
  // Lets the two Validate instantiations call each other's private
  // renderAppend, so a sub-query can be rendered with the validator matching
  // its own checked-ness (see renderSubQuery) regardless of the outer query's.
  template <typename, bool>
  friend class QueryRenderer;

  // Render an embedded sub-query, dispatching on the SUB-query's own mode
  // rather than the enclosing query's. A non-checked sub-query embedded in a
  // checked query must still be validated: checked() proved nothing about the
  // inner query's format.
  static void renderSubQuery(
      StringType& output,
      const Query& subQuery,
      EscapeMode escapeMode,
      const InternalConnection* conn);

  static void escapeAndAppend(
      StringType* dest,
      const folly::fbstring& value,
      EscapeMode mode,
      const InternalConnection* conn);

  static void appendValue(
      StringType* s,
      std::string_view queryText,
      size_t offset,
      char type,
      const QueryArgument& d,
      EscapeMode escapeMode,
      const InternalConnection* conn);

  static void appendValueClauses(
      StringType* ret,
      std::string_view queryText,
      size_t* idx,
      const char* sep,
      const QueryArgument& param,
      EscapeMode escapeMode,
      const InternalConnection* conn);

  static void renderAppend(
      StringType& output,
      std::string_view queryText,
      bool unsafeQuery,
      const std::vector<QueryArgument>& params,
      EscapeMode escapeMode,
      const InternalConnection* conn);

  static void appendColumnTableName(StringType* s, const QueryArgument& d);
  static void appendComment(StringType* s, const QueryArgument& d);

  // Truncate output to maxSize if exceeded. Returns true if truncated.
  static bool checkTruncation(
      StringType& output,
      size_t maxSize,
      std::string_view truncationIndicator);
};

extern template class QueryRenderer<folly::fbstring, true>;
extern template class QueryRenderer<std::string, true>;
extern template class QueryRenderer<folly::fbstring, false>;
extern template class QueryRenderer<std::string, false>;

} // namespace facebook::common::mysql_client
