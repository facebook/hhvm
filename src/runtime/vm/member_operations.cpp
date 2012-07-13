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

#include "runtime/vm/member_operations.h"

namespace HPHP {
namespace VM {

void objArrayAccess(TypedValue* base) {
  ASSERT(base->m_type == KindOfObject);
  if (!instanceOf(tvAsCVarRef(base), "ArrayAccess")) {
    raise_error("Object does not implement ArrayAccess");
  }
}

TypedValue* objOffsetGet(TypedValue& tvRef, TypedValue* base,
                         CVarRef offset, bool validate /* = true */) {
  if (validate) {
    objArrayAccess(base);
  }
  TypedValue* result;
  ObjectData* obj = base->m_data.pobj;
  Instance* instance = static_cast<Instance*>(obj);
  static StringData* sd__offsetGet = StringData::GetStaticString("offsetGet");
  const Func* method = instance->methodNamed(sd__offsetGet);
  ASSERT(method != NULL);
  instance->invokeUserMethod(&tvRef, method, CREATE_VECTOR1(offset));
  result = &tvRef;
  return result;
}

bool objOffsetExists(TypedValue* base, CVarRef offset) {
  objArrayAccess(base);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  static StringData* sd__offsetExists
    = StringData::GetStaticString("offsetExists");
  ObjectData* obj = base->m_data.pobj;
  Instance* instance = static_cast<Instance*>(obj);
  const Func* method = instance->methodNamed(sd__offsetExists);
  ASSERT(method != NULL);
  instance->invokeUserMethod(&tvResult, method, CREATE_VECTOR1(offset));
  tvCastToBooleanInPlace(&tvResult);
  return bool(tvResult.m_data.num);
}

void objOffsetSet(TypedValue* base, CVarRef offset, TypedValue* val,
                  bool validate /* = true */) {
  if (validate) {
    objArrayAccess(base);
  }
  static StringData* sd__offsetSet = StringData::GetStaticString("offsetSet");
  ObjectData* obj = base->m_data.pobj;
  Instance* instance = static_cast<Instance*>(obj);
  const Func* method = instance->methodNamed(sd__offsetSet);
  ASSERT(method != NULL);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  instance->invokeUserMethod(&tvResult, method,
                             CREATE_VECTOR2(offset, tvAsCVarRef(val)));
  tvRefcountedDecRef(&tvResult);
}

void objOffsetUnset(TypedValue* base, CVarRef offset) {
  objArrayAccess(base);
  static StringData* sd__offsetUnset
    = StringData::GetStaticString("offsetUnset");
  ObjectData* obj = base->m_data.pobj;
  Instance* instance = static_cast<Instance*>(obj);
  const Func* method = instance->methodNamed(sd__offsetUnset);
  ASSERT(method != NULL);
  TypedValue tv;
  tvWriteUninit(&tv);
  instance->invokeUserMethod(&tv, method, CREATE_VECTOR1(offset));
  tvRefcountedDecRef(&tv);
}

///////////////////////////////////////////////////////////////////////////////
}
}
