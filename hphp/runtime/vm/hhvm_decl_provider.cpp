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

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/vm/hhvm_decl_provider.h"

namespace HPHP {

Decls const* HhvmDeclProvider::getDecl(AutoloadMap::KindOf kind, char const* symbol) {
  String sym = String(symbol, CopyStringMode::CopyString);
  Optional<String> filename_opt = AutoloadHandler::s_instance->getFile(symbol, kind);
  if (filename_opt.has_value()) {
    String filename = filename_opt.value();
    auto result = m_cache.find(filename.data());

    if (result != m_cache.end()) {
      return &(*result->second.first.decls);
    }

    ::rust::Box<Bump> arena = hackc_create_arena();
    // TODO: get correct parameters
    ::rust::Box<DeclParserOptions> opts = hackc_create_direct_decl_parse_options(true, true);

    std::ifstream s(filename.data());
    std::string text {
      std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>() };

    DeclResult decl_result = hackc_direct_decl_parse(*opts, filename.toCppString(), text, *arena);

    m_cache.insert({filename.data(), std::pair(std::move(decl_result), std::move(arena))});
    return &*decl_result.decls;
  } else {
    return nullptr;
  }
}

HhvmDeclProvider::~HhvmDeclProvider() {
  for (auto &it: m_cache) {
    hackc_free_decl_result(std::move(it.second.first));
    hackc_free_arena(std::move(it.second.second));
  }
}

Decls const* hhvm_decl_provider_get_decl( void* provider, char const* symbol) {
  return ((HhvmDeclProvider*)provider)->getDecl(HPHP::AutoloadMap::KindOf::Type/* TODO: pass correct symbol kind */, symbol);
}

}
