/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/intcompare.h"
#include <fmt/core.h>
#include "watchman/Errors.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"

#include <memory>

namespace watchman {

// Helper functions for integer comparisons in query expressions

static const struct {
  const char* opname;
  enum w_query_icmp_op op;
} opname_to_op[] = {
    {"eq", W_QUERY_ICMP_EQ},
    {"ne", W_QUERY_ICMP_NE},
    {"gt", W_QUERY_ICMP_GT},
    {"ge", W_QUERY_ICMP_GE},
    {"lt", W_QUERY_ICMP_LT},
    {"le", W_QUERY_ICMP_LE},
};

// term is a json array that looks like:
// ["size", "eq", 1024]
void parse_int_compare(const json_ref& term, struct w_query_int_compare* comp) {
  const char* opname;
  size_t i;
  bool found = false;

  auto& arr = term.array();

  if (arr.size() != 3) {
    throw QueryParseError("integer comparator must have 3 elements");
  }
  if (!arr[1].isString()) {
    throw QueryParseError("integer comparator op must be a string");
  }
  if (!arr[2].isInt()) {
    throw QueryParseError("integer comparator operand must be an integer");
  }

  opname = json_string_value(arr[1]);
  for (i = 0; i < sizeof(opname_to_op) / sizeof(opname_to_op[0]); i++) {
    if (!strcmp(opname_to_op[i].opname, opname)) {
      comp->op = opname_to_op[i].op;
      found = true;
      break;
    }
  }

  if (!found) {
    throw QueryParseError(
        fmt::format("integer comparator opname `{}' is invalid", opname));
  }

  comp->operand = arr[2].asInt();
}

bool eval_int_compare(json_int_t ival, struct w_query_int_compare* comp) {
  switch (comp->op) {
    case W_QUERY_ICMP_EQ:
      return ival == comp->operand;
    case W_QUERY_ICMP_NE:
      return ival != comp->operand;
    case W_QUERY_ICMP_GT:
      return ival > comp->operand;
    case W_QUERY_ICMP_GE:
      return ival >= comp->operand;
    case W_QUERY_ICMP_LT:
      return ival < comp->operand;
    case W_QUERY_ICMP_LE:
      return ival <= comp->operand;
    default:
      // Not possible to get here, but some compilers don't realize
      return false;
  }
}

class SizeExpr : public QueryExpr {
  w_query_int_compare comp;

 public:
  explicit SizeExpr(w_query_int_compare comp) : comp(comp) {}

  EvaluateResult evaluate(QueryContextBase*, FileResult* file) override {
    auto exists = file->exists();
    auto size = file->size();

    if (!exists.has_value()) {
      return std::nullopt;
    }

    // Removed files never match
    if (!exists.value()) {
      return false;
    }

    if (!size.has_value()) {
      return std::nullopt;
    }

    return eval_int_compare(size.value(), &comp);
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref& term) {
    if (!term.isArray()) {
      throw QueryParseError("Expected array for 'size' term");
    }

    w_query_int_compare comp;
    parse_int_compare(term, &comp);

    return std::make_unique<SizeExpr>(comp);
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // `size` doesn't constrain the path.
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
W_TERM_PARSER(size, SizeExpr::parse);

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
