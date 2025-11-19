/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// This class represents queries to execute against a MySQL database.
//
// DO NOT ENCODE SQL VALUES DIRECTLY.  That's evil.  The library will
// try to prevent this kind of thing.  All values for where clauses,
// inserts, etc should be parameterized via the encoding methods
// below.  This is will make your code more robust and reliable while
// also avoiding common security issues.
//
// Usage is simple; construct the query using special printf-like
// markup, provide parameters for the substitution, and then hand to
// the database libraries.  Alternatively, you can call one of render*()
// methods to see the actual SQL it would run.
//
// Example:
//
// Query q("SELECT foo, bar FROM Table WHERE id = %d", 17);
// LOG(INFO) << "query: " << q.renderInsecure();
//
// folly::dynamic condition(dynamic::object("id1", 7)("id2", 14));
// Query q("SELECT %LC FROM %T WHERE %W",
//         folly::dynamic({"id1_type", "data"}),
//         "assoc_info", condition);
// auto op = Connection::beginQuery(std::move(conn), q);
//
// Values for substitution into the query should be folly::dynamic
// values (or convertible to them).  Composite values expected by some
// codes such as %W, %U, etc, are also folly::dynamic objects that
// have array or map values.
//
// Codes:
//
// %s, %d, %u, %f - strings, integers, unsigned integers or floats;
//                  NULL if a nullptr is passed in.
// %m - folly::dynamic, gets converted to string/integer/float/boolean.
//      nullptr becomes "NULL", throws otherwise
// %=s, %=d, %=u, %=f, %=m - like the previous except suitable for comparison,
//                 so "%s" becomes " = VALUE".  nullptr becomes "IS NULL"
// %T - a table name.  enclosed with ``.
// %C - like %T, except for column names. Optionally supply two-/three-tuple
//      to define qualified column name or qualified column name with
//      an alias. `QualifiedColumn{"table_name", "column_name"}` will become
//      "`table_name`.`column_name`" and
//      `AliasedQualifiedColumn{"table_name", "column_name", "alias"}`
//      will become "`table_name`.`column_name` AS `alias`"
// %V - VALUES style row list; expects a list of lists, each of the same
//      length.
// %Ls, %Ld, %Lu, %Lf, %Lm - strings/ints/uints/floats separated by commas.
//      nullptr becomes "NULL"
// %LC - list of column names separated by commas. Optionally supplied as
//       a list of two-/three-tuples to define qualified column names or
//       qualified column names with aliases. Similar to %C.
// %LO, %LA - key/value pair rendered as key1=val1 OR/AND key2=val2 (similar
//            to %W)
// %U, %W - keys and values suitable for UPDATE and WHERE clauses,
//          respectively.  %U becomes "`col1` = val1, `col2` = val2"
//          and %W becomes "`col1` = val1 AND `col2` = val2". Does not currently
//          support unsigned integers.
// %Q - literal string, evil evil.  don't use.
// %K - an SQL comment.  Will put the /* and */ for you.
// %% - literal % character.
//
// For more details, check out queryfx in the www codebase.

#pragma once

#include <folly/Memory.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/json/dynamic.h>

#include <glog/logging.h>

#include <mysql.h>

#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>

#include "squangle/base/Base.h"
#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client {

class QueryArgument;
class InternalConnection;

enum class AggregateFunction {
  AVG = 0,
  AVG_DISTINCT = 1,
  BIT_AND = 2,
  BIT_OR = 3,
  BIT_XOR = 4,
  COUNT = 5,
  COUNT_DISTINCT = 6,
  GROUP_CONCAT = 7,
  GROUP_CONCAT_DISTINCT = 8,
  JSON_ARRAYAGG = 9,
  MAX = 10,
  MAX_DISTINCT = 11,
  MIN = 12,
  MIN_DISTINCT = 13,
  STD = 14,
  STDDEV = 15,
  STDDEV_POP = 16,
  STDDEV_SAMP = 17,
  SUM = 18,
  SUM_DISTINCT = 19,
  VAR_POP = 20,
  VAR_SAMP = 21,
  VARIANCE = 22,
};

using ArgumentPair = std::pair<folly::fbstring, QueryArgument>;
using QueryAttributes = AttributeMap;

using QualifiedColumn = std::tuple<folly::fbstring, folly::fbstring>;
using AliasedQualifiedColumn =
    std::tuple<folly::fbstring, folly::fbstring, folly::fbstring>;
using AggregateColumn = std::tuple<AggregateFunction, QualifiedColumn>;
using AliasedAggregateColumn =
    std::tuple<AggregateFunction, AliasedQualifiedColumn>;

/*
 * This class will be responsible of passing various per query options.
 * For the time being we only have attributes but class will be extended
 * as we introduce additional options.
 */
class QueryOptions {
 public:
  const QueryAttributes& getAttributes() const {
    return attributes_;
  }

  QueryAttributes& getAttributes() {
    return attributes_;
  }

  bool operator==(const QueryOptions& other) const {
    return attributes_ == other.attributes_;
  }

  std::size_t hashValue() const {
    return folly::hash::commutative_hash_combine_range(
        attributes_.begin(), attributes_.end());
  }

  QueryOptions& setQueryTimeout(Duration timeout) {
    queryTimeoutOverride_ = timeout;
    return *this;
  }

  const std::optional<Duration>& getQueryTimeout() const {
    return queryTimeoutOverride_;
  }

  void setLoggingFunction(
      const std::string& name,
      std::function<std::string()> func) {
    if (func == nullptr) {
      loggingFuncs_.erase(name);
    } else {
      loggingFuncs_[name] = std::move(func);
    }
  }

  std::unordered_map<std::string, std::function<std::string()>>
  stealLoggingFuncs() {
    std::unordered_map<std::string, std::function<std::string()>> funcs;
    std::swap(funcs, loggingFuncs_);
    return funcs;
  }

 protected:
  QueryAttributes attributes_;
  std::optional<Duration> queryTimeoutOverride_;
  std::unordered_map<std::string, std::function<std::string()>> loggingFuncs_;
};

class Query {
  struct QueryText;

 public:
  explicit Query(const folly::StringPiece query_text);
  explicit Query(QueryText&& query_text);

  ~Query();

  // default copy and move constructible
  Query(const Query&);
  Query(Query&&) noexcept;

  Query& operator=(const Query&);
  Query& operator=(Query&&) noexcept;

  // Parameters will be coerced into folly::dynamic.
  template <typename... Args>
  /* implicit */ Query(const folly::StringPiece query_text, Args&&... args);
  Query(const folly::StringPiece query_text, std::vector<QueryArgument> params);

  void append(const Query& query2);
  void append(Query&& query2);

  Query& operator+=(const Query& query2) {
    append(query2);
    return *this;
  }

  Query& operator+=(Query&& query2) {
    append(std::move(query2));
    return *this;
  }

  Query operator+(const Query& query2) const {
    Query ret(*this);
    ret.append(query2);
    return ret;
  }

  // If you need to construct a raw query, use this evil function.
  static Query unsafe(
      const folly::StringPiece query_text,
      bool shallowCopy = false) {
    Query ret{
        shallowCopy ? QueryText::makeShallow(query_text)
                    : QueryText{query_text}};
    ret.allowUnsafeEvilQueries();
    return ret;
  }

  bool isUnsafe() const noexcept {
    return unsafe_query_;
  }

  // Wrapper around mysql_real_escape_string() - please use placeholders
  // instead.
  //
  // This is provided so that non-Facebook users of the HHVM extension have
  // a familiar API.
  // template <typename string>
  // static string escapeString(MYSQL* conn, const string& unescaped) {
  //   return escapeString<string>(conn, folly::StringPiece(unescaped));
  // }

  static std::string escapeString(
      const InternalConnection& conn,
      std::string_view unescaped) {
    return conn.escapeString(unescaped);
  }

  static folly::fbstring renderMultiQuery(
      const InternalConnection* conn,
      const std::vector<Query>& queries,
      std::string_view prefix = "");

  // render either with the parameters to the constructor or specified
  // ones.
  folly::fbstring render(const InternalConnection* conn) const;
  folly::fbstring render(
      const InternalConnection* conn,
      const std::vector<QueryArgument>& params) const;

  // render either with the parameters to the constructor or specified
  // ones.  This is mainly for testing as it does not properly escape
  // the MySQL strings.
  folly::fbstring renderInsecure() const;
  folly::fbstring renderInsecure(
      const std::vector<QueryArgument>& params) const;

  // Render with basic query escaping. Not suitable for generating a query to
  // be sent to MySQL, but it should be good enough for logging.
  folly::fbstring renderPartiallyEscaped() const;

  folly::StringPiece getQueryFormat() const {
    return query_text_.getQuery();
  }

 private:
  // QueryText is a container for query stmt used by the Query (see below).
  // Its a union like structure that supports managing either a shallow copy
  // or a deep copy of a query stmt. If QueryText holds a shallow reference
  // and a modification is requested, it will automatically copy the data
  // before modifying the data.
  //
  // Invariants:
  // sp -> string piece field representing the query stmt
  // sb -> string buffer that contains the query if deep copy
  //
  // if shallow copy, sb is empty and sp point to the query stmt
  // if deep copy, sb has the query stmt and sp points to sb
  struct QueryText {
    // By default make a deep copy of the query
    explicit QueryText(folly::StringPiece query) {
      query_buffer_.assign(folly::fbstring(query.begin(), query.size()));
      query_ = folly::StringPiece(*query_buffer_);
      sanityChecks();
    }

    ~QueryText() = default;

    // Make a shallow copy of the query
    static QueryText makeShallow(folly::StringPiece query) {
      QueryText res{};
      res.query_ = query;
      res.sanityChecks();
      return res;
    }

    // Copy constructor and copy assignment
    QueryText(const QueryText& other) {
      *this = other;
    }
    QueryText& operator=(const QueryText& other) {
      if (this == &other) {
        return *this;
      }
      if (!other.query_buffer_.has_value()) {
        /* shallow copy string */
        query_buffer_.reset();
        query_ = other.query_;
      } else {
        query_buffer_ = other.query_buffer_;
        query_ = folly::StringPiece(*query_buffer_);
      }
      sanityChecks();
      return *this;
    }

    /// Move constructor and move assignment
    QueryText(QueryText&& other) noexcept {
      *this = std::move(other);
    }
    QueryText& operator=(QueryText&& other) {
      if (this == &other) {
        return *this;
      }
      if (!other.query_buffer_.has_value()) {
        /* shallow copy */
        query_buffer_.reset();
        query_ = other.query_;
      } else {
        query_buffer_ = std::move(other.query_buffer_);
        query_ = folly::StringPiece(*query_buffer_);
        other.query_ = {};
        other.query_buffer_.reset();
      }
      sanityChecks();
      return *this;
    }

    QueryText& operator+=(const QueryText& other) {
      if (!query_buffer_.has_value()) {
        // this was a shallow copy before; we need to copy now
        query_buffer_.assign(folly::fbstring(query_.begin(), query_.size()));
      }
      DCHECK_EQ(query_, *query_buffer_);
      *query_buffer_ += " ";
      *query_buffer_ += other.getQuery().to<folly::fbstring>();
      query_ = folly::StringPiece(*query_buffer_);
      sanityChecks();
      return *this;
    }

    folly::StringPiece getQuery() const noexcept {
      return query_;
    }

   private:
    QueryText() {}

    // ensures invariants are met
    void sanityChecks() {
      if (!query_buffer_.has_value()) {
        /* shallow copy */
        return;
      }
      DCHECK_EQ((uintptr_t)query_.data(), (uintptr_t)query_buffer_->data());
      DCHECK_EQ(query_.size(), query_buffer_->length());
    }

    folly::Optional<folly::fbstring> query_buffer_;
    folly::StringPiece query_;
  }; // end QueryText class

  // Allow queries that look evil (aka, raw queries).  Don't use this.
  // It's horrible.
  void allowUnsafeEvilQueries() {
    unsafe_query_ = true;
  }

  using EscapeFunc =
      std::function<void(folly::fbstring*, const folly::fbstring&)>;

  folly::fbstring renderInternal(const EscapeFunc& escapeFunc) const;
  folly::fbstring renderInternal(
      const EscapeFunc& escapeFunc,
      const std::vector<QueryArgument>& params) const;

  // append an int, float, or string to the specified buffer
  void appendValue(
      folly::fbstring* s,
      size_t offset,
      char type,
      const QueryArgument& d,
      const EscapeFunc& escapeFunc) const;

  // append a dynamic::object param as key=value joined with sep;
  // values are passed to appendValue
  void appendValueClauses(
      folly::fbstring* ret,
      size_t* idx,
      const char* sep,
      const QueryArgument& param,
      const EscapeFunc& escapeFunc) const;

  template <typename Arg, typename... Args>
  void unpack(Arg&& arg, Args&&... args);
  void unpack() {}

  QueryText query_text_;
  bool unsafe_query_ = false;
  std::vector<QueryArgument> params_{};
};

// Wraps many queries and holds a buffer that contains the rendered multi query
// from all the subqueries.
class MultiQuery {
 public:
  explicit MultiQuery(std::vector<Query>&& queries)
      : queries_(std::move(queries)) {}

  std::shared_ptr<folly::fbstring> renderQuery(
      const InternalConnection* conn,
      std::string_view prefix = "") const;

  const Query& getQuery(size_t index) const {
    CHECK_THROW(index < queries_.size(), std::invalid_argument);
    return queries_[index];
  }

  const std::vector<Query>& getQueries() const {
    return queries_;
  }

 private:
  mutable std::shared_ptr<folly::fbstring> rendered_multi_query_;
  std::vector<Query> queries_;
};

class QueryArgument {
 private:
  std::variant<
      // monostate (implying NULL) needs to be the first entry
      std::monostate,
      int64_t,
      double,
      bool,
      folly::fbstring,
      Query,
      std::vector<QueryArgument>,
      std::vector<ArgumentPair>,
      QualifiedColumn,
      AliasedQualifiedColumn,
      AggregateColumn,
      AliasedAggregateColumn>
      value_;

 public:
  /* implicit */ QueryArgument(folly::StringPiece val);
  /* implicit */ QueryArgument(std::string_view val);
  /* implicit */ QueryArgument(char const* val);
  /* implicit */ QueryArgument(const std::string& string_value);
  /* implicit */ QueryArgument(const folly::fbstring& val);
  /* implicit */ QueryArgument(folly::fbstring&& val);
  /* implicit */ QueryArgument(Query q);

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
  /* implicit */ QueryArgument(T int_val)
      : value_(static_cast<int64_t>(int_val)) {}
  template <
      typename T,
      typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
  /* implicit */ QueryArgument(T enum_val)
      : value_(static_cast<int64_t>(enum_val)) {}
  /* implicit */ QueryArgument(double double_val);

  /* implicit */ QueryArgument(
      const std::initializer_list<QueryArgument>& list);
  /* implicit */ QueryArgument(std::vector<QueryArgument> arg_list);
  /* implicit */ QueryArgument(QualifiedColumn tup) : value_(std::move(tup)) {}
  /* implicit */ QueryArgument(AliasedQualifiedColumn tup)
      : value_(std::move(tup)) {}
  /* implicit */ QueryArgument(AggregateColumn tup) : value_(std::move(tup)) {}
  /* implicit */ QueryArgument(AliasedAggregateColumn tup)
      : value_(std::move(tup)) {}
  /* implicit */ QueryArgument(std::nullptr_t /*n*/) : value_() {}

  /* implicit */ QueryArgument(const std::optional<bool>& opt) {
    if (opt) {
      value_ = static_cast<int64_t>(opt.value());
    }
  }

  template <typename T>
  /* implicit */ QueryArgument(const std::optional<T>& opt) {
    if (opt) {
      if constexpr (std::is_enum_v<T>) {
        value_ = static_cast<int64_t>(opt.value());
      } else {
        value_ = opt.value();
      }
    }
  }

  // Special handling for nullopt optionals to enable
  // callers to directly pass them in as a query argument
  /* implicit */ QueryArgument(std::nullopt_t /*opt*/) {}

  /* implicit */ QueryArgument(const folly::Optional<bool>& opt) {
    if (opt) {
      value_ = static_cast<int64_t>(opt.value());
    }
  }

  template <typename T>
  /* implicit */ QueryArgument(const folly::Optional<T>& opt) {
    if (opt) {
      if constexpr (std::is_enum_v<T>) {
        value_ = static_cast<int64_t>(opt.value());
      } else {
        value_ = opt.value();
      }
    }
  }

  // Special handling for folly::none Optional values to enable
  // callers to directly pass them in as a query argument
  /* implicit */ QueryArgument(const folly::None& /*opt*/) {}

  // Pair constructors
  QueryArgument();
  QueryArgument(folly::StringPiece param1, const QueryArgument& param2);

  // Since we already have callsites that use dynamic, we are keeping the
  // support, but internally we unpack them.
  // This factory method will throw exception if the dynamic isn't acceptable
  // Creating this as a factory method has two benefits: one is it will prevent
  // accidentally adding more callsites, secondly it is easily bgs-able.
  // Also makes it explicit this might throw whereas the other constructors
  // might not.
  static inline QueryArgument fromDynamic(const folly::dynamic& dyn) {
    QueryArgument arg;
    arg.initFromDynamic(dyn);
    return arg;
  }

  QueryArgument&& operator()(folly::StringPiece q1, const QueryArgument& q2);
  QueryArgument&& operator()(folly::fbstring&& q1, QueryArgument&& q2);
  folly::fbstring asString() const;

  double getDouble() const;
  int64_t getInt() const;
  bool getBool() const;
  const Query& getQuery() const;
  const folly::fbstring& getString() const;
  const std::vector<std::pair<folly::fbstring, QueryArgument>>& getPairs()
      const;
  const std::vector<QueryArgument>& getList() const;
  const QualifiedColumn& getTwoTuple() const;
  const AliasedQualifiedColumn& getThreeTuple() const;
  const AggregateColumn& getAggregateColumn() const;
  const AliasedAggregateColumn& getAliasedAggregateColumn() const;

  bool isString() const;
  bool isQuery() const;
  bool isPairList() const;
  bool isBool() const;
  bool isNull() const;
  bool isList() const;
  bool isDouble() const;
  bool isInt() const;
  bool isTwoTuple() const;
  bool isThreeTuple() const;
  bool isAggregateColumn() const;
  bool isAliasedAggregateColumn() const;

  std::string_view typeName() const;

 private:
  void initFromDynamic(const folly::dynamic& dyn);
  std::vector<std::pair<folly::fbstring, QueryArgument>>& getPairs();
};

template <typename... Args>
Query::Query(const folly::StringPiece query_text, Args&&... args)
    : query_text_(query_text), unsafe_query_(false), params_() {
  params_.reserve(sizeof...(args));
  unpack(std::forward<Args>(args)...);
}
template <typename Arg, typename... Args>
void Query::unpack(Arg&& arg, Args&&... args /* lol */) {
  using V = folly::remove_cvref_t<Arg>;
  if constexpr (
      std::is_same_v<V, folly::dynamic> ||
      std::is_same_v<V, decltype(folly::dynamic::object())>) {
    // Have to forward<Arg> because dynamic(ObjectMaker const&) is deleted.
    params_.emplace_back(QueryArgument::fromDynamic(std::forward<Arg>(arg)));
  } else {
    params_.emplace_back(std::forward<Arg>(arg));
  }
  unpack(std::forward<Args>(args)...);
}

} // namespace facebook::common::mysql_client

// A formatter for the Query class for folly::format
template <>
class folly::FormatValue<facebook::common::mysql_client::Query> {
 public:
  explicit FormatValue(const facebook::common::mysql_client::Query& query)
      : query_(query) {}

  template <class FormatCallback>
  void format(FormatArg& /*arg*/, FormatCallback& cb) const {
    cb(query_.renderInsecure());
  }

 private:
  const facebook::common::mysql_client::Query& query_;
};

// A formatter for the Query class for fmt::format
template <>
class fmt::formatter<facebook::common::mysql_client::Query> {
 public:
  template <typename ParseContext>
  constexpr auto parse(const ParseContext& ctx) const {
    // No reading of the format needed
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(
      const facebook::common::mysql_client::Query& query,
      FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", query.renderInsecure());
  }
};

namespace std {
// A formatter for the Query class for operator<<
inline std::ostream& operator<<(
    std::ostream& os,
    const facebook::common::mysql_client::Query& query) {
  return os << query.renderInsecure();
}
} // namespace std

namespace std {
template <>
struct hash<facebook::common::mysql_client::QueryOptions> {
  std::size_t operator()(
      const facebook::common::mysql_client::QueryOptions& opt) const {
    return opt.hashValue();
  }
};
} // namespace std
