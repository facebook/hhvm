/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/version.h"
#include "hphp/util/exception.h"
#include "hphp/util/hdf.h"

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
  static bool IsLoaded(const String& name);
  static bool IsSystemlibPath(const std::string& path);
  static Array GetLoadedExtensions();
  static Extension *GetExtension(const String& name);

  // called by RuntimeOption to initialize all configurations of extension
  static void LoadModules(const IniSetting::Map& ini, Hdf hdf);

  // called by hphp_process_init/exit
  static void InitModules();
  static void MergeSystemlib();
  static void ShutdownModules();
  static bool ModulesInitialised();
  static void ThreadInitModules();
  static void ThreadShutdownModules();
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
  virtual void moduleLoad(const IniSetting::Map& ini, Hdf hdf) {}
  virtual void moduleInfo(Array &info) { info.set(String(m_name), true);}
  virtual void moduleInit() {}
  virtual void moduleShutdown() {}
  virtual void threadInit() {}
  virtual void threadShutdown() {}
  virtual void requestInit() {}
  virtual void requestShutdown() {}

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
  static void SortDependencies();

  // Indicates which version of the HHVM Extension API
  // this module was built against.
  int64_t m_hhvmAPIVersion;

  std::string m_name;
  std::string m_version;
  std::string m_dsoName;
};

#define HHVM_API_VERSION 20150112L

#ifdef HHVM_BUILD_DSO
#define HHVM_GET_MODULE(name) \
extern "C" Extension *getModule() { \
  return &s_##name##_extension; \
}
#else
#define HHVM_GET_MODULE(name)
#endif

/////////////////////////////////////////////////////////////////////////////
// Extension argument API

/**
 * Get numbererd arg (zero based) as a TypedValue*
 */
inline
TypedValue* getArg(ActRec *ar, unsigned arg) {
  if (arg >= ar->numArgs()) {
    return nullptr;
  }
  unsigned funcParams = ar->func()->numParams();
  if (arg < funcParams) {
    auto args = reinterpret_cast<TypedValue*>(ar);
    return args - (arg + 1);
  }
  return ar->getExtraArg(arg - funcParams);
}

/**
 * Get numbered arg (zero based) as a Variant
 */
inline Variant getArgVariant(ActRec *ar, unsigned arg,
                             Variant def = uninit_null()) {
  auto tv = getArg(ar, arg);
  return tv ? tvAsVariant(tv) : def;
}

/**
 * Get a reference value from the stack
 */
template <DataType DType>
typename std::enable_if<DType == KindOfRef, VRefParam>::type
getArg(ActRec *ar, unsigned arg) {
  auto tv = getArg(ar, arg);
  if (!tv) {
    raise_warning("Required parameter %d not passed", (int)arg);
    return directRef(Variant());
  }
  if (tv->m_type != KindOfRef) {
    raise_warning("Argument %d not passed as reference", (int)arg);
  }
  return directRef(tvAsVariant(tv));
}

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g.: double dval = getArg<KindOfDouble>(ar, 0);
 *
 * Throws warning and returns 0/nullptr if arg not passed
 */
template <DataType DType>
typename std::enable_if<DType != KindOfRef,
  typename DataTypeCPPType<DType>::type>::type
getArg(ActRec *ar, unsigned arg) {
  auto tv = getArg(ar, arg);
  if (!tv) {
    raise_warning("Required parameter %d not passed", (int)arg);
    return 0L;
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    tvCastInPlace(tv, DType);
  }
  return unpack_tv<DType>(tv);
}

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g. int64_t lval = getArg<KindOfInt64>(ar, 1, 42);
 *
 * Returns default value (42 in example) if arg not passed
 */
template <DataType DType>
typename DataTypeCPPType<DType>::type
getArg(ActRec *ar, unsigned arg,
       typename DataTypeCPPType<DType>::type def) {
  TypedValue *tv = getArg(ar, arg);
  if (!tv) {
    return def;
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    tvCastInPlace(tv, DType);
  }
  return unpack_tv<DType>(tv);
}

struct IncoercibleArgumentException : Exception {};

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g.: double dval = getArg<KindOfDouble>(ar, 0);
 *
 * Raise warning and throw IncoercibleArgumentException if argument
 * is not provided/not coercible
 */
template <DataType DType>
typename std::enable_if<DType != KindOfRef,
  typename DataTypeCPPType<DType>::type>::type
getArgStrict(ActRec *ar, unsigned arg) {
  auto tv = getArg(ar, arg);
  if (!tv) {
    raise_warning("Required parameter %d not passed", (int)arg);
    throw IncoercibleArgumentException();
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    throw IncoercibleArgumentException();
  }
  return unpack_tv<DType>(tv);
}

/**
 * Get numbered arg (zero based) and return data (coerce if needed)
 *
 * e.g. int64_t lval = getArg<KindOfInt64>(ar, 1, 42);
 *
 * Raise warning and throw IncoercibleArgumentException if argument
 * is not coercible
 *
 * Returns default value (42 in example) if arg not passed
 */
template <DataType DType>
typename DataTypeCPPType<DType>::type
getArgStrict(ActRec *ar, unsigned arg,
       typename DataTypeCPPType<DType>::type def) {
  TypedValue *tv = getArg(ar, arg);
  if (!tv) {
    return def;
  }
  if (!tvCoerceParamInPlace(tv, DType)) {
    raise_param_type_warning(ar->func()->name()->data(),
                             arg + 1, DType, tv->m_type);
    throw IncoercibleArgumentException();
  }
  return unpack_tv<DType>(tv);
}

/**
 * Parse typed values from the function call
 * based on an expect format.
 *
 * e.g.:
 *   int64_t lval;
 *   double dval;
 *   TypedValye *tv = nullptr;
 *   if (!parseArgs(ar, "ld|v", &lval, &dval, &tv)) {
 *     return false;
 *   }
 */
bool parseArgs(ActRec *ar, const char *format, ...);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
#endif // incl_HPHP_EXTENSION_H_
