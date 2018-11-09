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

#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Static asserts.

static_assert(isStringType(KindOfString),           "");
static_assert(isStringType(KindOfPersistentString), "");
static_assert(!isStringType(KindOfUninit),          "");
static_assert(!isStringType(KindOfNull),            "");
static_assert(!isStringType(KindOfBoolean),         "");
static_assert(!isStringType(KindOfInt64),           "");
static_assert(!isStringType(KindOfDouble),          "");
static_assert(!isStringType(KindOfPersistentArray), "");
static_assert(!isStringType(KindOfArray),           "");
static_assert(!isStringType(KindOfPersistentVec),   "");
static_assert(!isStringType(KindOfVec),             "");
static_assert(!isStringType(KindOfPersistentDict),  "");
static_assert(!isStringType(KindOfDict),            "");
static_assert(!isStringType(KindOfPersistentKeyset),"");
static_assert(!isStringType(KindOfKeyset),          "");
static_assert(!isStringType(KindOfObject),          "");
static_assert(!isStringType(KindOfResource),        "");
static_assert(!isStringType(KindOfRef),             "");

static_assert(isArrayType(KindOfArray),             "");
static_assert(isArrayType(KindOfPersistentArray),   "");
static_assert(!isArrayType(KindOfVec),              "");
static_assert(!isArrayType(KindOfPersistentVec),    "");
static_assert(!isArrayType(KindOfDict),             "");
static_assert(!isArrayType(KindOfPersistentDict),   "");
static_assert(!isArrayType(KindOfKeyset),           "");
static_assert(!isArrayType(KindOfPersistentKeyset), "");
static_assert(!isArrayType(KindOfUninit),           "");
static_assert(!isArrayType(KindOfNull),             "");
static_assert(!isArrayType(KindOfBoolean),          "");
static_assert(!isArrayType(KindOfInt64),            "");
static_assert(!isArrayType(KindOfDouble),           "");
static_assert(!isArrayType(KindOfPersistentString), "");
static_assert(!isArrayType(KindOfString),           "");
static_assert(!isArrayType(KindOfObject),           "");
static_assert(!isArrayType(KindOfResource),         "");
static_assert(!isArrayType(KindOfRef),              "");

static_assert(isVecType(KindOfVec),                 "");
static_assert(isVecType(KindOfPersistentVec),       "");
static_assert(!isVecType(KindOfArray),              "");
static_assert(!isVecType(KindOfPersistentArray),    "");
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
static_assert(!isVecType(KindOfRef),                "");

static_assert(isDictType(KindOfDict),               "");
static_assert(isDictType(KindOfPersistentDict),     "");
static_assert(!isDictType(KindOfArray),             "");
static_assert(!isDictType(KindOfPersistentArray),   "");
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
static_assert(!isDictType(KindOfRef),               "");

static_assert(isKeysetType(KindOfKeyset),           "");
static_assert(isKeysetType(KindOfPersistentKeyset), "");
static_assert(!isKeysetType(KindOfArray),           "");
static_assert(!isKeysetType(KindOfPersistentArray), "");
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
static_assert(!isKeysetType(KindOfRef),             "");

static_assert(isArrayLikeType(KindOfArray),             "");
static_assert(isArrayLikeType(KindOfPersistentArray),   "");
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
static_assert(!isArrayLikeType(KindOfRef),              "");

static_assert(isNullType(KindOfUninit),            "");
static_assert(isNullType(KindOfNull),              "");
static_assert(!isNullType(KindOfArray),            "");
static_assert(!isNullType(KindOfPersistentArray),  "");
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
static_assert(!isNullType(KindOfRef),              "");

static_assert(isRefcountedType(KindOfString),            "");
static_assert(isRefcountedType(KindOfObject),            "");
static_assert(isRefcountedType(KindOfResource),          "");
static_assert(isRefcountedType(KindOfRef),               "");
static_assert(isRefcountedType(KindOfArray),             "");
static_assert(isRefcountedType(KindOfVec),               "");
static_assert(isRefcountedType(KindOfDict),              "");
static_assert(isRefcountedType(KindOfKeyset),            "");
static_assert(!isRefcountedType(KindOfUninit),           "");
static_assert(!isRefcountedType(KindOfNull),             "");
static_assert(!isRefcountedType(KindOfBoolean),          "");
static_assert(!isRefcountedType(KindOfInt64),            "");
static_assert(!isRefcountedType(KindOfDouble),           "");
static_assert(!isRefcountedType(KindOfPersistentString), "");
static_assert(!isRefcountedType(KindOfPersistentArray),  "");
static_assert(!isRefcountedType(KindOfPersistentVec),    "");
static_assert(!isRefcountedType(KindOfPersistentDict),   "");
static_assert(!isRefcountedType(KindOfPersistentKeyset), "");

/* Too many cases to test exhaustively, so try to capture most scenarios */
static_assert(!sameDataTypes(KindOfNull, KindOfUninit),             "");
static_assert(sameDataTypes(KindOfArray, KindOfPersistentArray),   "");
static_assert(sameDataTypes(KindOfVec, KindOfPersistentVec),       "");
static_assert(sameDataTypes(KindOfDict, KindOfPersistentDict),     "");
static_assert(sameDataTypes(KindOfKeyset, KindOfPersistentKeyset), "");
static_assert(sameDataTypes(KindOfString, KindOfPersistentString), "");
static_assert(!sameDataTypes(KindOfNull, KindOfString),            "");
static_assert(!sameDataTypes(KindOfNull, KindOfInt64),             "");
static_assert(!sameDataTypes(KindOfNull, KindOfVec),               "");
static_assert(!sameDataTypes(KindOfBoolean, KindOfInt64),          "");
static_assert(!sameDataTypes(KindOfUninit, KindOfArray),           "");
static_assert(!sameDataTypes(KindOfUninit, KindOfDict),            "");
static_assert(!sameDataTypes(KindOfUninit, KindOfKeyset),          "");
static_assert(!sameDataTypes(KindOfObject, KindOfResource),        "");
static_assert(!sameDataTypes(KindOfObject, KindOfVec),             "");
static_assert(!sameDataTypes(KindOfObject, KindOfPersistentVec),   "");
static_assert(!sameDataTypes(KindOfObject, KindOfKeyset),          "");
static_assert(!sameDataTypes(KindOfObject, KindOfPersistentKeyset),"");
static_assert(!sameDataTypes(KindOfArray, KindOfString),           "");
static_assert(!sameDataTypes(KindOfArray, KindOfPersistentString), "");
static_assert(!sameDataTypes(KindOfArray, KindOfObject),           "");
static_assert(!sameDataTypes(KindOfArray, KindOfVec),              "");
static_assert(!sameDataTypes(KindOfArray, KindOfDict),             "");
static_assert(!sameDataTypes(KindOfArray, KindOfKeyset),           "");
static_assert(!sameDataTypes(KindOfArray, KindOfPersistentVec),    "");
static_assert(!sameDataTypes(KindOfArray, KindOfPersistentDict),   "");
static_assert(!sameDataTypes(KindOfArray, KindOfPersistentKeyset), "");
static_assert(!sameDataTypes(KindOfString, KindOfVec),             "");
static_assert(!sameDataTypes(KindOfString, KindOfDict),            "");
static_assert(!sameDataTypes(KindOfString, KindOfKeyset),          "");
static_assert(!sameDataTypes(KindOfString, KindOfPersistentVec),   "");
static_assert(!sameDataTypes(KindOfString, KindOfPersistentDict),  "");
static_assert(!sameDataTypes(KindOfString, KindOfPersistentKeyset),"");

static_assert(KindOfUninit == static_cast<DataType>(0),
              "Several things assume this tag is 0, especially RDS");

///////////////////////////////////////////////////////////////////////////////

MaybeDataType get_datatype(
  const std::string& name,
  bool can_be_collection,
  bool is_function,
  bool is_xhp,
  bool is_tuple,
  bool is_nullable,
  bool is_soft
) {
  if (is_function || is_xhp || is_tuple) {
    return KindOfObject;
  }
  if (can_be_collection) {
    if (!strcasecmp(name.c_str(), "array"))      return KindOfArray;
    if (!strcasecmp(name.c_str(), "HH\\vec"))    return KindOfVec;
    if (!strcasecmp(name.c_str(), "HH\\dict"))   return KindOfDict;
    if (!strcasecmp(name.c_str(), "HH\\keyset")) return KindOfKeyset;
    if (!strcasecmp(name.c_str(), "HH\\varray")) {
      return RuntimeOption::EvalHackArrDVArrs ? KindOfVec : KindOfArray;
    }
    if (!strcasecmp(name.c_str(), "HH\\darray")) {
      return RuntimeOption::EvalHackArrDVArrs ? KindOfDict : KindOfArray;
    }
    if (!strcasecmp(name.c_str(), "HH\\varray_or_darray")) return folly::none;
    if (!strcasecmp(name.c_str(), "HH\\vec_or_dict")) return folly::none;
    if (!strcasecmp(name.c_str(), "HH\\arraylike")) return folly::none;
    return KindOfObject;
  }
  if (is_nullable || is_soft) {
    return folly::none;
  }
  if (!strcasecmp(name.c_str(), "null") ||
      !strcasecmp(name.c_str(), "HH\\null") ||
      !strcasecmp(name.c_str(), "HH\\void")) {
    return KindOfNull;
  }
  if (!strcasecmp(name.c_str(), "HH\\bool"))     return KindOfBoolean;
  if (!strcasecmp(name.c_str(), "HH\\int"))      return KindOfInt64;
  if (!strcasecmp(name.c_str(), "HH\\float"))    return KindOfDouble;
  if (!strcasecmp(name.c_str(), "HH\\num"))      return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\arraykey")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\string"))   return KindOfString;
  if (!strcasecmp(name.c_str(), "array"))        return KindOfArray;
  if (!strcasecmp(name.c_str(), "HH\\dict"))     return KindOfDict;
  if (!strcasecmp(name.c_str(), "HH\\vec"))      return KindOfVec;
  if (!strcasecmp(name.c_str(), "HH\\keyset"))   return KindOfKeyset;
  if (!strcasecmp(name.c_str(), "HH\\varray")) {
    return RuntimeOption::EvalHackArrDVArrs ? KindOfVec : KindOfArray;
  }
  if (!strcasecmp(name.c_str(), "HH\\darray")) {
    return RuntimeOption::EvalHackArrDVArrs ? KindOfDict : KindOfArray;
  }
  if (!strcasecmp(name.c_str(), "HH\\varray_or_darray")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\vec_or_dict")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\arraylike")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\resource")) return KindOfResource;
  if (!strcasecmp(name.c_str(), "HH\\mixed"))    return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\nonnull"))  return folly::none;

  return KindOfObject;
}

bool isArrayOrShapeType(DataType t) {
  return isArrayType(t) ||
    (!RuntimeOption::EvalHackArrDVArrs && isShapeType(t));
}
bool isArrayOrShapeType(MaybeDataType t) {
  return t && isArrayOrShapeType(*t);
}

bool isDictOrShapeType(DataType t) {
  return isDictType(t) ||
    (RuntimeOption::EvalHackArrDVArrs && isShapeType(t));
}
bool isDictOrShapeType(MaybeDataType t) {
  return t && isDictOrShapeType(*t);
}

bool equivDataTypes(DataType t1, DataType t2) {
  return sameDataTypes(t1, t2) ||
    (RuntimeOption::EvalHackArrDVArrs ?
      ((isShapeType(t1) && isDictType(t2)) ||
      (isDictType(t1) && isShapeType(t2))) :
      ((isShapeType(t1) && isArrayType(t2)) ||
      (isArrayType(t1) && isShapeType(t2))));
}

///////////////////////////////////////////////////////////////////////////////

}
