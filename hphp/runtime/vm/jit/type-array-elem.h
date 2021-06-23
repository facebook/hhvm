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

#include "hphp/runtime/vm/jit/type.h"

#include <cstdint>

namespace HPHP {

struct Class;

namespace jit {

struct SSATmp;

///////////////////////////////////////////////////////////////////////////////

/*
 * Statically check whether a vec or varray access is within bounds, based on
 * the type of the array. If the index isn't provided, a check valid for all
 * possible indices will be done (which only returns Out if the array is known
 * to be empty).
 */
enum class VecBounds { In, Out, Unknown };
VecBounds vecBoundsStaticCheck(Type, Optional<int64_t>);

/*
* Get the most specific type of the element at idx for an array type using
* available knowledge on the types. The first value is the type of the value
* (if present). The second element is true if the key is guaranteed to be
* present and false otherwise.
 */
std::pair<Type, bool> arrLikeElemType(Type arr, Type idx, const Class* ctx);

/*
* Get the type of first or last element or key for an array using available
* knowledge on the types. The first element of the pair is the type of the
* key/value (if present). The second element is true if the key is guaranteed
* to be present and false otherwise.
*/
std::pair<Type, bool> arrLikeFirstLastType(
    Type arr, bool isFirst, bool isKey, const Class* ctx);

/*
* Get the type of the element or key at the given iterator position for an
* array using available knowledge on the types. The first element of the pair
* is the type of the key/value.
*/
Type arrLikePosType(Type arr, Type pos, bool isKey, const Class* ctx);

///////////////////////////////////////////////////////////////////////////////

}}

