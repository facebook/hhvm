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

#include "hphp/hack/src/decl/cpp_ffi/decl_ffi_types.h"
#include "hphp/hack/src/decl/cpp_ffi/decl_ffi.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/vm/hhvm_decl_provider.h"

namespace HPHP {

hackc::decl::decls const* HhvmDeclProvider::getDecl(AutoloadMap::KindOf kind, char const* symbol) {
  String sym = String(symbol, CopyStringMode::CopyString);
  Optional<String> filename_opt = AutoloadHandler::s_instance->getFile(symbol, kind);
  if (filename_opt.has_value()) {
    String filename = filename_opt.value();
    auto result = m_cache.find(filename.data());

    if (result != m_cache.end()) {
      return result->second.decl_list;
    }

    hackc::decl::bump_allocator const* arena = hackc_create_arena();
    hackc::decl::decl_parser_options const* opts = hackc_create_direct_decl_parse_options(
        true, true);  // TODO: get correct parameters

    std::ifstream s(filename.data());
    std::string text {
      std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>() };

    hackc::decl::decl_result decl_result = hackc_direct_decl_parse(opts, filename.data(), text.data(), arena);

    m_cache.insert({filename.data(), decl_result});
    return decl_result.decl_list;
  } else {
    return nullptr;
  }
}

HhvmDeclProvider::~HhvmDeclProvider() {
  // TODO:
}

hackc::decl::decls const*
  hhvm_decl_provider_get_decl(
         void* provider, char const* symbol) {
  return ((HhvmDeclProvider*)provider)->getDecl(HPHP::AutoloadMap::KindOf::Type/* TODO: pass correct symbol kind */, symbol);
}

}
