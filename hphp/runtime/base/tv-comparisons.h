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
 * Returns whether a TypedValue has the same value as an unpackaged type, in
 * the sense of php's == operator.
 */
bool tvEqual(TypedValue, bool);
bool tvEqual(TypedValue, int64_t);
bool tvEqual(TypedValue, double);
bool tvEqual(TypedValue, const StringData*);
bool tvEqual(TypedValue, const ArrayData*);
bool tvEqual(TypedValue, const ObjectData*);
bool tvEqual(TypedValue, const ResourceData*);

/*
 * Returns whether two TypedValues have the same value, in the sense
 * of php's == operator.
 */
bool tvEqual(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////
// Php's operator <

/*
 * Returns whether a TypedValue is less than an unpackaged type, in the
 * sense of php's < operator.
 */
bool tvLess(TypedValue, bool);
bool tvLess(TypedValue, int64_t);
bool tvLess(TypedValue, double);
bool tvLess(TypedValue, const StringData*);
bool tvLess(TypedValue, const ArrayData*);
bool tvLess(TypedValue, const ObjectData*);
bool tvLess(TypedValue, const ResourceData*);

/*
 * Returns whether tv1 is less than tv2, as in php's < operator.
 */
bool tvLess(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////
// Php's operator >

/*
 * Returns whether a TypedValue is greater than an unpackaged type, in the
 * sense of php's > operator.
 */
bool tvGreater(TypedValue, bool);
bool tvGreater(TypedValue, int64_t);
bool tvGreater(TypedValue, double);
bool tvGreater(TypedValue, const StringData*);
bool tvGreater(TypedValue, const ArrayData*);
bool tvGreater(TypedValue, const ObjectData*);
bool tvGreater(TypedValue, const ResourceData*);

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
bool tvLessOrEqual(TypedValue, TypedValue);
bool tvGreaterOrEqual(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Php's operator <=>

/*
 * Returns a TypedValue's comparison against an unpackaged type, in the sense of php's
 * <=> operator.
 */
int64_t tvCompare(TypedValue, bool);
int64_t tvCompare(TypedValue, int64_t);
int64_t tvCompare(TypedValue, double);
int64_t tvCompare(TypedValue, const StringData*);
int64_t tvCompare(TypedValue, const ArrayData*);
int64_t tvCompare(TypedValue, const ObjectData*);
int64_t tvCompare(TypedValue, const ResourceData*);

/*
 * Returns the result of tv1's comparison against tv2, as in php's <=> operator.
 */
int64_t tvCompare(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

}

#endif
