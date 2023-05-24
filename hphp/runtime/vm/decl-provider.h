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
#include "hphp/runtime/base/autoload-map.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/decl-dep.h"
#include "hphp/util/hash-map.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>
#include <utility>

namespace HPHP {

struct RepoOptionsFlags;

struct HhvmDeclProvider: hackc::DeclProvider {
  HhvmDeclProvider(hackc::DeclParserConfig, AutoloadMap*,
                   const std::filesystem::path&);
  virtual ~HhvmDeclProvider() override = default;
  HhvmDeclProvider(HhvmDeclProvider const&) = delete;
  HhvmDeclProvider& operator=(HhvmDeclProvider const&) = delete;

  // Factory to create the provider. Constructor is only public
  // for use by std::unique_ptr within create().
  static std::unique_ptr<HhvmDeclProvider> create(
    AutoloadMap*,
    const RepoOptionsFlags&,
    const std::filesystem::path&
  );

  // Callback invoked by hackc's ExternalDeclProvider.
  hackc::ExternalDeclProviderResult
  getType(std::string_view symbol, uint64_t depth) noexcept override;
  hackc::ExternalDeclProviderResult
  getFunc(std::string_view symbol) noexcept override;
  hackc::ExternalDeclProviderResult
  getConst(std::string_view symbol) noexcept override;
  hackc::ExternalDeclProviderResult
  getModule(std::string_view symbol) noexcept override;

  // Get a list of observed dependencies from the decl provider, which may
  // optionally be indexed by the depth of the dependency
  std::vector<DeclDep> getFlatDeps() const;
  std::vector<std::vector<DeclLoc>> getLocsByDepth() const;

  const std::filesystem::path& repoRoot() const { return m_repo; }
  AutoloadMap* map() const { return m_map; }

  // Was there a decl we were unable to resolve?
  bool sawMissing() const { return m_sawMissing; }

 private:
  hackc::ExternalDeclProviderResult getDecls(
      std::string_view symbol,
      uint64_t depth,
      AutoloadMap::KindOf
  ) noexcept;

  struct DepInfo {
    // Source filename
    std::string file;

    // Minimum number of indirect references traversed to this file.
    uint64_t depth;

    // Source text hash of this file.
    SHA1 hash;
  };

  bool m_sawMissing{false};

  hackc::DeclParserConfig m_config;

  // Map from filename to DeclsAndBlob containing the cached results of calling
  // hackc_direct_decl_parse_and_serialize().
  hphp_hash_map<std::string, hackc::DeclsAndBlob> m_cache;

  // Record of dependencies collected from queries to the decl provider
  hphp_hash_map<DeclSym, DepInfo> m_deps;

  AutoloadMap* m_map;
  std::filesystem::path m_repo;
};
}
