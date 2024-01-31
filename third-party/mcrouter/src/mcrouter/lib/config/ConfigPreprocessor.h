/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <folly/Function.h>
#include <folly/dynamic.h>
#include <folly/experimental/StringKeyedUnorderedMap.h>
#include <folly/json.h>

namespace facebook {
namespace memcache {

class ImportResolverIf;

/**
 * Removes comments from JSONC (JSON with Comments) and expands macros
 * inside this JSON.
 */
class ConfigPreprocessor {
 public:
  /**
   * Method to expand macros and get resulting dynamic object.
   *
   * @param jsonC JSON with comments and macros
   * @param importResolver resolves @import macros
   * @param globalParams parameters available in all macros. Should not have
   *                     macros.
   * @param configMetadataMap config metadata map containing linenumbers, etc
   * @param nestedLimit maximum number of nested macros/objects.
   *
   * @return JSON without macros
   * @throws std::logic_error/folly::ParseError if jsonC is invalid
   */
  static folly::dynamic getConfigWithoutMacros(
      folly::StringPiece jsonC,
      ImportResolverIf& importResolver,
      folly::F14NodeMap<std::string, folly::dynamic> globalParams,
      folly::json::metadata_map* configMetadataMap,
      size_t nestedLimit = 250);

 private:
  /**
   * Inner representation of macro object
   */
  class Macro;

  /**
   * Inner representation of const/global param
   */
  class Const;

  /**
   * Inner representation of arguments/locals.
   */
  class Context;

  /**
   * Built-in calls and macros
   */
  class BuiltIns;

  folly::F14NodeMap<std::string, std::unique_ptr<Macro>> macros_;
  folly::F14NodeMap<std::string, std::unique_ptr<Const>> consts_;
  folly::F14NodeMap<std::string, folly::dynamic> importCache_;
  folly::F14NodeMap<
      std::string,
      folly::Function<folly::dynamic(folly::dynamic&&, const Context&) const>>
      builtInCalls_;

  folly::json::metadata_map& configMetadataMap_;

  mutable size_t nestedLimit_;

  /**
   * Create preprocessor with given macros
   *
   * @param importResolver resolves @import macros
   * @param globals parameters available in all macros. Should not have
   *                macros.
   * @param configMetadataMap config metadata map containing linenumbers, etc
   * @param nestedLimit maximum number of nested macros/objects.
   */
  ConfigPreprocessor(
      ImportResolverIf& importResolver,
      folly::F14NodeMap<std::string, folly::dynamic> globals,
      folly::json::metadata_map& configMetadataMap,
      size_t nestedLimit);

  /**
   * Expands all macros found in json
   *
   * @param json object with macros
   * @param context current context (parameters that should be substituted)
   *
   * @return json object without macros
   */
  folly::dynamic expandMacros(folly::dynamic json, const Context& context)
      const;

  /**
   * Parses parameters passed to macro call inside string like in
   * @a(param1,%substituteMe%)
   *    ^...................^
   *             str
   */
  std::vector<folly::StringPiece> getCallParams(folly::StringPiece str) const;

  /**
   * Substitute params from context (substrings like %paramName%)
   * with their values
   */
  folly::dynamic replaceParams(folly::StringPiece str, const Context& context)
      const;

  /**
   * Expand macro inside string like in "@macroName(params)"
   *                                     ^................^
   *                                           macro
   */
  folly::dynamic expandStringMacro(
      folly::StringPiece str,
      const Context& params) const;

  void addMacro(
      folly::StringPiece name,
      const std::vector<folly::dynamic>& params,
      folly::Function<folly::dynamic(Context&&) const> func,
      bool autoExpand = true);

  void parseMacroDef(const folly::dynamic& key, const folly::dynamic& obj);

  void parseMacroDefs(folly::dynamic jmacros);

  void addConst(folly::StringPiece name, folly::dynamic result);
};
} // namespace memcache
} // namespace facebook
