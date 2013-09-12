/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/code-gen-helpers.h"

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP { namespace JIT {

ALWAYS_INLINE
int64_t ak_exist_string_impl(ArrayData* arr, StringData* key) {
  int64_t n;
  if (key->isStrictlyInteger(n)) {
    return arr->exists(n);
  }
  return arr->exists(key);
}

HOT_FUNC_VM
int64_t ak_exist_string(ArrayData* arr, StringData* key) {
  return ak_exist_string_impl(arr, key);
}

HOT_FUNC_VM
int64_t ak_exist_int(ArrayData* arr, int64_t key) {
  bool res = arr->exists(key);
  return res;
}

HOT_FUNC_VM
int64_t ak_exist_string_obj(ObjectData* obj, StringData* key) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  int64_t res = ak_exist_string_impl(arr.get(), key);
  return res;
}

HOT_FUNC_VM
int64_t ak_exist_int_obj(ObjectData* obj, int64_t key) {
  if (obj->isCollection()) {
    return collectionOffsetContains(obj, key);
  }
  CArrRef arr = obj->o_toArray();
  bool res = arr.get()->exists(key);
  return res;
}

ALWAYS_INLINE
TypedValue& getDefaultIfNullCell(TypedValue* tv, TypedValue& def) {
  if (UNLIKELY(nullptr == tv)) {
    // refcount is already correct since def was never decrefed
    return def;
  }
  tvRefcountedDecRef(&def);
  TypedValue* ret = tvToCell(tv);
  tvRefcountedIncRef(ret);
  return *ret;
}

HOT_FUNC_VM
TypedValue arrayIdxS(ArrayData* a, StringData* key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue arrayIdxSi(ArrayData* a, StringData* key, TypedValue def) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ?
         getDefaultIfNullCell(a->nvGet(i), def) :
         getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue arrayIdxI(ArrayData* a, int64_t key, TypedValue def) {
  return getDefaultIfNullCell(a->nvGet(key), def);
}

HOT_FUNC_VM
TypedValue* ldGblAddrHelper(StringData* name) {
  return g_vmContext->m_globalVarEnv->lookup(name);
}

HOT_FUNC_VM
TypedValue* ldGblAddrDefHelper(StringData* name) {
  TypedValue* r = g_vmContext->m_globalVarEnv->lookupAdd(name);
  decRefStr(name);
  return r;
}

}}
