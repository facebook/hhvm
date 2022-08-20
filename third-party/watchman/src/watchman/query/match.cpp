/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <string>
#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"
#include "watchman/thirdparty/wildmatch/wildmatch.h"

namespace watchman {

class WildMatchExpr : public QueryExpr {
  std::string pattern;
  CaseSensitivity caseSensitive;
  bool wholename;
  bool noescape;
  bool includedotfiles;

 public:
  WildMatchExpr(
      const char* pat,
      CaseSensitivity caseSensitive,
      bool wholename,
      bool noescape,
      bool includedotfiles)
      : pattern(pat),
        caseSensitive(caseSensitive),
        wholename(wholename),
        noescape(noescape),
        includedotfiles(includedotfiles) {}

  EvaluateResult evaluate(QueryContextBase* ctx, FileResult* file) override {
    w_string_piece str;
    bool res;

    if (wholename) {
      str = ctx->getWholeName();
    } else {
      str = file->baseName();
    }

#ifdef _WIN32
    // Translate to unix style slashes for wildmatch
    w_string normBuf = str.asWString().normalizeSeparators();
    str = normBuf;
#endif

    res =
        wildmatch(
            pattern.c_str(),
            str.data(),
            (includedotfiles ? 0 : WM_PERIOD) | (noescape ? WM_NOESCAPE : 0) |
                (wholename ? WM_PATHNAME : 0) |
                (caseSensitive == CaseSensitivity::CaseInSensitive ? WM_CASEFOLD
                                                                   : 0),
            0) == WM_MATCH;

    return res;
  }

  static std::unique_ptr<QueryExpr>
  parse(Query*, const json_ref& term, CaseSensitivity case_sensitive) {
    const char *pattern, *scope = "basename";
    const char* which =
        case_sensitive == CaseSensitivity::CaseInSensitive ? "imatch" : "match";
    int noescape = 0;
    int includedotfiles = 0;

    if (term.array().size() > 1 && term.at(1).isString()) {
      pattern = json_string_value(term.at(1));
    } else {
      QueryParseError::throwf(
          "First parameter to \"{}\" term must be a pattern string", which);
    }

    if (term.array().size() > 2) {
      if (term.at(2).isString()) {
        scope = json_string_value(term.at(2));
      } else {
        QueryParseError::throwf(
            "Second parameter to \"{}\" term must be an optional scope string",
            which);
      }
    }

    if (term.array().size() > 3) {
      auto& opts = term.at(3);
      if (!opts.isObject()) {
        QueryParseError::throwf(
            "Third parameter to \"{}\" term must be an optional object", which);
      }

      auto ele = opts.get_default("noescape", json_false());
      if (!ele.isBool()) {
        QueryParseError::throwf(
            "noescape option for \"{}\" term must be a boolean", which);
      }
      noescape = ele.asBool();

      ele = opts.get_default("includedotfiles", json_false());
      if (!ele.isBool()) {
        QueryParseError::throwf(
            "includedotfiles option for \"{}\" term must be a boolean", which);
      }
      includedotfiles = ele.asBool();
    }

    if (term.array().size() > 4) {
      QueryParseError::throwf(
          "too many parameters passed to \"{}\" expression", which);
    }

    if (strcmp(scope, "basename") && strcmp(scope, "wholename")) {
      QueryParseError::throwf(
          "Invalid scope '{}' for {} expression", scope, which);
    }

    return std::make_unique<WildMatchExpr>(
        pattern,
        case_sensitive,
        !strcmp(scope, "wholename"),
        noescape,
        includedotfiles);
  }
  static std::unique_ptr<QueryExpr> parseMatch(
      Query* query,
      const json_ref& term) {
    return parse(query, term, query->case_sensitive);
  }
  static std::unique_ptr<QueryExpr> parseIMatch(
      Query* query,
      const json_ref& term) {
    return parse(query, term, CaseSensitivity::CaseInSensitive);
  }
};
W_TERM_PARSER(match, WildMatchExpr::parseMatch);
W_TERM_PARSER(imatch, WildMatchExpr::parseIMatch);
W_CAP_REG("wildmatch")
W_CAP_REG("wildmatch-multislash")

} // namespace watchman

/* vim:ts=2:sw=2:et:
 */
