#include "hphp/runtime/ext/extension-registry.h"

#include <sstream>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/version.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/system/systemlib.h"

#ifdef HAVE_LIBDL
# include <dlfcn.h>
# ifndef RTLD_LAZY
#  define RTLD_LAZY 1
# endif
# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
# endif
# if defined(RTLD_GROUP) && defined(RTLD_WORLD) && defined(RTLD_PARENT)
#  define DLOPEN_FLAGS (RTLD_LAZY|RTLD_GLOBAL|RTLD_GROUP|RTLD_WORLD|RTLD_PARENT)
# else
#  define DLOPEN_FLAGS (RTLD_LAZY|RTLD_GLOBAL)
# endif
#endif

namespace HPHP::ExtensionRegistry {
/////////////////////////////////////////////////////////////////////////////

// s_exts isn't necessarily initialized by the time Extensions
// start registering themselves, so we have to be explicit about
// allocating/initializing/destroying this rather than just
// putting it global and letting the compiler deal with it. :(
typedef std::map<std::string, Extension*, stdltistr> ExtensionMap;
static ExtensionMap *s_exts = nullptr;

// just to make valgrind cleaner
struct ExtensionRegistryUninitializer {
  ~ExtensionRegistryUninitializer() {
    delete s_exts;
  }
};
static ExtensionRegistryUninitializer s_extension_registry_uninitializer;

typedef std::vector<Extension*> OrderedExtensionVector;
static OrderedExtensionVector s_ordered;

static bool s_sorted = false;
static bool s_initialized = false;

static OrderedExtensionVector sortDependencies(const ExtensionMap& map);

///////////////////////////////////////////////////////////////////////////////
// dlfcn wrappers

static void* dlopen(ATTRIBUTE_UNUSED const char* dso) {
#ifdef HAVE_LIBDL
  return ::dlopen(dso, DLOPEN_FLAGS);
#else
  return nullptr;
#endif
}

static void*
dlsym(ATTRIBUTE_UNUSED void* mod, ATTRIBUTE_UNUSED const char* sym) {
#ifdef HAVE_LIBDL
# ifdef LIBDL_NEEDS_UNDERSCORE
  std::string tmp("_");
  tmp += sym;
  sym = tmp.c_str();
# endif
  return ::dlsym(mod, sym);
#else
  return nullptr;
#endif
}

static const char* dlerror() {
#ifdef HAVE_LIBDL
  return ::dlerror();
#else
  return "Your system does not support dlopen()";
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Registration API

void registerExtension(Extension* ext) {
  if (!s_exts) {
    // Not really thread-safe,
    // but in practice this happens at process startup
    // before any threads are spun up.
    s_exts = new ExtensionMap;
  }
  const auto& name = ext->getName();
  assertx(s_exts->find(name) == s_exts->end());
  (*s_exts)[name] = ext;
}

bool isLoaded(const char* name) {
  assertx(s_exts);
  auto it = s_exts->find(name);
  return (it != s_exts->end()) && it->second->moduleEnabled();
}

Extension* get(const char* name) {
  assertx(s_exts);
  auto it = s_exts->find(name);
  if ((it != s_exts->end()) && it->second->moduleEnabled()) {
    return it->second;
  }
  return nullptr;
}

Array getLoaded() {
  assertx(s_exts);
  // Overestimate.
  VecInit ret(s_exts->size());
  for (auto& kv : (*s_exts)) {
    if (kv.second->moduleEnabled()) {
      ret.append(String(kv.second->getName()));
    }
  }
  return ret.toArray();
}

const std::vector<const Extension*> getExtensions() {
  std::vector<const Extension*> all;
  for (auto& kv : (*s_exts)) {
    all.push_back(kv.second);
  }
  return all;
}

/////////////////////////////////////////////////////////////////////////////
// Lifecycle delegators

void moduleLoad(const IniSetting::Map& ini, Hdf hdf) {
  for (auto& it : (*s_exts)) {
    it.second->moduleLoad(ini, hdf);
  }

  s_ordered = sortDependencies(*s_exts);
  s_sorted = true;
}

void moduleInit() {
  auto db = RuntimeOption::EvalDumpBytecode;
  auto rp = Cfg::Server::AlwaysUseRelativePath;
  auto sf = Cfg::Server::SafeFileAccess;
  auto ah = RuntimeOption::EvalAllowHhas;

  RuntimeOption::EvalDumpBytecode &= ~1;
  Cfg::Server::AlwaysUseRelativePath = false;
  Cfg::Server::SafeFileAccess = false;
  RuntimeOption::EvalAllowHhas = true;

  SCOPE_EXIT {
    RuntimeOption::EvalDumpBytecode = db;
    Cfg::Server::AlwaysUseRelativePath = rp;
    Cfg::Server::SafeFileAccess = sf;
    RuntimeOption::EvalAllowHhas = ah;
  };

  assertx(s_sorted);
  for (auto& ext : s_ordered) {
    ext->moduleInit();
    assertx(ext->nativeFuncs().empty());
    ext->moduleRegisterNative();
    ext->loadEmitters();
  }
  s_initialized = true;
}

void moduleRegisterNative() {
  for (auto& it : *s_exts) {
    auto &ext = it.second;
    assertx(ext->nativeFuncs().empty());
    ext->moduleRegisterNative();
  }
}

void moduleDeclInit() {
  assertx(s_sorted);
  for (auto& ext : s_ordered) {
    ext->loadDecls();
  }
}

void cliClientInit() {
  assertx(s_sorted);
  for (auto& ext : s_ordered) {
    ext->cliClientInit();
  }
  s_initialized = true;
}

void moduleShutdown() {
  assertx(s_exts);
  assertx(s_sorted);
  for (auto it = s_ordered.rbegin();
       it != s_ordered.rend(); ++it) {
    (*it)->moduleShutdown();
  }
  s_exts->clear();
  s_ordered.clear();
  s_sorted = false;
  s_initialized = false;
}

void threadInit() {
  assertx(s_sorted);
  for (auto& ext : s_ordered) {
    ext->threadInit();
  }
}

void threadShutdown() {
  assertx(s_sorted);
  for (auto it = s_ordered.rbegin();
       it != s_ordered.rend(); ++it) {
    (*it)->threadShutdown();
  }
}

void requestInit() {
  assertx(s_sorted);
  for (auto& ext : s_ordered) {
    ext->requestInit();
  }
}

void requestShutdown() {
  assertx(s_sorted);
  for (auto it = s_ordered.rbegin();
       it != s_ordered.rend(); ++it) {
    (*it)->requestShutdown();
  }
}

bool modulesInitialised() { return s_initialized; }

void serialize(jit::ProfDataSerializer& ser) {
  std::vector<std::pair<std::string, std::string>> extData;
  for (auto& ext : s_ordered) {
    auto name = ext->getName();
    auto data = ext->serialize();
    if (!data.size()) continue;
    extData.push_back({std::move(name), std::move(data)});
  }
  uint32_t len = extData.size();
  jit::write_raw<uint32_t>(ser, len);
  for (const auto& ext : extData) {
    len = ext.first.size();
    jit::write_raw<uint32_t>(ser, len);
    jit::write_raw(ser, ext.first.c_str(), len);
    len = ext.second.size();
    jit::write_raw<uint32_t>(ser, len);
    jit::write_raw(ser, ext.second.c_str(), len);
  }
}

void deserialize(jit::ProfDataDeserializer& des) {
  auto const nExts = jit::read_raw<uint32_t>(des);
  for (uint32_t i = 0; i < nExts; ++i) {
    uint32_t len = jit::read_raw<uint32_t>(des);
    std::string str;
    str.resize(len);
    jit::read_raw(des, str.data(), len);
    auto ext = get(str.data());
    if (!ext) continue;
    len = jit::read_raw<uint32_t>(des);
    str.resize(len);
    jit::read_raw(des, str.data(), len);
    ext->deserialize(std::move(str));
  }
}

/////////////////////////////////////////////////////////////////////////////

static
Extension* findResolvedExt(const Extension::DependencySetMap& unresolved,
                           const Extension::DependencySet& resolved) {
  if (unresolved.empty()) return nullptr;

  for (auto& ed : unresolved) {
    Extension* ret = ed.first;
    for (auto& req : ed.second) {
      if (resolved.find(req) == resolved.end()) {
        // Something we depend on still isn't resolved, try another
        ret = nullptr;
        break;
      }
    }
    if (ret) return ret;
  }
  return nullptr;
}

static OrderedExtensionVector sortDependencies(const ExtensionMap& exts) {
  OrderedExtensionVector ordered;

  Extension::DependencySet resolved;
  Extension::DependencySetMap unresolved;

  // First put core at the beginning of the list
  {
    auto kv = exts.find("core");
    // core has to exist
    always_assert(kv != exts.end());
    auto ext = kv->second;
    // code has to be enabled
    always_assert(ext->moduleEnabled());
    // core can not have dependencies
    always_assert(ext->getDeps().empty());
    ordered.push_back(ext);
    resolved.insert(kv->first);
  }

  // First pass, identify the easy(common) case of modules
  // with no dependencies and put that at the front of the list but skip core
  // defer all other for slower resolution
  for (auto& kv : exts) {
    auto ext = kv.second;
    if (kv.first.compare("core") == 0) {
      continue;
    }
    if (!ext->moduleEnabled()) {
      continue;
    }
    auto deps = ext->getDeps();
    if (deps.empty()) {
      ordered.push_back(ext);
      resolved.insert(kv.first);
      continue;
    }
    unresolved[ext] = deps;
  }

  // Second pass, check each remaining extension against
  // their dependency list until they have all been loaded
  while (auto ext = findResolvedExt(unresolved, resolved)) {
    ordered.push_back(ext);
    resolved.insert(ext->getName());
    unresolved.erase(ext);
  }

  if (UNLIKELY(!unresolved.empty())) {
    // Alerts user to cirular dependency in extensions
    // e.g. Unable to resovle dependencies for extension(s):
    //         A(depends: B) B(depends: C) C(depends: A)

    std::stringstream ss;
    ss << "Unable to resolve dependencies for extension(s):";
    for (auto& kv : unresolved) {
      ss << " " << kv.first->getName() << "(depends:";
      for (auto& req : kv.second) {
        ss << " " << req;
      }
      ss << ")";
    }
    throw Exception(ss.str());
  }

  assertx(ordered.size() <= exts.size());
  return ordered;
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::ExtensionRegistry
