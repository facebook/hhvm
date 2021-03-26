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
#include "hphp/hhbbc/type-ops.h"

#include <folly/Optional.h>

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
  if (t1.subtypeOf(BInt) && t2.subtypeOf(BInt)) return TInt;
  if (t1.subtypeOf(BInt) && t2.subtypeOf(BDbl)) return TDbl;
  if (t1.subtypeOf(BDbl) && t2.subtypeOf(BInt)) return TDbl;
  if (t1.subtypeOf(BDbl) && t2.subtypeOf(BDbl)) return TDbl;
  if (t1.subtypeOf(BNum) && t2.subtypeOf(BNum)) return TNum;
  return folly::none;
}

template<class Fun>
folly::Optional<Type> eval_const(Type t1, Type t2, Fun fun) {
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) return eval_cell([&] { return fun(*v1, *v2); });
  return folly::none;
}

template<class Fun>
Type bitwise_impl(Type t1, Type t2, Fun op) {
  if (auto t = eval_const(t1, t2, op))          return *t;
  if (t1.subtypeOf(BStr) && t2.subtypeOf(BStr)) return TStr;
  if (!t1.couldBe(BStr) || !t2.couldBe(BStr))   return TInt;
  return TInitCell;
}

template<class Fun>
Type shift_impl(Type t1, Type t2, Fun op) {
  if (RuntimeOption::EvalNoticeOnCoerceForBitOp > 0 &&
      (!t1.subtypeOf(BInt) || !t2.subtypeOf(BInt))) {
    // because typeToInt is triggered outside of eval_const, we need to do the
    // check for bailing manually
    return TInt;
  }

  t1 = typeToInt(t1);
  t2 = typeToInt(t2);

  if (auto const t = eval_const(t1, t2, op)) return *t;
  return TInt;
}
}

//////////////////////////////////////////////////////////////////////

Type typeToInt(Type ty) {
  if (auto const v = tv(ty)) return ival(tvToInt(*v));
  if (ty.subtypeOf(BNull))   return ival(0);
  return TInt;
}

//////////////////////////////////////////////////////////////////////

bool okTypesForConstMath(Type t1, Type t2) {
  // we're banning math on non-num types and they're gonna be rare on literals
  // and immediately codemodded away anyway, so just kill the optimization now
  return t1.subtypeOf(BNum) && t2.subtypeOf(BNum);
}

Type typeAdd(Type t1, Type t2) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, tvAdd)) return *t;
  }
  if (auto t = usual_arith_conversions(t1, t2))       return *t;
  if (t1.subtypeOf(BVec) && t2.subtypeOf(BVec))       return TVec;
  if (t1.subtypeOf(BDict) && t2.subtypeOf(BDict))     return TDict;
  if (t1.subtypeOf(BKeyset) && t2.subtypeOf(BKeyset)) return TKeyset;
  return TInitCell;
}

Type typeAddO(Type t1, Type t2) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, tvAddO))          return *t;
  }
  if (t1.subtypeOf(BInt) && t2.subtypeOf(BInt))       return TNum;
  if (auto t = usual_arith_conversions(t1, t2))       return *t;
  if (t1.subtypeOf(BVec) && t2.subtypeOf(BVec))       return TVec;
  if (t1.subtypeOf(BDict) && t2.subtypeOf(BDict))     return TDict;
  if (t1.subtypeOf(BKeyset) && t2.subtypeOf(BKeyset)) return TKeyset;
  return TInitCell;
}

template <class CellOp>
Type typeSubMulImpl(Type t1, Type t2, CellOp op) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, op))        return *t;
  }
  if (auto t = usual_arith_conversions(t1, t2)) return *t;
  return TInitPrim;
}

template <class CellOp>
Type typeSubMulImplO(Type t1, Type t2, CellOp op) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, op))        return *t;
  }
  if (t1.subtypeOf(BInt) && t2.subtypeOf(BInt)) return TNum;
  if (auto t = usual_arith_conversions(t1, t2)) return *t;
  return TInitPrim;
}

Type typeSub(Type t1, Type t2)  { return typeSubMulImpl(t1, t2, tvSub); }
Type typeMul(Type t1, Type t2)  { return typeSubMulImpl(t1, t2, tvMul); }

Type typeSubO(Type t1, Type t2) { return typeSubMulImplO(t1, t2, tvSubO); }
Type typeMulO(Type t1, Type t2) { return typeSubMulImplO(t1, t2, tvMulO); }

Type typeDiv(Type t1, Type t2) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, tvDiv)) return *t;
  }
  return TInitPrim;
}

Type typeMod(Type t1, Type t2) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, tvMod)) return *t;
  }
  return TInitPrim;
}

Type typePow(Type t1, Type t2) {
  if (okTypesForConstMath(t1, t2)) {
    if (auto t = eval_const(t1, t2, tvPow)) return *t;
  }
  return TNum;
}

Type typeConcat(Type t1, Type t2) {
  auto const tv = eval_const(t1, t2, [&] (auto v1, auto v2) {
    tvConcatEq(&v1, v2);
    return v1;
  });
  if (tv) return loosen_staticness(*tv);
  return TStr;
}

//////////////////////////////////////////////////////////////////////

Type typeBitAnd(Type t1, Type t2) { return bitwise_impl(t1, t2, tvBitAnd); }
Type typeBitOr(Type t1, Type t2)  { return bitwise_impl(t1, t2, tvBitOr); }
Type typeBitXor(Type t1, Type t2) { return bitwise_impl(t1, t2, tvBitXor); }

Type typeShl(Type t1, Type t2) { return shift_impl(t1, t2, tvShl); }
Type typeShr(Type t1, Type t2) { return shift_impl(t1, t2, tvShr); }

//////////////////////////////////////////////////////////////////////

Type typeIncDec(IncDecOp op, Type t) {
  auto const overflowToDbl = isIncDecO(op);
  auto const val = tv(t);

  if (!val) {
    // Doubles always stay doubles
    if (t.subtypeOf(BDbl)) return TDbl;

    if (t.subtypeOf(BOptInt)) {
      // Ints stay ints unless they can overflow to doubles
      if (t.subtypeOf(BInt)) {
        return overflowToDbl ? TNum : TInt;
      }
      // Null goes to 1 on ++, stays null on --. Uninit is folded to init.
      if (t.subtypeOf(BNull)) {
        return isInc(op) ? ival(1) : TInitNull;
      }
      // Optional integer case. The union of the above two cases.
      if (isInc(op)) return overflowToDbl ? TNum : TInt;
      return overflowToDbl? TOptNum : TOptInt;
    }

    // No-op on bool, array, resource, object.
    if (t.subtypeOf(BBool | BArrLike | BRes | BObj)) return t;

    return TInitCell;
  }

  auto const inc = isInc(op);

  // We can't constprop with this eval_cell, because of the effects
  // on locals.
  auto resultTy = eval_cell([inc,overflowToDbl,val] {
    auto c = *val;
    if (inc) {
      (overflowToDbl ? tvIncO : tvInc)(&c);
    } else {
      (overflowToDbl ? tvDecO : tvDec)(&c);
    }
    return c;
  });
  if (!resultTy) resultTy = TInitCell;

  // We may have inferred a TSStr or TSArr with a value here, but at
  // runtime it will not be static.
  resultTy = loosen_staticness(*resultTy);
  return *resultTy;
}

Type typeSetOp(SetOpOp op, Type lhs, Type rhs) {
  auto const lhsV = tv(lhs);
  auto const rhsV = tv(rhs);

  if (lhsV && rhsV) {
    // Can't constprop at this eval_cell, because of the effects on
    // locals.
    auto resultTy = eval_cell([&] {
      TypedValue c = *lhsV;
      TypedValue rhs = *rhsV;
      setopBody(&c, op, &rhs);
      return c;
    });
    if (!resultTy) resultTy = TInitCell;

    // We may have inferred a TSStr or TSArr with a value here, but
    // at runtime it will not be static.  For now just throw that
    // away.  TODO(#3696042): should be able to loosen_staticness here.
    if (resultTy->subtypeOf(BStr)) resultTy = TStr;
    else if (resultTy->subtypeOf(BVec)) resultTy = TVec;
    else if (resultTy->subtypeOf(BDict)) resultTy = TDict;
    else if (resultTy->subtypeOf(BKeyset)) resultTy = TKeyset;

    return *resultTy;
  }

  switch (op) {
  case SetOpOp::PlusEqual:   return typeAdd(lhs, rhs);
  case SetOpOp::MinusEqual:  return typeSub(lhs, rhs);
  case SetOpOp::MulEqual:    return typeMul(lhs, rhs);

  case SetOpOp::DivEqual:    return typeDiv(lhs, rhs);
  case SetOpOp::ModEqual:    return typeMod(lhs, rhs);
  case SetOpOp::PowEqual:    return typePow(lhs, rhs);

  case SetOpOp::AndEqual:    return typeBitAnd(lhs, rhs);
  case SetOpOp::OrEqual:     return typeBitOr(lhs, rhs);
  case SetOpOp::XorEqual:    return typeBitXor(lhs, rhs);

  case SetOpOp::PlusEqualO:  return typeAddO(lhs, rhs);
  case SetOpOp::MinusEqualO: return typeSubO(lhs, rhs);
  case SetOpOp::MulEqualO:   return typeMulO(lhs, rhs);

  case SetOpOp::ConcatEqual: return typeConcat(lhs, rhs);
  case SetOpOp::SlEqual:     return typeShl(lhs, rhs);
  case SetOpOp::SrEqual:     return typeShr(lhs, rhs);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

Type typeSame(const Type& a, const Type& b) {
  // The comparison will recurse into array values, so we need to
  // loosen the likeness recursively (unlike normal).
  auto const nsa = loosen_likeness_recursively(loosen_staticness(a));
  auto const nsb = loosen_likeness_recursively(loosen_staticness(b));
  if (!nsa.couldBe(nsb)) return TFalse;
  return TBool;
}

Type typeNSame(const Type& a, const Type& b) {
  auto const ty = typeSame(a, b);
  assertx(ty.subtypeOf(BBool));
  return ty.subtypeOf(BFalse) ? TTrue :
         ty.subtypeOf(BTrue) ? TFalse :
         TBool;
}

//////////////////////////////////////////////////////////////////////

}}
