/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
  if (isStringType(tv->m_type)) {
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
  if (!base->instanceof(SystemLib::s_ArrayAccessClass)) {
    raise_error("Object does not implement ArrayAccess");
  }
}

TypedValue objOffsetGet(
  ObjectData* base,
  TypedValue offset,
  bool validate /* = true */
) {
  if (validate) {
    objArrayAccess(base);
  }

  assertx(!base->isCollection());
  assertx(offset.m_type != KindOfRef);

  auto const method = base->methodNamed(s_offsetGet.get());
  assert(method != nullptr);

  return g_context->invokeMethod(base, method, InvokeArgs(&offset, 1));
}

enum class OffsetExistsResult {
  DoesNotExist = 0,
  DefinitelyExists = 1,
  IssetIfNonNull = 2
};

static OffsetExistsResult objOffsetExists(ObjectData* base, TypedValue offset) {
  objArrayAccess(base);

  assertx(!base->isCollection());
  assertx(offset.m_type != KindOfRef);

  auto const method = base->methodNamed(s_offsetExists.get());
  assert(method != nullptr);

  auto result = g_context->invokeMethod(base, method, InvokeArgs(&offset, 1));
  // In-place cast decrefs the function call result.
  tvCastToBooleanInPlace(&result);

  if (!result.m_data.num) {
    return OffsetExistsResult::DoesNotExist;
  }

  return method->cls() == SystemLib::s_ArrayObjectClass
    ? OffsetExistsResult::IssetIfNonNull
    : OffsetExistsResult::DefinitelyExists;
}

bool objOffsetIsset(
  ObjectData* base,
  TypedValue offset,
  bool validate /* = true */
) {
  auto exists = objOffsetExists(base, offset);

  // Unless we called ArrayObject::offsetExists, there's nothing more to do.
  if (exists != OffsetExistsResult::IssetIfNonNull) {
    return (int)exists;
  }

  // For ArrayObject::offsetExists, we need to check the value at `offset`.  If
  // it's null, then we return false.  We can't call the offsetGet method on
  // `base` because users aren't expecting offsetGet to be called for
  // `isset(...)` expressions, so call the method on the base ArrayObject class.
  auto const cls = SystemLib::s_ArrayObjectClass;
  auto const method = cls->lookupMethod(s_offsetGet.get());
  assert(method != nullptr);

  auto result = g_context->invokeMethodV(base, method, InvokeArgs(&offset, 1));
  return !result.isNull();
}

bool objOffsetEmpty(
  ObjectData* base,
  TypedValue offset,
  bool validate /* = true */
) {
  if (objOffsetExists(base, offset) == OffsetExistsResult::DoesNotExist) {
    return true;
  }

  auto value = objOffsetGet(base, offset, false);
  auto result = !cellToBool(*tvToCell(&value));
  tvRefcountedDecRef(value);
  return result;
}

void objOffsetAppend(
  ObjectData* base,
  TypedValue* val,
  bool validate /* = true */
) {
  assertx(!base->isCollection());
  if (validate) {
    objArrayAccess(base);
  }
  objOffsetSet(base, make_tv<KindOfNull>(), val, false);
}

void objOffsetSet(
  ObjectData* base,
  TypedValue offset,
  TypedValue* val,
  bool validate /* = true */
) {
  if (validate) {
    objArrayAccess(base);
  }

  assertx(!base->isCollection());
  assertx(offset.m_type != KindOfRef);

  auto const method = base->methodNamed(s_offsetSet.get());
  assert(method != nullptr);

  TypedValue args[2] = { offset, *tvToCell(val) };
  g_context->invokeMethodV(base, method, folly::range(args));
}

void objOffsetUnset(ObjectData* base, TypedValue offset) {
  objArrayAccess(base);

  assertx(!base->isCollection());
  assertx(offset.m_type != KindOfRef);

  auto const method = base->methodNamed(s_offsetUnset.get());
  assert(method != nullptr);

  g_context->invokeMethodV(base, method, InvokeArgs(&offset, 1));
}

// Mutable collections support appending new elements using [] without a key
// like so: "$vector[] = 123;". However, collections do not support using []
// without a key to implicitly create a new element without supplying assigning
// an initial value (ex "$vector[]['a'] = 73;").
void throw_cannot_use_newelem_for_lval_read() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with collections for reading in an lvalue context");
}

void incDecBodySlow(IncDecOp op, Cell* fr, TypedValue* to) {
  assert(cellIsPlausible(*fr));
  assert(fr->m_type != KindOfUninit);

  auto dup = [&]() { cellDup(*fr, *to); };

  switch (op) {
  case IncDecOp::PreInc:
    cellInc(*fr);
    dup();
    return;
  case IncDecOp::PostInc:
    dup();
    cellInc(*fr);
    return;
  case IncDecOp::PreDec:
    cellDec(*fr);
    dup();
    return;
  case IncDecOp::PostDec:
    dup();
    cellDec(*fr);
    return;
  default: break;
  }

  switch (op) {
  case IncDecOp::PreIncO:
    cellIncO(*fr);
    dup();
    return;
  case IncDecOp::PostIncO:
    dup();
    cellIncO(*fr);
    return;
  case IncDecOp::PreDecO:
    cellDecO(*fr);
    dup();
    return;
  case IncDecOp::PostDecO:
    dup();
    cellDecO(*fr);
    return;
  default: break;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
