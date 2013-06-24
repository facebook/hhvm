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
#ifndef incl_HPHP_RUNTIME_BASE_TV_ARITH_H_
#define incl_HPHP_RUNTIME_BASE_TV_ARITH_H_

#include "hphp/runtime/base/complex_types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Functions that implement php arithmetic.
 *
 * These functions return Cells by value.  In cases where they may
 * return reference counted types, the value is already incRef'd when
 * returned.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Operator +.
 *
 * Returns a TypedNum, unless both arguments are KindOfArray, in which
 * case it returns an Cell that contains an Array.
 */
Cell cellAdd(Cell, Cell);

/*
 * Operators - and *.
 *
 * These arithmetic operators on any php value only return numbers.
 */
TypedNum cellSub(Cell, Cell);
TypedNum cellMul(Cell, Cell);

/*
 * Operators / and %.
 *
 * The operators return numbers unless the second argument converts to
 * zero, in which case they return boolean false.
 */
Cell cellDiv(Cell, Cell);
Cell cellMod(Cell, Cell);

//////////////////////////////////////////////////////////////////////

}

#endif
