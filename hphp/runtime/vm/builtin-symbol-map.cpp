/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/hack/src/hackc/ffi_bridge/decl_provider.h"
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"

#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/builtin-symbol-map.h"

#include "hphp/util/optional.h"
#include <memory>

namespace HPHP::Native {

namespace {

/**
 * Represents a case sensitive mapping from a symbol name to a blob of decls
 * where *a* decl in said blob has its name as the key.
 */
using SymbolMap = hphp_fast_map<const StringData*,
                                std::shared_ptr<hackc::DeclResult>>;
/** Represents a case insensitive version of [SymbolMap] */
using ISymbolMap = hphp_fast_map<const StringData*,
                                 std::shared_ptr<hackc::DeclResult>,
                                 string_data_hash,
                                 string_data_isame>;

ISymbolMap s_types;
ISymbolMap s_functions;
SymbolMap s_constants;
SymbolMap s_modules;

}

void registerBuiltinSymbols(
  const std::string& name,
  const std::string& contents
) {
  // We should *never* call this function unless decl driven bytecode is enabled
  assertx(RuntimeOption::EvalEnableDecl);

  // Systemlib and extensions are compiled before we've even loaded HHVM
  // options, so our recourse here is to use the defaults.
  auto const& defaults = RepoOptions::defaults();

  hackc::DeclParserConfig options;
  defaults.flags().initDeclConfig(options);

  auto decls = hackc::direct_decl_parse(options, name, contents);
  auto const facts = hackc::decls_to_facts_cpp_ffi(
    decls,
    "" // This is *meant* to be the hash of the source file, but it's not used
  );
  auto const decls_ptr = std::make_shared<hackc::DeclResult>(std::move(decls));
  for (auto const& e : facts.facts.types) {
    s_types.emplace(
      makeStaticString(std::string(e.name)),
      decls_ptr
    );
  }
  for (auto const& e : facts.facts.functions) {
    s_functions.emplace(
      makeStaticString(std::string(e)),
      decls_ptr
    );
  }
  for (auto const& e : facts.facts.constants) {
    s_constants.emplace(
      makeStaticString(std::string(e)),
      decls_ptr
    );
  }
  for (auto const& e : facts.facts.modules) {
    s_modules.emplace(
      makeStaticString(std::string(e.name)),
      decls_ptr
    );
  }
}

Optional<hackc::ExternalDeclProviderResult> getBuiltinDecls(
  const StringData* symbol,
  AutoloadMap::KindOf kind
) {

  auto const maybeGetDecls = [symbol](auto const& map) {
    auto const res = map.find(symbol);
    return res != map.end() ?
      HPHP::make_optional(hackc::ExternalDeclProviderResult::from_decls(*res->second))
    : std::nullopt;
  };
  // We should only ever call this if `EnableDecl` is set, and asserting on
  // `symbol` here is a smoke check
  assertx(RuntimeOption::EvalEnableDecl && symbol);
  switch (kind) {
    case AutoloadMap::KindOf::Type:
    case AutoloadMap::KindOf::TypeAlias:
    case AutoloadMap::KindOf::TypeOrTypeAlias:
      return maybeGetDecls(s_types);
    case AutoloadMap::KindOf::Function:
      return maybeGetDecls(s_functions);
    case AutoloadMap::KindOf::Constant:
      return maybeGetDecls(s_constants);
    case AutoloadMap::KindOf::Module:
      return maybeGetDecls(s_modules);
  }
  not_reached();
}

hphp_fast_set<const hackc::DeclResult*> getAllBuiltinDecls() {
  assertx(RuntimeOption::EvalEnableDecl);
  hphp_fast_set<const hackc::DeclResult*> res;
  for (const auto& it: s_types) {
    res.emplace(it.second.get());
  }
  for (const auto& it: s_functions) {
    res.emplace(it.second.get());
  }
  for (const auto& it: s_constants) {
    res.emplace(it.second.get());
  }
  for (const auto& it: s_modules) {
    res.emplace(it.second.get());
  }
  return res;
}

} // namespace HPHP::Native
