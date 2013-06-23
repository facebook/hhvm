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

#include "hphp/runtime/vm/core_types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
// Php's operator ===

/*
 * Returns whether two Cells have the same value, in the sense of
 * php's === operator.
 */
bool cellSame(const Cell*, const Cell*);

/*
 * Returns whether two TypedValues have the same value, in sense of
 * php's === operator.
 */
bool tvSame(const TypedValue*, const TypedValue*);

//////////////////////////////////////////////////////////////////////
// Php's operator ==

/*
 * Returns whether a Cell has the same value as an unpackaged type, in
 * the sense of php's == operator.
 */
bool cellEqual(const Cell*, bool);
bool cellEqual(const Cell*, int);
bool cellEqual(const Cell*, int64_t);
bool cellEqual(const Cell*, double);
bool cellEqual(const Cell*, const StringData*);
bool cellEqual(const Cell*, const ArrayData*);
bool cellEqual(const Cell*, const ObjectData*);

/*
 * Returns whether two Cells have the same value, in the same of php's
 * == operator.
 */
bool cellEqual(const Cell*, const Cell*);

/*
 * Returns whether two TypedValues have the same value, in the sense
 * of php's == operator.
 */
bool tvEqual(const TypedValue*, const TypedValue*);

//////////////////////////////////////////////////////////////////////
// Php's operator <

/*
 * Returns whether a Cell is less than an unpackaged type, in the
 * sense of php's < operator.
 */
bool cellLess(const Cell*, bool);
bool cellLess(const Cell*, int);
bool cellLess(const Cell*, int64_t);
bool cellLess(const Cell*, double);
bool cellLess(const Cell*, const StringData*);
bool cellLess(const Cell*, const ArrayData*);
bool cellLess(const Cell*, const ObjectData*);

/*
 * Returns whether a Cell is greater than another Cell, in the sense
 * of php's < operator.
 */
bool cellLess(const Cell*, const Cell*);

/*
 * Returns whether tv1 is less than tv2, as in php's < operator.
 */
bool tvLess(const TypedValue*, const TypedValue*);

//////////////////////////////////////////////////////////////////////
// Php's operator >

/*
 * Returns whether a Cell is greater than an unpackaged type, in the
 * sense of php's > operator.
 */
bool cellGreater(const Cell*, bool);
bool cellGreater(const Cell*, int);
bool cellGreater(const Cell*, int64_t);
bool cellGreater(const Cell*, double);
bool cellGreater(const Cell*, const StringData*);
bool cellGreater(const Cell*, const ArrayData*);
bool cellGreater(const Cell*, const ObjectData*);

/*
 * Returns whether a Cell is greater than another Cell, in the sense
 * of php's > operator.
 */
bool cellGreater(const Cell*, const Cell*);

/*
 * Returns whether tv1 is greather than tv2, as in php's > operator.
 */
bool tvGreater(const TypedValue*, const TypedValue*);

//////////////////////////////////////////////////////////////////////

/*
 * Php operator >= and <=
 *
 * Note that in php $x <= $y is not equivalent to !($x > $y) for
 * objects and arrays.
 *
 * These functions are necessary to handle those cases specially.
 */
bool cellLessOrEqual(const Cell*, const Cell*);
bool cellGreaterOrEqual(const Cell*, const Cell*);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/tv_comparisons-inl.h"

#endif
