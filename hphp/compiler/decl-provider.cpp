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
#include "hphp/compiler/decl-provider.h"

namespace HPHP {

BatchDeclProvider::BatchDeclProvider(
    const std::vector<Package::UnitDecls>& decls
) {
  for (auto const& unit_decls : decls) {
    assertx(!unit_decls.symbols.empty());
    auto const& symbols = unit_decls.symbols;
    auto const& data = unit_decls.decls;
    for (auto name : symbols.types) m_types.emplace(name, data);
    for (auto name : symbols.funcs) m_funcs.emplace(name, data);
    for (auto name : symbols.constants) m_constants.emplace(name, data);
    for (auto name : symbols.modules) m_modules.emplace(name, data);
  }
}

namespace {

template<typename T, typename V> hackc::ExternalDeclProviderResult find(
  std::string_view symbol, const T& map, V& list
) {
  // TODO(T110866581): symbol should be normalized by hackc
  std::string_view normalized(normalizeNS(symbol));
  auto interned = makeStaticString(normalized);
  auto const it = map.find(interned);
  if (it != map.end()) {
    return hackc::ExternalDeclProviderResult::from_string(it->second);
  }
  list.emplace_back(interned);
  return hackc::ExternalDeclProviderResult::missing();
}

}

hackc::ExternalDeclProviderResult
BatchDeclProvider::getType(std::string_view symbol, uint64_t) noexcept {
  return find(symbol, m_types, m_missing.types);
}

hackc::ExternalDeclProviderResult
BatchDeclProvider::getFunc(std::string_view symbol) noexcept {
  return find(symbol, m_funcs, m_missing.funcs);
}

hackc::ExternalDeclProviderResult
BatchDeclProvider::getConst(std::string_view symbol) noexcept {
  return find(symbol, m_constants, m_missing.constants);
}

hackc::ExternalDeclProviderResult
BatchDeclProvider::getModule(std::string_view symbol) noexcept {
  return find(symbol, m_modules, m_missing.modules);
}

void BatchDeclProvider::finish() {
  // Dedup but preserve case so we can see all case mismatches.
  auto dedup = [&](auto& names) {
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
  };
  dedup(m_missing.types);
  dedup(m_missing.funcs);
  dedup(m_missing.constants);
  dedup(m_missing.modules);
}

}
