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
#ifndef incl_HPHP_RUNTIME_BASE_TV_ARITH_H_
#define incl_HPHP_RUNTIME_BASE_TV_ARITH_H_

#include "hphp/runtime/base/complex-types.h"

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
 * PHP operators &, |, and ^.
 *
 * These functions return a KindOfInt64, unless both arguments are
 * KindOfString, in which case they return a KindOfString.
 */
Cell cellBitAnd(Cell, Cell);
Cell cellBitOr(Cell, Cell);
Cell cellBitXor(Cell, Cell);

//////////////////////////////////////////////////////////////////////

/*
 * PHP operator +=
 *
 * Mutates the first argument in place, by adding the second argument
 * to it in the sense of php's operator +=.
 *
 * Post: isTypedNum(c1), unless both arguments are KindOfArray, in
 * which case it will contain a Cell of KindOfArray.
 */
void cellAddEq(Cell& c1, Cell);

/*
 * PHP operators -= and *=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of either php operator -= or *=.
 *
 * Post: isTypedNum(c1)
 */
void cellSubEq(Cell& c1, Cell);
void cellMulEq(Cell& c1, Cell);

/*
 * Same as their corresponding non-O functions, but will cast their sources to
 * doubles instead of doing integer overflow.
 */
void cellAddEqO(Cell& c1, Cell c2);
void cellSubEqO(Cell& c1, Cell c2);
void cellMulEqO(Cell& c1, Cell c2);

/*
 * PHP operators /= and %=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of either php operator /= or %=.
 *
 * Post: isTypedNum(c1), unless the second argument converts to zero,
 * in which case c1 will contain boolean false.
 */
void cellDivEq(Cell& c1, Cell);
void cellModEq(Cell& c1, Cell);

/*
 * PHP operators &=, |=, and ^=.
 *
 * Mutates the first argument in place, by combining the second
 * argument with it in the sense of the appropriate operator.
 *
 * Post: c1.m_type == KindOfString || c1.m_type == KindOfInt64
 */
void cellBitAndEq(Cell& c1, Cell);
void cellBitOrEq(Cell& c1, Cell);
void cellBitXorEq(Cell& c1, Cell);

//////////////////////////////////////////////////////////////////////

/*
 * PHP operator ++ and --.
 *
 * Mutates the argument in place, with the effects of php's
 * pre-increment or pre-decrement operators.
 */
void cellInc(Cell&);
void cellDec(Cell&);

void cellIncO(Cell&);
void cellDecO(Cell&);

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
