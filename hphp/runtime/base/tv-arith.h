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
#ifndef incl_HPHP_RUNTIME_BASE_TV_ARITH_H_
#define incl_HPHP_RUNTIME_BASE_TV_ARITH_H_

#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * These functions return Cells by value.  In cases where they may
 * return reference counted types, the value is already incRef'd when
 * returned.
 */

/*
 * PHP operator +
 *
 * Returns a TypedNum, unless both arguments are KindOfArray, in which
 * case it returns an TypedValue that contains an Array.
 */
TypedValue tvAdd(TypedValue, TypedValue);

/*
 * PHP operators - and *.
 *
 * These arithmetic operators on any php value only return numbers.
 */
TypedNum tvSub(TypedValue, TypedValue);
TypedNum tvMul(TypedValue, TypedValue);

/*
 * Same as their corresponding non-O functions, but will cast their sources to
 * doubles instead of doing integer overflow.
 */
TypedValue tvAddO(TypedValue, TypedValue);
TypedNum tvSubO(TypedValue, TypedValue);
TypedNum tvMulO(TypedValue, TypedValue);

/*
 * PHP operators / and %.
 *
 * The operators return numbers unless the second argument converts to
 * zero, in which case they return boolean false.
 */
TypedValue tvDiv(TypedValue, TypedValue);
TypedValue tvMod(TypedValue, TypedValue);

/*
 * PHP Operator **.
 *
 * Always returns a TypedNum.
 */
TypedValue tvPow(TypedValue, TypedValue);

/*
 * PHP operators &, |, and ^.
 *
 * These operators return a KindOfInt64, unless both arguments are
 * KindOfString, in which case they return a KindOfString that the caller owns
 * a reference to.
 */
TypedValue tvBitAnd(TypedValue, TypedValue);
TypedValue tvBitOr(TypedValue, TypedValue);
TypedValue tvBitXor(TypedValue, TypedValue);

/*
 * PHP operators << and >>.
 *
 * These operators always return a KindOfInt64.
 */
TypedValue tvShl(TypedValue, TypedValue);
TypedValue tvShr(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

/*
 * PHP operator +=
 *
 * Mutates the first argument in place, by adding the second argument
 * to it in the sense of php's operator +=.
 *
 * Post: c1 is a KindOfInt or KindOfDouble, unless both arguments are
 * KindOfArray, in which case it will contain a TypedValue of KindOfArray.
 */
void tvAddEq(tv_lval c1, TypedValue);

/*
 * PHP operators -= and *=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of either php operator -= or *=.
 *
 * Post: c1 is a KindOfInt or KindOfDouble
 */
void tvSubEq(tv_lval c1, TypedValue);
void tvMulEq(tv_lval c1, TypedValue);

/*
 * Same as their corresponding non-O functions, but will cast their sources to
 * doubles instead of doing integer overflow.
 */
void tvAddEqO(tv_lval c1, TypedValue c2);
void tvSubEqO(tv_lval c1, TypedValue c2);
void tvMulEqO(tv_lval c1, TypedValue c2);

/*
 * PHP operators /= and %=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of either php operator /= or %=.
 *
 * Post: c1 is a KindOfInt or KindOfDouble, unless the second argument converts
 * to zero, in which case c1 will contain boolean false.
 */
void tvDivEq(tv_lval c1, TypedValue);
void tvModEq(tv_lval c1, TypedValue);

/*
 * PHP operator **=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the of php operator **=.
 */
void tvPowEq(tv_lval c1, TypedValue);

/*
 * PHP operators &=, |=, and ^=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of the appropriate operator.
 *
 * Post: c1.m_type == KindOfString || c1.m_type == KindOfInt64
 */
void tvBitAndEq(tv_lval c1, TypedValue);
void tvBitOrEq(tv_lval c1, TypedValue);
void tvBitXorEq(tv_lval c1, TypedValue);

/*
 * PHP operators <<= and >>=.
 *
 * Mutates the first argument in place, by combining the second argument
 * with it in the sense of the appropriate operator.
 *
 * Post: c1.m_type == KindOfInt64
 */
void tvShlEq(tv_lval c1, TypedValue);
void tvShrEq(tv_lval c1, TypedValue);

/*
 * PHP operator .=.
 *
 * Mutates the first argument in place, by concatenating the second argument
 * onto its end.
 *
 * Post: lhs.m_type == KindOfString
 */
inline void tvConcatEq(tv_lval lhs, TypedValue rhs) {
  concat_assign(lhs, tvAsCVarRef(rhs).toString());
}

//////////////////////////////////////////////////////////////////////

/*
 * PHP operator ++ and --.
 *
 * Mutates the argument in place, with the effects of php's
 * pre-increment or pre-decrement operators.
 */
void tvInc(tv_lval);
void tvDec(tv_lval);

void tvIncO(tv_lval);
void tvDecO(tv_lval);

/*
 * PHP unary operator ~.
 *
 * Mutates the argument in place, with the effects of php's unary
 * bitwise not operator.
 */
void tvBitNot(TypedValue&);

//////////////////////////////////////////////////////////////////////

}

#endif
