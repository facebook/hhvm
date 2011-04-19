/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef __HPHP_SWIG_H__
#define __HPHP_SWIG_H__

#include <vector>
#include <runtime/base/hphp_ffi.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/externals.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/hphp_system.h>
#include <util/logger.h>
#include <runtime/base/util/extended_logger.h>

#define SWIG_NO_LLONG_MAX 1

///////////////////////////////////////////////////////////////////////////////
// helpers, not exposed

namespace HPHP {

class HphpSession {
public:
  HphpSession() {
    hphp_ffi_session_init();
    m_context = hphp_ffi_context_init();
  }

  ~HphpSession() {
    for (unsigned int i = 0; i < m_pointers.size(); i++) {
      hphp_ffi_freeVariant(m_pointers[i]);
    }
    m_pointers.clear();
    hphp_ffi_context_exit(m_context);
    m_context = NULL;
    hphp_ffi_session_exit();
  }

  void addVariant(HPHP::Variant *v) {
    m_pointers.push_back(v);
  }

private:
  HPHP::ExecutionContext *m_context;
  std::vector<HPHP::Variant *> m_pointers;
};

}

HPHP::Variant *hphpBuildVariant(int kind, void *v) {
  switch (kind) {
  case 0:
  case 1:
  case 2:
  case 3:
    return hphp_ffi_buildVariant(kind, v, 0);
  case 4:
    v = NEW(HPHP::StringData)((const char *)v, HPHP::CopyString);
  case 5:
    return hphp_ffi_buildVariant(5, v, 0);
  case 6:
    return hphp_ffi_buildVariant(7, v, 0);
  case 7:
    return hphp_ffi_buildVariant(8, v, 0);
  default:
    // impossible
    return NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// initialization and session management

void hphpStart() {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    hphp_ffi_init();
  }
}

HPHP::HphpSession *hphpStartSession() {
  return new HPHP::HphpSession();
}

void hphpFinishSession(HPHP::HphpSession *session) {
  delete session;
}

///////////////////////////////////////////////////////////////////////////////
// value manipulation and type inspection

#define TypeNull 0
#define TypeBoolean 1
#define TypeInt 2
#define TypeDouble 3
#define TypeString 4
#define TypeArray 5
#define TypeObject 6

HPHP::Variant *hphpNull(HPHP::HphpSession *s) {
  HPHP::Variant *ret = hphp_ffi_buildVariant(0, 0, 0);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpBoolean(HPHP::HphpSession *s, bool value) {
  HPHP::Variant *ret = hphp_ffi_buildVariant(1, (void *)value, 0);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpInt(HPHP::HphpSession *s, long long value) {
  HPHP::Variant *ret = hphp_ffi_buildVariant(2, (void *)value, 0);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpDouble(HPHP::HphpSession *s, double value) {
  union {
    double d;
    void *p;
  } u;
  u.d = value;
  HPHP::Variant *ret = hphp_ffi_buildVariant(3, u.p, 0);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpString(HPHP::HphpSession *s, const char *value) {
  HPHP::Variant *ret = hphp_ffi_buildVariant(6, (void *)value, strlen(value));
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpArray(HPHP::HphpSession *s) {
  HPHP::Variant *ret = hphp_ffi_buildVariant(9, 0, 0);
  s->addVariant(ret);
  return ret;
}

int hphpGetType(HPHP::HphpSession *s, HPHP::Variant *v) {
  switch (v->getType()) {
  case HPHP::KindOfUninit:
  case HPHP::KindOfNull:
    return TypeNull;
  case HPHP::KindOfBoolean:
    return TypeBoolean;
  case HPHP::KindOfInt32:
  case HPHP::KindOfInt64:
    return TypeInt;
  case HPHP::KindOfDouble:
    return TypeDouble;
  case HPHP::KindOfStaticString:
  case HPHP::KindOfString:
    return TypeString;
  case HPHP::KindOfArray:
    return TypeArray;
  case HPHP::KindOfObject:
    return TypeObject;
  default:
    // impossible
    return -1;
  }
}

bool hphpGetBoolean(HPHP::HphpSession *s, HPHP::Variant *v) {
  return v->toBoolean();
}

long long hphpGetInt(HPHP::HphpSession *s, HPHP::Variant *v) {
  return v->toInt64();
}

double hphpGetDouble(HPHP::HphpSession *s, HPHP::Variant *v) {
  return v->toDouble();
}

const char *hphpGetString(HPHP::HphpSession *s, HPHP::Variant *v) {
  return v->toString().data();
}

///////////////////////////////////////////////////////////////////////////////
// array operations

void hphpSet(HPHP::HphpSession *s, HPHP::Variant *map, HPHP::Variant *key,
             HPHP::Variant *value) {
  hphp_ffi_addMapItem(map, key, value);
}

HPHP::Variant *hphpGet(HPHP::HphpSession *s, HPHP::Variant *map,
                       HPHP::Variant *key) {
  void *result;
  int kind = hphp_ffi_getMapItem(&result, map, key);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

void hphpAppend(HPHP::HphpSession *s, HPHP::Variant *arr,
                HPHP::Variant *value) {
  arr->append(*value);
}

long long hphpIterBegin(HPHP::HphpSession *s, HPHP::Variant *arr) {
  return hphp_ffi_iter_begin(arr->getArrayData());
}

long long hphpIterAdvance(HPHP::HphpSession *s, HPHP::Variant *arr,
                          long long iter) {
  return hphp_ffi_iter_advance(arr->getArrayData(), iter);
}

bool hphpIterValid(HPHP::HphpSession *s, long long iter) {
  return !hphp_ffi_iter_invalid(iter);
}

HPHP::Variant *hphpIterGetKey(HPHP::HphpSession *s, HPHP::Variant *arr,
                              long long iter) {
  void *result;
  int kind = hphp_ffi_iter_getKey(arr->getArrayData(), iter, &result);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpIterGetValue(HPHP::HphpSession *s, HPHP::Variant *arr,
                                long long iter) {
  void *result;
  int kind = hphp_ffi_iter_getValue(arr->getArrayData(), iter, &result);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

int hphpCount(HPHP::HphpSession *s, HPHP::Variant *arr) {
  return (int)arr->getArrayData()->size();
}

void hphpUnset(HPHP::HphpSession *s, HPHP::Variant *arr, HPHP::Variant *key) {
  arr->weakRemove(*key);
}

bool hphpIsset(HPHP::HphpSession *s, HPHP::Variant *arr, HPHP::Variant *key) {
  return HPHP::isset(*arr, *key);
}

///////////////////////////////////////////////////////////////////////////////
// dynamic invocation of files and functions

bool hphpIncludeFile(HPHP::HphpSession *s, const char *file) {
  try {
    hphp_ffi_include_file(file);
    return true;
  } catch (const HPHP::Exception &e) {
    HPHP::Logger::Error("%s", e.getMessage().c_str());
    const HPHP::ExtendedException *ee =
      dynamic_cast<const HPHP::ExtendedException *>(&e);
    if (ee) HPHP::ExtendedLogger::Log(true, *ee->getBackTrace());
    return false;
  }
}

HPHP::Variant *hphpInvoke(HPHP::HphpSession *s, const char *func,
                          HPHP::Variant *args) {
  try {
    void *result;
    int kind = hphp_ffi_invoke_function(&result, func, args->getArrayData());
    HPHP::Variant *ret = hphpBuildVariant(kind, result);
    s->addVariant(ret);
    return ret;
  } catch (const HPHP::Exception &e) {
    HPHP::Logger::Error("%s", e.getMessage().c_str());
    const HPHP::ExtendedException *ee =
      dynamic_cast<const HPHP::ExtendedException *>(&e);
    if (ee) HPHP::ExtendedLogger::Log(true, *ee->getBackTrace());
    return NULL;
  }
}

HPHP::Variant *hphpInvokeMethod(HPHP::HphpSession *s, HPHP::Variant *target,
                                const char *func, HPHP::Variant *args) {
  try {
    void *result;
    int kind = hphp_ffi_invoke_object_method(&result, target->getObjectData(),
                                             func, args->getArrayData());
    HPHP::Variant *ret = hphpBuildVariant(kind, result);
    s->addVariant(ret);
    return ret;
  } catch (const HPHP::Exception &e) {
    HPHP::Logger::Error("%s", e.getMessage().c_str());
    const HPHP::ExtendedException *ee =
      dynamic_cast<const HPHP::ExtendedException *>(&e);
    if (ee) HPHP::ExtendedLogger::Log(true, *ee->getBackTrace());
    return NULL;
  }
}

HPHP::Variant *hphpInvokeStaticMethod(HPHP::HphpSession *s, const char *cls,
                                      const char *func, HPHP::Variant *args) {
  try {
    void *result;
    int kind = hphp_ffi_invoke_static_method(&result, cls, func,
                                             args->getArrayData());
    HPHP::Variant *ret = hphpBuildVariant(kind, result);
    s->addVariant(ret);
    return ret;
  } catch (const HPHP::Exception &e) {
    HPHP::Logger::Error("%s", e.getMessage().c_str());
    const HPHP::ExtendedException *ee =
      dynamic_cast<const HPHP::ExtendedException *>(&e);
    if (ee) HPHP::ExtendedLogger::Log(true, *ee->getBackTrace());
    return NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// object operations

HPHP::Variant *hphpNewObject(HPHP::HphpSession *s, const char *cls,
                             HPHP::Variant *args) {
  void *result;
  int kind = hphp_ffi_create_object(&result, cls, args->getArrayData());
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpGetField(HPHP::HphpSession *s, HPHP::Variant *obj,
                            const char *field) {
  HPHP::Variant value = obj->getObjectData()->o_get(field, -1);
  void *result;
  int kind = hphp_ffi_exportVariant(value, &result);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

void hphpSetField(HPHP::HphpSession *s, HPHP::Variant *obj, const char *field,
                  HPHP::Variant *value) {
  obj->getObjectData()->o_set(field, -1, *value);
}

void hphpUnsetField(HPHP::HphpSession *s, HPHP::Variant *obj,
                    const char *field) {
  toObject(*obj)->t___unset(field);
}

bool hphpIssetField(HPHP::HphpSession *s, HPHP::Variant *obj,
                    const char *field) {
  return toObject(*obj)->t___isset(field);
}

///////////////////////////////////////////////////////////////////////////////
// global variables and constants

HPHP::Variant *hphpGetGlobal(HPHP::HphpSession *s, const char *name) {
  void *result;
  int kind = hphp_ffi_get_global(&result, name);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

void hphpSetGlobal(HPHP::HphpSession *s, const char *name,
                   HPHP::Variant *value) {
  hphp_ffi_set_global(name, value);
}

HPHP::Variant *hphpGetConstant(HPHP::HphpSession *s, const char *constant) {
  void *result;
  int kind = hphp_ffi_get_constant(&result, constant);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

HPHP::Variant *hphpGetClassConstant(HPHP::HphpSession *s, const char *cls,
                                    const char *constant) {
  void *result;
  int kind = hphp_ffi_get_class_constant(&result, cls, constant);
  HPHP::Variant *ret = hphpBuildVariant(kind, result);
  s->addVariant(ret);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// for debugging

void hphpDump(HPHP::HphpSession *s, HPHP::Variant *v) {
  v->dump();
}

///////////////////////////////////////////////////////////////////////////////
#endif /* __HPHP_SWIG_H__ */
