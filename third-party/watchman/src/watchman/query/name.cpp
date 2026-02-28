/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Errors.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/GlobEscaping.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"

#include <unordered_set>

using namespace watchman;

class NameExpr : public QueryExpr {
  w_string name;
  std::unordered_set<w_string> set;
  CaseSensitivity caseSensitive;
  bool wholename;
  explicit NameExpr(
      std::unordered_set<w_string>&& set,
      CaseSensitivity caseSensitive,
      bool wholename)
      : set(std::move(set)),
        caseSensitive(caseSensitive),
        wholename(wholename) {}

 public:
  EvaluateResult evaluate(QueryContextBase* ctx, FileResult* file) override {
    if (!set.empty()) {
      bool matched;
      w_string str;

      if (wholename) {
        str = ctx->getWholeName();
        if (caseSensitive == CaseSensitivity::CaseInSensitive) {
          str = str.piece().asLowerCase();
        }
      } else {
        str = caseSensitive == CaseSensitivity::CaseInSensitive
            ? file->baseName().asLowerCase()
            : file->baseName().asWString();
      }

      matched = set.find(str) != set.end();

      return matched;
    }

    w_string_piece str;

    if (wholename) {
      str = ctx->getWholeName();
    } else {
      str = file->baseName();
    }

    if (caseSensitive == CaseSensitivity::CaseInSensitive) {
      return w_string_equal_caseless(str, name);
    }
    return str == name;
  }

  static std::unique_ptr<QueryExpr>
  parse(Query*, const json_ref& term, CaseSensitivity caseSensitive) {
    const char *pattern = nullptr, *scope = "basename";
    const char* which =
        caseSensitive == CaseSensitivity::CaseInSensitive ? "iname" : "name";
    std::unordered_set<w_string> set;

    if (!term.isArray()) {
      QueryParseError::throwf("Expected array for '{}' term", which);
    }

    if (json_array_size(term) > 3) {
      QueryParseError::throwf(
          "Invalid number of arguments for '{}' term", which);
    }

    if (json_array_size(term) == 3) {
      const auto& jscope = term.at(2);
      if (!jscope.isString()) {
        QueryParseError::throwf("Argument 3 to '{}' must be a string", which);
      }

      scope = json_string_value(jscope);

      if (strcmp(scope, "basename") && strcmp(scope, "wholename")) {
        QueryParseError::throwf(
            "Invalid scope '{}' for {} expression", scope, which);
      }
    }

    const auto& name = term.at(1);

    if (name.isArray()) {
      const auto& name_array = name.array();

      for (const auto& ele : name_array) {
        if (!ele.isString()) {
          QueryParseError::throwf(
              "Argument 2 to '{}' must be either a string or an array of string",
              which);
        }
      }

      set.reserve(name_array.size());
      for (const auto& jele : name_array) {
        w_string element;
        auto ele = json_to_w_string(jele);

        if (caseSensitive == CaseSensitivity::CaseInSensitive) {
          element = ele.piece().asLowerCase(ele.type()).normalizeSeparators();
        } else {
          element = ele.normalizeSeparators();
        }

        set.insert(element);
      }

    } else if (name.isString()) {
      pattern = json_string_value(name);
    } else {
      QueryParseError::throwf(
          "Argument 2 to '{}' must be either a string or an array of string",
          which);
    }

    auto data = new NameExpr(
        std::move(set), caseSensitive, !strcmp(scope, "wholename"));

    if (pattern) {
      data->name = json_to_w_string(name).normalizeSeparators();
    }

    return std::unique_ptr<QueryExpr>(data);
  }

  static std::unique_ptr<QueryExpr> parseName(
      Query* query,
      const json_ref& term) {
    return parse(query, term, query->case_sensitive);
  }
  static std::unique_ptr<QueryExpr> parseIName(
      Query* query,
      const json_ref& term) {
    return parse(query, term, CaseSensitivity::CaseInSensitive);
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity outputCaseSensitive) const override {
    if (caseSensitive == CaseSensitivity::CaseInSensitive &&
        outputCaseSensitive != CaseSensitivity::CaseInSensitive) {
      // The caller asked for a case-sensitive upper bound, so treat iname as
      // unbounded.
      return std::nullopt;
    }
    if (!wholename) {
      // basename matches don't bound the prefix, so they're not very useful.
      return std::nullopt;
    }
    std::unordered_set<std::string> globUpperBound;
    if (!set.empty()) {
      for (const auto& s : set) {
        w_string outputPattern = convertLiteralPathToGlob(s);
        if (outputCaseSensitive == CaseSensitivity::CaseInSensitive) {
          outputPattern = outputPattern.piece().asLowerCase();
        }
        globUpperBound.insert(outputPattern.string());
      }
    } else {
      w_string outputPattern = convertLiteralPathToGlob(name);
      if (outputCaseSensitive == CaseSensitivity::CaseInSensitive) {
        outputPattern = outputPattern.piece().asLowerCase();
      }
      globUpperBound.insert(outputPattern.string());
    }
    return std::vector<std::string>(
        globUpperBound.begin(), globUpperBound.end());
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

W_TERM_PARSER(name, NameExpr::parseName);
W_TERM_PARSER(iname, NameExpr::parseIName);

/* vim:ts=2:sw=2:et:
 */
