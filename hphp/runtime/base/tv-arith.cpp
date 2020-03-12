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
#include "hphp/runtime/base/tv-arith.h"

#include <type_traits>
#include <limits>
#include <algorithm>

#include <folly/CPortability.h>
#include <folly/ScopeGuard.h>
#include <folly/tracing/StaticTracepoint.h>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/overflow.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

[[noreturn]] NEVER_INLINE
void throw_bad_array_operand(const ArrayData* ad) {
  const char* type = [&]{
    if (ad->isVecArrayType()) return "vecs";
    if (ad->isDictType()) return "dicts";
    if (ad->isKeysetType()) return "keysets";
    assertx(ad->isPHPArrayType());
    return "arrays";
  }();
  throw ExtendedException(
    folly::sformat(
      "Invalid operand type was used: "
      "cannot perform this operation with {}", type
    )
  );
}

/*
 * unsigned to signed conversion when the unsigned value is out of
 * range is implementation defined behavior. We can work around it
 * with something like
 *
 *  return v > std::numeric_limits<int64_t>::max() ?
 *             -int64_t(~v) - 1 : int64_t(v);
 *
 * which gcc appears to optimize to a no-op. But I'd rather avoid that
 * until it becomes a problem.
 *
 * Putting this here as a placeholder.
 */
inline int64_t u2s(uint64_t v) {
  return v;
}

inline int64_t add_ignore_overflow(int64_t a, int64_t b) {
  return u2s(static_cast<uint64_t>(a) + b);
}

inline int64_t sub_ignore_overflow(int64_t a, int64_t b) {
  return u2s(static_cast<uint64_t>(a) - b);
}

inline int64_t mul_ignore_overflow(int64_t a, int64_t b) {
  return u2s(static_cast<uint64_t>(a) * b);
}

inline int64_t shl_ignore_overflow(int64_t a, int64_t b) {
  return u2s(static_cast<uint64_t>(a) << (b & 63));
}

TypedValue make_int(int64_t n) { return make_tv<KindOfInt64>(n); }
TypedValue make_dbl(double d)  { return make_tv<KindOfDouble>(d); }

// Helper for converting String, Array, Bool, Null or Obj to Dbl|Int.
// Other types (i.e. Int and Double) must be handled outside of this.
TypedNum numericConvHelper(TypedValue cell) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return make_int(0);

    case KindOfBoolean:
      return make_int(cell.m_data.num);

    case KindOfFunc:
      return stringToNumeric(funcToStringHelper(cell.m_data.pfunc));

    case KindOfClass:
      return stringToNumeric(classToStringHelper(cell.m_data.pclass));

    case KindOfString:
    case KindOfPersistentString:
      return stringToNumeric(cell.m_data.pstr);

    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray:
      throw_bad_array_operand(cell.m_data.parr);
    case KindOfClsMeth:
      throw ExtendedException("Invalid operand type was used: cannot perform "
                              "this operation with clsmeth");

    case KindOfRecord:
      raise_error(Strings::RECORD_NOT_SUPPORTED);
    case KindOfObject:
      return make_int(cell.m_data.pobj->toInt64());

    case KindOfResource:
      return make_int(cell.m_data.pres->data()->o_toInt64());

    case KindOfInt64:
    case KindOfDouble:
      break;
  }
  not_reached();
}

template<class Op>
TypedValue tvArith(Op o, TypedValue c1, TypedValue c2) {
again:
  if (c1.m_type == KindOfInt64) {
    for (;;) {
      if (c2.m_type == KindOfInt64)  return o(c1.m_data.num, c2.m_data.num);
      if (c2.m_type == KindOfDouble) return o(c1.m_data.num, c2.m_data.dbl);
      tvCopy(numericConvHelper(c2), c2);
      assertx(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfDouble) {
    for (;;) {
      if (c2.m_type == KindOfDouble) return o(c1.m_data.dbl, c2.m_data.dbl);
      if (c2.m_type == KindOfInt64)  return o(c1.m_data.dbl, c2.m_data.num);
      tvCopy(numericConvHelper(c2), c2);
      assertx(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (isArrayLikeType(c1.m_type) && isArrayLikeType(c2.m_type)) {
    return make_array_like_tv(o(c1.m_data.parr, c2.m_data.parr));
  }

  tvCopy(numericConvHelper(c1), c1);
  assertx(c1.m_type == KindOfInt64 || c1.m_type == KindOfDouble);
  goto again;
}

// Check is the function that checks for overflow, Over is the function that
// returns the overflowed value.
template<class Op, class Check, class Over>
TypedValue tvArithO(Op o, Check ck, Over ov, TypedValue c1, TypedValue c2) {
  if (isArrayLikeType(c1.m_type) && isArrayLikeType(c2.m_type)) {
    return tvArith(o, c1, c2);
  }

  auto ensure_num = [](TypedValue& c) {
    if (c.m_type != KindOfInt64 && c.m_type != KindOfDouble) {
      tvCopy(numericConvHelper(c), c);
    }
  };

  ensure_num(c1);
  ensure_num(c2);
  auto both_ints = (c1.m_type == KindOfInt64 && c2.m_type == KindOfInt64);
  int64_t a = c1.m_data.num;
  int64_t b = c2.m_data.num;

  return (both_ints && ck(a,b)) ? ov(a,b) : tvArith(o, c1, c2);
}

struct Add {
  TypedValue operator()(double  a, int64_t b) const { return make_dbl(a + b); }
  TypedValue operator()(double  a, double  b) const { return make_dbl(a + b); }
  TypedValue operator()(int64_t a, double  b) const { return make_dbl(a + b); }
  TypedValue operator()(int64_t a, int64_t b) const {
    return make_int(add_ignore_overflow(a, b));
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    if (UNLIKELY(a1->isHackArrayType())) throwInvalidAdditionException(a1);
    if (UNLIKELY(a2->isHackArrayType())) throwInvalidAdditionException(a2);
    if (checkHACArrayPlus()) raiseHackArrCompatAdd();
    a1->incRefCount(); // force COW
    SCOPE_EXIT { a1->decRefCount(); };
    return a1->plusEq(a2);
  }
};

struct Sub {
  TypedValue operator()(double  a, int64_t b) const { return make_dbl(a - b); }
  TypedValue operator()(double  a, double  b) const { return make_dbl(a - b); }
  TypedValue operator()(int64_t a, double  b) const { return make_dbl(a - b); }
  TypedValue operator()(int64_t a, int64_t b) const {
    return make_int(sub_ignore_overflow(a, b));
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* /*a2*/) const {
    throw_bad_array_operand(a1);
  }
};

struct Mul {
  TypedValue operator()(double  a, int64_t b) const { return make_dbl(a * b); }
  TypedValue operator()(double  a, double  b) const { return make_dbl(a * b); }
  TypedValue operator()(int64_t a, double  b) const { return make_dbl(a * b); }
  TypedValue operator()(int64_t a, int64_t b) const {
    return make_int(mul_ignore_overflow(a, b));
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* /*a2*/) const {
    throw_bad_array_operand(a1);
  }
};

struct Div {
  TypedValue operator()(int64_t t, int64_t u) const {
    if (UNLIKELY(u == 0)) {
      if (RuntimeOption::EvalForbidDivisionByZero) {
        SystemLib::throwDivisionByZeroExceptionObject();
      }
      raise_warning(Strings::DIVISION_BY_ZERO);
      return make_tv<KindOfBoolean>(false);
    }

    // Avoid SIGFPE when dividing the miniumum respresentable integer
    // by -1.
    auto const minInt = std::numeric_limits<int64_t>::min();
    if (UNLIKELY(u == -1 && t == minInt)) {
      return make_dbl(static_cast<double>(minInt) / -1);
    }

    return (t % u == 0) ? make_int(t / u) : make_dbl(double(t) / u);
  }

  template<class T, class U>
  typename std::enable_if<
    std::is_floating_point<T>::value || std::is_floating_point<U>::value,
    TypedValue
  >::type operator()(T t, U u) const {
    if (UNLIKELY(u == 0)) {
      if (RuntimeOption::EvalForbidDivisionByZero) {
        SystemLib::throwDivisionByZeroExceptionObject();
      }
      raise_warning(Strings::DIVISION_BY_ZERO);
      return make_tv<KindOfBoolean>(false);
    }
    return make_dbl(t / u);
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* /*a2*/) const {
    throw_bad_array_operand(a1);
  }
};

template<class Op>
void tvOpEq(Op op, tv_lval c1, TypedValue c2) {
again:
  if (type(c1) == KindOfInt64) {
    for (;;) {
      if (c2.m_type == KindOfInt64) {
        val(c1).num = op(val(c1).num, c2.m_data.num);
        return;
      }
      if (c2.m_type == KindOfDouble) {
        type(c1) = KindOfDouble;
        val(c1).dbl = op(val(c1).num, c2.m_data.dbl);
        return;
      }
      tvCopy(numericConvHelper(c2), c2);
      assertx(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (type(c1) == KindOfDouble) {
    for (;;) {
      if (c2.m_type == KindOfInt64) {
        val(c1).dbl = op(val(c1).dbl, c2.m_data.num);
        return;
      }
      if (c2.m_type == KindOfDouble) {
        val(c1).dbl = op(val(c1).dbl, c2.m_data.dbl);
        return;
      }
      tvCopy(numericConvHelper(c2), c2);
      assertx(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (isArrayLikeType(type(c1)) && isArrayLikeType(c2.m_type)) {
    auto const ad1    = val(c1).parr;
    auto const newArr = op(ad1, c2.m_data.parr);
    if (newArr != ad1) {
      val(c1).parr = newArr;
      type(c1) = newArr->toDataType();
      decRefArr(ad1);
    } else if (RuntimeOption::EvalEmitDVArray) {
      type(c1) = ad1->toDataType();
    }
    return;
  }

  tvSet(numericConvHelper(*c1), c1);
  assertx(type(c1) == KindOfInt64 || type(c1) == KindOfDouble);
  goto again;
}

struct AddEq {
  int64_t operator()(int64_t a, int64_t b) const {
    return add_ignore_overflow(a, b);
  }
  double  operator()(double  a, int64_t b) const { return a + b; }
  double  operator()(int64_t a, double  b) const { return a + b; }
  double  operator()(double  a, double  b) const { return a + b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* ad2) const {
    if (UNLIKELY(ad1->isHackArrayType())) throwInvalidAdditionException(ad1);
    if (UNLIKELY(ad2->isHackArrayType())) throwInvalidAdditionException(ad2);
    if (checkHACArrayPlus()) raiseHackArrCompatAdd();
    if (ad2->empty() || ad1 == ad2) return ad1;
    if (ad1->empty()) {
      ad2->incRefCount();
      return ad2;
    }
    return ad1->plusEq(ad2);
  }
};

struct SubEq {
  int64_t operator()(int64_t a, int64_t b) const {
    return sub_ignore_overflow(a, b);
  }
  double  operator()(double  a, int64_t b) const { return a - b; }
  double  operator()(int64_t a, double  b) const { return a - b; }
  double  operator()(double  a, double  b) const { return a - b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* /*ad2*/) const {
    throw_bad_array_operand(ad1);
  }
};

struct MulEq {
  int64_t operator()(int64_t a, int64_t b) const { return a * b; }
  double  operator()(double  a, int64_t b) const { return a * b; }
  double  operator()(int64_t a, double  b) const { return a * b; }
  double  operator()(double  a, double  b) const { return a * b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* /*ad2*/) const {
    throw_bad_array_operand(ad1);
  }
};

template<class SzOp, class BitOp>
StringData* stringBitOp(BitOp bop, SzOp sop, StringData* s1, StringData* s2) {
  auto const s1Size = s1->size();
  auto const s2Size = s2->size();
  auto const newLen = sop(s1Size, s2Size);
  auto const newStr = StringData::Make(newLen);
  auto const s1Data = s1->data();
  auto const s2Data = s2->data();
  auto const outData = newStr->mutableData();

  for (uint32_t i = 0; i < newLen; ++i) {
    outData[i] = bop((i < s1Size) ? s1Data[i] : 0,
                     (i < s2Size) ? s2Data[i] : 0);
  }
  newStr->setSize(newLen);

  return newStr;
}

template<template<class> class BitOp, class StrLenOp>
TypedValue tvBitOp(StrLenOp strLenOp, TypedValue c1, TypedValue c2) {
  assertx(tvIsPlausible(c1));
  assertx(tvIsPlausible(c2));

  if (isStringType(c1.m_type) && isStringType(c2.m_type)) {
    return make_tv<KindOfString>(
      stringBitOp(
        BitOp<char>(),
        strLenOp,
        c1.m_data.pstr,
        c2.m_data.pstr
      )
    );
  }

  return make_int(BitOp<int64_t>()(tvToInt(c1), tvToInt(c2)));
}

template<class Op>
void tvBitOpEq(Op op, tv_lval c1, TypedValue c2) {
  auto const result = op(*c1, c2);
  auto const old = *c1;
  tvCopy(result, c1);
  tvDecRefGen(old);
}

// Op must implement the interface described for cellIncDecOp.
template<class Op>
void stringIncDecOp(Op op, tv_lval cell, StringData* sd) {
  assertx(isStringType(type(cell)) || isFuncType(type(cell)) ||
          isClassType(type(cell)));

  if (sd->empty()) {
    decRefStr(sd);
    tvCopy(op.emptyString(), cell);
    return;
  }

  int64_t ival;
  double dval;
  auto const dt = sd->isNumericWithVal(ival, dval, false /* allow_errors */);

  if (dt == KindOfInt64) {
    decRefStr(sd);
    tvCopy(make_int(ival), cell);
    op.intCase(cell);
  } else if (dt == KindOfDouble) {
    decRefStr(sd);
    tvCopy(make_dbl(dval), cell);
    op.dblCase(cell);
  } else {
    assertx(dt == KindOfNull);
    op.nonNumericString(cell);
  }
}

void raiseIncDecInvalidType(tv_lval cell) {
  switch (RuntimeOption::EvalWarnOnIncDecInvalidType) {
    case 0:
      break;
    case 1:
      raise_warning("Unsupported operand type (%s) for IncDec",
                    describe_actual_type(cell).c_str());
      break;
    case 2:
      raise_error("Unsupported operand type (%s) for IncDec",
                  describe_actual_type(cell).c_str());
      // fallthrough
    default:
      always_assert(false);
  }
}

/*
 * Inc or Dec for a string, depending on Op.  Op must implement
 *
 *   - a function call operator for numeric types
 *   - a nullCase(TypedValue&) function that returns the result for null types
 *   - an emptyString() function that performs the operation for empty strings
 *   - and a nonNumericString(TypedValue&) function used for non-numeric strings
 *
 * PHP's Inc and Dec behave differently in all these cases, so this
 * abstracts out the common parts from those differences.
 */
template<class Op>
void tvIncDecOp(Op op, tv_lval cell) {
  assertx(tvIsPlausible(*cell));

  switch (type(cell)) {
    case KindOfUninit:
    case KindOfNull:
      raiseIncDecInvalidType(cell);
      op.nullCase(cell);
      return;

    case KindOfInt64:
      op.intCase(cell);
      return;

    case KindOfDouble:
      op.dblCase(cell);
      return;

    case KindOfFunc: {
      raiseIncDecInvalidType(cell);
      auto s = funcToStringHelper(val(cell).pfunc);
      stringIncDecOp(op, cell, const_cast<StringData*>(s));
      return;
    }

    case KindOfClass: {
      raiseIncDecInvalidType(cell);
      auto s = classToStringHelper(val(cell).pclass);
      stringIncDecOp(op, cell, const_cast<StringData*>(s));
      return;
    }

    case KindOfPersistentString:
    case KindOfString:
      raiseIncDecInvalidType(cell);
      stringIncDecOp(op, cell, val(cell).pstr);
      return;

    case KindOfBoolean:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfRecord:
      raiseIncDecInvalidType(cell);
      return;
  }
  not_reached();
}

const StaticString s_1("1");


struct IncBase {
  void dblCase(tv_lval cell) const { ++val(cell).dbl; }
  void nullCase(tv_lval cell) const { tvCopy(make_int(1), cell); }

  TypedValue emptyString() const {
    return make_tv<KindOfPersistentString>(s_1.get());
  }

  void nonNumericString(tv_lval cell) const {
    auto const sd = val(cell).pstr;
    auto const newSd = [&]() -> StringData* {
      auto const tmp = StringData::Make(sd, CopyString);
      auto const tmp2 = tmp->increment();
      if (tmp2 != tmp) {
        assertx(tmp->hasExactlyOneRef());
        tmp->release();
        return tmp2;
      }
      return tmp;
    }();
    decRefStr(sd);
    tvCopy(make_tv<KindOfString>(newSd), cell);
  }
};

struct Inc : IncBase {
  void intCase(tv_lval cell) const {
    auto& n = val(cell).num;
    n = add_ignore_overflow(n, 1);
  }
};

struct IncO : IncBase {
  void intCase(tv_lval cell) const {
    if (add_overflow(val(cell).num, int64_t{1})) {
      tvCopy(tvAddO(*cell, make_int(1)), cell);
    } else {
      Inc().intCase(cell);
    }
  }
};

struct DecBase {
  void dblCase(tv_lval cell) { --val(cell).dbl; }
  TypedValue emptyString() const { return make_int(-1); }
  void nullCase(tv_lval) const {}
  void nonNumericString(tv_lval cell) const {
    raise_notice("Decrement on string '%s'", val(cell).pstr->data());
  }
};

struct Dec : DecBase {
  void intCase(tv_lval cell) {
    auto& n = val(cell).num;
    n = sub_ignore_overflow(n, 1);
  }
};

struct DecO : DecBase {
  void intCase(tv_lval cell) {
    if (sub_overflow(val(cell).num, int64_t{1})) {
      tvCopy(tvSubO(*cell, make_int(1)), cell);
    } else {
      Dec().intCase(cell);
    }
  }
};

}

//////////////////////////////////////////////////////////////////////

TypedValue tvAdd(TypedValue c1, TypedValue c2) {
  return tvArith(Add(), c1, c2);
}

TypedNum tvSub(TypedValue c1, TypedValue c2) {
  return tvArith(Sub(), c1, c2);
}

TypedNum tvMul(TypedValue c1, TypedValue c2) {
  return tvArith(Mul(), c1, c2);
}

TypedValue tvAddO(TypedValue c1, TypedValue c2) {
  auto over = [](int64_t a, int64_t b) {
    if (RuntimeOption::CheckIntOverflow > 1) {
      SystemLib::throwArithmeticErrorObject(Strings::INTEGER_OVERFLOW);
    } else if (RuntimeOption::CheckIntOverflow == 1) {
      raise_warning(Strings::INTEGER_OVERFLOW);
    }
    return make_int(a + b);
  };
  return tvArithO(Add(), add_overflow<int64_t>, over, c1, c2);
}

TypedNum tvSubO(TypedValue c1, TypedValue c2) {
  auto over = [](int64_t a, int64_t b) {
    if (RuntimeOption::CheckIntOverflow > 1) {
      SystemLib::throwArithmeticErrorObject(Strings::INTEGER_OVERFLOW);
    } else if (RuntimeOption::CheckIntOverflow == 1) {
      raise_warning(Strings::INTEGER_OVERFLOW);
    }
    return make_int(a - b);
  };
  return tvArithO(Sub(), sub_overflow<int64_t>, over, c1, c2);
}

TypedNum tvMulO(TypedValue c1, TypedValue c2) {
  auto over = [](int64_t a, int64_t b) {
    if (RuntimeOption::CheckIntOverflow > 1) {
      SystemLib::throwArithmeticErrorObject(Strings::INTEGER_OVERFLOW);
    } else if (RuntimeOption::CheckIntOverflow == 1) {
      raise_warning(Strings::INTEGER_OVERFLOW);
    }
    return make_int(a * b);
  };
  return tvArithO(Mul(), mul_overflow<int64_t>, over, c1, c2);
}

TypedValue tvDiv(TypedValue c1, TypedValue c2) {
  return tvArith(Div(), c1, c2);
}

TypedValue tvPow(TypedValue c1, TypedValue c2) {
  return *HHVM_FN(pow)(tvAsVariant(&c1), tvAsVariant(&c2)).asTypedValue();
}

TypedValue tvMod(TypedValue c1, TypedValue c2) {
  auto const i1 = tvToInt(c1);
  auto const i2 = tvToInt(c2);
  if (UNLIKELY(i2 == 0)) {
    if (RuntimeOption::EvalForbidDivisionByZero) {
      SystemLib::throwDivisionByZeroExceptionObject();
    } else {
      raise_warning(Strings::DIVISION_BY_ZERO);
      return make_tv<KindOfBoolean>(false);
    }
  }

  // This is to avoid SIGFPE in the case of INT64_MIN % -1.
  return make_int(UNLIKELY(i2 == -1) ? 0 : i1 % i2);
}

TypedValue tvBitAnd(TypedValue c1, TypedValue c2) {
  return tvBitOp<std::bit_and>(
    [] (uint32_t a, uint32_t b) { return std::min(a, b); },
    c1, c2
  );
}

TypedValue tvBitOr(TypedValue c1, TypedValue c2) {
  return tvBitOp<std::bit_or>(
    [] (uint32_t a, uint32_t b) { return std::max(a, b); },
    c1, c2
  );
}

TypedValue tvBitXor(TypedValue c1, TypedValue c2) {
  return tvBitOp<std::bit_xor>(
    [] (uint32_t a, uint32_t b) { return std::min(a, b); },
    c1, c2
  );
}

TypedValue tvShl(TypedValue c1, TypedValue c2) {
  int64_t lhs = tvToInt(c1);
  int64_t shift = tvToInt(c2);

  return make_int(shl_ignore_overflow(lhs, shift));
}

TypedValue tvShr(TypedValue c1, TypedValue c2) {
  int64_t lhs = tvToInt(c1);
  int64_t shift = tvToInt(c2);

  return make_int(lhs >> (shift & 63));
}

void tvAddEq(tv_lval c1, TypedValue c2) {
  tvOpEq(AddEq(), c1, c2);
}

void tvSubEq(tv_lval c1, TypedValue c2) {
  tvOpEq(SubEq(), c1, c2);
}

void tvMulEq(tv_lval c1, TypedValue c2) {
  tvOpEq(MulEq(), c1, c2);
}

void tvAddEqO(tv_lval c1, TypedValue c2) { tvSet(tvAddO(*c1, c2), c1); }
void tvSubEqO(tv_lval c1, TypedValue c2) { tvSet(tvSubO(*c1, c2), c1); }
void tvMulEqO(tv_lval c1, TypedValue c2) { tvSet(tvMulO(*c1, c2), c1); }

void tvDivEq(tv_lval c1, TypedValue c2) {
  assertx(tvIsPlausible(*c1));
  assertx(tvIsPlausible(c2));
  if (!isIntType(type(c1)) && !isDoubleType(type(c1))) {
    tvSet(numericConvHelper(*c1), c1);
  }
  tvCopy(tvDiv(*c1, c2), c1);
}

void tvPowEq(tv_lval c1, TypedValue c2) {
  tvSet(tvPow(*c1, c2), c1);
}

void tvModEq(tv_lval c1, TypedValue c2) {
  tvSet(tvMod(*c1, c2), c1);
}

void tvBitAndEq(tv_lval c1, TypedValue c2) {
  tvBitOpEq(tvBitAnd, c1, c2);
}

void tvBitOrEq(tv_lval c1, TypedValue c2) {
  tvBitOpEq(tvBitOr, c1, c2);
}

void tvBitXorEq(tv_lval c1, TypedValue c2) {
  tvBitOpEq(tvBitXor, c1, c2);
}

void tvShlEq(tv_lval c1, TypedValue c2) { tvSet(tvShl(*c1, c2), c1); }
void tvShrEq(tv_lval c1, TypedValue c2) { tvSet(tvShr(*c1, c2), c1); }

void tvInc(tv_lval cell) { tvIncDecOp(Inc(), cell); }
void tvIncO(tv_lval cell) { tvIncDecOp(IncO(), cell); }
void tvDec(tv_lval cell) { tvIncDecOp(Dec(), cell); }
void tvDecO(tv_lval cell) { tvIncDecOp(DecO(), cell); }

void tvBitNot(TypedValue& cell) {
  assertx(tvIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfInt64:
      cell.m_data.num = ~cell.m_data.num;
      break;

    case KindOfDouble:
      cell.m_type     = KindOfInt64;
      cell.m_data.num = ~double_to_int64(cell.m_data.dbl);
      break;

    case KindOfClass:
    case KindOfFunc:
      cell.m_data.pstr = isFuncType(cell.m_type)
        ? const_cast<StringData*>(funcToStringHelper(cell.m_data.pfunc))
        : const_cast<StringData*>(classToStringHelper(cell.m_data.pclass));
      cell.m_type = KindOfString;
    case KindOfString:
      if (cell.m_data.pstr->cowCheck()) {
    case KindOfPersistentString:
        auto const sl = cell.m_data.pstr->slice();
        FOLLY_SDT(hhvm, hhvm_cow_bitnot, sl.size());
        auto const newSd = StringData::Make(sl, CopyString);
        cell.m_data.pstr->decRefCount(); // can't go to zero
        cell.m_data.pstr = newSd;
        cell.m_type = KindOfString;
      } else {
        // Unless we go through this branch, the string was just freshly
        // created, so the following mutation will be safe wrt its
        // internal hash caching.
        cell.m_data.pstr->invalidateHash();
        FOLLY_SDT(hhvm, hhvm_mut_bitnot, cell.m_data.pstr->size());
      }

      {
        auto const sd   = cell.m_data.pstr;
        auto const len  = sd->size();
        auto const data = sd->mutableData();
        assertx(sd->hasExactlyOneRef());
        for (uint32_t i = 0; i < len; ++i) {
          data[i] = ~data[i];
        }
      }
      break;

    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray:
    case KindOfPersistentArray:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfRecord:
      raise_error("Unsupported operand type for ~");
  }
}

//////////////////////////////////////////////////////////////////////

}
