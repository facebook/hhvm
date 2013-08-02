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

#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP {

StringData* prepareAnyKey(TypedValue* tv) {
  if (IS_STRING_TYPE(tv->m_type)) {
    StringData* str = tv->m_data.pstr;
    str->incRefCount();
    return str;
  } else {
    return tvAsCVarRef(tv).toString().detach();
  }
}

void objArrayAccess(ObjectData* base) {
  assert(!base->isCollection());
  if (!base->getVMClass()->classof(SystemLib::s_ArrayAccessClass)) {
    raise_error("Object does not implement ArrayAccess");
  }
}

TypedValue* objOffsetGet(TypedValue& tvRef, ObjectData* base,
                         CVarRef offset, bool validate /* = true */) {
  if (validate) {
    objArrayAccess(base);
  }
  TypedValue* result;
  assert(!base->isCollection());
  static StringData* sd__offsetGet = StringData::GetStaticString("offsetGet");
  const Func* method = base->methodNamed(sd__offsetGet);
  assert(method != nullptr);
  base->invokeUserMethod(&tvRef, method, CREATE_VECTOR1(offset));
  result = &tvRef;
  return result;
}

static bool objOffsetExists(ObjectData* base, CVarRef offset) {
  objArrayAccess(base);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  static StringData* sd__offsetExists
    = StringData::GetStaticString("offsetExists");
  assert(!base->isCollection());
  const Func* method = base->methodNamed(sd__offsetExists);
  assert(method != nullptr);
  base->invokeUserMethod(&tvResult, method, CREATE_VECTOR1(offset));
  tvCastToBooleanInPlace(&tvResult);
  return bool(tvResult.m_data.num);
}

bool objOffsetIsset(TypedValue& tvRef, ObjectData* base, CVarRef offset,
                    bool validate /* = true */) {
  return objOffsetExists(base, offset);
}

bool objOffsetEmpty(TypedValue& tvRef, ObjectData* base, CVarRef offset,
                    bool validate /* = true */) {
  if (!objOffsetExists(base, offset)) {
    return true;
  }
  TypedValue* result = objOffsetGet(tvRef, base, offset, false);
  assert(result);
  return empty(tvAsCVarRef(result));
}

void objOffsetAppend(ObjectData* base, TypedValue* val,
                     bool validate /* = true */) {
  assert(!base->isCollection());
  if (validate) {
    objArrayAccess(base);
  }
  objOffsetSet(base, init_null_variant, val, false);
}

void objOffsetSet(ObjectData* base, CVarRef offset, TypedValue* val,
                  bool validate /* = true */) {
  if (validate) {
    objArrayAccess(base);
  }
  static StringData* sd__offsetSet = StringData::GetStaticString("offsetSet");
  assert(!base->isCollection());
  const Func* method = base->methodNamed(sd__offsetSet);
  assert(method != nullptr);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  base->invokeUserMethod(&tvResult, method,
                         CREATE_VECTOR2(offset, tvAsCVarRef(val)));
  tvRefcountedDecRef(&tvResult);
}

void objOffsetUnset(ObjectData* base, CVarRef offset) {
  objArrayAccess(base);
  static StringData* sd__offsetUnset
    = StringData::GetStaticString("offsetUnset");
  assert(!base->isCollection());
  const Func* method = base->methodNamed(sd__offsetUnset);
  assert(method != nullptr);
  TypedValue tv;
  tvWriteUninit(&tv);
  base->invokeUserMethod(&tv, method, CREATE_VECTOR1(offset));
  tvRefcountedDecRef(&tv);
}

///////////////////////////////////////////////////////////////////////////////
}
