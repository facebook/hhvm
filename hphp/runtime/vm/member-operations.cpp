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
  static StringData* sd__offsetGet = makeStaticString("offsetGet");
  const Func* method = base->methodNamed(sd__offsetGet);
  assert(method != nullptr);
  g_vmContext->invokeFuncFew(&tvRef, method, base, nullptr, 1, offset.asCell());
  result = &tvRef;
  return result;
}

static bool objOffsetExists(ObjectData* base, CVarRef offset) {
  objArrayAccess(base);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  static StringData* sd__offsetExists
    = makeStaticString("offsetExists");
  assert(!base->isCollection());
  const Func* method = base->methodNamed(sd__offsetExists);
  assert(method != nullptr);
  g_vmContext->invokeFuncFew(&tvResult, method, base, nullptr, 1,
                             offset.asCell());
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
  return !cellToBool(*tvToCell(result));
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
  static StringData* sd__offsetSet = makeStaticString("offsetSet");
  assert(!base->isCollection());
  const Func* method = base->methodNamed(sd__offsetSet);
  assert(method != nullptr);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  TypedValue args[2] = { *offset.asCell(), *tvToCell(val) };
  g_vmContext->invokeFuncFew(&tvResult, method, base, nullptr, 2, args);
  tvRefcountedDecRef(&tvResult);
}

void objOffsetUnset(ObjectData* base, CVarRef offset) {
  objArrayAccess(base);
  static StringData* sd__offsetUnset
    = makeStaticString("offsetUnset");
  assert(!base->isCollection());
  const Func* method = base->methodNamed(sd__offsetUnset);
  assert(method != nullptr);
  TypedValue tv;
  tvWriteUninit(&tv);
  g_vmContext->invokeFuncFew(&tv, method, base, nullptr, 1, offset.asCell());
  tvRefcountedDecRef(&tv);
}

///////////////////////////////////////////////////////////////////////////////
}
