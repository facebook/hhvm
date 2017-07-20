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

#ifndef incl_HPHP_TV_CONVERSIONS_H_
#define incl_HPHP_TV_CONVERSIONS_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct StringData;

///////////////////////////////////////////////////////////////////////////////
/*
 * TypedValue conversions that update `tv' in place (decrefing the old value,
 * if necessary).
 *
 * We have two kinds of type conversions:
 *
 * - Cast forcibly changes the value to the new type and will not fail (though
 *   the result may be silly).
 * - Coerce attempts to convert the type and returns false on failure.
 */

#define X(kind) \
void tvCastTo##kind##InPlace(TypedValue* tv); \
bool tvCoerceParamTo##kind##InPlace(TypedValue* tv, \
                                    bool builtin);
X(Boolean)
X(Int64)
X(Double)
X(String)
X(Vec)
X(Dict)
X(Keyset)
X(Array)
X(Object)
X(NullableObject)
X(Resource)
#undef X

void tvCastToVArrayInPlace(TypedValue* tv);
void tvCastToDArrayInPlace(TypedValue* tv);

ALWAYS_INLINE void tvCastInPlace(TypedValue* tv, DataType DType) {
#define X(kind) \
  if (DType == KindOf##kind) { tvCastTo##kind##InPlace(tv); return; }
  X(Boolean)
  X(Int64)
  X(Double)
  X(String)
  X(Vec)
  X(Dict)
  X(Keyset)
  X(Array)
  X(Object)
  X(Resource)
#undef X
  not_reached();
}

ALWAYS_INLINE bool tvCoerceParamInPlace(TypedValue* tv, DataType DType,
                                        bool builtin) {
#define X(kind) \
  if (DType == KindOf##kind) \
    return tvCoerceParamTo##kind##InPlace(tv, \
                                          builtin);
  X(Boolean)
  X(Int64)
  X(Double)
  X(String)
  X(Vec)
  X(Dict)
  X(Keyset)
  X(Array)
  X(Object)
  X(Resource)
#undef X
  not_reached();
}

/*
 * Non-in-place casts.
 */
double tvCastToDouble(TypedValue tv);
StringData* tvCastToString(TypedValue tv);
ArrayData* tvCastToArrayLike(TypedValue tv);
ObjectData* tvCastToObject(TypedValue tv);

/*
 * Convert a cell to various raw data types, without changing the Cell.
 */
bool cellToBool(Cell);
int64_t cellToInt(Cell);
double cellToDouble(Cell);

/*
 * Convert `tv' or `cell' to a valid array key for `ad', or throw an exception.
 */
Cell cellToKey(Cell cell, const ArrayData* ad);
Cell tvToKey(TypedValue tv, const ArrayData* ad);

/*
 * Convert a string to a TypedNum following PHP semantics, allowing strings
 * that have only a partial number in them (i.e. the string may have junk after
 * the number).
 */
TypedNum stringToNumeric(const StringData*);

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/tv-conversions-inl.h"

#endif
