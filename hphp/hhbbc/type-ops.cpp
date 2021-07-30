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

#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/tv-arith.h"

#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

template<class Fun>
Optional<Type> eval_const(Type t1, Type t2, Fun fun) {
  auto const v1 = tv(t1);
  auto const v2 = tv(t2);
  if (v1 && v2) return eval_cell([&] { return fun(*v1, *v2); });
  return std::nullopt;
}

template<class Fun>
Type bitwise_impl(Type t1, Type t2, Fun op) {
  if (!(t1.couldBe(BInt) && t2.couldBe(BInt)) &&
      !(t1.couldBe(BStr) && t2.couldBe(BStr))) {
    return TBottom;
  }
  if (auto t = eval_const(t1, t2, op))          return *t;
  if (t1.subtypeOf(BStr) && t2.subtypeOf(BStr)) return TStr;
  if (!t1.couldBe(BStr) || !t2.couldBe(BStr))   return TInt;
  return TInitCell;
}

template<class Fun>
Type shift_impl(Type t1, Type t2, Fun op) {
  if (!t1.couldBe(BInt) || !t2.couldBe(BInt)) return TBottom;
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

template <class CellOp>
Type typeArithImpl(Type t1, Type t2, CellOp op) {
  // TODO: this should be TBottom, but HHBBC straight up chokes on stuff like
  // vec[null + 58] in Type::checkInvariants
  if (!t1.couldBe(BNum) || !t2.couldBe(BNum)) return TNum;
  if (auto t = eval_const(t1, t2, op)) return *t;
  if (op == tvMod) return TInt;

  /*
   * TODO(#3577303): some of these could be nothrow, which is probably
   * information we have want to propagate back out through the return
   * value here (rather than bundling everything into the
   * interpreter).
   */
  if (t1.subtypeOf(BInt) && t2.subtypeOf(BInt)) {
    // can't switch on pointers, so use template magic
    auto is_any = [](auto first, auto ...t) { return ((first == t) || ...); };
    return is_any(op, tvSubO, tvMulO, tvAddO, tvDiv, tvPow) ? TNum : TInt;
  }
  if (t1.subtypeOf(BNum) && t2.subtypeOf(BNum) &&
     (t1.subtypeOf(BDbl) || t2.subtypeOf(BDbl))) return TDbl;
  return TNum;
}

Type typeAdd(Type t1, Type t2) { return typeArithImpl(t1, t2, tvAdd); }
Type typeSub(Type t1, Type t2) { return typeArithImpl(t1, t2, tvSub); }
Type typeMul(Type t1, Type t2) { return typeArithImpl(t1, t2, tvMul); }
Type typeDiv(Type t1, Type t2) { return typeArithImpl(t1, t2, tvDiv); }
Type typePow(Type t1, Type t2) { return typeArithImpl(t1, t2, tvPow); }
Type typeMod(Type t1, Type t2) { return typeArithImpl(t1, t2, tvMod); }

Type typeSubO(Type t1, Type t2) { return typeArithImpl(t1, t2, tvSubO); }
Type typeMulO(Type t1, Type t2) { return typeArithImpl(t1, t2, tvMulO); }
Type typeAddO(Type t1, Type t2) { return typeArithImpl(t1, t2, tvAddO); }

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
      // ++ on null throws, stays null on --. Uninit is folded to init.
      if (t.subtypeOf(BNull)) return isInc(op) ? TBottom : TInitNull;
      // Optional integer case. The union of the above two cases.
      if (isInc(op)) return overflowToDbl ? TNum : TInt;
      return overflowToDbl ? TOptNum : TOptInt;
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

  // We may have inferred a TSStr or TSArr with a value here, but at
  // runtime it will not be static.
  return resultTy ? loosen_staticness(*resultTy) : TInitCell;
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
    if (resultTy) {
      // We may have inferred a TSStr or TSArr with a value here, but
      // at runtime it will not be static.  For now just throw that
      // away.  TODO(#3696042): should be able to loosen_staticness here.
      if (resultTy->subtypeOf(BStr)) return TStr;
      else if (resultTy->subtypeOf(BVec)) return TVec;
      else if (resultTy->subtypeOf(BDict)) return TDict;
      else if (resultTy->subtypeOf(BKeyset)) return TKeyset;

      return *resultTy;
    }
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
