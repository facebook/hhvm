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
#ifndef incl_HPHP_RUNTIME_BASE_TV_CONVERSIONS_H_
#define incl_HPHP_RUNTIME_BASE_TV_CONVERSIONS_H_

#include "hphp/runtime/base/complex-types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Convert a cell to various types, without changing the Cell.
 */
bool cellToBool(Cell);
int64_t cellToInt(Cell);
double cellToDouble(double);

/*
 * Convert a string to a TypedNum following php semantics, allowing
 * strings that have only a partial number in them.  (I.e. the string
 * may have junk after the number.)
 */
TypedNum stringToNumeric(const StringData*);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/tv-conversions-inl.h"

#endif
