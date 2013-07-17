/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_RUNTIME_BASE_TV_ARITH_H_
#define incl_HPHP_RUNTIME_BASE_TV_ARITH_H_

#include "hphp/runtime/base/complex_types.h"

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

}

#endif
