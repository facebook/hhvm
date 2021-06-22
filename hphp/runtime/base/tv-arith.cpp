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
    if (ad->isVecType()) return "vecs";
    if (ad->isDictType()) return "dicts";
    if (ad->isKeysetType()) return "keysets";
    always_assert(false);
  }();
  SystemLib::throwInvalidOperationExceptionObject(
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


  auto handleConvToIntNotice = [&](const char* from) {
    handleConvNoticeLevel(
      flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForMath),
      from,
      "int",
      s_ConvNoticeReasonMath.get());
  };

  auto stringToNumeric_ = [](const StringData* str) {
    return stringToNumeric(str,
      flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForMath),
      s_ConvNoticeReasonMath.get());
  };

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      handleConvToIntNotice("null");
      return make_int(0);

    case KindOfBoolean:
      handleConvToIntNotice("bool");
      return make_int(cell.m_data.num);

    case KindOfRFunc:
      raise_convert_rfunc_to_type("num");

    case KindOfFunc:
      invalidFuncConversion("int");

    case KindOfClass:
      return stringToNumeric_(classToStringHelper(cell.m_data.pclass));

    case KindOfLazyClass:
      return stringToNumeric_(lazyClassToStringHelper(cell.m_data.plazyclass));

    case KindOfString:
    case KindOfPersistentString:
      return stringToNumeric_(cell.m_data.pstr);

    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
      throw_bad_array_operand(cell.m_data.parr);
    case KindOfClsMeth:
      throw ExtendedException("Invalid operand type was used: cannot perform "
                              "this operation with clsmeth");

    case KindOfRClsMeth:
      raise_convert_rcls_meth_to_type("num");

    case KindOfRecord:
      raise_error(Strings::RECORD_NOT_SUPPORTED);
    case KindOfObject: {
      // do the conversion first as it often throws due to handling of the
      // unconditional Object->num warning. Soon that should be an unconditional
      // exception
      const auto res = make_int(cell.m_data.pobj->toInt64());
      handleConvToIntNotice("object");
      return res;
    }

    case KindOfResource:
      handleConvToIntNotice("resource");
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

  ArrayData* operator()(ArrayData* a1, ArrayData* /*ad2*/) const {
    throw_bad_array_operand(a1);
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
      SystemLib::throwDivisionByZeroExceptionObject();
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
      SystemLib::throwDivisionByZeroExceptionObject();
    }
    return make_dbl(t / u);
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* /*a2*/) const {
    throw_bad_array_operand(a1);
  }
};

template<class Op>
void tvOpEq(Op op, tv_lval c1, TypedValue c2) {
  if (UNLIKELY(isArrayLikeType(type(c1)) && isArrayLikeType(c2.m_type))) {
    throw_bad_array_operand(val(c1).parr);
    return;
  }

  auto lhs = *c1;
  if (UNLIKELY(!tvIsInt(lhs) && !tvIsDouble(lhs))) tvCopy(numericConvHelper(lhs), lhs);
  if (UNLIKELY(!tvIsInt(c2)  && !tvIsDouble(c2)))  tvCopy(numericConvHelper(c2), c2);
  assertx(tvIsInt(c2) || tvIsDouble(c2));
  tvSet(lhs, c1); // do the write-back after both conversions in case one throws

  if (tvIsDouble(c1)) {
    val(c1).dbl = op(val(c1).dbl, tvIsInt(c2) ? c2.m_data.num : c2.m_data.dbl);
    return;
  }

  assertx(tvIsInt(c1));
  if (tvIsInt(c2)) {
    val(c1).num = op(val(c1).num, c2.m_data.num);
  } else {
    type(c1) = KindOfDouble;
    val(c1).dbl = op(val(c1).num, c2.m_data.dbl);
  }

}

struct AddEq {
  int64_t operator()(int64_t a, int64_t b) const {
    return add_ignore_overflow(a, b);
  }
  double  operator()(double  a, int64_t b) const { return a + b; }
  double  operator()(int64_t a, double  b) const { return a + b; }
  double  operator()(double  a, double  b) const { return a + b; }
};

struct SubEq {
  int64_t operator()(int64_t a, int64_t b) const {
    return sub_ignore_overflow(a, b);
  }
  double  operator()(double  a, int64_t b) const { return a - b; }
  double  operator()(int64_t a, double  b) const { return a - b; }
  double  operator()(double  a, double  b) const { return a - b; }
};

struct MulEq {
  int64_t operator()(int64_t a, int64_t b) const { return a * b; }
  double  operator()(double  a, int64_t b) const { return a * b; }
  double  operator()(int64_t a, double  b) const { return a * b; }
  double  operator()(double  a, double  b) const { return a * b; }
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

  const ConvNoticeLevel notice_level =
    flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForBitOp);
  // pull out of fn invocation to prevent reordering making testing a nightmare
  auto i1 = tvToInt(c1, notice_level, s_ConvNoticeReasonBitOp.get());
  auto i2 = tvToInt(c2, notice_level, s_ConvNoticeReasonBitOp.get());
  return make_int(BitOp<int64_t>()(i1, i2));
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

  if (RuntimeOption::EvalNoticeOnCoerceForIncDec > 0 &&
      (dt == KindOfInt64 || dt == KindOfDouble)) {
    handleConvNoticeLevel(
      flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForIncDec),
      "string",
      dt == KindOfInt64 ? "int" : "double",
      s_ConvNoticeReasonIncDec.get());
  }

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
      invalidFuncConversion("int");
    }

    case KindOfClass: {
      raiseIncDecInvalidType(cell);
      auto s = classToStringHelper(val(cell).pclass);
      stringIncDecOp(op, cell, const_cast<StringData*>(s));
      return;
    }

    case KindOfLazyClass: {
      raiseIncDecInvalidType(cell);
      auto s = lazyClassToStringHelper(val(cell).plazyclass);
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
    case KindOfObject:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRFunc:
    case KindOfRecord:
      raiseIncDecInvalidType(cell);
      return;
  }
  not_reached();
}

const StaticString s_1("1");


struct IncBase {
  void dblCase(tv_lval cell) const { ++val(cell).dbl; }
  void nullCase(tv_lval cell) const {
    if (RuntimeOption::EvalNoticeOnCoerceForIncDec > 0) {
      handleConvNoticeLevel(
        flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForIncDec),
        "null",
        "int",
        s_ConvNoticeReasonIncDec.get());
    }
    tvCopy(make_int(1), cell);
  }

  TypedValue emptyString() const {
    if (RuntimeOption::EvalNoticeOnCoerceForIncDec > 0) {
      const auto level =
        flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForIncDec);
      const auto str = "Increment on empty string";
      if (level == ConvNoticeLevel::Throw) {
        SystemLib::throwInvalidOperationExceptionObject(str);
      } else if (level == ConvNoticeLevel::Log) {
        raise_notice(str);
      }
    }
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
  TypedValue emptyString() const {
    if (RuntimeOption::EvalNoticeOnCoerceForIncDec > 0) {
      handleConvNoticeLevel(
        flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForIncDec),
        "string",
        "int",
        s_ConvNoticeReasonIncDec.get());
    }
    return make_int(-1);
  }
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
  auto toIntWithNotice = [](TypedValue i) {
    return tvToInt(
          i,
          flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForMath),
          s_ConvNoticeReasonMath.get(),
          false);
  };
  auto const i1 = toIntWithNotice(c1);
  auto const i2 = toIntWithNotice(c2);
  if (UNLIKELY(i2 == 0)) {
    SystemLib::throwDivisionByZeroExceptionObject();
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
  const ConvNoticeLevel notice_level =
    flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForBitOp);
  int64_t lhs = tvToInt(c1, notice_level, s_ConvNoticeReasonBitOp.get());
  int64_t shift = tvToInt(c2, notice_level, s_ConvNoticeReasonBitOp.get());

  return make_int(shl_ignore_overflow(lhs, shift));
}

TypedValue tvShr(TypedValue c1, TypedValue c2) {
  const ConvNoticeLevel notice_level =
    flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForBitOp);

  int64_t lhs = tvToInt(c1, notice_level, s_ConvNoticeReasonBitOp.get());
  int64_t shift = tvToInt(c2, notice_level, s_ConvNoticeReasonBitOp.get());

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
      {
        const ConvNoticeLevel notice_level =
          flagToConvNoticeLevel(RuntimeOption::EvalNoticeOnCoerceForBitOp);
        handleConvNoticeLevel(
          notice_level, "double", "int", s_ConvNoticeReasonBitOp.get());
      }
      cell.m_type     = KindOfInt64;
      cell.m_data.num = ~double_to_int64(cell.m_data.dbl);
      break;

    case KindOfFunc:
      invalidFuncConversion("int");
    case KindOfClass:
      // Fall-through
    case KindOfLazyClass:
      cell.m_data.pstr =
        isClassType(cell.m_type) ?
        const_cast<StringData*>(classToStringHelper(cell.m_data.pclass)) :
        const_cast<StringData*>(lazyClassToStringHelper(cell.m_data.plazyclass));
      cell.m_type = KindOfString;
      // Fall-through
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
    case KindOfObject:
    case KindOfResource:
    case KindOfClsMeth:
    case KindOfRClsMeth:
    case KindOfRFunc:
    case KindOfRecord:
      raise_error("Unsupported operand type for ~");
  }
}

//////////////////////////////////////////////////////////////////////

}
