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
#include "hphp/hack/src/hackc/decl_provider/decl_provider.h"
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/decl-dep.h"
#include "hphp/util/hash-map.h"

#include <folly/experimental/io/FsUtil.h>
#include <map>
#include <memory>
#include <string_view>
#include <utility>

namespace HPHP {

struct RepoOptionsFlags;

struct HhvmDeclProvider {
  HhvmDeclProvider(int32_t flags, std::string const& aliased_namespaces,
                   AutoloadMap*, folly::fs::path const&);
  HhvmDeclProvider(HhvmDeclProvider const&) = delete;
  HhvmDeclProvider& operator=(HhvmDeclProvider const&) = delete;

  // Factory to create the provider. Constructor is only public
  // for use by std::unique_ptr within create().
  static std::unique_ptr<HhvmDeclProvider> create(
    AutoloadMap*,
    const RepoOptionsFlags&
  );

  // Callback invoked by hackc's ExternalDeclProvider.
  DeclProviderResult getDecl(HPHP::AutoloadMap::KindOf kind,
                             std::string_view symbol,
                             uint64_t depth);

  // Get a list of observed dependencies from the decl provider, which may
  // optionally be indexed by the depth of the dependency
  std::vector<DeclDep> getFlatDeps() const;
  std::vector<std::vector<DeclLoc>> getDeps() const;

  const folly::fs::path& repoRoot() const { return m_repo; }
  AutoloadMap* map() const { return m_map; }

  // Was there a decl we were unable to resolve?
  bool sawMissing() const { return m_sawMissing; }

 private:
  struct DepInfo {
    std::string file;
    uint64_t depth;
    SHA1 hash;
  };

  bool m_sawMissing{false};

  rust::Box<DeclParserOptions> m_opts;

  // Map from filename to DeclResult containing the cached results of calling
  // hackc_direct_decl_parse().
  hphp_hash_map<std::string, DeclResult> m_cache;

  // Record of dependencies collected from queries to the decl provider
  hphp_hash_map<DeclSym, DepInfo> m_deps;

  AutoloadMap* m_map;
  folly::fs::path m_repo;
};

extern "C" {
  DeclProviderResult hhvm_decl_provider_get_decl(
      void* provider, int kind, char const* symbol, size_t len, uint64_t depth
  );
}
}
