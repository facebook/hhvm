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

#include <runtime/base/type_conversions.h>
#include <runtime/base/hphp_ffi.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/string_data.h>
#include <runtime/base/object_data.h>
#include <runtime/base/externals.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/variable_table.h>
#include <runtime/base/thread_init_fini.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

void hphp_ffi_ArrayData_decRef(ArrayData *p) {
  if (!p->decRefCount()) {
    p->release();
  }
}
void hphp_ffi_StringData_decRef(StringData *p) {
  if (!p->decRefCount()) {
    p->release();
  }
}
void hphp_ffi_ObjectData_decRef(ObjectData *p) {
  if (!p->decRefCount()) {
    p->release();
  }
}
int hphp_ffi_exportVariant(CVarRef v, void** result) {
  switch (v.getType()) {
  case KindOfUninit:
  case KindOfNull:    return 0;
  case KindOfBoolean: *result = (void*)v.toBoolean(); return 1;
  case KindOfInt32:
  case KindOfInt64: {
    *result = (void*)v.toInt64();
    return 2;
  }
  case KindOfDouble: {
    union {
      double d;
      void* p;
    } u;
    u.d = v.toDouble();
    *result = u.p;
    return 3;
  }
  case KindOfStaticString: {
    StringData *sd = v.getStringData();
    *result = (void*)sd;
    return 5;
  }
  case KindOfString: {
    StringData *sd = v.getStringData();
    sd->incRefCount();
    *result = (void*)sd;
    return 5;
  }
  case KindOfArray: {
    ArrayData *ad = v.getArrayData();
    ad->incRefCount();
    *result = (void*)ad;
    return 6;
  }
  case KindOfObject: {
    ObjectData *od = v.getObjectData();
    od->incRefCount();
    *result = (void*)od;
    return 7;
  }
  default:
    ASSERT(false);
    return 0;
  }
}

Variant *hphp_ffi_buildVariant(int t, void* v, int len) {
  switch (t) {
  case 0: return NEW(Variant)();
  case 1: return NEW(Variant)((bool)v);
  case 2: return NEW(Variant)((int64)v);
  case 3: {
    union {
      double d;
      void* p;
    } u;
    u.p = v;
    return NEW(Variant)(u.d);
  }
  case 4: return NEW(Variant)((char*)v);
  case 5: return NEW(Variant)(String((StringData*)v));
  case 6:
    return NEW(Variant)
      (String(NEW(StringData)((const char*)v, len, CopyString)));
  case 7: return NEW(Variant)((ArrayData*)v);
  case 8: return NEW(Variant)((ObjectData*)v);
  case 9: return NEW(Variant)(Array::Create());
  }
  ASSERT(false);
  return NULL;
}

void hphp_ffi_freeVariant(Variant *v) {
  DELETE(Variant)(v);
}

void hphp_ffi_addMapItem(Variant *map, Variant *key, Variant *val) {
  map->set(*key, *val);
}

int hphp_ffi_getMapItem(void **result, Variant *map, Variant *key) {
  return hphp_ffi_exportVariant(map->rvalAt(*key), result);
}

long long hphp_ffi_iter_begin(ArrayData *arr) {
  return arr->iter_begin();
}
long long hphp_ffi_iter_advance(ArrayData *arr, long long pos) {
  return arr->iter_advance(pos);
}
long long hphp_ffi_iter_end(ArrayData *arr) {
  return arr->iter_end();
}
int hphp_ffi_iter_invalid(long long pos) {
  return pos == ArrayData::invalid_index;
}

int hphp_ffi_iter_getKey(ArrayData *arr, long long pos, void** res) {
  return hphp_ffi_exportVariant(arr->getKey(pos), res);
}
int hphp_ffi_iter_getValue(ArrayData *arr, long long pos, void** res) {
  return hphp_ffi_exportVariant(arr->getValue(pos), res);
}

int hphp_ffi_string_data(StringData *sd, const char** data) {
  *data = sd->data();
  return sd->size();
}

void hphp_ffi_include_file(const char* file) {
  include_impl_invoke(file);
}

int hphp_ffi_invoke_function(void** ret, const char* func, ArrayData* args) {
  Array argsArr(args);
  Variant result = invoke(func, argsArr, -1);
  return hphp_ffi_exportVariant(result, ret);
}

int hphp_ffi_invoke_static_method(void** ret, const char* cls,
                                  const char* func, ArrayData* args) {
  Array argsArr(args);
  Variant result = invoke_static_method(cls, func, argsArr);
  return hphp_ffi_exportVariant(result, ret);
}

int hphp_ffi_invoke_object_method(void** ret, ObjectData* receiver,
                                  const char* func, ArrayData* args) {
  Array argsArr(args);
  Variant result = receiver->o_invoke(func, argsArr, -1);
  return hphp_ffi_exportVariant(result, ret);
}

int hphp_ffi_create_object(void **ret, const char *cls, ArrayData *args) {
  Array argsArr(args);
  Object result = create_object(cls, argsArr);
  return hphp_ffi_exportVariant(result, ret);
}

int hphp_ffi_get_global(void **ret, const char *name) {
  LVariableTable *g = get_variable_table();
  Variant result = g->get(name);
  return hphp_ffi_exportVariant(result, ret);
}

void hphp_ffi_set_global(const char *name, Variant *value) {
  LVariableTable *g = get_variable_table();
  g->get(name) = *value;
}

int hphp_ffi_get_constant(void **ret, const char *constant) {
  Variant result = get_constant(constant);
  return hphp_ffi_exportVariant(result, ret);
}

int hphp_ffi_get_class_constant(void **ret, const char *cls,
                                const char *constant) {
  Variant result = get_class_constant(cls, constant);
  return hphp_ffi_exportVariant(result, ret);
}

void hphp_ffi_init() {
  RuntimeOption::SourceRoot = "/";
  init_thread_locals();
  init_static_variables();
}

void hphp_ffi_init_globals() {
  init_global_variables();
}

void hphp_ffi_free_globals() {
  free_global_variables();
}

void hphp_ffi_session_init() {
  hphp_session_init();
}

void hphp_ffi_session_exit() {
  hphp_session_exit();
}

ExecutionContext *hphp_ffi_context_init() {
  return hphp_context_init();
}

void hphp_ffi_context_exit(ExecutionContext *context) {
  hphp_context_exit(context, true);
}

///////////////////////////////////////////////////////////////////////////////
