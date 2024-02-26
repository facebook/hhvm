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
                                std::shared_ptr<hackc::DeclsAndBlob>>;
/** Represents a case insensitive version of [SymbolMap] */
using TSymbolMap = hphp_fast_map<const StringData*,
                                 std::shared_ptr<hackc::DeclsAndBlob>,
                                 string_data_hash,
                                 string_data_tsame>;
using FSymbolMap = hphp_fast_map<const StringData*,
                                 std::shared_ptr<hackc::DeclsAndBlob>,
                                 string_data_hash,
                                 string_data_fsame>;

TSymbolMap s_types;
FSymbolMap s_functions;
SymbolMap s_constants;
SymbolMap s_modules;

}

void registerBuiltinSymbols(const std::string& serialized_decls) {
  // We should *never* call this function unless decl driven bytecode is enabled
  assertx(RuntimeOption::EvalEnableDecl);

  auto decls = hackc::binary_to_decls_and_blob(serialized_decls);

  auto const symbols = hackc::decls_to_symbols(*decls.decls);
  auto const decls_ptr = std::make_shared<hackc::DeclsAndBlob>(std::move(decls));
  for (auto const& e : symbols.types) {
    s_types.emplace(makeStaticString(std::string(e)), decls_ptr);
  }
  for (auto const& e : symbols.functions) {
    s_functions.emplace(makeStaticString(std::string(e)), decls_ptr);
  }
  for (auto const& e : symbols.constants) {
    s_constants.emplace(makeStaticString(std::string(e)), decls_ptr);
  }
  for (auto const& e : symbols.modules) {
    s_modules.emplace(makeStaticString(std::string(e)), decls_ptr);
  }
}

Optional<hackc::ExternalDeclProviderResult> getBuiltinDecls(
  const StringData* symbol,
  AutoloadMap::KindOf kind
) {

  auto const maybeGetDecls = [symbol](auto const& map) {
    auto const res = map.find(symbol);
    return res != map.end() ?
      HPHP::make_optional(hackc::ExternalDeclProviderResult::from_decls(*res->second->decls))
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

hphp_fast_set<const hackc::DeclsAndBlob*> getAllBuiltinDecls() {
  assertx(RuntimeOption::EvalEnableDecl);
  hphp_fast_set<const hackc::DeclsAndBlob*> res;
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
