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
#ifndef incl_HPHP_TV_COMPARISONS_H_
#define incl_HPHP_TV_COMPARISONS_H_

#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct ResourceData;

//////////////////////////////////////////////////////////////////////
// Php's operator ===

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
bool tvEqual(Cell, bool);
bool tvEqual(Cell, int);
bool tvEqual(Cell, int64_t);
bool tvEqual(Cell, double);
bool tvEqual(Cell, const StringData*);
bool tvEqual(Cell, const ArrayData*);
bool tvEqual(Cell, const ObjectData*);
bool tvEqual(Cell, const ResourceData*);

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
bool tvLess(Cell, bool);
bool tvLess(Cell, int);
bool tvLess(Cell, int64_t);
bool tvLess(Cell, double);
bool tvLess(Cell, const StringData*);
bool tvLess(Cell, const ArrayData*);
bool tvLess(Cell, const ObjectData*);
bool tvLess(Cell, const ResourceData*);

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
bool tvGreater(Cell, bool);
bool tvGreater(Cell, int);
bool tvGreater(Cell, int64_t);
bool tvGreater(Cell, double);
bool tvGreater(Cell, const StringData*);
bool tvGreater(Cell, const ArrayData*);
bool tvGreater(Cell, const ObjectData*);
bool tvGreater(Cell, const ResourceData*);

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
bool tvLessOrEqual(Cell, Cell);
bool tvGreaterOrEqual(Cell, Cell);

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Php's operator <=>

/*
 * Returns a Cell's comparison against an unpackaged type, in the sense of php's
 * <=> operator.
 */
int64_t tvCompare(Cell, bool);
int64_t tvCompare(Cell, int);
int64_t tvCompare(Cell, int64_t);
int64_t tvCompare(Cell, double);
int64_t tvCompare(Cell, const StringData*);
int64_t tvCompare(Cell, const ArrayData*);
int64_t tvCompare(Cell, const ObjectData*);
int64_t tvCompare(Cell, const ResourceData*);

/*
 * Returns the result of tv1's comparison against tv2, as in php's <=> operator.
 */
int64_t tvCompare(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/tv-comparisons-inl.h"

#endif
