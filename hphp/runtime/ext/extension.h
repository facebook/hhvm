/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXTENSION_H_
#define incl_HPHP_EXTENSION_H_

#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/version.h"
#include "hphp/util/hdf.h"
#include "hphp/runtime/base/imarker.h"

#include <set>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class of extension modules. See ext_session.cpp for an example. It is
 * NOT required to have an extension class to derive from this base class,
 * unless one of these is needed:
 *
 *   - register an extension's name so extension_loaded() can work;
 *   - register extension's detailed information;
 *   - implement moduleInit() and moduleShutdown() logics that static
 *     initialization is too early.
 *
 * If only "name" is needed to register, use
 * IMPLEMENT_DEFAULT_EXTENSION_VERSION(name, NO_EXTENSION_VERSION_YET).
 */

#define NO_EXTENSION_VERSION_YET "\0"

#define IMPLEMENT_DEFAULT_EXTENSION_VERSION(name, v)    \
  static class name ## Extension final : public Extension {   \
  public:                                               \
    name ## Extension() : Extension(#name, #v) {}       \
  } s_ ## name ## _extension

///////////////////////////////////////////////////////////////////////////////

class Extension : public IDebuggable {
public:
  static bool IsSystemlibPath(const std::string& path);

  // Look for "ext.{namehash}" in the binary and compile/merge it
  void loadSystemlib(const std::string& name = "");

  // Compile and merge an systemlib fragment
  static void CompileSystemlib(const std::string &slib,
                               const std::string &name);
public:
  explicit Extension(const char* name, const char* version = "");
  virtual ~Extension() {}

  const char* getVersion() const { return m_version.c_str();}

  // override these functions to implement module specific init/shutdown
  // sequences and information display.
  virtual void moduleLoad(const IniSetting::Map& ini, Hdf hdf) {}
  virtual void moduleInfo(Array &info) { info.set(String(m_name), true);}
  virtual void moduleInit() {}
  virtual void moduleShutdown() {}
  virtual void threadInit() {}
  virtual void threadShutdown() {}
  virtual void requestInit() {}
  virtual void requestShutdown() {}
  virtual void vscan(IMarker&) const {} // TODO 6495061 pure virtual

  // override this to control extension_loaded() return value
  virtual bool moduleEnabled() const { return true; }

  typedef std::set<std::string> DependencySet;
  typedef std::map<Extension*, DependencySet> DependencySetMap;
  virtual const DependencySet getDeps() const {
    // No dependencies by default
    return DependencySet();
  }

  void setDSOName(const std::string &name) {
    m_dsoName = name;
  }

  const std::string& getName() const {
    return m_name;
  }

private:
  std::string m_name;
  std::string m_version;
  std::string m_dsoName;
};

struct ExtensionBuildInfo {
  uint64_t dso_version;
  uint64_t branch_id;
};


// Versioned ID for Extension class, do not use for feature selection
#define HHVM_DSO_VERSION 20150223L

#ifdef HHVM_BUILD_DSO
#define HHVM_GET_MODULE(name) \
static ExtensionBuildInfo s_##name##_extension_build_info = { \
  HHVM_DSO_VERSION, \
  HHVM_VERSION_BRANCH, \
}; \
extern "C" ExtensionBuildInfo* getModuleBuildInfo() { \
  return &s_##name##_extension_build_info; \
} \
extern "C" Extension* getModule() { \
  return &s_##name##_extension; \
}
#else
#define HHVM_GET_MODULE(name)
#endif

// Deprecated: Use HHVM_VERSION_BRANCH for source compat checks
#define HHVM_API_VERSION 20150212L

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
#endif // incl_HPHP_EXTENSION_H_
