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

#include <cstdint>
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/type-structure-kinds.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {

struct String;
struct Array;
struct DictInit;

/* Utility for representing full type information in the runtime. */
namespace TypeStructure {

TypeStructureKind kind(const ArrayData* arr);
inline TypeStructureKind kind(const Array& arr) { return kind(arr.get()); }
Optional<TypeStructureKind> optKind(const ArrayData* arr);
void setKind(Array& arr, TypeStructureKind kind);
void setKind(DictInit& arr, TypeStructureKind kind);

enum class TSDisplayType : uint8_t {
  TSDisplayTypeReflection = 0,
  TSDisplayTypeUser       = 1,
  TSDisplayTypeInternal   = 2,
};

String toString(const Array& arr, TSDisplayType type);

/*
 * Coerces the given array to a valid, resolved, list of type structures that
 * may be used as an object's reified-generics property.
 *
 * This function does coercion in place; for that to be possible, all arrays
 * reachable from the given array must either have refcount 1 or be empty.
 *
 * The only coercion this function performs are to convert list-like darrays
 * in the appropriate places to varrays. The reason we have to do that is a
 * consequence of how our serialization format for PHP arrays broke down:
 * we cannot distinguish between list-like darrays and varrays based on the
 * serialized data. Converts list-like dicts to vecs.
 *
 * In summary, this function is tied closely to variable-unserializer formats.
 * If you think you need to use it for any other reason: THINK AGAIN. Think
 * very hard, and then come talk to kshaunak@ or dneiter@.
 */
bool coerceToTypeStructureList_SERDE_ONLY(tv_lval lval);

/*
 * All resolve functions ignore the initial value present in the
 * persistent flag
 */

Array resolve(const ArrayData* ts,
              const StringData* clsName,
              const Class* declCls,
              const Class* typeCnsCls,
              bool& persistent);

Array resolve(const String& aliasName,
              const Array& arr,
              bool& persistent,
              const Array& generics = Array());

Array resolve(const Array& ts,
              const Class* typeCnsCls,
              const Class* declCls,
              const req::vector<Array>& tsList,
              bool& persistent);

/*
 * Allows partially resolving a type structure.
 * Does not call the autoloader.
 * If the resulting type structure is persistent, persistent will be set.
 * If the resulting type structure is only partially resolved,
 * partial will be set, otherwise it will be unset.
 * If the type structure contains an invalid type for is/as expressions,
 * invalidType will be set.
 */
Array resolvePartial(const Array& ts,
                     const Class* typeCnsCls,
                     const Class* declCls,
                     bool& persistent,
                     bool& partial,
                     bool& invalidType);

}

std::string xhpNameFromTS(const Array& arr);

}
