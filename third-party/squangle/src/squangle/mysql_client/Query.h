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

#include <fmt/core.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/json/dynamic.h>

#include <glog/logging.h>

#include <mysql/server/include/mysql.h>

#include <functional>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "squangle/base/Base.h"
#include "squangle/mysql_client/InternalConnection.h"

namespace facebook::common::mysql_client {

class Query;
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

using QueryValues = std::vector<QueryArgument>;
using QueryValuesList = std::vector<QueryValues>;

// ---------------------------------------------------------------------------
// Checked format string support for compile-time validated queries.
// ---------------------------------------------------------------------------

namespace detail {

// fixed_string for use as non-type template parameter to capture string
// literal content at compile time without function parameter issues.
// Similar to fmt's approach but simplified for our use case.
template <size_t N>
struct fixed_string {
  char data[N]{};
  /* implicit */ consteval fixed_string(const char (&s)[N]) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = s[i];
    }
  }
  constexpr std::string_view view() const {
    return {data, N - 1};
  }
  constexpr size_t size() const {
    return N - 1;
  }
};

template <size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

// Maximum number of specifiers we support in a single checked query. If a
// format string exceeds this, consteval_parse_checked sets ok=false with a
// "too many params" error, which surfaces as a compile-time failure.
constexpr size_t kMaxCheckedSpecs = 256;

struct CheckedParseResult {
  size_t param_count = 0;
  std::array<char, kMaxCheckedSpecs>
      spec_char{}; // exact specifier char for precise type checking:
                   // 's','d','u','f','m','T','C', etc. For %L variants store
                   // sub-type char, for %= variants store sub-type char, for
                   // %LO/%LA store 'O'/'A', etc.
  std::array<bool, kMaxCheckedSpecs>
      is_list{}; // true for the element-list specifiers %Ls/%Ld/%Lu/%Lf/%Lm and
                 // %LC, whose argument must be a list (not a scalar). %LO/%LA
                 // are pair lists and use spec_char 'O'/'A' instead.
  bool ok = true;
  const char* error = nullptr;
  size_t error_offset = 0;
};

consteval bool is_dangerous_char(char c) {
  return c == ';' || c == '\'' || c == '"' || c == '`';
}

// Core parser over the format string's characters (s does not include a
// trailing NUL). The char-array and fixed_string overloads below forward here.
consteval CheckedParseResult consteval_parse_checked(std::string_view s) {
  CheckedParseResult res{};
  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];
    if (is_dangerous_char(c)) {
      res.ok = false;
      res.error = "dangerous character in query literal";
      res.error_offset = i;
      return res;
    }
    if (c == '%') {
      if (i + 1 >= s.size()) {
        res.ok = false;
        res.error = "string ended with unfinished %";
        res.error_offset = i;
        return res;
      }
      char n = s[i + 1];
      i += 2;
      if (n == '%') {
        continue; // literal percent, no param
      } else if (n == 's' || n == 'd' || n == 'u' || n == 'f' || n == 'm') {
        // single value specs from original spec set
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = n;
        res.param_count++;
      } else if (n == 'T' || n == 'C') {
        // table or column identifier
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = n;
        res.param_count++;
      } else if (n == '=') {
        // expect %=s %=d %=u %=f %=m
        if (i >= s.size()) {
          res.ok = false;
          res.error = "incomplete %=";
          res.error_offset = i - 2;
          return res;
        }
        char t = s[i];
        i++;
        if (t != 's' && t != 'd' && t != 'u' && t != 'f' && t != 'm') {
          res.ok = false;
          res.error =
              "only %=s %=d %=u %=f %=m allowed in checked mode after %=";
          res.error_offset = i - 2;
          return res;
        }
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = t;
        res.param_count++;
      } else if (n == 'L') {
        // list variants: %Ls %Ld %Lu %Lf %Lm %LC %LO %LA
        // %Q is explicitly disallowed.
        if (i >= s.size()) {
          res.ok = false;
          res.error = "incomplete %L";
          res.error_offset = i - 2;
          return res;
        }
        char t = s[i];
        i++;
        if (t == 's' || t == 'd' || t == 'u' || t == 'f' || t == 'm') {
          // list of values
          if (res.param_count >= kMaxCheckedSpecs) {
            res.ok = false;
            res.error = "too many params";
            return res;
          }
          res.spec_char[res.param_count] = t;
          res.is_list[res.param_count] = true;
          res.param_count++;
        } else if (t == 'C') {
          // list of column identifiers.
          if (res.param_count >= kMaxCheckedSpecs) {
            res.ok = false;
            res.error = "too many params";
            return res;
          }
          res.spec_char[res.param_count] = 'C';
          res.is_list[res.param_count] = true;
          res.param_count++;
        } else if (t == 'O' || t == 'A') {
          if (res.param_count >= kMaxCheckedSpecs) {
            res.ok = false;
            res.error = "too many params";
            return res;
          }
          res.spec_char[res.param_count] = t;
          res.param_count++;
        } else {
          res.ok = false;
          res.error =
              "unknown or disallowed specifier after %L in checked mode; "
              "allowed: %Ls %Ld %Lu %Lf %Lm %LC %LO %LA";
          res.error_offset = i - 2;
          return res;
        }
      } else if (n == 'U') {
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = 'U';
        res.param_count++;
      } else if (n == 'W') {
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = 'W';
        res.param_count++;
      } else if (n == 'V') {
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = 'V';
        res.param_count++;
      } else if (n == 'K') {
        if (res.param_count >= kMaxCheckedSpecs) {
          res.ok = false;
          res.error = "too many params";
          return res;
        }
        res.spec_char[res.param_count] = 'K';
        res.param_count++;
      } else {
        res.ok = false;
        res.error =
            "unknown or disallowed specifier in checked mode; allowed: %% "
            "%s %d %u %f %m %T %C %LC %Ls %Ld %Lu %Lf %Lm "
            "%=s %=d %=u %=f %=m %U %W %V %K %LO %LA";
        res.error_offset = i - 2;
        return res;
      }
      continue;
    }
    i++;
  }
  return res;
}

// Char-array overload: forwards to the string_view core using the exact length
// (N - 1 excludes the trailing NUL). Preferred over the implicit string_view
// conversion for char-array arguments.
template <size_t N>
consteval CheckedParseResult consteval_parse_checked(const char (&s)[N]) {
  return consteval_parse_checked(std::string_view(s, N - 1));
}

// Parse a format string supplied as a non-type template parameter. fixed_string
// stores the literal as a char array, so this just forwards to the array
// overload above — there is a single parser implementation.
template <fixed_string Str>
consteval CheckedParseResult consteval_parse_checked() {
  return consteval_parse_checked(Str.data);
}

// Type trait for the scalar value categories a value specifier accepts. Note
// std::is_arithmetic already covers bool. Null literals, optionals, and value
// lists are handled separately (is_null_arg_v, is_optional_*_v, ListArg).
template <typename T>
constexpr bool is_value_arg_v = std::disjunction_v<
    std::is_arithmetic<std::decay_t<T>>,
    // Enums collapse to int64 in QueryArgument, so they are valid %m/%=m values
    // (and integer-specifier values via is_any_int_v), matching legacy.
    std::is_enum<std::decay_t<T>>,
    std::is_same<std::decay_t<T>, std::string>,
    std::is_same<std::decay_t<T>, std::string_view>,
    std::is_same<std::decay_t<T>, folly::fbstring>,
    std::is_same<std::decay_t<T>, folly::StringPiece>,
    std::is_same<std::decay_t<T>, const char*>,
    std::is_same<std::decay_t<T>, Query>,
    std::is_same<std::decay_t<T>, std::nullptr_t>>;

template <typename T>
struct is_optional : std::false_type {};
template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};
template <typename T>
struct is_optional<folly::Optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_value_arg_or_optional_v =
    is_value_arg_v<T> || is_optional<std::decay_t<T>>::value ||
    std::is_same_v<std::decay_t<T>, std::nullopt_t> ||
    std::is_same_v<std::decay_t<T>, folly::None>;

// A bare null literal (nullptr / nullopt / folly::none) is a valid argument for
// any value specifier — it renders as NULL (or "IS NULL" for the %= variants).
template <typename T>
constexpr bool is_null_arg_v =
    std::is_same_v<std::decay_t<T>, std::nullptr_t> ||
    std::is_same_v<std::decay_t<T>, std::nullopt_t> ||
    std::is_same_v<std::decay_t<T>, folly::None>;

template <typename T>
constexpr bool is_string_like_v = std::disjunction_v<
    std::is_same<std::decay_t<T>, std::string>,
    std::is_same<std::decay_t<T>, std::string_view>,
    std::is_same<std::decay_t<T>, folly::fbstring>,
    std::is_same<std::decay_t<T>, folly::StringPiece>,
    std::is_same<std::decay_t<T>, const char*>,
    std::is_same<std::decay_t<T>, char*>>;

// Matches legacy QueryRenderer/QueryArgument: every integral (including bool)
// and every enum is accepted for both %d and %u. QueryArgument collapses bool
// and enums to int64 at construction, so by render time they are indistinguish-
// able from a plain integer; the checked validator mirrors that here so callers
// need not static_cast an enum to its underlying integer.
template <typename T>
constexpr bool is_any_int_v =
    std::is_integral_v<std::decay_t<T>> || std::is_enum_v<std::decay_t<T>>;

template <typename T>
constexpr bool is_float_v = std::is_floating_point_v<std::decay_t<T>>;

template <typename T, typename = void>
struct is_optional_string_like_helper : std::false_type {};
template <typename T>
struct is_optional_string_like_helper<
    T,
    std::void_t<typename std::decay_t<T>::value_type>>
    : std::bool_constant<
          is_optional<std::decay_t<T>>::value &&
          is_string_like_v<typename std::decay_t<T>::value_type>> {};
template <typename T>
constexpr bool is_optional_string_like_v =
    is_optional_string_like_helper<T>::value;

template <typename T, typename = void>
struct is_optional_int_helper : std::false_type {};
template <typename T>
struct is_optional_int_helper<
    T,
    std::void_t<typename std::decay_t<T>::value_type>>
    : std::bool_constant<
          is_optional<std::decay_t<T>>::value &&
          is_any_int_v<typename std::decay_t<T>::value_type>> {};
template <typename T>
constexpr bool is_optional_int_v = is_optional_int_helper<T>::value;

template <typename T, typename = void>
struct is_optional_float_helper : std::false_type {};
template <typename T>
struct is_optional_float_helper<
    T,
    std::void_t<typename std::decay_t<T>::value_type>>
    : std::bool_constant<
          is_optional<std::decay_t<T>>::value &&
          is_float_v<typename std::decay_t<T>::value_type>> {};
template <typename T>
constexpr bool is_optional_float_v = is_optional_float_helper<T>::value;

template <typename T>
constexpr bool is_identifier_arg_v = std::disjunction_v<
    std::is_same<std::decay_t<T>, std::string>,
    std::is_same<std::decay_t<T>, std::string_view>,
    std::is_same<std::decay_t<T>, folly::fbstring>,
    std::is_same<std::decay_t<T>, folly::StringPiece>,
    std::is_same<std::decay_t<T>, const char*>,
    std::is_same<std::decay_t<T>, QualifiedColumn>,
    std::is_same<std::decay_t<T>, AliasedQualifiedColumn>,
    std::is_same<std::decay_t<T>, AggregateColumn>,
    std::is_same<std::decay_t<T>, AliasedAggregateColumn>>;

template <typename T>
constexpr bool is_list_of_values_v =
    false; // simplified: accept vector<QueryArgument> via existing
           // QueryArgument ctor
template <>
inline constexpr bool is_list_of_values_v<std::vector<QueryArgument>> = true;
template <>
inline constexpr bool
    is_list_of_values_v<std::initializer_list<QueryArgument>> = true;

template <typename T>
struct is_std_pair : std::false_type {};
template <typename A, typename B>
struct is_std_pair<std::pair<A, B>> : std::true_type {};

// The explicit std::vector<std::pair<...>> forms below are the canonical typed
// pair lists. Arbitrary key/value ranges (std::map / std::unordered_map / F14
// maps, and other vector<pair<...>> element types) are additionally accepted
// via the PairRange concept, which PairListArg folds in -- see below.
template <typename T>
constexpr bool is_pair_list_v = false;
template <>
inline constexpr bool
    is_pair_list_v<std::vector<std::pair<folly::fbstring, QueryArgument>>> =
        true;
template <>
inline constexpr bool
    is_pair_list_v<std::vector<std::pair<std::string, QueryArgument>>> = true;
// Note: folly::dynamic and folly::dynamic::object(...) are intentionally NOT
// accepted as pair lists by Query::checked. Pass a typed
// std::vector<ArgumentPair> (or std::vector<std::pair<std::string,
// QueryArgument>>), or wrap runtime-shaped data in an explicit
// QueryArgument::fromDynamic(). The legacy Query() constructor still accepts
// folly::dynamic for %U/%W/%O/%A.

// The VALUES matrix is supplied in type-erased form as a
// vector/initializer_list<QueryArgument> whose elements are themselves lists
// (one per row) -- the row-list form a QueryArgument holds directly. We also
// accept std::vector<std::vector<QueryArgument>> as a convenience: the
// QueryArgumentCollection constructor turns each inner vector<QueryArgument>
// into a row list, yielding the same representation. A generic
// vector<vector<T>> (uniform cell type) is intentionally not accepted -- values
// rows are usually heterogeneous, so a per-cell QueryArgument is the right
// form.
template <typename T>
constexpr bool is_values_matrix_v = false;
template <>
inline constexpr bool is_values_matrix_v<std::vector<QueryArgument>> = true;
template <>
inline constexpr bool is_values_matrix_v<std::initializer_list<QueryArgument>> =
    true;
template <>
inline constexpr bool
    is_values_matrix_v<std::vector<std::vector<QueryArgument>>> = true;

// Concepts for clearer compiler diagnostics in checked mode. These names appear
// directly in compiler error output when a type does not satisfy the expected
// category for a given format specifier position.
//
// The only list forms a QueryArgument can hold are vector<QueryArgument> and
// initializer_list<QueryArgument>, so that is what every element-list specifier
// (%Ls/%Ld/%Lu/%Lf/%Lm/%LC) accepts; per-element types are validated at render
// time (as the legacy QueryRenderer does) since the elements are type-erased.
template <typename T>
concept ListArg = is_list_of_values_v<std::decay_t<T>>;

template <typename T>
concept ValueArg = is_value_arg_or_optional_v<T>;

template <typename T>
concept IdentifierArg = is_identifier_arg_v<T>;

// A range of key/value pairs usable as a pair list for %U/%W/%LO/%LA: any range
// whose element is a std::pair with a string-like key and a
// QueryArgument-constructible value. This covers std::map / std::unordered_map
// / folly F14 maps (and std::vector<std::pair<...>> with any convertible
// value), letting callers build and mutate a column->value set by key before
// handing it to the query. Iteration order follows the container -- hash maps
// are unordered, so use a sorted map or a std::vector<ArgumentPair> when a
// specific column order is required (order never affects SET/WHERE-AND/OR
// correctness).
template <typename T>
concept PairRange =
    std::ranges::range<std::remove_cvref_t<T>> &&
    is_std_pair<std::remove_cvref_t<
        std::ranges::range_value_t<std::remove_cvref_t<T>>>>::value &&
    is_string_like_v<typename std::remove_cvref_t<
        std::ranges::range_value_t<std::remove_cvref_t<T>>>::first_type> &&
    std::is_constructible_v<
        QueryArgument,
        const typename std::remove_cvref_t<
            std::ranges::range_value_t<std::remove_cvref_t<T>>>::second_type&>;

template <typename T>
concept PairListArg = is_pair_list_v<std::decay_t<T>> || PairRange<T>;

template <typename T>
concept ValuesMatrixArg = is_values_matrix_v<std::decay_t<T>>;

template <typename T>
concept CommentArg = is_string_like_v<T> || is_optional_string_like_v<T>;

// Per-specifier type check. The specifier char and its list-ness are *function*
// arguments (not template arguments) so this works both when the format string
// is a non-type template parameter and when it is a consteval-constructor
// parameter (the Query::checked function path), where the parsed values are not
// constant expressions and so could not be used as template arguments.
//
// Acceptance mirrors the legacy QueryRenderer: an element-list specifier
// requires a list; %s a string; %d/%u any integer; %f a float; %m any value;
// %T/%C an identifier; %U/%W/%LO/%LA a pair list; %V a values matrix; %K a
// comment string. A null (nullptr/nullopt/folly::none) is valid for any scalar
// value specifier (it renders as NULL, or IS NULL for the %= variants).
//
// A type-erased value — a QueryArgument or a folly::dynamic (incl. the
// dynamic::object() builder) — whose concrete contents can't be known at
// compile time. These are the canonical ways existing call sites pass values
// (columnName(), QueryArgument::fromDynamic(), a runtime-built folly::dynamic).
template <typename T>
constexpr bool is_type_erased_value_v =
    std::is_same_v<std::decay_t<T>, QueryArgument> ||
    std::is_same_v<std::decay_t<T>, folly::dynamic> ||
    std::is_same_v<std::decay_t<T>, decltype(folly::dynamic::object())>;

// A homogeneous collection usable as an element-list argument: any iterable
// range whose element type converts to a QueryArgument. This covers
// std::vector, std::set/unordered_set, folly's F14 sets, std::array,
// std::deque, etc. without naming each container. Strings (ranges of char) and
// type-erased values (QueryArgument, folly::dynamic) are ranges too but must
// NOT be treated as element lists, so they are excluded; a
// std::vector<QueryArgument> is excluded here as well since it is handled by
// the type-erased ListArg path, and pair-list/map types (whose element is a
// std::pair, not QueryArgument- constructible) fall through to the pair-list
// handling.
template <typename T>
concept QueryArgumentCollection = std::ranges::range<std::remove_cvref_t<T>> &&
    !is_string_like_v<T> && !is_type_erased_value_v<T> &&
    !std::is_same_v<std::ranges::range_value_t<std::remove_cvref_t<T>>,
                    QueryArgument> &&
    std::is_constructible_v<QueryArgument,
                            const std::ranges::range_value_t<
                                std::remove_cvref_t<T>>&>;

// Type-erasure that Query::checked accepts for ANY specifier: only a
// QueryArgument, which the caller constructs explicitly (e.g. columnName() or
// QueryArgument::fromDynamic()). A bare folly::dynamic is intentionally NOT
// accepted by checked -- pass a concrete typed argument, or an explicit
// QueryArgument::fromDynamic() for genuinely runtime-shaped data. (The legacy
// Query() constructor still accepts folly::dynamic directly.)
template <typename T>
constexpr bool is_checked_erased_value_v =
    std::is_same_v<std::decay_t<T>, QueryArgument>;

// The format syntax and argument count are still checked at compile time; a
// QueryArgument's value-vs-specifier check is deferred to the runtime renderer
// (which validates and DCHECKs the actual shape, and throws on a real
// mismatch).
template <typename T>
constexpr bool check_arg_for_spec_precise(char spec, bool is_list) {
  if constexpr (is_checked_erased_value_v<T>) {
    return true;
  } else if (is_list) {
    if constexpr (ListArg<T>) {
      // std::vector<QueryArgument> / initializer_list<QueryArgument>: a list
      // whose elements are type-erased, so accepted for any list specifier and
      // validated per-element at render time (matches folly::dynamic).
      return true;
    } else if constexpr (QueryArgumentCollection<T>) {
      // A homogeneous typed container (e.g. std::vector/std::set<std::string>
      // for %Ls): check its element type against the list specifier's element
      // subtype, mirroring the scalar checks below.
      using E = std::ranges::range_value_t<std::remove_cvref_t<T>>;
      switch (spec) {
        case 's':
          return is_string_like_v<E>;
        case 'd':
        case 'u':
          return is_any_int_v<E>;
        case 'f':
          return is_float_v<E>;
        case 'm':
          return is_value_arg_or_optional_v<E>;
        case 'C':
          return is_identifier_arg_v<E>;
        default:
          return false;
      }
    } else {
      return false;
    }
  } else {
    switch (spec) {
      case 's': // string value, null, or a sub-Query (rendered as a subquery,
                // matching legacy; %m also accepts a sub-Query)
        return is_string_like_v<T> || is_optional_string_like_v<T> ||
            is_null_arg_v<T> || std::is_same_v<std::decay_t<T>, Query>;
      case 'd': // integer — legacy accepts any integral for both %d and %u
      case 'u':
        return is_any_int_v<T> || is_optional_int_v<T> || is_null_arg_v<T>;
      case 'f': // float/double
        return is_float_v<T> || is_optional_float_v<T> || is_null_arg_v<T>;
      case 'm': // any value
        return ValueArg<T>;
      case 'T': // table or column identifier
      case 'C':
        return IdentifierArg<T>;
      case 'U': // update / where / list object OR/AND expect a pair list
      case 'W':
      case 'O':
      case 'A':
        return PairListArg<T>;
      case 'V':
        return ValuesMatrixArg<T>;
      case 'K':
        return CommentArg<T>;
      default:
        return false;
    }
  }
}

// Folds the per-argument type check over each specifier position. The caller
// must have verified parse success and that the specifier count equals
// sizeof...(Args), so spec_char[Is] is valid for every Is.
template <typename... Args, size_t... Is>
constexpr bool checked_types_ok(
    const CheckedParseResult& parsed,
    std::index_sequence<Is...>) {
  return (
      ... &&
      check_arg_for_spec_precise<std::tuple_element_t<Is, std::tuple<Args...>>>(
          parsed.spec_char[Is], parsed.is_list[Is]));
}

// Uniform compile-time predicate over a non-type template format string:
// returns false (never hard-asserts) for a parse failure, an argument-count
// mismatch, or a per-argument type mismatch. Used by the static_asserts in
// Query::checked and by negative test cases that expect a false result.
template <detail::fixed_string Str, typename... Args>
consteval bool check_args_fixed() {
  constexpr auto parsed = consteval_parse_checked<Str>();
  if constexpr (!parsed.ok) {
    return false;
  } else if constexpr (parsed.param_count != sizeof...(Args)) {
    return false;
  } else {
    return checked_types_ok<Args...>(
        parsed, std::make_index_sequence<sizeof...(Args)>{});
  }
}

// Compile-time accessors over a non-type template format string, used by the
// checked-query unit tests to assert parse success and specifier count
// directly.
template <detail::fixed_string Str>
consteval size_t count_specs_fixed() {
  return consteval_parse_checked<Str>().param_count;
}

template <detail::fixed_string Str>
consteval bool parse_ok_fixed() {
  return consteval_parse_checked<Str>().ok;
}

// Error reporters for the Query::checked function path. These are
// intentionally NOT constexpr: a checked_format_string constructor is
// consteval, so if its validation reaches one of these calls the constant
// evaluation becomes ill-formed and the compiler reports a hard error at the
// call site — naming the function, which names the problem. For a valid
// format string the calls are never reached, so the consteval evaluation
// succeeds. This is how an ordinary (non-macro) function can still reject a
// bad format at compile time.
[[noreturn]] void
checked_query_format_has_unsupported_specifier_or_dangerous_character();
[[noreturn]] void checked_query_wrong_number_of_arguments_for_specifiers();
[[noreturn]] void checked_query_argument_type_not_valid_for_its_specifier();

} // namespace detail

// Public wrapper type that lets Query::checked be an ordinary (non-macro)
// function while still validating the format string at compile time. Mirrors
// the std::format_string / fmt::format_string pattern: the consteval
// constructor parses the literal and rejects an invalid format/argument set by
// reaching a non-constexpr error reporter (see detail::checked_format_* above),
// which the compiler turns into a hard error. Args is deduced from the call
// arguments (via std::type_identity_t in Query::checked) and pinned here, so
// the constructor knows the argument types when validating.
template <typename... Args>
struct checked_format_string {
  std::string_view str;

  // Primary validating constructor. Accepts a compile-time-constant
  // std::string_view (e.g. a constexpr std::string_view query constant);
  // because it is consteval, a runtime string_view (from a runtime std::string
  // or const char*) is rejected at compile time. The char-array overload below
  // delegates here so a string literal works too without duplicating logic.
  /* implicit */ consteval checked_format_string(std::string_view sv)
      : str(sv) {
    auto parsed = detail::consteval_parse_checked(sv);
    if (!parsed.ok) {
      detail::
          checked_query_format_has_unsupported_specifier_or_dangerous_character();
    } else if (parsed.param_count != sizeof...(Args)) {
      detail::checked_query_wrong_number_of_arguments_for_specifiers();
    } else if (!detail::checked_types_ok<Args...>(
                   parsed, std::make_index_sequence<sizeof...(Args)>{})) {
      detail::checked_query_argument_type_not_valid_for_its_specifier();
    }
  }

  // String-literal / char-array overload: exact length (excludes the trailing
  // NUL), delegating to the string_view constructor for validation.
  template <size_t N>
  /* implicit */ consteval checked_format_string(const char (&s)[N])
      : checked_format_string(std::string_view(s, N - 1)) {}

  // folly::StringPiece overload (e.g. a constexpr folly::StringPiece, or a
  // folly::FixedString via its explicit .toRange()), delegating to the
  // string_view constructor. Being consteval, it too requires a
  // compile-time-constant value.
  /* implicit */ consteval checked_format_string(folly::StringPiece sp)
      : checked_format_string(std::string_view(sp.data(), sp.size())) {}

  // Allow implicit construction only from a compile-time string (literal,
  // constexpr char array, or constexpr std::string_view), never a runtime one.
  // Copy/move are defaulted (the object is a trivial string_view holder that is
  // materialized at compile time and then passed by value at runtime).
  checked_format_string() = delete;
  checked_format_string(const checked_format_string&) = default;
  checked_format_string(checked_format_string&&) = default;
  checked_format_string& operator=(const checked_format_string&) = default;
  checked_format_string& operator=(checked_format_string&&) = default;
  ~checked_format_string() = default;
};

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
      loggingFuncs_.reset();
    } else {
      if (!loggingFuncs_) {
        loggingFuncs_ = std::make_shared<LoggingFuncs>();
      }

      (*loggingFuncs_)[name] = std::move(func);
    }
  }

  LoggingFuncsPtr stealLoggingFuncs() {
    return std::move(loggingFuncs_);
  }

 protected:
  QueryAttributes attributes_;
  std::optional<Duration> queryTimeoutOverride_;
  LoggingFuncsPtr loggingFuncs_;
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

  // Delegate to the rvalue overload with a copy so the append (and mode
  // downgrade) logic lives in exactly one place.
  void append(const Query& query2) {
    append(Query(query2));
  }
  void append(Query&& query2);

  Query& operator+=(const Query& query2) {
    append(query2);
    return *this;
  }

  Query& operator+=(Query&& query2) {
    append(std::move(query2));
    return *this;
  }

  // operator+ is ref-qualified so a chain of temporaries (`a + b + c + ...`,
  // left-associative) reuses the accumulating left-hand side's storage instead
  // of copying it at every step; the `Query&&` right-hand overloads likewise
  // move the right-hand side in rather than copying. An lvalue left-hand side
  // must still be copied (it can't be stolen).
  Query operator+(const Query& query2) const& {
    Query ret(*this);
    ret.append(query2);
    return ret;
  }

  Query operator+(Query&& query2) const& {
    Query ret(*this);
    ret.append(std::move(query2));
    return ret;
  }

  Query operator+(const Query& query2) && {
    append(query2);
    return std::move(*this);
  }

  Query operator+(Query&& query2) && {
    append(std::move(query2));
    return std::move(*this);
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

  static folly::fbstring renderMultiQueryFb(
      const InternalConnection* conn,
      const std::vector<Query>& queries,
      std::string_view prefix = "");

  static std::string renderMultiQueryStr(
      const InternalConnection* conn,
      const std::vector<Query>& queries,
      std::string_view prefix = "");

  // Backwards-compatible alias
  static folly::fbstring renderMultiQuery(
      const InternalConnection* conn,
      const std::vector<Query>& queries,
      std::string_view prefix = "") {
    return renderMultiQueryFb(conn, queries, prefix);
  }

  // -- Fb variants (return folly::fbstring) --

  // Render with connection-based escaping, suitable for sending to MySQL.
  folly::fbstring renderFb(const InternalConnection* conn) const;
  folly::fbstring renderFb(
      const InternalConnection* conn,
      const std::vector<QueryArgument>& params) const;

  // Render without escaping. Mainly for testing.
  folly::fbstring renderInsecureFb() const;
  folly::fbstring renderInsecureFb(
      const std::vector<QueryArgument>& params) const;
  // Render without escaping, truncated to at most maxSize characters.
  // When truncation occurs, truncationIndicator is appended (its length is
  // accounted for within maxSize). Pass "" to suppress the indicator.
  folly::fbstring renderInsecureFb(
      size_t maxSize,
      std::string_view truncationIndicator = "...") const;

  // Render with basic escaping. Not suitable for MySQL, but good for logging.
  folly::fbstring renderPartiallyEscapedFb() const;
  folly::fbstring renderPartiallyEscapedFb(
      size_t maxSize,
      std::string_view truncationIndicator = "...") const;

  // -- Str variants (return std::string) --

  std::string renderStr(const InternalConnection* conn) const;
  std::string renderStr(
      const InternalConnection* conn,
      const std::vector<QueryArgument>& params) const;

  std::string renderInsecureStr() const;
  std::string renderInsecureStr(const std::vector<QueryArgument>& params) const;
  std::string renderInsecureStr(
      size_t maxSize,
      std::string_view truncationIndicator = "...") const;

  std::string renderPartiallyEscapedStr() const;
  std::string renderPartiallyEscapedStr(
      size_t maxSize,
      std::string_view truncationIndicator = "...") const;

  // -- Backwards-compatible aliases (delegate to Fb variants) --

  folly::fbstring render(const InternalConnection* conn) const {
    return renderFb(conn);
  }
  folly::fbstring render(
      const InternalConnection* conn,
      const std::vector<QueryArgument>& params) const {
    return renderFb(conn, params);
  }
  folly::fbstring renderInsecure() const {
    return renderInsecureFb();
  }
  folly::fbstring renderInsecure(
      const std::vector<QueryArgument>& params) const {
    return renderInsecureFb(params);
  }
  folly::fbstring renderPartiallyEscaped() const {
    return renderPartiallyEscapedFb();
  }

  folly::StringPiece getQueryFormat() const {
    // For both legacy and checked queries the (format) text lives in
    // query_text_; mode_ only selects which renderer validates it.
    return query_text_.getQuery();
  }

  const std::vector<QueryArgument>& getParams() const {
    return params_;
  }

  bool isChecked() const noexcept {
    return mode_ == Mode::Checked;
  }

  // Compile-time checked construction entry point.
  //
  //   auto q = Query::checked("SELECT * FROM %T WHERE id = %d", tbl, id);
  //
  // The format string must be a string literal. It is validated at compile time
  // by the consteval checked_format_string constructor: a dangerous character,
  // an unknown/disallowed specifier, a parameter-count mismatch, or an argument
  // whose type does not match its specifier's category is a hard compile error.
  // This is an ordinary function — no macro required.
  //
  // What compile-time checking can and cannot prove: it verifies the format
  // syntax, the argument count, and each argument's *category* (value vs.
  // identifier vs. pair-list vs. list vs. values-matrix vs. comment). It does
  // NOT prove element/shape correctness for type-erased collection arguments —
  // e.g. that each row of a %V matrix is itself a list, that a folly::dynamic
  // passed to %W is actually an object, or that a %Ls list's elements match the
  // sub-type. Those remain runtime checks (and a bad folly::dynamic can still
  // throw at construction). A sub-Query value is accepted for %s and %m.
  //
  // Allowed specifiers: %%  %s %d %u %f %m  %T %C  %LC %Ls %Ld %Lu %Lf %Lm
  // %=s %=d %=u %=f %=m  %U %W %V %K %LO %LA. %Q is intentionally unsupported;
  // use the unsafe Query() constructor if you truly need raw SQL (discouraged).
  //
  // Diagnostic gotcha: a bad arg type/count normally errors right at the
  // checked() call. But if the call sits inside a folly::coro lambda that is
  // passed to a template and invoked through a runtime reference (e.g. a retry
  // helper's `Func const& f; ... f()`), the consteval failure instead surfaces
  // as "call to immediate function ... is not a constant expression" at that
  // invocation. If you see that, check the checked() argument types in the
  // lambda (a common cause is passing a folly::dynamic where a typed collection
  // is required).
  template <typename... Args>
  static Query checked(
      checked_format_string<std::type_identity_t<Args>...> fmt,
      Args&&... args) {
    return Query(CheckedTag{}, fmt.str, std::forward<Args>(args)...);
  }

 private:
  enum class Mode : uint8_t { Legacy, Checked };

  struct CheckedTag {};

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
    QueryText& operator=(QueryText&& other) noexcept {
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

  template <typename Arg, typename... Args>
  void unpack(Arg&& arg, Args&&... args);
  void unpack() {}

  // Private constructor for checked mode. The (compile-time validated) format
  // string is stored in query_text_ just like a legacy query's text;
  // mode_ == Checked selects the unvalidated renderer at render time.
  template <typename... Args>
  explicit Query(CheckedTag, std::string_view fmt, Args&&... args)
      : query_text_(folly::StringPiece{fmt.data(), fmt.size()}),
        unsafe_query_(false),
        mode_(Mode::Checked) {
    params_.reserve(sizeof...(Args));
    unpack(std::forward<Args>(args)...);
  }

  // mode_ is a normal member, so the defaulted copy/move constructors preserve
  // it (and the format text in query_text_) automatically.
  QueryText query_text_;
  bool unsafe_query_ = false;
  std::vector<QueryArgument> params_;
  Mode mode_ = Mode::Legacy;
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
  // NEVER raw-assign a caller value to value_ (e.g. `value_ = someArg`). Route
  // through the scalar ctors below instead (directly, or by delegating like the
  // std::optional ctor). A raw variant converting-assignment silently
  // mishandles several types: it rejects string_view/StringPiece (fbstring's
  // ctor from them is explicit) and, because `bool` is an alternative,
  // mis-selects bool for types with a standard conversion to bool (e.g. const
  // char* -> bool beats the user-defined -> fbstring). The scalar ctors exist
  // precisely to avoid this.
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

  // Build a list-valued QueryArgument from any homogeneous collection of scalar
  // elements (e.g. std::vector/std::set<std::string> for %Ls, an F14 set, a
  // std::array<int64_t>, ...). Each element is converted to a QueryArgument.
  // The QueryArgumentCollection concept admits any iterable range whose element
  // converts to a QueryArgument, while excluding strings, type-erased values,
  // std::vector<QueryArgument> (handled by the overload above), and pair-list/
  // map types. Element/iteration order is preserved, which is fine for SQL list
  // contexts (IN, etc.).
  template <typename C>
    requires detail::QueryArgumentCollection<C>
  /* implicit */ QueryArgument(const C& arg_list)
      : value_(toQueryArgumentList(arg_list)) {}

  // Adopt a prebuilt pair list (column -> value) for %U/%W/%O/%A. Distinct from
  // the empty-list default ctor and the operator() builder: this takes an
  // already-populated vector. The std::string-keyed overload converts keys to
  // fbstring. These make the std::vector<pair<...>> forms that is_pair_list_v
  // already advertises actually constructible.
  /* implicit */ QueryArgument(std::vector<ArgumentPair> pairs)
      : value_(std::move(pairs)) {}
  /* implicit */ QueryArgument(
      const std::vector<std::pair<std::string, QueryArgument>>& pairs) {
    std::vector<ArgumentPair> converted;
    converted.reserve(pairs.size());
    for (const auto& [key, value] : pairs) {
      converted.emplace_back(folly::fbstring(key), value);
    }
    value_ = std::move(converted);
  }

  // Adopt any range of key/value pairs (column -> value) as a pair list for
  // %U/%W/%O/%A: std::map / std::unordered_map / folly F14 maps, or a
  // std::vector<std::pair<...>> whose value is QueryArgument-constructible.
  // Keys (string-like) are converted to fbstring and values to QueryArgument.
  // Iteration order follows the container. The prebuilt std::vector<...>
  // overloads above are preferred by overload resolution for those exact types.
  template <typename M>
    requires detail::PairRange<M>
  /* implicit */ QueryArgument(const M& pairs) {
    std::vector<ArgumentPair> converted;
    if constexpr (std::ranges::sized_range<const M&>) {
      converted.reserve(std::ranges::size(pairs));
    }
    for (const auto& [key, value] : pairs) {
      const folly::StringPiece keyPiece(key);
      converted.emplace_back(
          folly::fbstring(keyPiece.data(), keyPiece.size()),
          QueryArgument(value));
    }
    value_ = std::move(converted);
  }

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
    // Delegate to the scalar ctors so an engaged optional accepts exactly what
    // a bare value does (integral/enum -> int64_t, floats, and the dedicated
    // string_view/StringPiece/char* ctors). A raw `value_ = opt.value()` would
    // reject string_view/StringPiece and mis-store char* as bool. nullopt stays
    // NULL (default monostate).
    if (opt) {
      *this = QueryArgument(opt.value());
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
    // See the std::optional<T> ctor above: delegate to the scalar ctors so an
    // engaged optional accepts exactly what a bare value does.
    if (opt) {
      *this = QueryArgument(opt.value());
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

  // Convert any homogeneous container of QueryArgument-convertible elements
  // into the list representation a QueryArgument holds.
  template <typename Container>
  static std::vector<QueryArgument> toQueryArgumentList(
      const Container& container) {
    std::vector<QueryArgument> list;
    // QueryArgumentCollection only requires std::ranges::range, so a non-sized
    // range (e.g. std::forward_list) can reach here -- only reserve when size()
    // is available.
    if constexpr (std::ranges::sized_range<const Container&>) {
      list.reserve(container.size());
    }
    for (const auto& elem : container) {
      list.emplace_back(elem);
    }
    return list;
  }
};

// A compile-time-schema'd VALUES row for the %V specifier. The format string
// declares the per-column value specifiers (e.g. ValueRow<"%d %s %f">); the
// variadic constructor accepts exactly that many arguments and validates each
// against its column specifier at compile time, reusing the same checks as
// Query::checked. A std::vector<ValueRow<Fmt>> is therefore a values matrix
// whose rows are guaranteed at compile time to have uniform arity and
// per-column types -- turning the ragged-row and wrong-cell-type runtime checks
// into compile errors. Accepted for %V by both the legacy Query(...)
// constructor and Query::checked(...).
//
// The schema validates each row's shape; it does not correlate with the query's
// actual column list (the %LC / literal "(a, b, c)" text). Fmt must use only
// the concrete value specifiers %s/%d/%u/%f -- one per column. %m ("any value")
// is rejected on purpose: it opts out of per-column type checking, which
// defeats the point of a compile-time-schema'd row. Identifier, list, and pair
// specifiers are likewise not meaningful for a value cell and are rejected.
template <detail::fixed_string Fmt>
class ValueRow {
 public:
  static constexpr detail::CheckedParseResult kParsed =
      detail::consteval_parse_checked(Fmt.view());
  static_assert(kParsed.ok, "ValueRow: invalid format string");
  static constexpr size_t kArity = kParsed.param_count;

  // The schema is a sequence of concrete value specifiers (%s/%d/%u/%f)
  // separated by optional delimiters. Because the schema is never rendered (it
  // only pins each column's type), only these characters are permitted:
  //   - the value specifiers %s / %d / %u / %f
  //   - whitespace (space, tab, newline, carriage return), commas, and vertical
  //     bars as delimiters between specifiers
  // Anything else -- %m, identifier/list/pair specifiers, or stray literal text
  // -- is a compile error, so a typo (e.g. a dropped %) can't be silently
  // swallowed as a delimiter.
  static constexpr bool kValidSchema = [] {
    const std::string_view s = Fmt.view();
    for (size_t i = 0; i < s.size();) {
      const char c = s[i];
      if (c == '%') {
        if (i + 1 >= s.size()) {
          return false;
        }
        const char n = s[i + 1];
        if (n != 's' && n != 'd' && n != 'u' && n != 'f') {
          return false;
        }
        i += 2;
        continue;
      }
      if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',' &&
          c != '|') {
        return false;
      }
      ++i;
    }
    return true;
  }();
  static_assert(
      kValidSchema,
      "ValueRow schema must be concrete value specifiers (%s/%d/%u/%f) "
      "separated only by whitespace, commas, or vertical bars");

  template <typename... Args>
    requires(
        sizeof...(Args) == kArity &&
        detail::checked_types_ok<std::remove_cvref_t<Args>...>(
            kParsed,
            std::make_index_sequence<sizeof...(Args)>{}))
  /* implicit */ ValueRow(Args&&... args) {
    cells_.reserve(sizeof...(Args));
    (cells_.emplace_back(std::forward<Args>(args)), ...);
  }

  const std::vector<QueryArgument>& cells() const {
    return cells_;
  }

  // Render as one %V row: a list-valued QueryArgument holding the cells.
  explicit operator QueryArgument() const {
    return QueryArgument(cells_);
  }

 private:
  std::vector<QueryArgument> cells_;
};

namespace detail {
// A std::vector<ValueRow<Fmt>> is a values matrix (a list of rows). Each row's
// arity/types were already checked when the ValueRow was constructed.
template <fixed_string Fmt>
inline constexpr bool is_values_matrix_v<std::vector<ValueRow<Fmt>>> = true;
} // namespace detail

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
