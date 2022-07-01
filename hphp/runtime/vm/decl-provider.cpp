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
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/decl-provider.h"
#include "hphp/util/sha1.h"

#include <boost/algorithm/string/predicate.hpp>

namespace HPHP {

TRACE_SET_MOD(unit_parse);

std::unique_ptr<HhvmDeclProvider>
HhvmDeclProvider::create(AutoloadMap* map, const RepoOptionsFlags& options) {
  if (!RuntimeOption::EvalEnableDecl) {
    return {nullptr};
  }
  if (!map || !map->isNative()) {
    // Either compiling systemlib.php with no autoload map initialized yet,
    // or AutoloadHandler was not configured with a native AutoloadMap.
    return {nullptr};
  }

  // Create the DeclProvider.
  // This will only cache entries it sees during this invocation of
  // the compiler. Hackc should have a per-session cache indexed by symbol,
  // and this file-to-decls cache should span sessions.
  auto decl_flags = options.getDeclFlags();
  auto aliased_namespaces = options.getAliasedNamespacesConfig();
  return std::make_unique<HhvmDeclProvider>(
      decl_flags, aliased_namespaces, map, options.repoRoot()
  );
}

HhvmDeclProvider::HhvmDeclProvider(
    int32_t flags,
    std::string const& aliased_namespaces,
    AutoloadMap* map,
    folly::fs::path const& repo
)
  : m_opts{hackc_create_direct_decl_parse_options(flags, aliased_namespaces)}
  , m_map{map}
  , m_repo{repo}
{}

// Called by hackc, potentially on a different thread.
DeclProviderResult HhvmDeclProvider::getDecl(
  AutoloadMap::KindOf kind,
  std::string_view symbol,
  uint64_t depth
) {
  // TODO(T110866581): symbol should be normalized by hackc
  std::string_view sym(normalizeNS(symbol));
  ITRACE(3, "DP lookup {}\n", sym);

  // Lookup the file where sym is defined in the autoload map.
  auto filename_opt = m_map->getFile(kind, sym);
  if (filename_opt) {
    auto filename = filename_opt->native();
    auto result = m_cache.find(filename);

    if (result != m_cache.end()) {
      ITRACE(3, "DP found cached decls for {} in {}\n", sym, filename);
      return DeclProviderResult::from_decls(result->second);
    }

    // Nothing cached: Load file, parse decls.
    std::ifstream s(filename);
    std::string text {
      std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>()
    };

    DeclResult decl_result = hackc_direct_decl_parse(*m_opts, filename, text);
    ITRACE(3, "DP parsed {} in {}\n", sym, filename);

    auto norm_filename = filename;
    if (folly::fs::starts_with(filename, m_repo)) {
      norm_filename.erase(0, m_repo.size());
    }

    auto const hash = SHA1{string_sha1(text)};
    m_deps.emplace(DeclSym{kind, symbol}, DepInfo{norm_filename, depth, hash});

    // Insert decl_result into the cache, return DeclResult::decls,
    // a pointer to rust decls in m_cache.
    auto [it, _] = m_cache.insert({filename, std::move(decl_result)});
    return DeclProviderResult::from_decls(it->second);
  }
  ITRACE(4, "DP {}: getFile() returned None\n", sym);
  m_sawMissing = true;
  return DeclProviderResult::missing();
}

std::vector<DeclDep> HhvmDeclProvider::getFlatDeps() const {
  hphp_hash_map<std::string, SHA1> deps;
  for (auto& [k, v] : m_deps) deps.emplace(v.file, v.hash);

  std::vector<DeclDep> ret;
  for (auto [k, v] : deps) ret.emplace_back(DeclDep{k, v});
  return ret;
}

std::vector<std::vector<DeclLoc>> HhvmDeclProvider::getDeps() const {
  uint64_t max_depth = 0;

  hphp_hash_map<std::string, std::pair<DeclSym, DepInfo>> deps;
  for (auto& [k, v] : m_deps) {
    auto [it, ins] = deps.emplace(v.file, std::make_pair(k, v));
    // smaller depth replaces entry with bigger depth, for equal depths,
    // first one wins.
    if (!ins && v.depth < it->second.second.depth) {
      it->second = std::make_pair(k, v);
    }
    max_depth = std::max(v.depth, max_depth);
  }

  std::vector<std::vector<DeclLoc>> ret(max_depth + 1);
  for (auto [k, v] : deps) {
    ret[v.second.depth].emplace_back(
      std::make_pair(v.first, DeclDep{k, v.second.hash})
    );
  }

  // Now that we've collapsed files in multiple dep lists to the list with the
  // lowest level there may be empty lists. Our clients care about an ordering
  // of dependencies not the specific levels themselves, so as a convenience
  // guarantee that each dependency list is non-empty.
  ret.erase(
    std::remove_if(ret.begin(), ret.end(), [] (auto& v) { return v.empty(); }),
    ret.end()
  );

  return ret;
}

extern "C" {

DeclProviderResult hhvm_decl_provider_get_decl(
    void* provider, int symbol_kind, char const* symbol, size_t len,
    uint64_t depth
) {
  try {
    // Unsafe: if `symbol_kind` is out of range the result of this cast is UB.
    HPHP::AutoloadMap::KindOf kind {
      static_cast<HPHP::AutoloadMap::KindOf>(symbol_kind)
    };
    return ((HhvmDeclProvider*)provider)->getDecl(
        kind, std::string_view(symbol, len), depth
    );
  } catch(...) {
    not_reached();
  }
}

} //extern "C"
}//namespace HPHP
