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
#include "hphp/runtime/vm/decl-provider.h"

namespace HPHP {

TRACE_SET_MOD(unit_parse);

namespace {

// Helper to return DeclProviderResult::Missing
DeclProviderResult missing() {
  return DeclProviderResult{DeclProviderResult::Tag::Missing, {}};
}

// Helper to return DeclProviderResult::Decls
DeclProviderResult decls(const Decls* decls) {
  DeclProviderResult r;
  r.tag = DeclProviderResult::Tag::Decls;
  r.decl_provider_decls_result._0 = decls;
  return r;
}

} // namespace {}

std::unique_ptr<HhvmDeclProvider>
HhvmDeclProvider::create(const RepoOptionsFlags& options) {
  if (!RuntimeOption::EvalEnableDecl) {
    return {nullptr};
  }
  if (!AutoloadHandler::s_instance) {
    // It is not safe to autoinit AutoloadHandler outside a normal request.
    return {nullptr};
  }
  auto map = AutoloadHandler::s_instance->getAutoloadMap();
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
      decl_flags, aliased_namespaces, map
  );
}

HhvmDeclProvider::HhvmDeclProvider(
    int32_t flags,
    std::string const& aliased_namespaces,
    AutoloadMap* map
)
  : m_opts{hackc_create_direct_decl_parse_options(flags, aliased_namespaces)}
  , m_map{map}
{}

// Called by hackc, potentially on a different thread.
DeclProviderResult HhvmDeclProvider::getDecl(
  AutoloadMap::KindOf kind,
  std::string_view symbol
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
      return decls(&(*result->second.decls));
    }

    // Nothing cached: Load file, parse decls.
    std::ifstream s(filename);
    std::string text {
      std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>()
    };

    DeclResult decl_result = hackc_direct_decl_parse(*m_opts, filename, text);
    ITRACE(3, "DP parsed {} in {}\n", sym, filename);

    // Insert decl_result into the cache, return DeclResult::decls,
    // a pointer to rust decls in m_cache.
    auto [it, _] = m_cache.insert({filename, std::move(decl_result)});
    return decls(&(*it->second.decls));
  }
  ITRACE(4, "DP {}: getFile() returned None\n", sym);
  return missing();
}

extern "C" {

DeclProviderResult hhvm_decl_provider_get_decl(
    void* provider, int symbol_kind, char const* symbol, size_t len
) {
  try {
    // Unsafe: if `symbol_kind` is out of range the result of this cast is UB.
    HPHP::AutoloadMap::KindOf kind {
      static_cast<HPHP::AutoloadMap::KindOf>(symbol_kind)
    };
    return ((HhvmDeclProvider*)provider)->getDecl(
        kind, std::string_view(symbol, len)
    );
  } catch(...) {
    not_reached();
  }
}

} //extern "C"
}//namespace HPHP
