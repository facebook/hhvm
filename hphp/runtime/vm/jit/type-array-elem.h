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

#ifndef incl_HPHP_JIT_TYPE_ARRAY_ELEM_H_
#define incl_HPHP_JIT_TYPE_ARRAY_ELEM_H_

#include "hphp/runtime/vm/jit/type.h"

#include <cstdint>

namespace HPHP {

struct Class;

namespace jit {

struct SSATmp;

///////////////////////////////////////////////////////////////////////////////

/*
 * Statically check whether a packed array access is within bounds, based on
 * the type of the array.
 */
enum class PackedBounds { In, Out, Unknown };
PackedBounds packedArrayBoundsStaticCheck(Type, int64_t idxVal);

/*
 * Get the type of `arr[idx]` for a packed array, considering constness,
 * staticness, and RAT types.
 *
 * Note that this function does not require the existence of `arr[idx]`.  If we
 * can statically determine that the access is out of bounds, InitNull is
 * returned.  Otherwise we return a type `t`, such that when the access is
 * within bounds, `arr[idx].isA(t)` holds.  (Thus, if this function is used in
 * contexts where the bounds are not statically known, TInitNull must be
 * unioned in for correctness.)
 */
Type packedArrayElemType(SSATmp* arr, SSATmp* idx, const Class* ctx);

/*
 * Get the type of `arr[idx]` for different Hack array types, considering
 * constness, staticness, and RAT types.
 *
 * Note that these functions do not require the existence of `arr[idx]`. If we
 * can statically determine that the access is out of bounds, TBottom is
 * returned. Otherwise we return a type `t`, such that when the access is within
 * bounds, `arr[idx].isA(t)` holds. (This, if this function is used in contexts
 * where the bounds are not statically known, one must account for the opcode
 * specific behavior of the failure case).
 *
 * `idx` is optional. If not provided, a more conservative type is returned
 * which holds for all elements in the array.
 */
Type vecElemType(SSATmp* arr, SSATmp* idx);
Type dictElemType(SSATmp* arr, SSATmp* idx);
Type keysetElemType(SSATmp* arr, SSATmp* idx);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
