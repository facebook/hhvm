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
#include "hphp/hhbbc/type-arith.h"

#include "folly/Optional.h"

#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-arith.h"

#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

folly::Optional<Type> usual_arith_conversions(Type t1, Type t2) {
  /*
   * TODO(#3577303): some of these could be nothrow, which is probably
   * information we have want to propagate back out through the return
   * value here (rather than bundling everything into the
   * interpreter).
   */
  if (t1.subtypeOf(TInt) && t2.subtypeOf(TInt)) return TInt;
  if (t1.subtypeOf(TInt) && t2.subtypeOf(TDbl)) return TDbl;
  if (t1.subtypeOf(TDbl) && t2.subtypeOf(TInt)) return TDbl;
  if (t1.subtypeOf(TDbl) && t2.subtypeOf(TDbl)) return TDbl;
  if (t1.subtypeOf(TNum) && t2.subtypeOf(TNum)) return TNum;
  return folly::none;
}

template<class Fun>
folly::Optional<Type> eval_const(Type t1, Type t2, Fun fun) {
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) return eval_cell([&] { return fun(*v1, *v2); });
  return folly::none;
}

// As eval_const, but don't divide/mod by zero at compile time.
template<class Fun>
folly::Optional<Type> eval_const_divmod(Type t1, Type t2, Fun fun) {
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2 && cellToInt(*v2) != 0 && cellToDouble(*v2) != 0.0) {
    return eval_cell([&] { return fun(*v1, *v2); });
  }
  return folly::none;
}

}

//////////////////////////////////////////////////////////////////////

Type typeToInt(Type ty) {
  if (auto const v = tv(ty)) return ival(cellToInt(*v));
  if (ty.subtypeOf(TNull))   return ival(0);
  return TInt;
}

//////////////////////////////////////////////////////////////////////

Type typeAdd(Type t1, Type t2) {
  if (auto t = eval_const(t1, t2, cellAdd))     return *t;
  if (auto t = usual_arith_conversions(t1, t2)) return *t;
  if (t1.subtypeOf(TArr) && t2.subtypeOf(TArr)) return TArr;
  return TInitCell;
}

Type typeAddO(Type t1, Type t2) {
  if (auto t = eval_const(t1, t2, cellAddO))    return *t;
  if (t1.subtypeOf(TInt) && t2.subtypeOf(TInt)) return TNum;
  if (auto t = usual_arith_conversions(t1, t2)) return *t;
  if (t1.subtypeOf(TArr) && t2.subtypeOf(TArr)) return TArr;
  return TInitCell;
}

template <class CellOp>
Type typeSubMulImpl(Type t1, Type t2, CellOp op) {
  if (auto t = eval_const(t1, t2, op))          return *t;
  if (auto t = usual_arith_conversions(t1, t2)) return *t;
  return TInitCell;
}

template <class CellOp>
Type typeSubMulImplO(Type t1, Type t2, CellOp op) {
  if (auto t = eval_const(t1, t2, op))          return *t;
  if (t1.subtypeOf(TInt) && t2.subtypeOf(TInt)) return TNum;
  if (auto t = usual_arith_conversions(t1, t2)) return *t;
  return TInitCell;
}

Type typeSub(Type t1, Type t2)  { return typeSubMulImpl(t1, t2, cellSub); }
Type typeMul(Type t1, Type t2)  { return typeSubMulImpl(t1, t2, cellMul); }

Type typeSubO(Type t1, Type t2) { return typeSubMulImplO(t1, t2, cellSubO); }
Type typeMulO(Type t1, Type t2) { return typeSubMulImplO(t1, t2, cellMulO); }

Type typeDiv(Type t1, Type t2) {
  if (auto t = eval_const_divmod(t1, t2, cellDiv)) return *t;
  return TInitCell;
}

Type typeMod(Type t1, Type t2) {
  if (auto t = eval_const_divmod(t1, t2, cellMod)) return *t;
  return TInitCell;
}

//////////////////////////////////////////////////////////////////////

Type typeBitAnd(Type t1, Type t2) {
  if (auto t = eval_const(t1, t2, cellBitAnd)) return *t;
  return TInitCell;
}

Type typeBitOr(Type t1, Type t2) {
  if (auto t = eval_const(t1, t2, cellBitOr)) return *t;
  return TInitCell;
}

Type typeBitXor(Type t1, Type t2) {
  if (auto t = eval_const(t1, t2, cellBitXor)) return *t;
  return TInitCell;
}

//////////////////////////////////////////////////////////////////////

}}

