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

#include "hphp/hack/src/hhbc/ffi_bridge/rust_compile_ffi_bridge.rs"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"

#include <map>
#include <memory>
#include <utility>

namespace HPHP {

// c.f. `enum ExternalDeclProviderResult` in 'hhbc/external_decl_provider/lib.rs'.
struct DeclProviderResult {
    enum class Tag {
      Missing,
      Decls,
      Bytes,
    };
    struct DeclProviderDecls_Body {
      Decls const* _0;
    };
    struct DeclProviderBytes_Body {
      Bytes const* _0;
    };
    Tag tag;
    union {
      DeclProviderDecls_Body decl_provider_decls_result;
      DeclProviderBytes_Body decl_provider_bytes_result;
    };
};

struct HhvmDeclProvider {
  explicit HhvmDeclProvider(std::string const& aliased_namespaces)
    : opts{hackc_create_direct_decl_parse_options(true, true, true, aliased_namespaces)}
  {}
  HhvmDeclProvider(HhvmDeclProvider const&) = delete;
  HhvmDeclProvider& operator=(HhvmDeclProvider const&) = delete;

  DeclProviderResult getDecl(HPHP::AutoloadMap::KindOf kind, char const* symbol);

 private:
  ::rust::Box<DeclParserOptions> opts;
  std::map<std::string, DeclResult> m_cache;
};

extern "C" {
  DeclProviderResult hhvm_decl_provider_get_decl(void* provider, int kind, char const* symbol);
}
}
