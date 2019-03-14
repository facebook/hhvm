/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/system/systemlib.h"

#include <type_traits>

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

void unknownBaseType(DataType type) {
  always_assert_flog(
    false,
    "Unknown KindOf: {} in member operation base",
    static_cast<uint8_t>(type)
  );
}

void objArrayAccess(ObjectData* base) {
  assertx(!base->isCollection());
  if (!base->instanceof(SystemLib::s_ArrayAccessClass)) {
    raise_error("Object does not implement ArrayAccess");
  }
  if (RuntimeOption::EvalNoticeOnArrayAccessUse) {
    raise_notice("ArrayAccess via Object of class %s",
                 base->getClassName().data());
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
  assertx(!isRefType(offset.m_type));

  auto const method = base->methodNamed(s_offsetGet.get());
  assertx(method != nullptr);

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
  assertx(!isRefType(offset.m_type));

  auto const method = base->methodNamed(s_offsetExists.get());
  assertx(method != nullptr);

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

bool objOffsetIsset(ObjectData* base, TypedValue offset) {
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
  assertx(method != nullptr);

  auto result = g_context->invokeMethodV(base, method, InvokeArgs(&offset, 1));
  return !result.isNull();
}

bool objOffsetEmpty(ObjectData* base, TypedValue offset) {
  if (objOffsetExists(base, offset) == OffsetExistsResult::DoesNotExist) {
    return true;
  }

  auto value = objOffsetGet(base, offset, false);
  auto result = !cellToBool(*tvToCell(&value));
  tvDecRefGen(value);
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
  assertx(!isRefType(offset.m_type));

  auto const method = base->methodNamed(s_offsetSet.get());
  assertx(method != nullptr);

  TypedValue args[2] = { offset, *tvToCell(val) };
  g_context->invokeMethodV(base, method, folly::range(args));
}

void objOffsetUnset(ObjectData* base, TypedValue offset) {
  objArrayAccess(base);

  assertx(!base->isCollection());
  assertx(!isRefType(offset.m_type));

  auto const method = base->methodNamed(s_offsetUnset.get());
  assertx(method != nullptr);

  g_context->invokeMethodV(base, method, InvokeArgs(&offset, 1));
}

// Mutable collections support appending new elements using [] without a key
// like so: "$vector[] = 123;". However, collections do not support using []
// without a key to implicitly create a new element without supplying assigning
// an initial value (ex "$vector[]['a'] = 73;").
void throw_cannot_use_newelem_for_lval_read_col() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with collections for reading in an lvalue context");
}

void throw_cannot_use_newelem_for_lval_read_vec() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with vecs for reading in an lvalue context"
  );
}

void throw_cannot_use_newelem_for_lval_read_dict() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with dicts for reading in an lvalue context"
  );
}

void throw_cannot_use_newelem_for_lval_read_keyset() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with keysets for reading in an lvalue context"
  );
}

void throw_cannot_use_newelem_for_lval_read_clsmeth() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with clsmeth for reading in an lvalue context"
  );
}

void throw_cannot_write_for_clsmeth() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use [] with clsmeth for writing");
}

void throw_cannot_unset_for_clsmeth() {
  SystemLib::throwInvalidOperationExceptionObject(
    "Cannot use unset with clsmeth");
}

void raise_inout_undefined_index(TypedValue tv) {
  if (tv.m_type == KindOfInt64) {
    raise_inout_undefined_index(tv.m_data.num);
    return;
  }
  assertx(isStringType(tv.m_type));
  raise_inout_undefined_index(tv.m_data.pstr);
}

void raise_inout_undefined_index(int64_t i) {
  raise_notice("Undefined index on inout parameter: %" PRId64, i);
}

void raise_inout_undefined_index(const StringData* sd) {
  raise_notice("Undefined index on inout parameter: %s", sd->data());
}

Cell incDecBodySlow(IncDecOp op, tv_lval fr) {
  assertx(cellIsPlausible(*fr));
  assertx(type(fr) != KindOfUninit);

  auto dup = [&]() { tvIncRefGen(*fr); return *fr; };

  switch (op) {
  case IncDecOp::PreInc:
    cellInc(fr);
    return dup();
  case IncDecOp::PostInc: {
    auto const tmp = dup();
    cellInc(fr);
    return tmp;
  }
  case IncDecOp::PreDec:
    cellDec(fr);
    return dup();
  case IncDecOp::PostDec: {
    auto const tmp = dup();
    cellDec(fr);
    return tmp;
  }
  case IncDecOp::PreIncO:
    cellIncO(fr);
    return dup();
  case IncDecOp::PostIncO: {
    auto const tmp = dup();
    cellIncO(fr);
    return tmp;
  }
  case IncDecOp::PreDecO:
    cellDecO(fr);
    return dup();
  case IncDecOp::PostDecO: {
    auto const tmp = dup();
    cellDecO(fr);
    return tmp;
  }
  }
  not_reached();
}

namespace {
ALWAYS_INLINE
void copy_int(void* dest, int64_t src, size_t size) {
  if (size == 1) {
    uint8_t i = src;
    memcpy(dest, &i, 1);
  } else if (size == 2) {
    uint16_t i = src;
    memcpy(dest, &i, 2);
  } else if (size == 4) {
    uint32_t i = src;
    memcpy(dest, &i, 4);
  } else {
    assertx(size == 8);
    uint64_t i = src;
    memcpy(dest, &i, 8);
  }
}

template<typename... Args>
[[noreturn]] NEVER_INLINE
void fail_oob(folly::StringPiece fmt, Args&&... args) {
  SystemLib::throwOutOfBoundsExceptionObject(
    folly::sformat(fmt, std::forward<Args>(args)...)
  );
}

template<typename... Args>
[[noreturn]] NEVER_INLINE
void fail_invalid(folly::StringPiece fmt, Args&&... args) {
  SystemLib::throwInvalidOperationExceptionObject(
    folly::sformat(fmt, std::forward<Args>(args)...)
  );
}

template<bool reverse, typename F>
void setRangeString(
  char* dest, TypedValue src, int64_t count, int64_t size, F range_check
) {
  auto const src_str = val(src).pstr;
  if (count == -1) {
    count = src_str->size();
  } else if (count < 0 || count > src_str->size()) {
    fail_oob("Cannot read {}-byte range from {}-byte source string",
             count, src_str->size());
  }
  if (size != 1) {
    fail_invalid("Invalid size {} for string source", size);
  }

  range_check(count);
  auto const data = src_str->data();
  if (reverse) {
    auto end = data + count;
    for (int64_t i = 0; i < count % 8; ++i) {
      *dest++ = *--end;
    }
    for (int64_t i = 0; i < count / 8; ++i) {
      end -= 8;
      uint64_t tmp;
      memcpy(&tmp, end, 8);
      tmp = __builtin_bswap64(tmp);
      memcpy(dest, &tmp, 8);
      dest += 8;
    }
  } else  {
    memcpy(dest, data, count);
  }
}

/*
 * The main vec loop is a template to avoid calling memcpy() with a
 * non-constant size, and to avoid the check for KindOfDouble in the more
 * common int case.
 */
template<bool reverse, int64_t size, DataType elem_type>
void setRangeVecLoop(char* dest, const TypedValue* vec_data, int64_t count) {
  static_assert(
    (isBoolType(elem_type) && size == 1) ||
    (isIntType(elem_type) &&
     (size == 1 || size == 2 || size == 4 || size == 8)) ||
    (isDoubleType(elem_type) && (size == 4 || size == 8)),
    "Unsupported elem_type and/or size"
  );

  if (reverse) dest += (count - 1) * size;
  for (int64_t i = 0; i < count; ++i) {
    auto elem = vec_data + i;
    if (UNLIKELY(type(elem) != elem_type)) {
      fail_invalid(
        "Multiple types in vec source: {} at index 0; {} at index {}",
        getDataTypeString(elem_type).data(),
        getDataTypeString(type(elem)).data(),
        i
      );
    }

    if (isDoubleType(elem_type)) {
      std::conditional_t<size == 4, float, double> f = val(elem).dbl;
      memcpy(dest, &f, size);
    } else {
      copy_int(dest, val(elem).num, size);
    }
    dest += reverse ? -size : size;
  }
}

template<bool reverse, typename F>
void setRangeVec(
  char* dest, TypedValue src, int64_t count, int64_t size, F range_check
) {
  auto const vec = val(src).parr;
  auto const vec_size = vec->size();
  if (count == -1) {
    count = vec_size;
  } else if (count < 0 || count > vec_size) {
    fail_oob("Cannot read {} elements from vec of size {}",
             count, vec_size);
  }

  range_check(count * size);
  auto const vec_data = packedData(vec);
  auto const elem_type = type(vec_data[0]);
  auto bad_type = [&]() {
    fail_invalid(
      "Bad type ({}) and element size ({}) combination in vec source",
      getDataTypeString(elem_type).data(), size
    );
  };

  switch (size) {
  case 1:
    if (isIntType(elem_type)) {
      setRangeVecLoop<reverse, 1, KindOfInt64>(dest, vec_data, count);
    } else if (isBoolType(elem_type)) {
      setRangeVecLoop<reverse, 1, KindOfBoolean>(dest, vec_data, count);
    } else {
      bad_type();
    }
    break;

  case 2:
    if (isIntType(elem_type)) {
      setRangeVecLoop<reverse, 2, KindOfInt64>(dest, vec_data, count);
    } else {
      bad_type();
    }
    break;

  case 4:
    if (isIntType(elem_type)) {
      setRangeVecLoop<reverse, 4, KindOfInt64>(dest, vec_data, count);
    } else if (isDoubleType(elem_type)) {
      setRangeVecLoop<reverse, 4, KindOfDouble>(dest, vec_data, count);
    } else {
      bad_type();
    }
    break;

  case 8:
    if (isIntType(elem_type)) {
      setRangeVecLoop<reverse, 8, KindOfInt64>(dest, vec_data, count);
    } else if (isDoubleType(elem_type)) {
      setRangeVecLoop<reverse, 8, KindOfDouble>(dest, vec_data, count);
    } else {
      bad_type();
    }
    break;

  default:
    not_reached();
  }
}
}

template<bool reverse>
void SetRange(
  tv_lval base, int64_t offset, TypedValue src, int64_t count, int64_t size
) {
  base = tvToCell(base);
  if (!tvIsString(base)) {
    fail_invalid("Invalid base type {} for range set operation",
                 getDataTypeString(type(base)).data());
  }
  if (count == 0) return;
  type(base) = KindOfString;

  auto& base_str = val(base).pstr;
  if (base_str->cowCheck()) {
    auto const old_str = base_str;
    base_str = StringData::Make(old_str, CopyStringMode{});
    FOLLY_SDT(hhvm, hhvm_cow_setrange, base_str->size(), size*count);
    decRefStr(old_str);
  } else {
    FOLLY_SDT(hhvm, hhvm_mut_setrange, base_str->size(), size*count);
  }

  auto range_check = [&](size_t data_len) {
    if (offset < 0 || offset + data_len > base_str->size()) {
      fail_oob(
        "Cannot set {}-byte range at offset {} in string of length {}",
        data_len,
        offset,
        base_str->size()
      );
    }
  };

  auto dest = base_str->mutableData() + offset;

  auto fail_if_reverse = [&]() {
    if (reverse) {
      fail_invalid("Cannot set reverse range for primitive type {}",
                   getDataTypeString(type(src)).data());
    }
  };

  if (tvIsBool(src)) {
    fail_if_reverse();

    if (size != 1) {
      fail_invalid("Invalid size {} for bool source", size);
    }

    range_check(1);
    copy_int(dest, val(src).num, 1);
  } else if (tvIsInt(src)) {
    fail_if_reverse();

    range_check(size);
    copy_int(dest, val(src).num, size);
  } else if (tvIsDouble(src)) {
    fail_if_reverse();

    if (size == 4) {
      float f = val(src).dbl;
      range_check(4);
      memcpy(dest, &f, 4);
    } else if (size == 8) {
      range_check(8);
      memcpy(dest, &val(src).dbl, 8);
    } else {
      fail_invalid("Invalid size {} for double source", size);
    }
  } else if (tvIsString(src)) {
    setRangeString<reverse>(dest, src, count, size, range_check);
  } else if (tvIsVec(src)) {
    setRangeVec<reverse>(dest, src, count, size, range_check);
  } else {
    fail_invalid("Invalid source type %s for range set operation",
                 tname(type(src)));
  }
}

template void SetRange<true>(tv_lval, int64_t, TypedValue, int64_t, int64_t);
template void SetRange<false>(tv_lval, int64_t, TypedValue, int64_t, int64_t);

///////////////////////////////////////////////////////////////////////////////
}
