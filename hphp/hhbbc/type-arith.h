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
#ifndef incl_HHBBC_TYPE_ARITH_H_
#define incl_HHBBC_TYPE_ARITH_H_

namespace HPHP { namespace HHBBC {

struct Type;

//////////////////////////////////////////////////////////////////////

/*
 * Operations on the typesystem that correspond to runtime operations
 * on Cells.
 *
 * Unless otherwise noted, all types in this module (both parameters
 * and returns) are required to be subtypes of TCell.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Attempt to convert `ty' to an int, with effects on types as the
 * effects at runtime of cellToInt.
 */
Type typeToInt(Type ty);

/*
 * Computes effects on types as for the effects at runtime of cellAdd,
 * cellSub, etc.
 */
Type typeAdd(Type, Type);
Type typeSub(Type, Type);
Type typeMul(Type, Type);
Type typeDiv(Type, Type);
Type typeMod(Type, Type);
Type typeAddO(Type, Type);
Type typeSubO(Type, Type);
Type typeMulO(Type, Type);

/*
 * Bitwise operations on types.  Computes effects on types as for the
 * runtime effects of cellBitAnd, cellBitOr, etc.
 */
Type typeBitAnd(Type, Type);
Type typeBitOr(Type, Type);
Type typeBitXor(Type, Type);

//////////////////////////////////////////////////////////////////////

}}

#endif
