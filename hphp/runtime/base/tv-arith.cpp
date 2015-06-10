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
#include "hphp/runtime/base/tv-arith.h"

#include <type_traits>
#include <limits>
#include <algorithm>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/util/overflow.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

NEVER_INLINE ATTRIBUTE_NORETURN
void throw_bad_array_operand() {
  throw ExtendedException("Invalid operand type was used: "
                          "cannot perform this operation with arrays");
}

Cell make_int(int64_t n) { return make_tv<KindOfInt64>(n); }
Cell make_dbl(double d)  { return make_tv<KindOfDouble>(d); }

// Helper for converting String, Array, Bool, Null or Obj to Dbl|Int.
// Other types (i.e. Int and Double) must be handled outside of this.
TypedNum numericConvHelper(Cell cell) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return make_int(0);

    case KindOfBoolean:
      return make_int(cell.m_data.num);

    case KindOfString:
    case KindOfStaticString:
      return stringToNumeric(cell.m_data.pstr);

    case KindOfArray:
      throw_bad_array_operand();

    case KindOfObject:
      return make_int(cell.m_data.pobj->toInt64());

    case KindOfResource:
      return make_int(cell.m_data.pres->o_toInt64());

    case KindOfInt64:
    case KindOfDouble:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

template<class Op>
Cell cellArith(Op o, Cell c1, Cell c2) {
again:
  if (c1.m_type == KindOfInt64) {
    for (;;) {
      if (c2.m_type == KindOfInt64)  return o(c1.m_data.num, c2.m_data.num);
      if (c2.m_type == KindOfDouble) return o(c1.m_data.num, c2.m_data.dbl);
      cellCopy(numericConvHelper(c2), c2);
      assert(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfDouble) {
    for (;;) {
      if (c2.m_type == KindOfDouble) return o(c1.m_data.dbl, c2.m_data.dbl);
      if (c2.m_type == KindOfInt64)  return o(c1.m_data.dbl, c2.m_data.num);
      cellCopy(numericConvHelper(c2), c2);
      assert(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfArray && c2.m_type == KindOfArray) {
    return make_tv<KindOfArray>(o(c1.m_data.parr, c2.m_data.parr));
  }

  cellCopy(numericConvHelper(c1), c1);
  assert(c1.m_type == KindOfInt64 || c1.m_type == KindOfDouble);
  goto again;
}

// Check is the function that checks for overflow, Over is the function that
// returns the overflowed value.
template<class Op, class Check, class Over>
Cell cellArithO(Op o, Check ck, Over ov, Cell c1, Cell c2) {
  if (c1.m_type == KindOfArray && c2.m_type == KindOfArray) {
    return cellArith(o, c1, c2);
  }

  auto ensure_num = [](Cell& c) {
    if (c.m_type != KindOfInt64 && c.m_type != KindOfDouble) {
      cellCopy(numericConvHelper(c), c);
    }
  };

  ensure_num(c1);
  ensure_num(c2);
  auto both_ints = (c1.m_type == KindOfInt64 && c2.m_type == KindOfInt64);
  int64_t a = c1.m_data.num;
  int64_t b = c2.m_data.num;

  return (both_ints && ck(a,b)) ? ov(a,b) : cellArith(o, c1, c2);
}

struct Add {
  Cell operator()(double  a, int64_t b) const { return make_dbl(a + b); }
  Cell operator()(double  a, double  b) const { return make_dbl(a + b); }
  Cell operator()(int64_t a, double  b) const { return make_dbl(a + b); }
  Cell operator()(int64_t a, int64_t b) const { return make_int(a + b); }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    a1->incRefCount(); // force COW
    SCOPE_EXIT { a1->decRefCount(); };
    return a1->plusEq(a2);
  }
};

struct Sub {
  Cell operator()(double  a, int64_t b) const { return make_dbl(a - b); }
  Cell operator()(double  a, double  b) const { return make_dbl(a - b); }
  Cell operator()(int64_t a, double  b) const { return make_dbl(a - b); }
  Cell operator()(int64_t a, int64_t b) const { return make_int(a - b); }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    throw_bad_array_operand();
  }
};

struct Mul {
  Cell operator()(double  a, int64_t b) const { return make_dbl(a * b); }
  Cell operator()(double  a, double  b) const { return make_dbl(a * b); }
  Cell operator()(int64_t a, double  b) const { return make_dbl(a * b); }
  Cell operator()(int64_t a, int64_t b) const { return make_int(a * b); }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    throw_bad_array_operand();
  }
};

struct Div {
  Cell operator()(int64_t t, int64_t u) const {
    if (UNLIKELY(u == 0)) {
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
    Cell
  >::type operator()(T t, U u) const {
    if (UNLIKELY(u == 0)) {
      raise_warning(Strings::DIVISION_BY_ZERO);
      return make_tv<KindOfBoolean>(false);
    }
    return make_dbl(t / u);
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    throw_bad_array_operand();
  }
};

template<class Op>
void cellOpEq(Op op, Cell& c1, Cell c2) {
again:
  if (c1.m_type == KindOfInt64) {
    for (;;) {
      if (c2.m_type == KindOfInt64) {
        c1.m_data.num = op(c1.m_data.num, c2.m_data.num);
        return;
      }
      if (c2.m_type == KindOfDouble) {
        c1.m_type = KindOfDouble;
        c1.m_data.dbl = op(c1.m_data.num, c2.m_data.dbl);
        return;
      }
      cellCopy(numericConvHelper(c2), c2);
      assert(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfDouble) {
    for (;;) {
      if (c2.m_type == KindOfInt64) {
        c1.m_data.dbl = op(c1.m_data.dbl, c2.m_data.num);
        return;
      }
      if (c2.m_type == KindOfDouble) {
        c1.m_data.dbl = op(c1.m_data.dbl, c2.m_data.dbl);
        return;
      }
      cellCopy(numericConvHelper(c2), c2);
      assert(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfArray && c2.m_type == KindOfArray) {
    auto const ad1    = c1.m_data.parr;
    auto const newArr = op(ad1, c2.m_data.parr);
    if (newArr != ad1) {
      c1.m_data.parr = newArr;
      decRefArr(ad1);
    }
    return;
  }

  cellSet(numericConvHelper(c1), c1);
  assert(c1.m_type == KindOfInt64 || c1.m_type == KindOfDouble);
  goto again;
}

struct AddEq {
  int64_t operator()(int64_t a, int64_t b) const { return a + b; }
  double  operator()(double  a, int64_t b) const { return a + b; }
  double  operator()(int64_t a, double  b) const { return a + b; }
  double  operator()(double  a, double  b) const { return a + b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* ad2) const {
    if (ad2->empty() || ad1 == ad2) return ad1;
    if (ad1->empty()) {
      ad2->incRefCount();
      return ad2;
    }
    return ad1->plusEq(ad2);
  }
};

struct SubEq {
  int64_t operator()(int64_t a, int64_t b) const { return a - b; }
  double  operator()(double  a, int64_t b) const { return a - b; }
  double  operator()(int64_t a, double  b) const { return a - b; }
  double  operator()(double  a, double  b) const { return a - b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* ad2) const {
    throw_bad_array_operand();
  }
};

struct MulEq {
  int64_t operator()(int64_t a, int64_t b) const { return a * b; }
  double  operator()(double  a, int64_t b) const { return a * b; }
  double  operator()(int64_t a, double  b) const { return a * b; }
  double  operator()(double  a, double  b) const { return a * b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* ad2) const {
    throw_bad_array_operand();
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
Cell cellBitOp(StrLenOp strLenOp, Cell c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));

  if (IS_STRING_TYPE(c1.m_type) && IS_STRING_TYPE(c2.m_type)) {
    return make_tv<KindOfString>(
      stringBitOp(
        BitOp<char>(),
        strLenOp,
        c1.m_data.pstr,
        c2.m_data.pstr
      )
    );
  }

  return make_int(BitOp<int64_t>()(cellToInt(c1), cellToInt(c2)));
}

template<class Op>
void cellBitOpEq(Op op, Cell& c1, Cell c2) {
  auto const result = op(c1, c2);
  auto const type = c1.m_type;
  auto const data = c1.m_data.num;
  tvCopy(result, c1);
  tvRefcountedDecRefHelper(type, data);
}

// Op must implement the interface described for cellIncDecOp.
template<class Op>
void stringIncDecOp(Op op, Cell& cell) {
  assert(IS_STRING_TYPE(cell.m_type));

  auto const sd = cell.m_data.pstr;
  if (sd->empty()) {
    decRefStr(sd);
    cellCopy(op.emptyString(), cell);
    return;
  }

  int64_t ival;
  double dval;
  auto const dt = sd->isNumericWithVal(ival, dval, false /* allow_errors */);

  if (dt == KindOfInt64) {
    decRefStr(sd);
    cellCopy(make_int(ival), cell);
    op.intCase(cell);
  } else if (dt == KindOfDouble) {
    decRefStr(sd);
    cellCopy(make_dbl(dval), cell);
    op.dblCase(cell);
  } else {
    assert(dt == KindOfNull);
    op.nonNumericString(cell);
  }
}

/*
 * Inc or Dec for a string, depending on Op.  Op must implement
 *
 *   - a function call operator for numeric types
 *   - a nullCase(Cell&) function that returns the result for null types
 *   - an emptyString() function that performs the operation for empty strings
 *   - and a nonNumericString(Cell&) function used for non-numeric strings
 *
 * PHP's Inc and Dec behave differently in all these cases, so this
 * abstracts out the common parts from those differences.
 */
template<class Op>
void cellIncDecOp(Op op, Cell& cell) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      op.nullCase(cell);
      return;

    case KindOfInt64:
      op.intCase(cell);
      return;

    case KindOfDouble:
      op.dblCase(cell);
      return;

    case KindOfStaticString:
    case KindOfString:
      stringIncDecOp(op, cell);
      return;

    case KindOfBoolean:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
      return;

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

const StaticString s_1("1");


struct IncBase {
  void dblCase(Cell& cell) const { ++cell.m_data.dbl; }
  void nullCase(Cell& cell) const { cellCopy(make_int(1), cell); }

  Cell emptyString() const {
    return make_tv<KindOfStaticString>(s_1.get());
  }

  void nonNumericString(Cell& cell) const {
    auto const sd = cell.m_data.pstr;
    auto const newSd = [&]() -> StringData* {
      auto const tmp = StringData::Make(sd, CopyString);
      auto const tmp2 = tmp->increment();
      if (tmp2 != tmp) {
        assert(tmp->hasExactlyOneRef());
        tmp->release();
        return tmp2;
      }
      return tmp;
    }();
    decRefStr(sd);
    cellCopy(make_tv<KindOfString>(newSd), cell);
  }
};

struct Inc : IncBase {
  void intCase(Cell& cell) const { ++cell.m_data.num; }
};

struct IncO : IncBase {
  void intCase(Cell& cell) const {
    if (add_overflow(cell.m_data.num, int64_t{1})) {
      cellCopy(cellAddO(cell, make_int(1)), cell);
    } else {
      Inc().intCase(cell);
    }
  }
};

struct DecBase {
  void dblCase(Cell& cell) { --cell.m_data.dbl; }
  Cell emptyString() const { return make_int(-1); }
  void nullCase(Cell&) const {}
  void nonNumericString(Cell&) const {}
};

struct Dec : DecBase {
  void intCase(Cell& cell) { --cell.m_data.num; }
};

struct DecO : DecBase {
  void intCase(Cell& cell) {
    if (sub_overflow(cell.m_data.num, int64_t{1})) {
      cellCopy(cellSubO(cell, make_int(1)), cell);
    } else {
      Dec().intCase(cell);
    }
  }
};

}

//////////////////////////////////////////////////////////////////////

Cell cellAdd(Cell c1, Cell c2) {
  return cellArith(Add(), c1, c2);
}

TypedNum cellSub(Cell c1, Cell c2) {
  return cellArith(Sub(), c1, c2);
}

TypedNum cellMul(Cell c1, Cell c2) {
  return cellArith(Mul(), c1, c2);
}

Cell cellAddO(Cell c1, Cell c2) {
  auto over = [](int64_t a, int64_t b) {
    return make_dbl(double(a) + double(b));
  };
  return cellArithO(Add(), add_overflow<int64_t>, over, c1, c2);
}

TypedNum cellSubO(Cell c1, Cell c2) {
  auto over = [](int64_t a, int64_t b) {
    return make_dbl(double(a) - double(b));
  };
  return cellArithO(Sub(), sub_overflow<int64_t>, over, c1, c2);
}

TypedNum cellMulO(Cell c1, Cell c2) {
  auto over = [](int64_t a, int64_t b) {
    return make_dbl(double(a) * double(b));
  };
  return cellArithO(Mul(), mul_overflow<int64_t>, over, c1, c2);
}

Cell cellDiv(Cell c1, Cell c2) {
  return cellArith(Div(), c1, c2);
}

Cell cellPow(Cell c1, Cell c2) {
  return *HHVM_FN(pow)(tvAsVariant(&c1), tvAsVariant(&c2)).asCell();
}

Cell cellMod(Cell c1, Cell c2) {
  auto const i1 = cellToInt(c1);
  auto const i2 = cellToInt(c2);
  if (UNLIKELY(i2 == 0)) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    return make_tv<KindOfBoolean>(false);
  }

  // This is to avoid SIGFPE in the case of INT64_MIN % -1.
  return make_int(UNLIKELY(i2 == -1) ? 0 : i1 % i2);
}

Cell cellBitAnd(Cell c1, Cell c2) {
  return cellBitOp<std::bit_and>(
    [] (uint32_t a, uint32_t b) { return std::min(a, b); },
    c1, c2
  );
}

Cell cellBitOr(Cell c1, Cell c2) {
  return cellBitOp<std::bit_or>(
    [] (uint32_t a, uint32_t b) { return std::max(a, b); },
    c1, c2
  );
}

Cell cellBitXor(Cell c1, Cell c2) {
  return cellBitOp<std::bit_xor>(
    [] (uint32_t a, uint32_t b) { return std::min(a, b); },
    c1, c2
  );
}

Cell cellShl(Cell c1, Cell c2) {
  return make_int(cellToInt(c1) << cellToInt(c2));
}

Cell cellShr(Cell c1, Cell c2) {
  return make_int(cellToInt(c1) >> cellToInt(c2));
}

void cellAddEq(Cell& c1, Cell c2) {
  cellOpEq(AddEq(), c1, c2);
}

void cellSubEq(Cell& c1, Cell c2) {
  cellOpEq(SubEq(), c1, c2);
}

void cellMulEq(Cell& c1, Cell c2) {
  cellOpEq(MulEq(), c1, c2);
}

void cellAddEqO(Cell& c1, Cell c2) { cellCopy(cellAddO(c1, c2), c1); }
void cellSubEqO(Cell& c1, Cell c2) { cellCopy(cellSubO(c1, c2), c1); }
void cellMulEqO(Cell& c1, Cell c2) { cellCopy(cellMulO(c1, c2), c1); }

void cellDivEq(Cell& c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));
  if (!isTypedNum(c1)) {
    cellSet(numericConvHelper(c1), c1);
  }
  cellCopy(cellDiv(c1, c2), c1);
}

void cellPowEq(Cell& c1, Cell c2) {
  cellSet(cellPow(c1, c2), c1);
}

void cellModEq(Cell& c1, Cell c2) {
  cellSet(cellMod(c1, c2), c1);
}

void cellBitAndEq(Cell& c1, Cell c2) {
  cellBitOpEq(cellBitAnd, c1, c2);
}

void cellBitOrEq(Cell& c1, Cell c2) {
  cellBitOpEq(cellBitOr, c1, c2);
}

void cellBitXorEq(Cell& c1, Cell c2) {
  cellBitOpEq(cellBitXor, c1, c2);
}

void cellShlEq(Cell& c1, Cell c2) { cellCopy(cellShl(c1, c2), c1); }
void cellShrEq(Cell& c1, Cell c2) { cellCopy(cellShr(c1, c2), c1); }

void cellInc(Cell& cell) { cellIncDecOp(Inc(), cell); }
void cellIncO(Cell& cell) { cellIncDecOp(IncO(), cell); }
void cellDec(Cell& cell) { cellIncDecOp(Dec(), cell); }
void cellDecO(Cell& cell) { cellIncDecOp(DecO(), cell); }

void cellBitNot(Cell& cell) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfInt64:
      cell.m_data.num = ~cell.m_data.num;
      break;

    case KindOfDouble:
      cell.m_type     = KindOfInt64;
      cell.m_data.num = ~toInt64(cell.m_data.dbl);
      break;

    case KindOfString:
      if (cell.m_data.pstr->hasMultipleRefs()) {
    case KindOfStaticString:
        auto const newSd = StringData::Make(
          cell.m_data.pstr->slice(),
          CopyString
        );
        cell.m_data.pstr->decRefCount(); // can't go to zero
        cell.m_data.pstr = newSd;
        cell.m_type = KindOfString;
      } else {
        // Unless we go through this branch, the string was just freshly
        // created, so the following mutation will be safe wrt its
        // internal hash caching.
        cell.m_data.pstr->invalidateHash();
      }

      {
        auto const sd   = cell.m_data.pstr;
        auto const len  = sd->size();
        auto const data = sd->mutableData();
        assert(sd->hasExactlyOneRef());
        for (uint32_t i = 0; i < len; ++i) {
          data[i] = ~data[i];
        }
      }
      break;

    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      raise_error("Unsupported operand type for ~");
  }
}

//////////////////////////////////////////////////////////////////////

}
