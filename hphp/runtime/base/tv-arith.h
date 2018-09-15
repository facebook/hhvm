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
 * case it returns an Cell that contains an Array.
 */
Cell cellAdd(Cell, Cell);

/*
 * PHP operators - and *.
 *
 * These arithmetic operators on any php value only return numbers.
 */
TypedNum cellSub(Cell, Cell);
TypedNum cellMul(Cell, Cell);

/*
 * Same as their corresponding non-O functions, but will cast their sources to
 * doubles instead of doing integer overflow.
 */
Cell cellAddO(Cell, Cell);
TypedNum cellSubO(Cell, Cell);
TypedNum cellMulO(Cell, Cell);

/*
 * PHP operators / and %.
 *
 * The operators return numbers unless the second argument converts to
 * zero, in which case they return boolean false.
 */
Cell cellDiv(Cell, Cell);
Cell cellMod(Cell, Cell);

/*
 * PHP Operator **.
 *
 * Always returns a TypedNum.
 */
Cell cellPow(Cell, Cell);

/*
 * PHP operators &, |, and ^.
 *
 * These operators return a KindOfInt64, unless both arguments are
 * KindOfString, in which case they return a KindOfString that the caller owns
 * a reference to.
 */
Cell cellBitAnd(Cell, Cell);
Cell cellBitOr(Cell, Cell);
Cell cellBitXor(Cell, Cell);

/*
 * PHP operators << and >>.
 *
 * These operators always return a KindOfInt64.
 */
Cell cellShl(Cell, Cell);
Cell cellShr(Cell, Cell);

//////////////////////////////////////////////////////////////////////

/*
 * PHP operator +=
 *
 * Mutates the first argument in place, by adding the second argument
 * to it in the sense of php's operator +=.
 *
 * Post: c1 is a KindOfInt or KindOfDouble, unless both arguments are
 * KindOfArray, in which case it will contain a Cell of KindOfArray.
 */
void cellAddEq(tv_lval c1, Cell);

/*
 * PHP operators -= and *=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of either php operator -= or *=.
 *
 * Post: c1 is a KindOfInt or KindOfDouble
 */
void cellSubEq(tv_lval c1, Cell);
void cellMulEq(tv_lval c1, Cell);

/*
 * Same as their corresponding non-O functions, but will cast their sources to
 * doubles instead of doing integer overflow.
 */
void cellAddEqO(tv_lval c1, Cell c2);
void cellSubEqO(tv_lval c1, Cell c2);
void cellMulEqO(tv_lval c1, Cell c2);

/*
 * PHP operators /= and %=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of either php operator /= or %=.
 *
 * Post: c1 is a KindOfInt or KindOfDouble, unless the second argument converts
 * to zero, in which case c1 will contain boolean false.
 */
void cellDivEq(tv_lval c1, Cell);
void cellModEq(tv_lval c1, Cell);

/*
 * PHP operator **=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the of php operator **=.
 */
void cellPowEq(tv_lval c1, Cell);

/*
 * PHP operators &=, |=, and ^=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of the appropriate operator.
 *
 * Post: c1.m_type == KindOfString || c1.m_type == KindOfInt64
 */
void cellBitAndEq(tv_lval c1, Cell);
void cellBitOrEq(tv_lval c1, Cell);
void cellBitXorEq(tv_lval c1, Cell);

/*
 * PHP operators <<= and >>=.
 *
 * Mutates the first argument in place, by combining the second argument
 * with it in the sense of the appropriate operator.
 *
 * Post: c1.m_type == KindOfInt64
 */
void cellShlEq(tv_lval c1, Cell);
void cellShrEq(tv_lval c1, Cell);

/*
 * PHP operator .=.
 *
 * Mutates the first argument in place, by concatenating the second argument
 * onto its end.
 *
 * Post: lhs.m_type == KindOfString
 */
inline void cellConcatEq(tv_lval lhs, Cell rhs) {
  concat_assign(lhs, cellAsCVarRef(rhs).toString());
}

//////////////////////////////////////////////////////////////////////

/*
 * PHP operator ++ and --.
 *
 * Mutates the argument in place, with the effects of php's
 * pre-increment or pre-decrement operators.
 */
void cellInc(tv_lval);
void cellDec(tv_lval);

void cellIncO(tv_lval);
void cellDecO(tv_lval);

/*
 * PHP unary operator ~.
 *
 * Mutates the argument in place, with the effects of php's unary
 * bitwise not operator.
 */
void cellBitNot(Cell&);

//////////////////////////////////////////////////////////////////////

}

#endif
