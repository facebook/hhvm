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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/req-root.h"
#include "hphp/runtime/base/tv-conv-notice.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-string.h"
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
template<typename T> \
enable_if_lval_t<T, void> tvCastTo##kind##InPlace(T tv);
#define Y(kind) \
template<typename T, IntishCast IC = IntishCast::None> \
enable_if_lval_t<T, void> tvCastTo##kind##InPlace(T tv);
X(Boolean)
X(Int64)
X(Double)
X(String)
Y(Array)
X(Vec)
X(Dict)
X(Keyset)
X(Object)
X(NullableObject)
X(Resource)
#undef Y
#undef X

template<typename T>
enable_if_lval_t<T, void> tvCastToStringInPlace(T tv);
void tvSetLegacyArrayInPlace(tv_lval tv, bool isLegacy);

/*
 * Non-in-place casts.
 */
bool tvCastToBoolean(TypedValue tv);
int64_t tvCastToInt64(TypedValue tv);
double tvCastToDouble(TypedValue tv);
String tvCastToString(TypedValue tv);
template <IntishCast IC = IntishCast::None>
Array tvCastToArrayLike(TypedValue tv);

StringData* tvCastToStringData(TypedValue tv);
StringData* tvCastToStringData(
   TypedValue tv, const ConvNoticeLevel, const StringData* notice_reason);
template <IntishCast IC /* = IntishCast::None */>
ArrayData* tvCastToArrayLikeData(TypedValue tv);
ObjectData* tvCastToObjectData(TypedValue tv);

/*
 * Convert a cell to various raw data types, without changing the TypedValue.
 */
bool tvToBool(TypedValue);
int64_t tvToInt(TypedValue);
int64_t tvToInt(TypedValue,
                const ConvNoticeLevel,
                const StringData* notice_reason,
                bool notice_within_num = true);

double tvToDouble(TypedValue);

/*
 * Convert `tv' or `cell' to a valid array key for `ad', or throw an exception.
 */
template <IntishCast IC = IntishCast::None>
TypedValue tvToKey(TypedValue cell, const ArrayData* ad);

/*
 * Convert a string to a TypedNum following PHP semantics, allowing strings
 * that have only a partial number in them (i.e. the string may have junk after
 * the number).
 */
TypedNum stringToNumeric(const StringData*);
TypedNum stringToNumeric(
    const StringData*, const ConvNoticeLevel, const StringData* notice_reason);

TypedValue tvClassToString(TypedValue key);

}

#include "hphp/runtime/base/tv-conversions-inl.h"
