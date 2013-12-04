/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/exception.h"

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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, Extension*, stdltistr> ExtensionMap;
static ExtensionMap *s_registered_extensions = NULL;
static bool s_modules_initialised = false;
static std::vector<Unit*> s_systemlib_units;

// just to make valgrind cleaner
class ExtensionUninitializer {
public:
  ~ExtensionUninitializer() {
    delete s_registered_extensions;
  }
};
static ExtensionUninitializer s_extension_uninitializer;

///////////////////////////////////////////////////////////////////////////////
// dlfcn wrappers

static void* dlopen(const char *dso) {
#ifdef HAVE_LIBDL
  return ::dlopen(dso, DLOPEN_FLAGS);
#else
  return nullptr;
#endif
}

static void* dlsym(void *mod, const char *sym) {
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

///////////////////////////////////////////////////////////////////////////////

Extension::Extension(litstr name, const char *version /* = "" */)
    : m_hhvmAPIVersion(HHVM_API_VERSION)
    , m_name(makeStaticString(name))
    , m_version(version ? version : "") {
  if (s_registered_extensions == NULL) {
    s_registered_extensions = new ExtensionMap();
  }
  assert(s_registered_extensions->find(name) ==
         s_registered_extensions->end());
  (*s_registered_extensions)[name] = this;
}

void Extension::LoadModules(Hdf hdf) {
  // Load up any dynamic extensions
  std::string path = hdf["DynamicExtensionPath"].getString(".");
  for (Hdf ext = hdf["DynamicExtensions"].firstChild();
       ext.exists(); ext = ext.next()) {
    std::string extLoc = ext.getString();
    if (extLoc.empty()) {
      continue;
    }
    if (extLoc[0] != '/') {
      extLoc = path + "/" + extLoc;
    }

    // Extensions are self-registering,
    // so we bring in the SO then
    // throw away its handle.
    void *ptr = dlopen(extLoc.c_str());
    if (!ptr) {
      throw Exception("Could not open extension %s: %s",
                      extLoc.c_str(), dlerror());
    }
    auto getModule = (Extension *(*)())dlsym(ptr, "getModule");
    if (!getModule) {
      throw Exception("Could not load extension %s: %s (%s)",
                      extLoc.c_str(),
                      "getModule() symbol not defined.",
                      dlerror());
    }
    Extension *mod = getModule();
    if (mod->m_hhvmAPIVersion != HHVM_API_VERSION) {
      throw Exception("Could not use extension %s: "
                      "Compiled with HHVM API Version %" PRId64 ", "
                      "this version of HHVM expects %ld",
                      extLoc.c_str(),
                      mod->m_hhvmAPIVersion,
                      HHVM_API_VERSION);
    }
    mod->setDSOName(extLoc);
  }

  // Invoke Extension::moduleLoad() callbacks
  assert(s_registered_extensions);
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    iter->second->moduleLoad(hdf);
  }
}

void Extension::InitModules() {
  assert(s_registered_extensions);
  bool wasInited = SystemLib::s_inited;
  LitstrTable::get().setWriting();
  SCOPE_EXIT {
    SystemLib::s_inited = wasInited;
    LitstrTable::get().setReading();
  };
  SystemLib::s_inited = false;
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    iter->second->moduleInit();
  }
  s_modules_initialised = true;
}

bool Extension::ModulesInitialised() {
  return s_modules_initialised;
}

void Extension::ShutdownModules() {
  assert(s_registered_extensions);
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    iter->second->moduleShutdown();
  }
  s_registered_extensions->clear();
}

const StaticString s_apc("apc");

bool Extension::IsLoaded(const String& name) {
  if (name == s_apc) {
    return apcExtension::Enable;
  }
  assert(s_registered_extensions);
  return s_registered_extensions->find(name.data()) !=
    s_registered_extensions->end();
}

Extension *Extension::GetExtension(const String& name) {
  assert(s_registered_extensions);
  ExtensionMap::iterator iter = s_registered_extensions->find(name.data());
  if (iter != s_registered_extensions->end()) {
    return iter->second;
  }
  return NULL;
}

Array Extension::GetLoadedExtensions() {
  assert(s_registered_extensions);
  Array ret = Array::Create();
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    if (!apcExtension::Enable && iter->second->m_name == s_apc) {
      continue;
    }
    ret.append(iter->second->m_name);
  }
  return ret;
}

void Extension::MergeSystemlib() {
  for (auto &unit : s_systemlib_units) {
    unit->merge();
  }
}

void Extension::CompileSystemlib(const std::string &slib,
                                 const std::string &name) {
  Unit *unit = compile_string(slib.c_str(), slib.size(), name.c_str());
  assert(unit);
  unit->merge();
  s_systemlib_units.push_back(unit);
}

/**
 * Loads a named systemlib section from the main binary (or DSO)
 * using the label "ext.{hash(name)}"
 *
 * If {name} is not passed, then {m_name} is assumed for
 * builtin extensions.  DSOs pull from the fixed "systemlib" label
 */
void Extension::loadSystemlib(const std::string& name /*= "" */) {
  std::string hhas, slib;
  if (m_dsoName.empty() || !name.empty()) {
    std::string section("ext.");
    section += f_md5(name.empty() ? m_name : name, false).substr(0, 12).data();
    slib = get_systemlib(&hhas, section);
  } else {
    slib = get_systemlib(&hhas, "systemlib", m_dsoName);
  }
  if (!slib.empty()) {
    std::string phpname("systemlib.php.");
    phpname += m_name.data();
    CompileSystemlib(slib, phpname);
  }
  if (!hhas.empty()) {
    std::string hhasname("systemlib.hhas.");
    hhasname += m_name.data();
    CompileSystemlib(hhas, hhasname);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
