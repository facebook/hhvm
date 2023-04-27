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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/datatype-macros.h"

#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

namespace {

///////////////////////////////////////////////////////////////////////////////
// Static asserts.

template<int A, int B> struct CheckEqual {
  static_assert(A == B);
  static constexpr bool value = A == B;
};
static_assert(CheckEqual<UNINIT_DT_VALUE, int8_t(KindOfUninit)>::value);

static_assert(isStringType(KindOfString),           "");
static_assert(isStringType(KindOfPersistentString), "");
static_assert(!isStringType(KindOfUninit),          "");
static_assert(!isStringType(KindOfNull),            "");
static_assert(!isStringType(KindOfBoolean),         "");
static_assert(!isStringType(KindOfInt64),           "");
static_assert(!isStringType(KindOfDouble),          "");
static_assert(!isStringType(KindOfPersistentVec),   "");
static_assert(!isStringType(KindOfVec),             "");
static_assert(!isStringType(KindOfPersistentDict),  "");
static_assert(!isStringType(KindOfDict),            "");
static_assert(!isStringType(KindOfPersistentKeyset),"");
static_assert(!isStringType(KindOfKeyset),          "");
static_assert(!isStringType(KindOfObject),          "");
static_assert(!isStringType(KindOfResource),        "");

static_assert(isVecType(KindOfVec),                 "");
static_assert(isVecType(KindOfPersistentVec),       "");
static_assert(!isVecType(KindOfDict),               "");
static_assert(!isVecType(KindOfPersistentDict),     "");
static_assert(!isVecType(KindOfKeyset),             "");
static_assert(!isVecType(KindOfPersistentKeyset),   "");
static_assert(!isVecType(KindOfUninit),             "");
static_assert(!isVecType(KindOfNull),               "");
static_assert(!isVecType(KindOfBoolean),            "");
static_assert(!isVecType(KindOfInt64),              "");
static_assert(!isVecType(KindOfDouble),             "");
static_assert(!isVecType(KindOfPersistentString),   "");
static_assert(!isVecType(KindOfString),             "");
static_assert(!isVecType(KindOfObject),             "");
static_assert(!isVecType(KindOfResource),           "");

static_assert(isDictType(KindOfDict),               "");
static_assert(isDictType(KindOfPersistentDict),     "");
static_assert(!isDictType(KindOfVec),               "");
static_assert(!isDictType(KindOfPersistentVec),     "");
static_assert(!isDictType(KindOfKeyset),            "");
static_assert(!isDictType(KindOfPersistentKeyset),  "");
static_assert(!isDictType(KindOfUninit),            "");
static_assert(!isDictType(KindOfNull),              "");
static_assert(!isDictType(KindOfBoolean),           "");
static_assert(!isDictType(KindOfInt64),             "");
static_assert(!isDictType(KindOfDouble),            "");
static_assert(!isDictType(KindOfPersistentString),  "");
static_assert(!isDictType(KindOfString),            "");
static_assert(!isDictType(KindOfObject),            "");
static_assert(!isDictType(KindOfResource),          "");

static_assert(isKeysetType(KindOfKeyset),           "");
static_assert(isKeysetType(KindOfPersistentKeyset), "");
static_assert(!isKeysetType(KindOfVec),             "");
static_assert(!isKeysetType(KindOfPersistentVec),   "");
static_assert(!isKeysetType(KindOfDict),            "");
static_assert(!isKeysetType(KindOfPersistentDict),  "");
static_assert(!isKeysetType(KindOfUninit),          "");
static_assert(!isKeysetType(KindOfNull),            "");
static_assert(!isKeysetType(KindOfBoolean),         "");
static_assert(!isKeysetType(KindOfInt64),           "");
static_assert(!isKeysetType(KindOfDouble),          "");
static_assert(!isKeysetType(KindOfPersistentString),"");
static_assert(!isKeysetType(KindOfString),          "");
static_assert(!isKeysetType(KindOfObject),          "");
static_assert(!isKeysetType(KindOfResource),        "");

static_assert(isArrayLikeType(KindOfVec),               "");
static_assert(isArrayLikeType(KindOfPersistentVec),     "");
static_assert(isArrayLikeType(KindOfDict),              "");
static_assert(isArrayLikeType(KindOfPersistentDict),    "");
static_assert(isArrayLikeType(KindOfKeyset),            "");
static_assert(isArrayLikeType(KindOfPersistentKeyset),  "");
static_assert(!isArrayLikeType(KindOfUninit),           "");
static_assert(!isArrayLikeType(KindOfNull),             "");
static_assert(!isArrayLikeType(KindOfBoolean),          "");
static_assert(!isArrayLikeType(KindOfInt64),            "");
static_assert(!isArrayLikeType(KindOfDouble),           "");
static_assert(!isArrayLikeType(KindOfPersistentString), "");
static_assert(!isArrayLikeType(KindOfString),           "");
static_assert(!isArrayLikeType(KindOfObject),           "");
static_assert(!isArrayLikeType(KindOfResource),         "");

static_assert(isNullType(KindOfUninit),            "");
static_assert(isNullType(KindOfNull),              "");
static_assert(!isNullType(KindOfVec),              "");
static_assert(!isNullType(KindOfPersistentVec),    "");
static_assert(!isNullType(KindOfDict),             "");
static_assert(!isNullType(KindOfPersistentDict),   "");
static_assert(!isNullType(KindOfKeyset),           "");
static_assert(!isNullType(KindOfPersistentKeyset), "");
static_assert(!isNullType(KindOfBoolean),          "");
static_assert(!isNullType(KindOfInt64),            "");
static_assert(!isNullType(KindOfDouble),           "");
static_assert(!isNullType(KindOfPersistentString), "");
static_assert(!isNullType(KindOfString),           "");
static_assert(!isNullType(KindOfObject),           "");
static_assert(!isNullType(KindOfResource),         "");

static_assert(isRealType(KindOfUninit), "");
static_assert(isRealType(KindOfNull), "");
static_assert(isRealType(KindOfVec), "");
static_assert(isRealType(KindOfPersistentVec), "");
static_assert(isRealType(KindOfDict), "");
static_assert(isRealType(KindOfPersistentDict), "");
static_assert(isRealType(KindOfKeyset), "");
static_assert(isRealType(KindOfPersistentKeyset), "");
static_assert(isRealType(KindOfBoolean), "");
static_assert(isRealType(KindOfInt64), "");
static_assert(isRealType(KindOfDouble), "");
static_assert(isRealType(KindOfPersistentString), "");
static_assert(isRealType(KindOfString), "");
static_assert(isRealType(KindOfObject), "");
static_assert(isRealType(KindOfResource), "");

static_assert(dt_with_rc(KindOfString) == KindOfString, "");
static_assert(dt_with_rc(KindOfPersistentString) == KindOfString, "");
static_assert(dt_with_rc(KindOfVec) == KindOfVec, "");
static_assert(dt_with_rc(KindOfPersistentVec) == KindOfVec, "");
static_assert(dt_with_rc(KindOfDict) == KindOfDict, "");
static_assert(dt_with_rc(KindOfPersistentDict) == KindOfDict, "");
static_assert(dt_with_rc(KindOfKeyset) == KindOfKeyset, "");
static_assert(dt_with_rc(KindOfPersistentKeyset) == KindOfKeyset, "");

static_assert(dt_modulo_persistence(KindOfPersistentString) == KindOfString, "");
static_assert(dt_modulo_persistence(KindOfString) == KindOfString, "");
static_assert(dt_modulo_persistence(KindOfPersistentVec) == KindOfVec, "");
static_assert(dt_modulo_persistence(KindOfVec) == KindOfVec, "");
static_assert(dt_modulo_persistence(KindOfPersistentDict) == KindOfDict, "");
static_assert(dt_modulo_persistence(KindOfDict) == KindOfDict, "");
static_assert(dt_modulo_persistence(KindOfPersistentKeyset) == KindOfKeyset, "");
static_assert(dt_modulo_persistence(KindOfKeyset) == KindOfKeyset, "");

static_assert(dt_with_persistence(KindOfString) == KindOfPersistentString, "");
static_assert(dt_with_persistence(KindOfPersistentString) ==
              KindOfPersistentString, "");
static_assert(dt_with_persistence(KindOfVec) == KindOfPersistentVec, "");
static_assert(dt_with_persistence(KindOfPersistentVec) == KindOfPersistentVec, "");
static_assert(dt_with_persistence(KindOfDict) == KindOfPersistentDict, "");
static_assert(dt_with_persistence(KindOfPersistentDict) ==
              KindOfPersistentDict, "");
static_assert(dt_with_persistence(KindOfKeyset) == KindOfPersistentKeyset, "");
static_assert(dt_with_persistence(KindOfPersistentKeyset) ==
              KindOfPersistentKeyset, "");

static_assert(isRefcountedType(KindOfString),            "");
static_assert(isRefcountedType(KindOfObject),            "");
static_assert(isRefcountedType(KindOfResource),          "");
static_assert(isRefcountedType(KindOfVec),               "");
static_assert(isRefcountedType(KindOfDict),              "");
static_assert(isRefcountedType(KindOfKeyset),            "");
static_assert(!isRefcountedType(KindOfUninit),           "");
static_assert(!isRefcountedType(KindOfNull),             "");
static_assert(!isRefcountedType(KindOfBoolean),          "");
static_assert(!isRefcountedType(KindOfInt64),            "");
static_assert(!isRefcountedType(KindOfDouble),           "");
static_assert(!isRefcountedType(KindOfPersistentString), "");
static_assert(!isRefcountedType(KindOfPersistentVec),    "");
static_assert(!isRefcountedType(KindOfPersistentDict),   "");
static_assert(!isRefcountedType(KindOfPersistentKeyset), "");

/* Too many cases to test exhaustively, so try to capture most scenarios */
static_assert(!equivDataTypes(KindOfNull, KindOfUninit),             "");
static_assert(equivDataTypes(KindOfVec, KindOfPersistentVec),       "");
static_assert(equivDataTypes(KindOfDict, KindOfPersistentDict),     "");
static_assert(equivDataTypes(KindOfKeyset, KindOfPersistentKeyset), "");
static_assert(equivDataTypes(KindOfString, KindOfPersistentString), "");
static_assert(!equivDataTypes(KindOfNull, KindOfString),            "");
static_assert(!equivDataTypes(KindOfNull, KindOfInt64),             "");
static_assert(!equivDataTypes(KindOfNull, KindOfVec),               "");
static_assert(!equivDataTypes(KindOfBoolean, KindOfInt64),          "");
static_assert(!equivDataTypes(KindOfUninit, KindOfDict),            "");
static_assert(!equivDataTypes(KindOfUninit, KindOfKeyset),          "");
static_assert(!equivDataTypes(KindOfObject, KindOfResource),        "");
static_assert(!equivDataTypes(KindOfObject, KindOfVec),             "");
static_assert(!equivDataTypes(KindOfObject, KindOfPersistentVec),   "");
static_assert(!equivDataTypes(KindOfObject, KindOfKeyset),          "");
static_assert(!equivDataTypes(KindOfObject, KindOfPersistentKeyset),"");
static_assert(!equivDataTypes(KindOfString, KindOfVec),             "");
static_assert(!equivDataTypes(KindOfString, KindOfDict),            "");
static_assert(!equivDataTypes(KindOfString, KindOfKeyset),          "");
static_assert(!equivDataTypes(KindOfString, KindOfPersistentVec),   "");
static_assert(!equivDataTypes(KindOfString, KindOfPersistentDict),  "");
static_assert(!equivDataTypes(KindOfString, KindOfPersistentKeyset),"");

} // namespace

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

}
