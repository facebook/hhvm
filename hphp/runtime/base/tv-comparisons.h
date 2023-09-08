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
#pragma once

#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct ResourceData;

struct ECLString { StringData* val; };

inline bool operator==(const ECLString& lhs, const ECLString& rhs) {
  return lhs.val == rhs.val;
}

/*
 * Returns whether arg1 is "the same as" arg2, in the sense of the === operator.
 */
bool tvSame(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether arg1 is equal to arg2, in the sense of the == operator.
 */
bool tvEqual(TypedValue, bool);
bool tvEqual(TypedValue, int64_t);
bool tvEqual(TypedValue, double);
bool tvEqual(TypedValue, const StringData*);
bool tvEqual(TypedValue, const ArrayData*);
bool tvEqual(TypedValue, const ObjectData*);
bool tvEqual(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether arg1 is less than arg2, in the sense of the < operator.
 */
bool tvLess(TypedValue, int64_t);
bool tvLess(TypedValue, double);
bool tvLess(TypedValue, TypedValue);

/*
 * Returns whether arg1 is greater than arg2, in the sense of the > operator.
 */
bool tvGreater(TypedValue, int64_t);
bool tvGreater(TypedValue, double);
bool tvGreater(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

/*
 * operator >= and <=
 *
 * Note that $x <= $y is not equivalent to !($x > $y) for objects and arrays.
 *
 * These functions are necessary to handle those cases specially.
 */
bool tvLessOrEqual(TypedValue, TypedValue);
bool tvGreaterOrEqual(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

/*
 * Returns a comparison of arg1 against arg2, in the sense of the <=> operator.
 */
int64_t tvCompare(TypedValue, int64_t);
int64_t tvCompare(TypedValue, double);
int64_t tvCompare(TypedValue, TypedValue);

//////////////////////////////////////////////////////////////////////

}
