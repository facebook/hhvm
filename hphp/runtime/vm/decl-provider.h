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

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/util/hash-map.h"

#include <map>
#include <memory>
#include <string_view>
#include <utility>

namespace HPHP {

struct RepoOptionsFlags;

// This must be kept in sync with `enum ExternalDeclProviderResult` in
// 'hhbc/decl_provider/external.rs' so they both are layout compatible.
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
  HhvmDeclProvider(int32_t flags, std::string const& aliased_namespaces,
                   AutoloadMap*);
  HhvmDeclProvider(HhvmDeclProvider const&) = delete;
  HhvmDeclProvider& operator=(HhvmDeclProvider const&) = delete;

  // Factory to create the provider. Constructor is only public
  // for use by std::unique_ptr within create().
  static std::unique_ptr<HhvmDeclProvider> create(const RepoOptionsFlags&);

  // Callback invoked by hackc's ExternalDeclProvider.
  DeclProviderResult getDecl(HPHP::AutoloadMap::KindOf kind, std::string_view symbol);

 private:
  rust::Box<DeclParserOptions> m_opts;

  // Map from filename to DeclResult containing the cached results of calling
  // hackc_direct_decl_parse().
  hphp_hash_map<std::string, DeclResult> m_cache;

  AutoloadMap* m_map;
};

extern "C" {
  DeclProviderResult hhvm_decl_provider_get_decl(
      void* provider, int kind, char const* symbol, size_t len
  );
}
}
