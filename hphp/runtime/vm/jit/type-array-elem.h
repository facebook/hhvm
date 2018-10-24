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
 * Statically check whether a packed array access is within bounds, based on the
 * type of the array. If the index isn't provided, a check valid for all
 * possible indices will be done (which only returns Out if the array is known
 * to be empty).
 */
enum class PackedBounds { In, Out, Unknown };
PackedBounds packedArrayBoundsStaticCheck(Type, folly::Optional<int64_t>);

/*
 * Get the type of `arr[idx]` for different array types, considering constness,
 * staticness, and RAT types.
 *
 * Note that these functions do not require the existence of `arr[idx]`. If we
 * can statically determine that the access is out of bounds, TBottom is
 * returned. Otherwise we return a type `t`, such that when the access is within
 * bounds, `arr[idx].isA(t)` holds. (This, if this function is used in contexts
 * where the bounds are not statically known, one must account for the opcode
 * specific behavior of the failure case). If the bool member of the returned
 * pair is true, then `arr[idx]` definitely exists.
 */
std::pair<Type, bool> arrElemType(Type arr, Type idx, const Class* ctx);
std::pair<Type, bool> vecElemType(Type arr, Type idx, const Class* ctx);
std::pair<Type, bool> dictElemType(Type arr, Type idx);
std::pair<Type, bool> keysetElemType(Type arr, Type idx);

/*
* Get the type of first or last element for different array type.
*/
std::pair<Type, bool> vecFirstLastType(
  Type arr, bool isFirst, const Class* ctx);
std::pair<Type, bool> dictFirstLastType(Type arr, bool isFirst, bool isKey);
std::pair<Type, bool> keysetFirstLastType(Type arr, bool isFirst);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
