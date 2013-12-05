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
#include "hphp/runtime/base/tv-arith.h"

#include <type_traits>
#include <limits>
#include <algorithm>

#include "folly/ScopeGuard.h"

#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-conversions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

// Helper for converting String, Array, Bool, Null or Obj to Dbl|Int.
// Other types (i.e. Int and Double) must be handled outside of this.
TypedNum numericConvHelper(Cell cell) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
  case KindOfString:
  case KindOfStaticString: return stringToNumeric(cell.m_data.pstr);
  case KindOfBoolean:      return make_tv<KindOfInt64>(cell.m_data.num);
  case KindOfUninit:
  case KindOfNull:         return make_tv<KindOfInt64>(0);
  case KindOfObject:       return make_tv<KindOfInt64>(
                             cell.m_data.pobj->o_toInt64());
  case KindOfResource:     return make_tv<KindOfInt64>(
                             cell.m_data.pres->o_toInt64());
  case KindOfArray:        throw BadArrayOperandException();
  default:                 break;
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
      c2 = numericConvHelper(c2);
      assert(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfDouble) {
    for (;;) {
      if (c2.m_type == KindOfDouble) return o(c1.m_data.dbl, c2.m_data.dbl);
      if (c2.m_type == KindOfInt64)  return o(c1.m_data.dbl, c2.m_data.num);
      c2 = numericConvHelper(c2);
      assert(c2.m_type == KindOfInt64 || c2.m_type == KindOfDouble);
    }
  }

  if (c1.m_type == KindOfArray && c2.m_type == KindOfArray) {
    return make_tv<KindOfArray>(o(c1.m_data.parr, c2.m_data.parr));
  }

  c1 = numericConvHelper(c1);
  assert(c1.m_type == KindOfInt64 || c1.m_type == KindOfDouble);
  goto again;
}

Cell num(int64_t n) { return make_tv<KindOfInt64>(n); }
Cell dbl(double d)  { return make_tv<KindOfDouble>(d); }

struct Add {
  Cell operator()(double  a, int64_t b) const { return dbl(a + b); }
  Cell operator()(double  a, double  b) const { return dbl(a + b); }
  Cell operator()(int64_t a, double  b) const { return dbl(a + b); }
  Cell operator()(int64_t a, int64_t b) const { return num(a + b); }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    a1->incRefCount(); // force COW
    SCOPE_EXIT { a1->decRefCount(); };
    return a1->plusEq(a2);
  }
};

struct Sub {
  Cell operator()(double  a, int64_t b) const { return dbl(a - b); }
  Cell operator()(double  a, double  b) const { return dbl(a - b); }
  Cell operator()(int64_t a, double  b) const { return dbl(a - b); }
  Cell operator()(int64_t a, int64_t b) const { return num(a - b); }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    throw BadArrayOperandException();
  }
};

struct Mul {
  Cell operator()(double  a, int64_t b) const { return dbl(a * b); }
  Cell operator()(double  a, double  b) const { return dbl(a * b); }
  Cell operator()(int64_t a, double  b) const { return dbl(a * b); }
  Cell operator()(int64_t a, int64_t b) const { return num(a * b); }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    throw BadArrayOperandException();
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
      return make_tv<KindOfDouble>(static_cast<double>(minInt) / -1);
    }

    if (t % u == 0) return make_tv<KindOfInt64>(t / u);
    return make_tv<KindOfDouble>(static_cast<double>(t) / u);
  }

  template<class T, class U>
  typename std::enable_if<
    std::is_floating_point<T>::value || std::is_floating_point<U>::value,
    Cell
  >::type operator()(T t, U u) const {
    static_assert(
      !(std::is_integral<T>::value && std::is_integral<U>::value), ""
    );
    if (UNLIKELY(u == 0)) {
      raise_warning(Strings::DIVISION_BY_ZERO);
      return make_tv<KindOfBoolean>(false);
    }
    return make_tv<KindOfDouble>(t / u);
  }

  ArrayData* operator()(ArrayData* a1, ArrayData* a2) const {
    throw BadArrayOperandException();
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
      c2 = numericConvHelper(c2);
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
      c2 = numericConvHelper(c2);
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
    throw BadArrayOperandException();
  }
};

struct MulEq {
  int64_t operator()(int64_t a, int64_t b) const { return a * b; }
  double  operator()(double  a, int64_t b) const { return a * b; }
  double  operator()(int64_t a, double  b) const { return a * b; }
  double  operator()(double  a, double  b) const { return a * b; }

  ArrayData* operator()(ArrayData* ad1, ArrayData* ad2) const {
    throw BadArrayOperandException();
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

  newStr->setRefCount(1);
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

  return make_tv<KindOfInt64>(
    BitOp<int64_t>()(cellToInt(c1), cellToInt(c2))
  );
}

template<class Op>
void cellBitOpEq(Op op, Cell& c1, Cell c2) {
  auto const result = op(c1, c2);
  cellSet(result, c1);
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
  auto const dt = sd->isNumericWithVal(ival, dval, true /* allow_errors */);
  switch (dt) {
  case KindOfInt64:
    decRefStr(sd);
    op(ival);
    cellCopy(num(ival), cell);
    break;
  case KindOfDouble:
    decRefStr(sd);
    op(dval);
    cellCopy(dbl(dval), cell);
    break;
  default:
    assert(dt == KindOfNull);
    op.nonNumericString(cell);
    break;
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
  case KindOfInt64:
    op(cell.m_data.num);
    break;
  case KindOfDouble:
    op(cell.m_data.dbl);
    break;

  case KindOfString:
  case KindOfStaticString:
    stringIncDecOp(op, cell);
    break;

  case KindOfUninit:
  case KindOfNull:
    op.nullCase(cell);
    break;

  case KindOfBoolean:
  case KindOfObject:
  case KindOfResource:
  case KindOfArray:
    break;
  default:
    not_reached();
  }
}

const StaticString s_1("1");

struct Inc {
  template<class T> void operator()(T& t) const { ++t; }
  void nullCase(Cell& cell) const { cellCopy(num(1), cell); }

  Cell emptyString() const {
    return make_tv<KindOfStaticString>(s_1.get());
  }

  void nonNumericString(Cell& cell) const {
    auto const sd = cell.m_data.pstr;
    auto const newSd = [&]() -> StringData* {
      auto const tmp = StringData::Make(sd, CopyString);
      auto const tmp2 = tmp->increment();
      if (tmp2 != tmp) {
        assert(tmp->getCount() == 0);
        tmp->release();
        return tmp2;
      }
      return tmp;
    }();
    newSd->incRefCount();
    decRefStr(sd);
    cellCopy(make_tv<KindOfString>(newSd), cell);
  }
};

struct Dec {
  template<class T> void operator()(T& t) const { --t; }
  void nullCase(Cell&) const {}
  Cell emptyString() const { return num(-1); }
  void nonNumericString(Cell&) const {}
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

Cell cellDiv(Cell c1, Cell c2) {
  return cellArith(Div(), c1, c2);
}

Cell cellMod(Cell c1, Cell c2) {
  auto const i1 = cellToInt(c1);
  auto const i2 = cellToInt(c2);
  if (UNLIKELY(i2 == 0)) {
    raise_warning(Strings::DIVISION_BY_ZERO);
    return make_tv<KindOfBoolean>(false);
  }

  // This is to avoid SIGFPE in the case of INT64_MIN % -1.
  if (i2 == -1) return make_tv<KindOfInt64>(0);

  return make_tv<KindOfInt64>(i1 % i2);
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

void cellAddEq(Cell& c1, Cell c2) {
  cellOpEq(AddEq(), c1, c2);
}

void cellSubEq(Cell& c1, Cell c2) {
  cellOpEq(SubEq(), c1, c2);
}

void cellMulEq(Cell& c1, Cell c2) {
  cellOpEq(MulEq(), c1, c2);
}

void cellDivEq(Cell& c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));
  if (!isTypedNum(c1)) {
    cellSet(numericConvHelper(c1), c1);
  }
  cellCopy(cellDiv(c1, c2), c1);
}

void cellModEq(Cell& c1, Cell c2) {
  cellCopy(cellMod(c1, c2), c1);
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

void cellInc(Cell& cell) {
  cellIncDecOp(Inc(), cell);
}

void cellDec(Cell& cell) {
  cellIncDecOp(Dec(), cell);
}

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
      newSd->incRefCount();
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
      assert(sd->getCount() == 1);
      for (uint32_t i = 0; i < len; ++i) {
        data[i] = ~data[i];
      }
    }
    break;

  default:
    raise_error("Unsupported operand type for ~");
  }
}

//////////////////////////////////////////////////////////////////////

}
