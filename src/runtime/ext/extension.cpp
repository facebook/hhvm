/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/extension.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, Extension*, stdltistr> ExtensionMap;
static ExtensionMap *s_registered_extensions = NULL;
static bool s_modules_initialised = false;

// just to make valgrind cleaner
class ExtensionUninitializer {
public:
  ~ExtensionUninitializer() {
    delete s_registered_extensions;
  }
};
static ExtensionUninitializer s_extension_uninitializer;

///////////////////////////////////////////////////////////////////////////////

Extension::Extension(litstr name, const char *version /* = "" */)
    : m_name(name), m_version(version ? version : "") {
  if (s_registered_extensions == NULL) {
    s_registered_extensions = new ExtensionMap();
  }
  ASSERT(s_registered_extensions->find(name) ==
         s_registered_extensions->end());
  (*s_registered_extensions)[name] = this;
}

void Extension::LoadModules(Hdf hdf) {
  ASSERT(s_registered_extensions);
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    iter->second->moduleLoad(hdf["Extensions"][iter->second->m_name]);
  }
}

void Extension::InitModules() {
  ASSERT(s_registered_extensions);
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
  ASSERT(s_registered_extensions);
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    iter->second->moduleShutdown();
  }
  s_registered_extensions->clear();
}

bool Extension::IsLoaded(CStrRef name) {
  if (name == "apc") {
    return RuntimeOption::EnableApc;
  }
  ASSERT(s_registered_extensions);
  return s_registered_extensions->find(name.data()) !=
    s_registered_extensions->end();
}

Extension *Extension::GetExtension(CStrRef name) {
  ASSERT(s_registered_extensions);
  ExtensionMap::iterator iter = s_registered_extensions->find(name.data());
  if (iter != s_registered_extensions->end()) {
    return iter->second;
  }
  return NULL;
}

Array Extension::GetLoadedExtensions() {
  ASSERT(s_registered_extensions);
  Array ret = Array::Create();
  for (ExtensionMap::const_iterator iter = s_registered_extensions->begin();
       iter != s_registered_extensions->end(); ++iter) {
    ret.append(iter->second->m_name);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
