/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP {

const StaticString
  s_offsetGet("offsetGet"),
  s_offsetSet("offsetSet"),
  s_offsetUnset("offsetUnset"),
  s_offsetExists("offsetExists");

StringData* prepareAnyKey(TypedValue* tv) {
  if (IS_STRING_TYPE(tv->m_type)) {
    StringData* str = tv->m_data.pstr;
    str->incRefCount();
    return str;
  } else {
    return tvAsCVarRef(tv).toString().detach();
  }
}

void unknownBaseType(const TypedValue* tv) {
  always_assert_flog(
    false,
    "Unknown KindOf: {} in member operation base",
    static_cast<uint8_t>(tv->m_type));
}

void objArrayAccess(ObjectData* base) {
  assert(!base->isCollection());
  if (!base->getVMClass()->classof(SystemLib::s_ArrayAccessClass)) {
    raise_error("Object does not implement ArrayAccess");
  }
}

TypedValue* objOffsetGet(TypedValue& tvRef, ObjectData* base,
                         const Variant& offset, bool validate /* = true */) {
  if (validate) {
    objArrayAccess(base);
  }
  TypedValue* result;
  assert(!base->isCollection());
  const Func* method = base->methodNamed(s_offsetGet.get());
  assert(method != nullptr);
  g_context->invokeFuncFew(&tvRef, method, base, nullptr, 1, offset.asCell());
  result = &tvRef;
  return result;
}

enum class OffsetExistsResult {
  DoesNotExist = 0,
  DefinitelyExists = 1,
  IssetIfNonNull = 2
};

static OffsetExistsResult objOffsetExists(ObjectData* base,
                                          const Variant& offset) {
  objArrayAccess(base);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  assert(!base->isCollection());
  const Func* method = base->methodNamed(s_offsetExists.get());
  assert(method != nullptr);
  g_context->invokeFuncFew(&tvResult, method, base, nullptr, 1,
                             offset.asCell());
  tvCastToBooleanInPlace(&tvResult);
  if (!tvResult.m_data.num) return OffsetExistsResult::DoesNotExist;
  return method->cls() == SystemLib::s_ArrayObjectClass ?
    OffsetExistsResult::IssetIfNonNull : OffsetExistsResult::DefinitelyExists;
}

bool objOffsetIsset(TypedValue& tvRef, ObjectData* base, const Variant& offset,
                    bool validate /* = true */) {
  auto exists = objOffsetExists(base, offset);

  // Unless we called ArrayObject::offsetExists, there's nothing more to do
  if (exists != OffsetExistsResult::IssetIfNonNull) {
    return (int)exists;
  }

  // For ArrayObject::offsetExists, we need to check the value at `offset`.
  // If it's null, then we return false.
  TypedValue tvResult;
  tvWriteUninit(&tvResult);

  // We can't call the offsetGet method on `base` because users aren't
  // expecting offsetGet to be called for `isset(...)` expressions, so call
  // the method on the base ArrayObject class.
  auto const method =
    SystemLib::s_ArrayObjectClass->lookupMethod(s_offsetGet.get());
  assert(method != nullptr);
  g_context->invokeFuncFew(&tvResult, method, base, nullptr, 1,
                           offset.asCell());
  auto const result = !IS_NULL_TYPE(tvResult.m_type);
  tvRefcountedDecRef(&tvResult);
  return result;
}

bool objOffsetEmpty(TypedValue& tvRef, ObjectData* base, const Variant& offset,
                    bool validate /* = true */) {
  if (objOffsetExists(base, offset) == OffsetExistsResult::DoesNotExist) {
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

void objOffsetSet(ObjectData* base, const Variant& offset, TypedValue* val,
                  bool validate /* = true */) {
  if (validate) {
    objArrayAccess(base);
  }
  assert(!base->isCollection());
  const Func* method = base->methodNamed(s_offsetSet.get());
  assert(method != nullptr);
  TypedValue tvResult;
  tvWriteUninit(&tvResult);
  TypedValue args[2] = { *offset.asCell(), *tvToCell(val) };
  g_context->invokeFuncFew(&tvResult, method, base, nullptr, 2, args);
  tvRefcountedDecRef(&tvResult);
}

void objOffsetUnset(ObjectData* base, const Variant& offset) {
  objArrayAccess(base);
  assert(!base->isCollection());
  const Func* method = base->methodNamed(s_offsetUnset.get());
  assert(method != nullptr);
  TypedValue tv;
  tvWriteUninit(&tv);
  g_context->invokeFuncFew(&tv, method, base, nullptr, 1, offset.asCell());
  tvRefcountedDecRef(&tv);
}

// Mutable collections support appending new elements using [] without a key
// like so: "$vector[] = 123;". However, collections do not support using []
// without a key to implicitly create a new element without supplying assigning
// an initial value (ex "$vector[]['a'] = 73;").
void throw_cannot_use_newelem_for_lval_read() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with collections for reading in an lvalue context");
}

///////////////////////////////////////////////////////////////////////////////
}
