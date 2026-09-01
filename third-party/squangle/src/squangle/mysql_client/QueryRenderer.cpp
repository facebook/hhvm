/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/QueryRenderer.h"

#include <boost/algorithm/string.hpp>
#include <fmt/core.h>
#include <folly/Conv.h>
#include <folly/container/Reserve.h>

#include "squangle/mysql_client/InternalConnection.h"
#include "squangle/mysql_client/Query.h"

namespace facebook::common::mysql_client {

namespace {

// Raise an exception with, hopefully, a helpful error message.
[[noreturn]] void
parseError(std::string_view s, size_t offset, std::string_view message) {
  const std::string msg = fmt::format(
      "Parse error at offset {}: {}, query: {}", offset, message, s);
  throw std::invalid_argument(msg);
}

// Raise an exception for format string/value mismatches
[[noreturn]] void formatStringParseError(
    std::string_view query_text,
    size_t offset,
    char format_specifier,
    std::string_view value_type) {
  parseError(
      query_text,
      offset,
      fmt::format(
          "invalid value type {} for format string %{}",
          value_type,
          format_specifier));
}

// Consume the next x bytes from s, updating offset, and raising an
// exception if there aren't sufficient bytes left.
std::string_view advance(std::string_view s, size_t* offset, size_t num) {
  if (s.size() <= *offset + num) {
    parseError(s, *offset, "unexpected end of string");
  }
  *offset += num;
  return std::string_view(s.data() + *offset - num + 1, num);
}

std::string_view resolveAggregateFunctionName(
    const AggregateFunction& aggFunc) {
  switch (aggFunc) {
    case AggregateFunction::AVG:
      return "AVG(";
    case AggregateFunction::AVG_DISTINCT:
      return "AVG(DISTINCT ";
    case AggregateFunction::BIT_AND:
      return "BIT_AND(";
    case AggregateFunction::BIT_OR:
      return "BIT_OR(";
    case AggregateFunction::BIT_XOR:
      return "BIT_XOR(";
    case AggregateFunction::COUNT:
      return "COUNT(";
    case AggregateFunction::COUNT_DISTINCT:
      return "COUNT(DISTINCT ";
    case AggregateFunction::GROUP_CONCAT:
      return "GROUP_CONCAT(";
    case AggregateFunction::GROUP_CONCAT_DISTINCT:
      return "GROUP_CONCAT(DISTINCT ";
    case AggregateFunction::JSON_ARRAYAGG:
      return "JSON_ARRAYAGG(";
    case AggregateFunction::MAX:
      return "MAX(";
    case AggregateFunction::MAX_DISTINCT:
      return "MAX(DISTINCT ";
    case AggregateFunction::MIN:
      return "MIN(";
    case AggregateFunction::MIN_DISTINCT:
      return "MIN(DISTINCT ";
    case AggregateFunction::STD:
      return "STD(";
    case AggregateFunction::STDDEV:
      return "STDDEV(";
    case AggregateFunction::STDDEV_POP:
      return "STDDEV_POP(";
    case AggregateFunction::STDDEV_SAMP:
      return "STDDEV_SAMP(";
    case AggregateFunction::SUM:
      return "SUM(";
    case AggregateFunction::SUM_DISTINCT:
      return "SUM(DISTINCT ";
    case AggregateFunction::VAR_POP:
      return "VAR_POP(";
    case AggregateFunction::VAR_SAMP:
      return "VAR_SAMP(";
    case AggregateFunction::VARIANCE:
      return "VARIANCE(";
  }
}

} // namespace

// -- QueryRenderer<StringType> implementation --

template <typename StringType>
bool QueryRenderer<StringType>::checkTruncation(
    StringType& output,
    size_t maxSize,
    std::string_view truncationIndicator) {
  if (maxSize == SIZE_MAX || output.size() <= maxSize) {
    return false;
  }
  if (maxSize >= truncationIndicator.size()) {
    output.resize(maxSize - truncationIndicator.size());
    output.append(truncationIndicator.data(), truncationIndicator.size());
  } else {
    output.resize(maxSize);
  }
  return true;
}

template <typename StringType>
void QueryRenderer<StringType>::escapeAndAppend(
    StringType* dest,
    const folly::fbstring& value,
    EscapeMode mode,
    const InternalConnection* conn) {
  switch (mode) {
    case EscapeMode::None:
      VLOG(3) << "connectionless escape performed; this should only occur in "
              << "testing.";
      dest->append(value.data(), value.size());
      break;

    case EscapeMode::Simple:
      folly::grow_capacity_by(*dest, value.size());
      for (const char ch : value) {
        switch (ch) {
          case '\\':
            dest->append("\\\\");
            break;
          case '\'':
            dest->append("\\'");
            break;
          case '\"':
            dest->append("\\\"");
            break;
          case '\0':
            dest->append("\\0");
            break;
          case '\b':
            dest->append("\\b");
            break;
          case '\n':
            dest->append("\\n");
            break;
          case '\r':
            dest->append("\\r");
            break;
          case '\t':
            dest->append("\\t");
            break;
          default:
            dest->push_back(ch);
            break;
        }
      }
      break;

    case EscapeMode::Full:
      if (!conn) {
        VLOG(3) << "connectionless escape performed; this should only occur in "
                << "testing.";
        escapeAndAppend(dest, value, EscapeMode::Simple, conn);
        return;
      }
      auto old_size = dest->size();
      dest->resize(old_size + 2 * value.size() + 1);
      auto* start = &(*dest)[old_size];
      auto actual = conn->escapeString(start, value.data(), value.size());
      dest->resize(old_size + actual);
      break;
  }
}

template <typename StringType>
void QueryRenderer<StringType>::appendComment(
    StringType* s,
    const QueryArgument& d) {
  auto str = d.asString();
  boost::replace_all(str, "/*", " / * ");
  boost::replace_all(str, "*/", " * / ");
  s->append(str.data(), str.size());
}

template <typename StringType>
void QueryRenderer<StringType>::appendColumnTableName(
    StringType* s,
    const QueryArgument& d) {
  if (d.isString()) {
    const auto& str = d.getString();
    // Fast path: if no backticks, batch-append with surrounding backticks
    if (str.find('`') == folly::fbstring::npos) {
      folly::grow_capacity_by(*s, str.size() + 2);
      s->push_back('`');
      s->append(str.data(), str.size());
      s->push_back('`');
    } else {
      folly::grow_capacity_by(*s, str.size() + 4);
      s->push_back('`');
      for (char c : str) {
        if (c == '`') {
          s->push_back('`');
        }
        s->push_back(c);
      }
      s->push_back('`');
    }
  } else if (d.isTwoTuple()) {
    auto t = d.getTwoTuple();
    appendColumnTableName(s, std::get<0>(t));
    s->push_back('.');
    appendColumnTableName(s, std::get<1>(t));
  } else if (d.isThreeTuple()) {
    auto t = d.getThreeTuple();
    appendColumnTableName(s, std::get<0>(t));
    s->push_back('.');
    appendColumnTableName(s, std::get<1>(t));
    s->append(" AS ");
    appendColumnTableName(s, std::get<2>(t));
  } else if (d.isAggregateColumn()) {
    auto t = d.getAggregateColumn();
    auto funcName = resolveAggregateFunctionName(std::get<0>(t));
    s->append(funcName.data(), funcName.size());
    appendColumnTableName(s, std::get<1>(t));
    s->push_back(')');
  } else if (d.isAliasedAggregateColumn()) {
    auto t = d.getAliasedAggregateColumn();
    auto funcName = resolveAggregateFunctionName(std::get<0>(t));
    s->append(funcName.data(), funcName.size());
    auto& [tableName, columnName, aliasName] = std::get<1>(t);
    appendColumnTableName(s, tableName);
    s->push_back('.');
    appendColumnTableName(s, columnName);
    s->append(") AS ");
    appendColumnTableName(s, aliasName);
  } else {
    auto str = d.asString();
    s->append(str.data(), str.size());
  }
}

template <typename StringType>
void QueryRenderer<StringType>::appendValue(
    StringType* s,
    std::string_view queryText,
    size_t offset,
    char type,
    const QueryArgument& d,
    EscapeMode escapeMode,
    const InternalConnection* conn) {
  if (d.isString()) {
    if (type != 's' && type != 'v' && type != 'm') {
      formatStringParseError(queryText, offset, type, "string");
    }
    const auto& value = d.getString();
    folly::grow_capacity_by(*s, value.size() + 4);
    s->push_back('"');
    escapeAndAppend(s, value, escapeMode, conn);
    s->push_back('"');
  } else if (d.isBool()) {
    if (type != 'v' && type != 'm') {
      formatStringParseError(queryText, offset, type, "bool");
    }
    folly::toAppend(d.getBool(), s);
  } else if (d.isInt()) {
    if (type != 'd' && type != 'v' && type != 'm' && type != 'u') {
      formatStringParseError(queryText, offset, type, "int");
    }
    if (type == 'u') {
      folly::toAppend(static_cast<uint64_t>(d.getInt()), s);
    } else {
      folly::toAppend(d.getInt(), s);
    }
  } else if (d.isDouble()) {
    if (type != 'f' && type != 'v' && type != 'm') {
      formatStringParseError(queryText, offset, type, "double");
    }
    folly::toAppend(d.getDouble(), s);
  } else if (d.isQuery()) {
    const auto& subQuery = d.getQuery();
    renderAppend(
        *s,
        subQuery.getQueryFormat(),
        subQuery.isUnsafe(),
        subQuery.getParams(),
        escapeMode,
        conn);
  } else if (d.isNull()) {
    s->append("NULL");
  } else {
    formatStringParseError(queryText, offset, type, d.typeName());
  }
}

template <typename StringType>
void QueryRenderer<StringType>::appendValueClauses(
    StringType* ret,
    std::string_view queryText,
    size_t* idx,
    const char* sep,
    const QueryArgument& param,
    EscapeMode escapeMode,
    const InternalConnection* conn) {
  if (!param.isPairList()) {
    parseError(
        queryText,
        *idx,
        fmt::format(
            "object expected for %Lx but received {}", param.typeName()));
  }
  bool first_param = true;
  for (const auto& key_value : param.getPairs()) {
    if (!first_param) {
      ret->append(sep);
    }
    first_param = false;
    appendColumnTableName(ret, key_value.first);
    if (key_value.second.isNull() && sep[0] != ',') {
      ret->append(" IS NULL");
    } else {
      ret->append(" = ");
      appendValue(
          ret, queryText, *idx, 'v', key_value.second, escapeMode, conn);
    }
  }
}

template <typename StringType>
void QueryRenderer<StringType>::renderAppend(
    StringType& output,
    std::string_view queryText,
    bool unsafeQuery,
    const std::vector<QueryArgument>& params,
    EscapeMode escapeMode,
    const InternalConnection* conn) {
  if (unsafeQuery) {
    output.append(queryText.data(), queryText.size());
    return;
  }

  auto offset = queryText.find_first_of(";'\"`");
  if (offset != std::string_view::npos) {
    parseError(queryText, offset, "Saw dangerous characters in SQL query");
  }

  auto current_param = params.begin();
  size_t idx = 0;
  while (idx < queryText.size()) {
    // Batch-append literal text up to the next '%'
    auto pct = queryText.find('%', idx);
    if (pct == std::string_view::npos) {
      output.append(queryText.data() + idx, queryText.size() - idx);
      break;
    }
    if (pct > idx) {
      output.append(queryText.data() + idx, pct - idx);
    }
    idx = pct;

    // We're at a '%' character
    if (idx + 1 >= queryText.size()) {
      parseError(queryText, idx, "string ended with unfinished % code");
    }

    idx++; // skip the '%'
    char c = queryText[idx];
    idx++; // skip the format character

    if (c == '%') {
      output.push_back('%');
      continue;
    }

    if (current_param == params.end()) {
      parseError(queryText, idx - 1, "too few parameters for query");
    }

    const auto& param = *current_param++;
    if (c == 'd' || c == 's' || c == 'f' || c == 'u') {
      appendValue(&output, queryText, idx - 1, c, param, escapeMode, conn);
    } else if (c == 'm') {
      if (!(param.isString() || param.isInt() || param.isDouble() ||
            param.isBool() || param.isNull() || param.isQuery())) {
        parseError(queryText, idx - 1, "%m expects int/float/string/bool");
      }
      appendValue(&output, queryText, idx - 1, c, param, escapeMode, conn);
    } else if (c == 'K') {
      output.append("/*");
      appendComment(&output, param);
      output.append("*/");
    } else if (c == 'T' || c == 'C') {
      appendColumnTableName(&output, param);
    } else if (c == '=') {
      // idx currently points past the '=', need to read the type character
      // Adjust: advance expects idx to point at the character before the one
      // we want to read. We need to back up by 1 since we already incremented.
      size_t advIdx = idx - 1;
      std::string_view type = advance(queryText, &advIdx, 1);
      idx = advIdx + 1;
      if (type != "d" && type != "s" && type != "f" && type != "u" &&
          type != "m") {
        parseError(queryText, idx - 1, "expected %=d, %=f, %=s, %=u, or %=m");
      }

      if (param.isNull()) {
        output.append(" IS NULL");
      } else {
        output.append(" = ");
        appendValue(
            &output, queryText, idx - 1, type[0], param, escapeMode, conn);
      }
    } else if (c == 'V') {
      if (param.isQuery()) {
        parseError(queryText, idx - 1, "%V doesn't allow subquery");
      }
      size_t col_idx;
      size_t row_len = 0;
      bool first_row = true;
      for (const auto& row : param.getList()) {
        bool first_in_row = true;
        col_idx = 0;
        if (!first_row) {
          output.append(", ");
        }
        output.append("(");
        for (const auto& col : row.getList()) {
          if (!first_in_row) {
            output.append(", ");
          }
          appendValue(&output, queryText, idx - 1, 'v', col, escapeMode, conn);
          col_idx++;
          first_in_row = false;
          if (first_row) {
            row_len++;
          }
        }
        output.append(")");
        if (first_row) {
          first_row = false;
        } else if (col_idx != row_len) {
          parseError(
              queryText,
              idx - 1,
              "not all rows provided for %V formatter are the same size");
        }
      }
    } else if (c == 'L') {
      // idx points past 'L', read the sub-type
      size_t advIdx = idx - 1;
      std::string_view type = advance(queryText, &advIdx, 1);
      idx = advIdx + 1;
      if (type == "O" || type == "A") {
        output.append("(");
        const char* sep = (type == "O") ? " OR " : " AND ";
        size_t clauseIdx = idx - 1;
        appendValueClauses(
            &output, queryText, &clauseIdx, sep, param, escapeMode, conn);
        output.append(")");
      } else {
        if (!param.isList()) {
          parseError(queryText, idx - 1, "expected array for %L formatter");
        }

        bool first_param = true;
        for (const auto& val : param.getList()) {
          if (!first_param) {
            output.append(", ");
          }
          first_param = false;
          if (type == "C") {
            appendColumnTableName(&output, val);
          } else {
            appendValue(
                &output, queryText, idx - 1, type[0], val, escapeMode, conn);
          }
        }
      }
    } else if (c == 'U' || c == 'W') {
      size_t clauseIdx = idx - 1;
      if (c == 'W') {
        appendValueClauses(
            &output, queryText, &clauseIdx, " AND ", param, escapeMode, conn);
      } else {
        appendValueClauses(
            &output, queryText, &clauseIdx, ", ", param, escapeMode, conn);
      }
    } else if (c == 'Q') {
      if (param.isQuery()) {
        const auto& subQuery = param.getQuery();
        renderAppend(
            output,
            subQuery.getQueryFormat(),
            subQuery.isUnsafe(),
            subQuery.getParams(),
            escapeMode,
            conn);
      } else {
        auto str = param.asString();
        output.append(str.data(), str.size());
      }
    } else {
      parseError(queryText, idx - 1, "unknown % code");
    }
  }

  if (current_param != params.end()) {
    parseError(queryText, 0, "too many parameters specified for query");
  }
}

template <typename StringType>
StringType QueryRenderer<StringType>::render(
    std::string_view queryText,
    bool unsafeQuery,
    const std::vector<QueryArgument>& params,
    EscapeMode escapeMode,
    const InternalConnection* conn,
    size_t maxSize,
    std::string_view truncationIndicator) {
  StringType ret;
  ret.reserve(queryText.size() + 8 * params.size());

  renderAppend(ret, queryText, unsafeQuery, params, escapeMode, conn);

  checkTruncation(ret, maxSize, truncationIndicator);
  return ret;
}

// Explicit template instantiations
template class QueryRenderer<folly::fbstring>;
template class QueryRenderer<std::string>;

} // namespace facebook::common::mysql_client
