/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/version.h"
#include "hphp/util/hdf.h"

#include <set>
#include <string>
#include <string_view>

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
#define NO_ONCALL_YET "\0"

#define IMPLEMENT_DEFAULT_EXTENSION_VERSION(name, v)                      \
  static class name ## Extension final : public Extension {               \
  public:                                                                 \
    name ## Extension() : Extension(#name, #v, NO_ONCALL_YET) {}          \
  } s_ ## name ## _extension

///////////////////////////////////////////////////////////////////////////////

struct Extension : IDebuggable {
  static bool IsSystemlibPath(const std::string& path);

  // Look for "ext.{namehash}" in the binary and compile/merge it
  void loadSystemlib() { loadSystemlib(m_name); }
  void loadSystemlib(const std::string& name);

  // Compile and merge an systemlib fragment
  static void CompileSystemlib(const std::string &slib,
                               const std::string &name,
                               const Native::FuncTable& nativeFuncs);
public:
  explicit Extension(const char name[],
                     const char version[]= NO_EXTENSION_VERSION_YET,
                     const char oncall[] = NO_ONCALL_YET);
  ~Extension() override {}

  const char* getName() const { return m_name; }
  const char* getVersion() const { return m_version; }
  const char* getOncall() const { return m_oncall; }

  // override these functions to implement module specific init/shutdown
  // sequences and information display.
  virtual void moduleLoad(const IniSetting::Map& /*ini*/, Hdf /*hdf*/);
  virtual void moduleInfo(Array &info);
  virtual void moduleInit();
  virtual void cliClientInit();
  virtual void moduleShutdown();
  virtual void threadInit();
  virtual void threadShutdown();
  virtual void requestInit();
  virtual void requestShutdown();

  // Override this function when your extension calls anything other than:
  //
  //  loadSystemlib();
  //
  // ... in `moduleInit`. This will load the decls for any symbols declared in
  // the PHP file for this module. Examples of when overriding this function is
  // required:
  // - ext_asio, which calls `loadSystemlib` twice with different arguments
  // - ext_collections*, which calls `loadSystemlib` for each collection kind
  // - ext_datetime, which happens to use a different name for the extension
  //   versus the binary section
  virtual void loadDecls();
  // Load the source contained in the binary section corresponding to [name],
  // parse it's decls, then store them in `s_builtin_symbols`. This is generally
  // going to be given the same string as whatever `loadSystemlib` takes, and
  // by default will be passed *just* the extension name.
  void loadDeclsFrom(std::string_view name);

  // override this to control extension_loaded() return value
  virtual bool moduleEnabled() const;

  // override these functions to perform extension-specific jumpstart,
  // leveraging the JIT profile data serialization mechanisms.
  virtual std::string serialize() { return {}; }
  // throws std::runtime_error to abort the whole thing if needed. The extension
  // can also choose to swallow the error.
  virtual void deserialize(std::string) {}

  using DependencySet = std::set<std::string>;
  using DependencySetMap = std::map<Extension*, DependencySet>;

  virtual const DependencySet getDeps() const;

  void setDSOName(const std::string &name) {
    m_dsoName = name;
  }

  void registerNativeFunc(const StringData* name,
                          const Native::NativeFunctionInfo&);

  // access the list of functions (excluding methods);
  // helper for get_extension_funcs()
  const std::vector<StringData*>& getExtensionFunctions() const;
  void registerExtensionFunction(const String& name);

  Native::FuncTable& nativeFuncs() {
    return m_nativeFuncs;
  }

private:
  const char* m_name;
  const char* m_version;
  const char* m_oncall;
  std::string m_dsoName;
  std::vector<StringData*> m_functions;
  Native::FuncTable m_nativeFuncs;
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

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
