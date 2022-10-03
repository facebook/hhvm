/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/TermRegistry.h"
#include <fmt/core.h>
#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/query/QueryExpr.h"

namespace watchman {

namespace {

struct RegisteredParser {
  std::string_view name;
  const QueryExprParser* parser;
};

constexpr RegisteredParser kParserTable[] = {
#define WATCHMAN_REGISTER_PARSER(name) {#name, &parsers::name##_parser},
    WATCHMAN_EXPRESSION_PARSER_LIST(WATCHMAN_REGISTER_PARSER)
#undef WATCHMAN_REGISTER_PARSER
};

// TODO: We could export the list of names and have CommandRegistry read it.
static struct Init {
  Init() {
    std::string prefix{"term-"};
    for (auto [parserName, parserp] : kParserTable) {
      // std::string_view concatenation is not actually nicer than sprintf, jeez
      capability_register((prefix + std::string{parserName}).c_str());
    }
  }
} init;

} // namespace

QueryExprParser getQueryExprParser(const w_string& name) {
  for (auto [parserName, parserp] : kParserTable) {
    if (parserName == name.view()) {
      if (auto* parser = *parserp) {
        return parser;
      }
      throw QueryParseError(
          fmt::format("unsupported expression term '{}'", name));
    }
  }
  throw QueryParseError(fmt::format("unknown expression term '{}'", name));
}

std::unique_ptr<QueryExpr> parseQueryExpr(Query* query, const json_ref& exp) {
  w_string name;

  if (exp.isString()) {
    name = json_to_w_string(exp);
  } else if (exp.isArray() && json_array_size(exp) > 0) {
    const auto& first = exp.at(0);

    if (!first.isString()) {
      throw QueryParseError("first element of an expression must be a string");
    }
    name = json_to_w_string(first);
  } else {
    throw QueryParseError("expected array or string for an expression");
  }

  return getQueryExprParser(name)(query, exp);
}

} // namespace watchman
