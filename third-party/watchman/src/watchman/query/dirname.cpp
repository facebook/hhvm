/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Errors.h"
#include "watchman/query/GlobEscaping.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"
#include "watchman/query/intcompare.h"

#include <memory>

using namespace watchman;

static inline bool is_dir_sep(int c) {
  return c == '/' || c == '\\';
}

class DirNameExpr : public QueryExpr {
  w_string dirname;
  struct w_query_int_compare depth;
  using StartsWith = bool (*)(w_string_piece str, w_string_piece prefix);
  StartsWith startswith;
  CaseSensitivity caseSensitive;

 public:
  explicit DirNameExpr(
      w_string dirname,
      struct w_query_int_compare depth,
      CaseSensitivity caseSensitive)
      : dirname(dirname), depth(depth), startswith(caseSensitive == CaseSensitivity::CaseInSensitive
            ? [](w_string_piece str,
                 w_string_piece
                     prefix) { return str.startsWithCaseInsensitive(prefix); }
            : [](w_string_piece str, w_string_piece prefix) {
                return str.startsWith(prefix);
              }), caseSensitive(caseSensitive) {}

  EvaluateResult evaluate(QueryContextBase* ctx, FileResult*) override {
    auto& str = ctx->getWholeName();

    if (str.size() <= dirname.size()) {
      // Either it doesn't prefix match, or file name is == dirname.
      // That means that the best case is that the wholename matches.
      // we only want to match if dirname(wholename) matches, so it
      // is not possible for us to match unless the length of wholename
      // is greater than the dirname operand
      return false;
    }

    // Want to make sure that wholename is a child of dirname, so
    // check for a dir separator.  Special case for dirname == '' (the root),
    // which won't have a slash in position 0.
    if (dirname.size() > 0 && !is_dir_sep(str.data()[dirname.size()])) {
      // may have a common prefix with, but is not a child of dirname
      return false;
    }

    if (!startswith(str, dirname)) {
      return false;
    }

    // Now compute the depth of file from dirname.  We do this by
    // counting dir separators, not including the one we saw above.
    json_int_t actual_depth = 0;
    for (size_t i = dirname.size() + 1; i < str.size(); i++) {
      if (is_dir_sep(str.data()[i])) {
        actual_depth++;
      }
    }

    return eval_int_compare(actual_depth, &depth);
  }

  // ["dirname", "foo"] -> ["dirname", "foo", ["depth", "ge", 0]]
  static std::unique_ptr<QueryExpr>
  parse(Query*, const json_ref& term, CaseSensitivity case_sensitive) {
    const char* which = case_sensitive == CaseSensitivity::CaseInSensitive
        ? "idirname"
        : "dirname";
    struct w_query_int_compare depth_comp;

    if (!term.isArray()) {
      QueryParseError::throwf("Expected array for '{}' term", which);
    }

    if (json_array_size(term) < 2) {
      QueryParseError::throwf(
          "Invalid number of arguments for '{}' term", which);
    }

    if (json_array_size(term) > 3) {
      QueryParseError::throwf(
          "Invalid number of arguments for '{}' term", which);
    }

    const auto& name = term.at(1);
    if (!name.isString()) {
      QueryParseError::throwf("Argument 2 to '{}' must be a string", which);
    }

    if (json_array_size(term) == 3) {
      const auto& depth = term.at(2);
      if (!depth.isArray()) {
        QueryParseError::throwf(
            "Invalid number of arguments for '{}' term", which);
      }

      const auto& depth_array = depth.array();

      parse_int_compare(depth, &depth_comp);

      if (strcmp("depth", json_string_value(depth_array.at(0)))) {
        QueryParseError::throwf(
            "Third parameter to '{}' should be a relational depth term", which);
      }
    } else {
      depth_comp.operand = 0;
      depth_comp.op = W_QUERY_ICMP_GE;
    }

    w_string dirname = json_to_w_string(name);
    auto view = dirname.view();
    auto last_not_slash = view.find_last_not_of('/');
    w_string trimmed_dirname;
    if (last_not_slash == std::string_view::npos) {
      trimmed_dirname = dirname;
    } else {
      trimmed_dirname = w_string{view.substr(0, last_not_slash + 1)};
    }

    return std::make_unique<DirNameExpr>(
        std::move(trimmed_dirname), depth_comp, case_sensitive);
  }
  static std::unique_ptr<QueryExpr> parseDirName(
      Query* query,
      const json_ref& term) {
    return parse(query, term, query->case_sensitive);
  }
  static std::unique_ptr<QueryExpr> parseIDirName(
      Query* query,
      const json_ref& term) {
    return parse(query, term, CaseSensitivity::CaseInSensitive);
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity outputCaseSensitive) const override {
    // We could leverage the depth parameter to generate a depth bound, e.g. `*`
    // for ["depth", "eq", "0"], but this risks taking precedence over a prefix
    // bound elsewhere in the query, so for simplicity we avoid depth bounds
    // right now.

    if (caseSensitive == CaseSensitivity::CaseInSensitive &&
        outputCaseSensitive != CaseSensitivity::CaseInSensitive) {
      // The caller asked for a case-sensitive upper bound, so treat idirname as
      // unbounded.
      return std::nullopt;
    }
    if (dirname.size() == 0) {
      // Treat ["dirname", ""] as unbounded.
      return std::nullopt;
    }
    // NOTE: This is the correct way to escape `dirname` because the only
    // separator it can contain is `/`. If there's a `\`, it's treated as a
    // literal character, and is therefore escaped.
    w_string outputPattern = convertLiteralPathToGlob(dirname);
    if (outputCaseSensitive == CaseSensitivity::CaseInSensitive) {
      outputPattern = outputPattern.piece().asLowerCase();
    }

    return std::vector<std::string>{outputPattern.string() + "/**"};
  }
};

W_TERM_PARSER(dirname, DirNameExpr::parseDirName);
W_TERM_PARSER(idirname, DirNameExpr::parseIDirName);

/* vim:ts=2:sw=2:et:
 */
