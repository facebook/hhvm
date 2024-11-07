/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>
#include <memory>
#include "watchman/Errors.h"
#include "watchman/fs/FileSystem.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"
#include "watchman/watchman_system.h"

#ifdef HAVE_PCRE_H

#include <pcre2.h> // @manual=fbsource//third-party/pcre2:pcre2-8

using namespace watchman;

class PcreExpr : public QueryExpr {
  pcre2_code* re;
  pcre2_match_data* matchData;
  bool wholename;

 public:
  explicit PcreExpr(pcre2_code* re, pcre2_match_data* matchData, bool wholename)
      : re(re), matchData(matchData), wholename(wholename) {}

  ~PcreExpr() override {
    if (re) {
      pcre2_code_free(re);
    }
    if (matchData) {
      pcre2_match_data_free(matchData);
    }
  }

  EvaluateResult evaluate(QueryContextBase* ctx, FileResult* file) override {
    w_string_piece str;
    int rc;

    if (wholename) {
      str = ctx->getWholeName();
    } else {
      str = file->baseName();
    }

    logf(ERR, "NAME: {}\n", str);

    rc = pcre2_match(
        re,
        reinterpret_cast<const unsigned char*>(str.data()),
        str.size(),
        0,
        0,
        matchData,
        nullptr);
    logf(ERR, "RC: {}\n", rc);
    // Errors are either PCRE2_ERROR_NOMATCH or non actionable. Thus only match
    // when we get a positive return value.
    return rc >= 0;
  }

  static std::unique_ptr<QueryExpr>
  parse(Query*, const json_ref& term, CaseSensitivity caseSensitive) {
    const char *pattern, *scope = "basename";
    const char* which =
        caseSensitive == CaseSensitivity::CaseInSensitive ? "ipcre" : "pcre";
    size_t erroff = 0;
    int errcode = 0;

    if (term.array().size() > 1 && term.at(1).isString()) {
      pattern = json_string_value(term.at(1));
    } else {
      throw QueryParseError(fmt::format(
          "First parameter to \"{}\" term must be a pattern string", which));
    }

    if (term.array().size() > 2) {
      if (term.at(2).isString()) {
        scope = json_string_value(term.at(2));
      } else {
        throw QueryParseError(fmt::format(
            "Second parameter to \"{}\" term must be an optional scope string",
            which));
      }
    }

    if (strcmp(scope, "basename") && strcmp(scope, "wholename")) {
      throw QueryParseError(
          fmt::format("Invalid scope '{}' for {} expression", scope, which));
    }

    logf(ERR, "PATTERN: {}\n", pattern);

    auto re = pcre2_compile(
        reinterpret_cast<const unsigned char*>(pattern),
        PCRE2_ZERO_TERMINATED,
        caseSensitive == CaseSensitivity::CaseInSensitive ? PCRE2_CASELESS : 0,
        &errcode,
        &erroff,
        nullptr);
    if (!re) {
      // From PCRE2 documentation:
      // https://www.pcre.org/current/doc/html/pcre2api.html#SEC32: "None of the
      // messages are very long; a buffer size of 120 code units is ample"
      PCRE2_UCHAR buffer[120];
      static_assert(
          sizeof(char) == sizeof(PCRE2_UCHAR),
          "Watchman uses the 8-bit PCRE2 library");
      pcre2_get_error_message(errcode, buffer, 120);
      throw QueryParseError(fmt::format(
          "invalid {}: code {} {} at offset {} in {}",
          which,
          errcode,
          reinterpret_cast<const char*>(&buffer),
          erroff,
          pattern));
    }

    auto matchData = pcre2_match_data_create_from_pattern(re, nullptr);
    if (!matchData) {
      throw std::bad_alloc();
    }

    return std::make_unique<PcreExpr>(
        re, matchData, !strcmp(scope, "wholename"));
  }
  static std::unique_ptr<QueryExpr> parsePcre(
      Query* query,
      const json_ref& term) {
    return parse(query, term, query->case_sensitive);
  }
  static std::unique_ptr<QueryExpr> parseIPcre(
      Query* query,
      const json_ref& term) {
    return parse(query, term, CaseSensitivity::CaseInSensitive);
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // We could, in principle, try to reverse-engineer the expression into a
    // prefix glob (if it's wholename-scoped and has the same case-sensitivity
    // as the one passed here).
    //
    // In practice, it's better to treat all `pcre` terms as unbounded, and ask
    // clients who care about bounding their queries to add a `match` or
    // `dirname` term explicitly.

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
W_TERM_PARSER(pcre, PcreExpr::parsePcre);
W_TERM_PARSER(ipcre, PcreExpr::parseIPcre);

#else

W_TERM_PARSER_UNSUPPORTED(pcre);
W_TERM_PARSER_UNSUPPORTED(ipcre);

#endif

/* vim:ts=2:sw=2:et:
 */
