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

#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include <runtime/base/complex_types.h>
#include <runtime/base/debuggable.h>
#include <util/hdf.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class of extension modules. See ext_session.cpp for an example. It is
 * NOT required to have an extention class to derive from this base class,
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
  static bool IsLoaded(CStrRef name);
  static Array GetLoadedExtensions();
  static Extension *GetExtension(CStrRef name);

  // called by RuntimeOption to initialize all configurations of extension
  static void LoadModules(Hdf hdf);

  // called by hphp_process_init/exit
  static void InitModules();
  static void ShutdownModules();
  static bool ModulesInitialised();
public:
  Extension(litstr name, const char *version = "");
  virtual ~Extension() {}

  const char *getVersion() const { return m_version.c_str();}

  // override these functions to implement module specific init/shutdown
  // sequences and information display.
  virtual void moduleLoad(Hdf hdf) {}
  virtual void moduleInfo(Array &info) { info.set(m_name, true);}
  virtual void moduleInit() {}
  virtual void moduleShutdown() {}

private:
  const char *m_name;
  std::string m_version;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXTENSION_H__
