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
#ifndef incl_HPHP_TV_COMPARISONS_H_
#define incl_HPHP_TV_COMPARISONS_H_

#include "hphp/runtime/base/complex-types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
// Php's operator ===

/*
 * Returns whether two Cells have the same value, in the sense of
 * php's === operator.
 */
bool cellSame(Cell, Cell);

/*
 * Returns whether two TypedValues have the same value, in sense of
 * php's === operator.
 */
bool tvSame(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////
// Php's operator ==

/*
 * Returns whether a Cell has the same value as an unpackaged type, in
 * the sense of php's == operator.
 */
bool cellEqual(Cell, bool);
bool cellEqual(Cell, int);
bool cellEqual(Cell, int64_t);
bool cellEqual(Cell, double);
bool cellEqual(Cell, const StringData*);
bool cellEqual(Cell, const ArrayData*);
bool cellEqual(Cell, const ObjectData*);

/*
 * Returns whether two Cells have the same value, in the same of php's
 * == operator.
 */
bool cellEqual(Cell, Cell);

/*
 * Returns whether two TypedValues have the same value, in the sense
 * of php's == operator.
 */
bool tvEqual(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////
// Php's operator <

/*
 * Returns whether a Cell is less than an unpackaged type, in the
 * sense of php's < operator.
 */
bool cellLess(Cell, bool);
bool cellLess(Cell, int);
bool cellLess(Cell, int64_t);
bool cellLess(Cell, double);
bool cellLess(Cell, const StringData*);
bool cellLess(Cell, const ArrayData*);
bool cellLess(Cell, const ObjectData*);

/*
 * Returns whether a Cell is greater than another Cell, in the sense
 * of php's < operator.
 */
bool cellLess(Cell, Cell);

/*
 * Returns whether tv1 is less than tv2, as in php's < operator.
 */
bool tvLess(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////
// Php's operator >

/*
 * Returns whether a Cell is greater than an unpackaged type, in the
 * sense of php's > operator.
 */
bool cellGreater(Cell, bool);
bool cellGreater(Cell, int);
bool cellGreater(Cell, int64_t);
bool cellGreater(Cell, double);
bool cellGreater(Cell, const StringData*);
bool cellGreater(Cell, const ArrayData*);
bool cellGreater(Cell, const ObjectData*);

/*
 * Returns whether a Cell is greater than another Cell, in the sense
 * of php's > operator.
 */
bool cellGreater(Cell, Cell);

/*
 * Returns whether tv1 is greather than tv2, as in php's > operator.
 */
bool tvGreater(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

/*
 * Php operator >= and <=
 *
 * Note that in php $x <= $y is not equivalent to !($x > $y) for
 * objects and arrays.
 *
 * These functions are necessary to handle those cases specially.
 */
bool cellLessOrEqual(Cell, Cell);
bool cellGreaterOrEqual(Cell, Cell);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/tv-comparisons-inl.h"

#endif
