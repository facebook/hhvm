/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fmt/core.h>
#include "watchman/Errors.h"
#include "watchman/fs/FileInformation.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/QueryExpr.h"
#include "watchman/query/TermRegistry.h"

#include <memory>

using namespace watchman;

class TypeExpr : public QueryExpr {
  char arg;

 public:
  explicit TypeExpr(char arg) : arg(arg) {}

  EvaluateResult evaluate(QueryContextBase*, FileResult* file) override {
    auto optionalDtype = file->dtype();
    if (!optionalDtype.has_value()) {
      return std::nullopt;
    }
    auto dtype = *optionalDtype;
    if (dtype != DType::Unknown) {
      switch (arg) {
        case 'b':
          return dtype == DType::Block;
        case 'c':
          return dtype == DType::Char;
        case 'p':
          return dtype == DType::Fifo;
        case 's':
          return dtype == DType::Socket;
        case 'd':
          return dtype == DType::Dir;
        case 'f':
          return dtype == DType::Regular;
        case 'l':
          return dtype == DType::Symlink;
      }
    }

    auto stat = file->stat();
    if (!stat.has_value()) {
      return std::nullopt;
    }

    switch (arg) {
#ifndef _WIN32
      case 'b':
        return S_ISBLK(stat->mode);
      case 'c':
        return S_ISCHR(stat->mode);
      case 'p':
        return S_ISFIFO(stat->mode);
      case 's':
        return S_ISSOCK(stat->mode);
#endif
      case 'd':
        return stat->isDir();
      case 'f':
        return stat->isFile();
      case 'l':
        return stat->isSymlink();
#ifdef S_ISDOOR
      case 'D':
        return S_ISDOOR(stat->mode);
#endif
      default:
        return false;
    }
  }

  static std::unique_ptr<QueryExpr> parse(Query*, const json_ref& term) {
    const char *typestr, *found;
    char arg;

    if (!term.isArray()) {
      throw QueryParseError{"\"type\" term requires a type string parameter"};
    }

    if (term.array().size() > 1 && term.at(1).isString()) {
      typestr = json_string_value(term.at(1));
    } else {
      throw QueryParseError(
          "First parameter to \"type\" term must be a type string");
    }

    found = strpbrk(typestr, "bcdfplsD");
    if (!found || strlen(typestr) > 1) {
      QueryParseError::throwf("invalid type string '{}'", typestr);
    }

    arg = *found;

    return std::make_unique<TypeExpr>(arg);
  }

  std::optional<std::vector<std::string>> computeGlobUpperBound(
      CaseSensitivity) const override {
    // `type` doesn't constrain the path.
    return std::nullopt;
  }

  /**
   * Determines if this expression will return only files.
   * An expression returns files if is not type 'd', for directories.
   */
  ReturnOnlyFiles listOnlyFiles() const override {
    if (arg == 'd') {
      return ReturnOnlyFiles::No;
    }
    return ReturnOnlyFiles::Yes;
  }

  SimpleSuffixType evaluateSimpleSuffix() const override {
    if (arg == 'f') {
      return SimpleSuffixType::Type;
    }
    return SimpleSuffixType::Excluded;
  }

  std::vector<std::string> getSuffixQueryGlobPatterns() const override {
    return std::vector<std::string>{};
  }
};
W_TERM_PARSER(type, TypeExpr::parse);

/* vim:ts=2:sw=2:et:
 */
