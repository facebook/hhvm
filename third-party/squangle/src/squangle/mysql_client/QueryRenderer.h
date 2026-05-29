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
// Template instantiations are provided for folly::fbstring and std::string in
// QueryRenderer.cpp. The extern template declarations below prevent implicit
// instantiation in other translation units.
template <typename StringType>
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

extern template class QueryRenderer<folly::fbstring>;
extern template class QueryRenderer<std::string>;

} // namespace facebook::common::mysql_client
