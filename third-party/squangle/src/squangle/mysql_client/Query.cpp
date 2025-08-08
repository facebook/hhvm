/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <boost/algorithm/string.hpp>
#include <folly/String.h>
#include <algorithm>
#include <type_traits>
#include <vector>

#include "squangle/mysql_client/Query.h"

namespace facebook::common::mysql_client {

#define TYPE_CHECK(expected) \
  if (type_ != expected)     \
  throw std::invalid_argument("DataType doesn't match with the call")

typedef std::pair<folly::fbstring, QueryArgument> ArgPair;

// fbstring constructors
QueryArgument::QueryArgument(folly::StringPiece val)
    : value_(folly::fbstring(val.data(), val.size())) {}

QueryArgument::QueryArgument(std::string_view val)
    : value_(folly::fbstring(val.data(), val.size())) {}

QueryArgument::QueryArgument(char const* val) : value_(folly::fbstring(val)) {}

QueryArgument::QueryArgument(const std::string& string_value)
    : value_(folly::fbstring(string_value)) {}

QueryArgument::QueryArgument(const folly::fbstring& val) : value_(val) {}

QueryArgument::QueryArgument(folly::fbstring&& val) : value_(std::move(val)) {}

QueryArgument::QueryArgument(double double_val) : value_(double_val) {}

QueryArgument::QueryArgument(const std::initializer_list<QueryArgument>& list)
    : value_(std::vector<QueryArgument>(list.begin(), list.end())) {}

QueryArgument::QueryArgument(std::vector<QueryArgument> arg_list)
    : value_(std::move(arg_list)) {}

QueryArgument::QueryArgument() : value_(std::vector<ArgumentPair>()) {}

QueryArgument::QueryArgument(
    folly::StringPiece param1,
    const QueryArgument& param2)
    : value_(std::vector<ArgumentPair>()) {
  getPairs().emplace_back(param1, param2);
}

QueryArgument::QueryArgument(Query q) : value_(std::move(q)) {}

bool QueryArgument::isString() const {
  return std::holds_alternative<folly::fbstring>(value_);
}

bool QueryArgument::isQuery() const {
  return std::holds_alternative<Query>(value_);
}

bool QueryArgument::isPairList() const {
  return std::holds_alternative<std::vector<ArgPair>>(value_);
}

bool QueryArgument::isBool() const {
  return std::holds_alternative<bool>(value_);
}

bool QueryArgument::isNull() const {
  return std::holds_alternative<std::monostate>(value_);
}

bool QueryArgument::isList() const {
  return std::holds_alternative<std::vector<QueryArgument>>(value_);
}

bool QueryArgument::isDouble() const {
  return std::holds_alternative<double>(value_);
}

bool QueryArgument::isInt() const {
  return std::holds_alternative<int64_t>(value_);
}

bool QueryArgument::isTwoTuple() const {
  return std::holds_alternative<QualifiedColumn>(value_);
}

bool QueryArgument::isThreeTuple() const {
  return std::holds_alternative<AliasedQualifiedColumn>(value_);
}

QueryArgument&& QueryArgument::operator()(
    folly::StringPiece q1,
    const QueryArgument& q2) {
  getPairs().emplace_back(ArgPair(q1.str(), q2));
  return std::move(*this);
}

QueryArgument&& QueryArgument::operator()(
    folly::fbstring&& q1,
    QueryArgument&& q2) {
  getPairs().emplace_back(ArgPair(std::move(q1), std::move(q2)));
  return std::move(*this);
}

namespace { // anonymous namespace to prevent class shadowing

template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

} // namespace

folly::fbstring QueryArgument::asString() const {
  return std::visit(
      [](auto&& arg) -> folly::fbstring {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (
            std::is_same_v<T, double> || std::is_same_v<T, bool> ||
            std::is_same_v<T, int64_t> || std::is_same_v<T, folly::fbstring>) {
          return folly::to<folly::fbstring>(arg);
        }

        throw std::invalid_argument(fmt::format(
            "Only allowed type conversions are Int, Double, Bool and String:"
            " type found: {}",
            typeid(arg).name()));
      },
      value_);
}

std::string_view QueryArgument::typeName() const {
  return std::visit(
      [](auto&& arg) -> std::string_view {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, double>) {
          return "double";
        } else if constexpr (std::is_same_v<T, bool>) {
          return "bool";
        } else if constexpr (std::is_same_v<T, int64_t>) {
          return "int64_t";
        } else if constexpr (std::is_same_v<T, folly::fbstring>) {
          return "string";
        } else if constexpr (std::is_same_v<T, std::monostate>) {
          return "monostate";
        } else if constexpr (std::is_same_v<T, Query>) {
          return "Query";
        } else if constexpr (std::is_same_v<T, std::vector<QueryArgument>>) {
          return "std::vector<QueryArgument>";
        } else if constexpr (std::is_same_v<T, std::vector<ArgumentPair>>) {
          return "std::vector<ArgumentPair>";
        } else if constexpr (std::is_same_v<T, QualifiedColumn>) {
          return "QualifiedColumn";
        } else if constexpr (std::is_same_v<T, AliasedQualifiedColumn>) {
          return "AliasedQualifiedColumn";
        } else {
          // Should be unreachable since we have an entry for each type in the
          // variant.
          CHECK(false);
        }
      },
      value_);
}

double QueryArgument::getDouble() const {
  return std::get<double>(value_);
}

int64_t QueryArgument::getInt() const {
  return std::get<int64_t>(value_);
}

bool QueryArgument::getBool() const {
  return std::get<bool>(value_);
}

const Query& QueryArgument::getQuery() const {
  return std::get<Query>(value_);
}

const folly::fbstring& QueryArgument::getString() const {
  return std::get<folly::fbstring>(value_);
}

const std::vector<QueryArgument>& QueryArgument::getList() const {
  return std::get<std::vector<QueryArgument>>(value_);
}

const std::vector<ArgPair>& QueryArgument::getPairs() const {
  return std::get<std::vector<ArgPair>>(value_);
}

const QualifiedColumn& QueryArgument::getTwoTuple() const {
  return std::get<QualifiedColumn>(value_);
}

const AliasedQualifiedColumn& QueryArgument::getThreeTuple() const {
  return std::get<AliasedQualifiedColumn>(value_);
}

void QueryArgument::initFromDynamic(const folly::dynamic& param) {
  // Convert to basic values and get type
  if (param.isObject()) {
    // List of pairs
    std::vector<folly::dynamic> keys(param.keys().begin(), param.keys().end());
    std::sort(keys.begin(), keys.end());
    value_ = std::vector<ArgPair>();
    auto& vec = std::get<std::vector<ArgPair>>(value_);
    vec.reserve(keys.size());

    for (const auto& key : keys) {
      QueryArgument q2(fromDynamic(param[key]));

      vec.emplace_back(ArgPair(key.asString(), q2));
    }
  } else if (param.isNull()) {
    value_ = {};
  } else if (param.isArray()) {
    value_ = std::vector<QueryArgument>();
    std::get<std::vector<QueryArgument>>(value_).reserve(param.size());
    auto& v = std::get<std::vector<QueryArgument>>(value_);
    for (const auto& val : param) {
      v.emplace_back(fromDynamic(val));
    }
  } else if (param.isString()) {
    value_ = folly::fbstring(param.getString());
  } else if (param.isBool()) {
    value_ = param.asBool();
  } else if (param.isDouble()) {
    value_ = param.asDouble();
  } else if (param.isInt()) {
    value_ = param.asInt();
  } else {
    throw std::invalid_argument("Dynamic type doesn't match to accepted ones");
  }
}

std::vector<ArgPair>& QueryArgument::getPairs() {
  return std::get<std::vector<ArgPair>>(value_);
}

// Query can be constructed with or without params.
// By default we deep copy the query text
Query::Query(const folly::StringPiece query_text) : query_text_(query_text) {}

Query::Query(QueryText&& query_text) : query_text_(std::move(query_text)) {}

Query::Query(
    const folly::StringPiece query_text,
    std::vector<QueryArgument> params)
    : query_text_(query_text),
      unsafe_query_(false),
      params_(std::move(params)) {}

Query::~Query() {}

Query::Query(Query&&) noexcept = default;
Query::Query(const Query&) = default;

Query& Query::operator=(Query&&) noexcept = default;
Query& Query::operator=(const Query&) = default;

namespace {

// Some helper functions for encoding/escaping.
void appendComment(folly::fbstring* s, const QueryArgument& d) {
  auto str = d.asString();
  boost::replace_all(str, "/*", " / * ");
  boost::replace_all(str, "*/", " * / ");
  s->append(str);
}

void appendColumnTableName(folly::fbstring* s, const QueryArgument& d) {
  if (d.isString()) {
    folly::grow_capacity_by(*s, d.getString().size() + 4);
    s->push_back('`');
    for (char c : d.getString()) {
      // Toss in an extra ` if we see one.
      if (c == '`') {
        s->push_back('`');
      }
      s->push_back(c);
    }
    s->push_back('`');
  } else if (d.isTwoTuple()) {
    // If a two-tuple is provided we have a qualified column name
    auto t = d.getTwoTuple();
    appendColumnTableName(s, std::get<0>(t));
    s->push_back('.');
    appendColumnTableName(s, std::get<1>(t));
  } else if (d.isThreeTuple()) {
    // If a three-tuple is provided we have a qualified column name
    // with an alias. This is helpful for constructing JOIN queries.
    auto t = d.getThreeTuple();
    appendColumnTableName(s, std::get<0>(t));
    s->push_back('.');
    appendColumnTableName(s, std::get<1>(t));
    s->append(" AS ");
    appendColumnTableName(s, std::get<2>(t));
  } else {
    s->append(d.asString());
  }
}

// Raise an exception with, hopefully, a helpful error message.
[[noreturn]] void parseError(
    const folly::StringPiece s,
    size_t offset,
    const folly::StringPiece message) {
  const std::string msg = fmt::format(
      "Parse error at offset {}: {}, query: {}", offset, message, s);
  throw std::invalid_argument(msg);
}

// Raise an exception for format string/value mismatches
[[noreturn]] void formatStringParseError(
    folly::StringPiece query_text,
    size_t offset,
    char format_specifier,
    folly::StringPiece value_type) {
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
folly::StringPiece
advance(const folly::StringPiece s, size_t* offset, size_t num) {
  if (s.size() <= *offset + num) {
    parseError(s, *offset, "unexpected end of string");
  }
  *offset += num;
  return folly::StringPiece(
      s.data() + *offset - num + 1, s.data() + *offset + 1);
}

// No escaping (useful for logging/testing only)
void noEscapeString(folly::fbstring* dest, const folly::fbstring& value) {
  VLOG(3) << "connectionless escape performed; this should only occur in "
          << "testing.";
  *dest += value;
}

// Simple escaping of default characters
void simpleEscapeString(folly::fbstring* dest, const folly::fbstring& value) {
  folly::grow_capacity_by(*dest, value.size());
  for (char ch : value) {
    static const std::unordered_map<char, folly::fbstring> replacements = {
        {'\\', "\\\\"},
        {'\'', "\\'"},
        {'\"', "\\\""},
        {'\0', "\\0"},
        {'\b', "\\b"},
        {'\n', "\\n"},
        {'\r', "\\r"},
        {'\t', "\\t"},
    };
    if (auto it = replacements.find(ch); it != replacements.end()) {
      dest->append(it->second);
    } else {
      dest->push_back(ch);
    }
  }
}

// Escape a string using the connection.  The connection allows for special
// handling of byte sequences that look like multi-byte characters but are not
// based on the connection's character set
void fullEscapeString(
    folly::fbstring* dest,
    const folly::fbstring& value,
    const InternalConnection* conn) {
  if (!conn) {
    return noEscapeString(dest, value);
  }

  auto old_size = dest->size();
  dest->resize(old_size + 2 * value.size() + 1);
  auto* start = &(*dest)[old_size];
  auto actual = conn->escapeString(start, value.data(), value.size());
  dest->resize(old_size + actual);
}

} // namespace

void Query::append(const Query& query2) {
  query_text_ += query2.query_text_;
  for (const auto& param2 : query2.params_) {
    params_.push_back(param2);
  }
}

void Query::append(Query&& query2) {
  query_text_ += query2.query_text_;
  for (auto& param2 : query2.params_) {
    params_.push_back(std::move(param2));
  }
}

// Append a dynamic to the query string we're building.  We ensure the
// type matches the dynamic's type (or allow a magic 'v' type to be
// any value, but this isn't exposed to the users of the library).
void Query::appendValue(
    folly::fbstring* s,
    size_t offset,
    char type,
    const QueryArgument& d,
    const EscapeFunc& escapeFunc) const {
  auto querySp = query_text_.getQuery();

  if (d.isString()) {
    if (type != 's' && type != 'v' && type != 'm') {
      formatStringParseError(querySp, offset, type, "string");
    }
    const auto& value = d.getString();
    folly::grow_capacity_by(*s, value.size() + 4);
    s->push_back('"');
    escapeFunc(s, value);
    s->push_back('"');
  } else if (d.isBool()) {
    if (type != 'v' && type != 'm') {
      formatStringParseError(querySp, offset, type, "bool");
    }
    s->append(d.asString());
  } else if (d.isInt()) {
    if (type != 'd' && type != 'v' && type != 'm' && type != 'u') {
      formatStringParseError(querySp, offset, type, "int");
    }
    if (type == 'u') {
      s->append(folly::to<folly::fbstring>(static_cast<uint64_t>(d.getInt())));
    } else {
      s->append(d.asString());
    }
  } else if (d.isDouble()) {
    if (type != 'f' && type != 'v' && type != 'm') {
      formatStringParseError(querySp, offset, type, "double");
    }
    s->append(d.asString());
  } else if (d.isQuery()) {
    s->append(d.getQuery().renderInternal(escapeFunc));
  } else if (d.isNull()) {
    s->append("NULL");
  } else {
    formatStringParseError(querySp, offset, type, d.typeName());
  }
}

void Query::appendValueClauses(
    folly::fbstring* ret,
    size_t* idx,
    const char* sep,
    const QueryArgument& param,
    const EscapeFunc& escapeFunc) const {
  auto querySp = query_text_.getQuery();

  if (!param.isPairList()) {
    parseError(
        querySp,
        *idx,
        fmt::format(
            "object expected for %Lx but received {}", param.typeName()));
  }
  // Sort these to get consistent query ordering (mainly for
  // testing, but also aesthetics of the final query).
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
      appendValue(ret, *idx, 'v', key_value.second, escapeFunc);
    }
  }
}

folly::fbstring Query::renderMultiQuery(
    const InternalConnection* conn,
    const std::vector<Query>& queries) {
  auto reserve_size = 0;
  for (const Query& query : queries) {
    reserve_size +=
        query.query_text_.getQuery().size() + 8 * query.params_.size();
  }
  folly::fbstring ret;
  ret.reserve(reserve_size);

  // Not adding `;` in the end
  for (const Query& query : queries) {
    if (!ret.empty()) {
      ret.append(";");
    }
    ret.append(query.render(conn));
  }

  return ret;
}

folly::fbstring Query::render(const InternalConnection* conn) const {
  return render(conn, params_);
}

folly::fbstring Query::render(
    const InternalConnection* conn,
    const std::vector<QueryArgument>& params) const {
  return renderInternal(
      [&](auto* dest, const auto& value) {
        return fullEscapeString(dest, value, conn);
      },
      params);
}

folly::fbstring Query::renderInsecure() const {
  return renderInternal(noEscapeString);
}

folly::fbstring Query::renderInsecure(
    const std::vector<QueryArgument>& params) const {
  return renderInternal(noEscapeString, params);
}

folly::fbstring Query::renderPartiallyEscaped() const {
  return renderInternal(simpleEscapeString);
}

folly::fbstring Query::renderInternal(const EscapeFunc& escapeFunc) const {
  return renderInternal(escapeFunc, params_);
}

folly::fbstring Query::renderInternal(
    const EscapeFunc& escapeFunc,
    const std::vector<QueryArgument>& params) const {
  auto querySp = query_text_.getQuery();

  if (unsafe_query_) {
    return querySp.to<folly::fbstring>();
  }

  auto offset = querySp.find_first_of(";'\"`");
  if (offset != folly::StringPiece::npos) {
    parseError(querySp, offset, "Saw dangerous characters in SQL query");
  }

  folly::fbstring ret;
  ret.reserve(querySp.size() + 8 * params.size());

  auto current_param = params.begin();
  bool after_percent = false;
  size_t idx;
  // Walk our string, watching for % values.
  for (idx = 0; idx < querySp.size(); ++idx) {
    char c = querySp[idx];
    if (!after_percent) {
      if (c != '%') {
        ret.push_back(c);
      } else {
        after_percent = true;
      }
      continue;
    }

    after_percent = false;
    if (c == '%') {
      ret.push_back('%');
      continue;
    }

    if (current_param == params.end()) {
      parseError(querySp, idx, "too few parameters for query");
    }

    const auto& param = *current_param++;
    if (c == 'd' || c == 's' || c == 'f' || c == 'u') {
      appendValue(&ret, idx, c, param, escapeFunc);
    } else if (c == 'm') {
      if (!(param.isString() || param.isInt() || param.isDouble() ||
            param.isBool() || param.isNull() || param.isQuery())) {
        parseError(querySp, idx, "%m expects int/float/string/bool");
      }
      appendValue(&ret, idx, c, param, escapeFunc);
    } else if (c == 'K') {
      ret.append("/*");
      appendComment(&ret, param);
      ret.append("*/");
    } else if (c == 'T' || c == 'C') {
      appendColumnTableName(&ret, param);
    } else if (c == '=') {
      folly::StringPiece type = advance(querySp, &idx, 1);
      if (type != "d" && type != "s" && type != "f" && type != "u" &&
          type != "m") {
        parseError(querySp, idx, "expected %=d, %=f, %=s, %=u, or %=m");
      }

      if (param.isNull()) {
        ret.append(" IS NULL");
      } else {
        ret.append(" = ");
        appendValue(&ret, idx, type[0], param, escapeFunc);
      }
    } else if (c == 'V') {
      if (param.isQuery()) {
        parseError(querySp, idx, "%V doesn't allow subquery");
      }
      size_t col_idx;
      size_t row_len = 0;
      bool first_row = true;
      for (const auto& row : param.getList()) {
        bool first_in_row = true;
        col_idx = 0;
        if (!first_row) {
          ret.append(", ");
        }
        ret.append("(");
        for (const auto& col : row.getList()) {
          if (!first_in_row) {
            ret.append(", ");
          }
          appendValue(&ret, idx, 'v', col, escapeFunc);
          col_idx++;
          first_in_row = false;
          if (first_row) {
            row_len++;
          }
        }
        ret.append(")");
        if (first_row) {
          first_row = false;
        } else if (col_idx != row_len) {
          parseError(
              querySp,
              idx,
              "not all rows provided for %V formatter are the same size");
        }
      }
    } else if (c == 'L') {
      folly::StringPiece type = advance(querySp, &idx, 1);
      if (type == "O" || type == "A") {
        ret.append("(");
        const char* sep = (type == "O") ? " OR " : " AND ";
        appendValueClauses(&ret, &idx, sep, param, escapeFunc);
        ret.append(")");
      } else {
        if (!param.isList()) {
          parseError(querySp, idx, "expected array for %L formatter");
        }

        bool first_param = true;
        for (const auto& val : param.getList()) {
          if (!first_param) {
            ret.append(", ");
          }
          first_param = false;
          if (type == "C") {
            appendColumnTableName(&ret, val);
          } else {
            appendValue(&ret, idx, type[0], val, escapeFunc);
          }
        }
      }
    } else if (c == 'U' || c == 'W') {
      if (c == 'W') {
        appendValueClauses(&ret, &idx, " AND ", param, escapeFunc);
      } else {
        appendValueClauses(&ret, &idx, ", ", param, escapeFunc);
      }
    } else if (c == 'Q') {
      if (param.isQuery()) {
        ret.append(param.getQuery().renderInternal(escapeFunc));
      } else {
        ret.append((param).asString());
      }
    } else {
      parseError(querySp, idx, "unknown % code");
    }
  }

  if (after_percent) {
    parseError(querySp, idx, "string ended with unfinished % code");
  }

  if (current_param != params.end()) {
    parseError(querySp, 0, "too many parameters specified for query");
  }

  return ret;
}

std::shared_ptr<folly::fbstring> MultiQuery::renderQuery(
    const InternalConnection* conn) const {
  if (!rendered_multi_query_) {
    rendered_multi_query_ = std::make_shared<folly::fbstring>(
        Query::renderMultiQuery(conn, queries_));
  }

  return rendered_multi_query_;
}

} // namespace facebook::common::mysql_client
