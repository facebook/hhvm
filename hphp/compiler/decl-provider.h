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
#pragma once
#include "hphp/hack/src/hackc/ffi_bridge/decl_provider.h"
#include "hphp/compiler/package.h"

namespace HPHP {

// A BatchDeclProvider is populated with a list of UnitDecls to make
// available to hackc. Each UnitDecl contains a list of symbols from
// that source unit and the serialized decls. These names are used
// to constructed a unified local autoload map of all the available decls.
//
// When hackc requests a symbol we don't have in the local map,
// remember it in m_missing so hphpc can look it up in its UnitIndex,
// then retry hackc with additional UnitDecls.
struct BatchDeclProvider final: hackc::DeclProvider {

  // Initialize provider from a list of UnitDecls. The given decls must
  // be live and unchanged for the lifetime of this BatchDeclProvider.
  explicit BatchDeclProvider(const std::vector<Package::UnitDecls>&);

  hackc::ExternalDeclProviderResult
  getType(std::string_view symbol, uint64_t) noexcept override;

  hackc::ExternalDeclProviderResult
  getFunc(std::string_view symbol) noexcept override;

  hackc::ExternalDeclProviderResult
  getConst(std::string_view symbol) noexcept override;

  hackc::ExternalDeclProviderResult
  getModule(std::string_view symbol) noexcept override;

  void finish();

  // Maps from Name to serialized inside the UnitDecls given in the
  // constructor.
  using Map = hphp_fast_map<const StringData*, const std::string&>;
  using IMap = hphp_fast_map<
    const StringData*, const std::string&, string_data_hash, string_data_isame
  >;

  // Symbols requested but not found
  Package::DeclNames m_missing;

  IMap m_types;
  IMap m_funcs;
  Map m_constants;
  Map m_modules;
};

}
