/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/thirdparty/jansson/jansson.h"

namespace watchman {

struct Query;
class QueryExpr;

typedef std::unique_ptr<QueryExpr> (
    *QueryExprParser)(Query* query, const json_ref& term);

QueryExprParser getQueryExprParser(const w_string& name);

#define WATCHMAN_EXPRESSION_PARSER_LIST(_) \
  _(allof)                                 \
  _(anyof)                                 \
  _(dirname)                               \
  _(empty)                                 \
  _(exists)                                \
  _(false)                                 \
  _(idirname)                              \
  _(imatch)                                \
  _(iname)                                 \
  _(ipcre)                                 \
  _(match)                                 \
  _(name)                                  \
  _(not )                                  \
  _(pcre)                                  \
  _(since)                                 \
  _(size)                                  \
  _(suffix)                                \
  _(true)                                  \
  _(type)

namespace parsers {
#define WATCHMAN_DECLARE_PARSER(name) \
  extern const QueryExprParser name##_parser;
WATCHMAN_EXPRESSION_PARSER_LIST(WATCHMAN_DECLARE_PARSER)
#undef WATCHMAN_DECLARE_PARSER
} // namespace parsers

#define W_TERM_PARSER(name, func) \
  const ::watchman::QueryExprParser watchman::parsers::name##_parser = (func)

#define W_TERM_PARSER_UNSUPPORTED(name) \
  const ::watchman::QueryExprParser watchman::parsers::name##_parser = nullptr

/**
 * Parse an expression term. It can be one of:
 * "term"
 * ["term" <parameters>]
 *
 * Throws QueryParseError if term is invalid.
 */
std::unique_ptr<QueryExpr> parseQueryExpr(Query* query, const json_ref& exp);

} // namespace watchman
