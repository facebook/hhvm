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

#ifndef incl_HPHP_EXTENSION_H_
#define incl_HPHP_EXTENSION_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/util/hdf.h"
#include "hphp/runtime/vm/native.h"

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
 * If only "name" is needed to register, use IMPLEMENT_DEFAULT_EXTENSION.
 */

#define IMPLEMENT_DEFAULT_EXTENSION(name)               \
  static class name ## Extension : public Extension {   \
  public:                                               \
    name ## Extension() : Extension(#name) {}           \
  } s_ ## name ## _extension

#define IMPLEMENT_DEFAULT_EXTENSION_VERSION(name, v)    \
  static class name ## Extension : public Extension {   \
  public:                                               \
    name ## Extension() : Extension(#name, #v) {}       \
  } s_ ## name ## _extension

///////////////////////////////////////////////////////////////////////////////

class Extension : public IDebuggable {
public:
  static bool IsLoaded(const String& name);
  static Array GetLoadedExtensions();
  static Extension *GetExtension(const String& name);

  // called by RuntimeOption to initialize all configurations of extension
  static void LoadModules(Hdf hdf);

  // called by hphp_process_init/exit
  static void InitModules();
  static void MergeSystemlib();
  static void ShutdownModules();
  static bool ModulesInitialised();
  static void RequestInitModules();
  static void RequestShutdownModules();

  // Look for "ext.{namehash}" in the binary and compile/merge it
  void loadSystemlib(const std::string& name = "");

  // Compile and merge an systemlib fragment
  static void CompileSystemlib(const std::string &slib,
                               const std::string &name);
public:
  explicit Extension(litstr name, const char *version = "");
  virtual ~Extension() {}

  const char *getVersion() const { return m_version.c_str();}

  // override these functions to implement module specific init/shutdown
  // sequences and information display.
  virtual void moduleLoad(Hdf hdf) {}
  virtual void moduleInfo(Array &info) { info.set(m_name, true);}
  virtual void moduleInit() {}
  virtual void moduleShutdown() {}
  virtual void requestInit() {}
  virtual void requestShutdown() {}

  void setDSOName(const std::string &name) {
    m_dsoName = name;
  }

private:
  // Indicates which version of the HHVM Extension API
  // this module was built against.
  int64_t m_hhvmAPIVersion;

  const String m_name;
  std::string m_version;
  std::string m_dsoName;
};

#define HHVM_API_VERSION 20131007L

#define HHVM_GET_MODULE(name) \
extern "C" Extension *getModule() { \
  return &s_##name##_extension; \
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXTENSION_H_
