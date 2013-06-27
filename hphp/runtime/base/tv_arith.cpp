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
#include "hphp/runtime/base/tv_arith.h"

#include <type_traits>
#include <limits>

#include "hphp/runtime/base/runtime_error.h"
#include "hphp/runtime/base/tv_conversions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

// Helper for converting String, Array, Bool, Null or Obj to Dbl|Int.
// Other types (i.e. Int and Double) must be handled outside of this.
TypedNum numericConvHelper(Cell cell) {
  assert(cellIsPlausible(&cell));

  switch (cell.m_type) {
  case KindOfString:
  case KindOfStaticString: return stringToNumeric(cell.m_data.pstr);
  case KindOfBoolean:      return make_tv<KindOfInt64>(cell.m_data.num);
  case KindOfUninit:
  case KindOfNull:         return make_tv<KindOfInt64>(0);
  case KindOfObject:       return make_tv<KindOfInt64>(
                             cell.m_data.pobj->o_toInt64());
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
    auto const newArr = a1->plus(a2, true /* copy */);
    newArr->incRefCount();
    return newArr;
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
      newArr->incRefCount();
      c1.m_data.parr = newArr;
      decRefArr(ad1);
    }
    return;
  }

  cellCopy(numericConvHelper(c1), c1);
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
    if (ad1->empty()) return ad2;
    return ad1->plus(ad2, ad1->getCount() > 1 /* copy */);
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
  assert(cellIsPlausible(&c1));
  assert(cellIsPlausible(&c2));
  if (!isTypedNum(c1)) {
    cellSet(numericConvHelper(c1), c1);
  }
  cellCopy(cellDiv(c1, c2), c1);
}

void cellModEq(Cell& c1, Cell c2) {
  cellCopy(cellMod(c1, c2), c1);
}

//////////////////////////////////////////////////////////////////////

}
