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

namespace {

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
static_assert(!isStringType(KindOfPersistentVArray),"");
static_assert(!isStringType(KindOfVArray),          "");
static_assert(!isStringType(KindOfPersistentVArray),"");
static_assert(!isStringType(KindOfDArray),          "");
static_assert(!isStringType(KindOfPersistentVec),   "");
static_assert(!isStringType(KindOfVec),             "");
static_assert(!isStringType(KindOfPersistentDict),  "");
static_assert(!isStringType(KindOfDict),            "");
static_assert(!isStringType(KindOfPersistentKeyset),"");
static_assert(!isStringType(KindOfKeyset),          "");
static_assert(!isStringType(KindOfObject),          "");
static_assert(!isStringType(KindOfResource),        "");

static_assert(isArrayType(KindOfArray),             "");
static_assert(isArrayType(KindOfPersistentArray),   "");
static_assert(isArrayType(KindOfVArray),            "");
static_assert(isArrayType(KindOfPersistentVArray),  "");
static_assert(isArrayType(KindOfDArray),            "");
static_assert(isArrayType(KindOfPersistentDArray),  "");
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

static_assert(isVecType(KindOfVec),                 "");
static_assert(isVecType(KindOfPersistentVec),       "");
static_assert(!isVecType(KindOfArray),              "");
static_assert(!isVecType(KindOfPersistentArray),    "");
static_assert(!isVecType(KindOfVArray),             "");
static_assert(!isVecType(KindOfPersistentVArray),   "");
static_assert(!isVecType(KindOfDArray),             "");
static_assert(!isVecType(KindOfPersistentDArray),   "");
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
static_assert(!isDictType(KindOfArray),             "");
static_assert(!isDictType(KindOfPersistentArray),   "");
static_assert(!isDictType(KindOfVArray),            "");
static_assert(!isDictType(KindOfPersistentVArray),  "");
static_assert(!isDictType(KindOfDArray),            "");
static_assert(!isDictType(KindOfPersistentDArray),  "");
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
static_assert(!isKeysetType(KindOfArray),           "");
static_assert(!isKeysetType(KindOfPersistentArray), "");
static_assert(!isKeysetType(KindOfVArray),          "");
static_assert(!isKeysetType(KindOfPersistentVArray),"");
static_assert(!isKeysetType(KindOfDArray),          "");
static_assert(!isKeysetType(KindOfPersistentDArray),"");
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

static_assert(isArrayLikeType(KindOfArray),             "");
static_assert(isArrayLikeType(KindOfPersistentArray),   "");
static_assert(isArrayLikeType(KindOfVArray),            "");
static_assert(isArrayLikeType(KindOfPersistentVArray),  "");
static_assert(isArrayLikeType(KindOfDArray),            "");
static_assert(isArrayLikeType(KindOfPersistentDArray),  "");
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

static_assert(isVArrayType(KindOfVArray),            "");
static_assert(isVArrayType(KindOfPersistentVArray),  "");
static_assert(!isVArrayType(KindOfArray),            "");
static_assert(!isVArrayType(KindOfPersistentArray),  "");
static_assert(!isVArrayType(KindOfDArray),           "");
static_assert(!isVArrayType(KindOfPersistentDArray), "");

static_assert(isDArrayType(KindOfDArray),            "");
static_assert(isDArrayType(KindOfPersistentDArray),  "");
static_assert(!isDArrayType(KindOfArray),            "");
static_assert(!isDArrayType(KindOfPersistentArray),  "");
static_assert(!isDArrayType(KindOfVArray),           "");
static_assert(!isDArrayType(KindOfPersistentVArray), "");

static_assert(isNullType(KindOfUninit),            "");
static_assert(isNullType(KindOfNull),              "");
static_assert(!isNullType(KindOfArray),            "");
static_assert(!isNullType(KindOfPersistentArray),  "");
static_assert(!isNullType(KindOfVArray),           "");
static_assert(!isNullType(KindOfPersistentVArray), "");
static_assert(!isNullType(KindOfDArray),           "");
static_assert(!isNullType(KindOfPersistentDArray), "");
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
static_assert(isRealType(KindOfArray), "");
static_assert(isRealType(KindOfPersistentArray), "");
static_assert(isRealType(KindOfVArray), "");
static_assert(isRealType(KindOfPersistentVArray), "");
static_assert(isRealType(KindOfDArray), "");
static_assert(isRealType(KindOfPersistentDArray), "");
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

static_assert(dt_with_rc(KindOfArray) == KindOfArray, "");
static_assert(dt_with_rc(KindOfPersistentArray) == KindOfArray, "");
static_assert(dt_with_rc(KindOfVArray) == KindOfVArray, "");
static_assert(dt_with_rc(KindOfPersistentVArray) == KindOfVArray, "");
static_assert(dt_with_rc(KindOfDArray) == KindOfDArray, "");
static_assert(dt_with_rc(KindOfPersistentDArray) == KindOfDArray, "");
static_assert(dt_with_rc(KindOfString) == KindOfString, "");
static_assert(dt_with_rc(KindOfPersistentString) == KindOfString, "");
static_assert(dt_with_rc(KindOfVec) == KindOfVec, "");
static_assert(dt_with_rc(KindOfPersistentVec) == KindOfVec, "");
static_assert(dt_with_rc(KindOfDict) == KindOfDict, "");
static_assert(dt_with_rc(KindOfPersistentDict) == KindOfDict, "");
static_assert(dt_with_rc(KindOfKeyset) == KindOfKeyset, "");
static_assert(dt_with_rc(KindOfPersistentKeyset) == KindOfKeyset, "");

static_assert(dt_modulo_persistence(KindOfPersistentArray) == KindOfArray, "");
static_assert(dt_modulo_persistence(KindOfArray) == KindOfArray, "");
static_assert(dt_modulo_persistence(KindOfPersistentVArray) == KindOfVArray, "");
static_assert(dt_modulo_persistence(KindOfVArray) == KindOfVArray, "");
static_assert(dt_modulo_persistence(KindOfPersistentDArray) == KindOfDArray, "");
static_assert(dt_modulo_persistence(KindOfDArray) == KindOfDArray, "");
static_assert(dt_modulo_persistence(KindOfPersistentString) == KindOfString, "");
static_assert(dt_modulo_persistence(KindOfString) == KindOfString, "");
static_assert(dt_modulo_persistence(KindOfPersistentVec) == KindOfVec, "");
static_assert(dt_modulo_persistence(KindOfVec) == KindOfVec, "");
static_assert(dt_modulo_persistence(KindOfPersistentDict) == KindOfDict, "");
static_assert(dt_modulo_persistence(KindOfDict) == KindOfDict, "");
static_assert(dt_modulo_persistence(KindOfPersistentKeyset) == KindOfKeyset, "");
static_assert(dt_modulo_persistence(KindOfKeyset) == KindOfKeyset, "");

static_assert(dt_with_persistence(KindOfArray) == KindOfPersistentArray, "");
static_assert(dt_with_persistence(KindOfPersistentArray) ==
              KindOfPersistentArray, "");
static_assert(dt_with_persistence(KindOfVArray) == KindOfPersistentVArray, "");
static_assert(dt_with_persistence(KindOfPersistentVArray) ==
              KindOfPersistentVArray, "");
static_assert(dt_with_persistence(KindOfDArray) == KindOfPersistentDArray, "");
static_assert(dt_with_persistence(KindOfPersistentDArray) ==
              KindOfPersistentDArray, "");
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
static_assert(isRefcountedType(KindOfArray),             "");
static_assert(isRefcountedType(KindOfVArray),            "");
static_assert(isRefcountedType(KindOfDArray),            "");
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
static_assert(!isRefcountedType(KindOfPersistentVArray), "");
static_assert(!isRefcountedType(KindOfPersistentDArray), "");
static_assert(!isRefcountedType(KindOfPersistentVec),    "");
static_assert(!isRefcountedType(KindOfPersistentDict),   "");
static_assert(!isRefcountedType(KindOfPersistentKeyset), "");

/* Too many cases to test exhaustively, so try to capture most scenarios */
static_assert(!equivDataTypes(KindOfNull, KindOfUninit),             "");
static_assert(equivDataTypes(KindOfArray, KindOfPersistentArray),   "");
static_assert(equivDataTypes(KindOfVArray, KindOfPersistentVArray), "");
static_assert(equivDataTypes(KindOfDArray, KindOfPersistentDArray), "");
static_assert(equivDataTypes(KindOfVec, KindOfPersistentVec),       "");
static_assert(equivDataTypes(KindOfDict, KindOfPersistentDict),     "");
static_assert(equivDataTypes(KindOfKeyset, KindOfPersistentKeyset), "");
static_assert(equivDataTypes(KindOfString, KindOfPersistentString), "");
static_assert(!equivDataTypes(KindOfNull, KindOfString),            "");
static_assert(!equivDataTypes(KindOfNull, KindOfInt64),             "");
static_assert(!equivDataTypes(KindOfNull, KindOfVec),               "");
static_assert(!equivDataTypes(KindOfBoolean, KindOfInt64),          "");
static_assert(!equivDataTypes(KindOfUninit, KindOfArray),           "");
static_assert(!equivDataTypes(KindOfUninit, KindOfDict),            "");
static_assert(!equivDataTypes(KindOfUninit, KindOfKeyset),          "");
static_assert(!equivDataTypes(KindOfObject, KindOfResource),        "");
static_assert(!equivDataTypes(KindOfObject, KindOfVec),             "");
static_assert(!equivDataTypes(KindOfObject, KindOfPersistentVec),   "");
static_assert(!equivDataTypes(KindOfObject, KindOfKeyset),          "");
static_assert(!equivDataTypes(KindOfObject, KindOfPersistentKeyset),"");
static_assert(!equivDataTypes(KindOfArray, KindOfString),           "");
static_assert(!equivDataTypes(KindOfArray, KindOfPersistentString), "");
static_assert(!equivDataTypes(KindOfArray, KindOfObject),           "");
static_assert(!equivDataTypes(KindOfArray, KindOfVec),              "");
static_assert(!equivDataTypes(KindOfArray, KindOfDict),             "");
static_assert(!equivDataTypes(KindOfArray, KindOfKeyset),           "");
static_assert(!equivDataTypes(KindOfArray, KindOfPersistentVec),    "");
static_assert(!equivDataTypes(KindOfArray, KindOfPersistentDict),   "");
static_assert(!equivDataTypes(KindOfArray, KindOfPersistentKeyset), "");
static_assert(!equivDataTypes(KindOfString, KindOfVec),             "");
static_assert(!equivDataTypes(KindOfString, KindOfDict),            "");
static_assert(!equivDataTypes(KindOfString, KindOfKeyset),          "");
static_assert(!equivDataTypes(KindOfString, KindOfPersistentVec),   "");
static_assert(!equivDataTypes(KindOfString, KindOfPersistentDict),  "");
static_assert(!equivDataTypes(KindOfString, KindOfPersistentKeyset),"");
// aggressive equivalence of php arraytypes, for now.
static_assert(equivDataTypes(KindOfArray, KindOfVArray),            "");
static_assert(equivDataTypes(KindOfArray, KindOfPersistentVArray),  "");
static_assert(equivDataTypes(KindOfArray, KindOfDArray),            "");
static_assert(equivDataTypes(KindOfArray, KindOfPersistentDArray),  "");
static_assert(equivDataTypes(KindOfVArray, KindOfPersistentArray),  "");
static_assert(equivDataTypes(KindOfVArray, KindOfPersistentDArray), "");
static_assert(equivDataTypes(KindOfDArray, KindOfPersistentArray),  "");
static_assert(equivDataTypes(KindOfDArray, KindOfPersistentVArray), "");
static_assert(equivDataTypes(KindOfPersistentVArray, KindOfArray),  "");
static_assert(equivDataTypes(KindOfPersistentDArray, KindOfArray),  "");

static_assert(KindOfUninit == static_cast<DataType>(0),
              "Several things assume this tag is 0, especially RDS");


} // namespace

///////////////////////////////////////////////////////////////////////////////

MaybeDataType get_datatype(
  const std::string& name,
  bool can_be_collection,
  bool is_nullable,
  bool is_soft
) {
  if (can_be_collection) {
    if (!strcasecmp(name.c_str(), "array"))      return KindOfArray;
    if (!strcasecmp(name.c_str(), "HH\\vec"))    return KindOfVec;
    if (!strcasecmp(name.c_str(), "HH\\dict"))   return KindOfDict;
    if (!strcasecmp(name.c_str(), "HH\\keyset")) return KindOfKeyset;
    if (!strcasecmp(name.c_str(), "HH\\varray")) {
      return RuntimeOption::EvalHackArrDVArrs
        ? KindOfVec : RuntimeOption::EvalSpecializeDVArray
        ? KindOfVArray : KindOfArray;
    }
    if (!strcasecmp(name.c_str(), "HH\\darray")) {
      return RuntimeOption::EvalHackArrDVArrs
        ? KindOfDict : RuntimeOption::EvalSpecializeDVArray
        ? KindOfDArray : KindOfArray;
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
    return RuntimeOption::EvalHackArrDVArrs
      ? KindOfVec : RuntimeOption::EvalSpecializeDVArray
      ? KindOfVArray : KindOfArray;
  }
  if (!strcasecmp(name.c_str(), "HH\\darray")) {
    return RuntimeOption::EvalHackArrDVArrs
      ? KindOfDict : RuntimeOption::EvalSpecializeDVArray
      ? KindOfDArray : KindOfArray;
  }
  if (!strcasecmp(name.c_str(), "HH\\varray_or_darray")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\vec_or_dict")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\arraylike")) return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\resource")) return KindOfResource;
  if (!strcasecmp(name.c_str(), "HH\\mixed"))    return folly::none;
  if (!strcasecmp(name.c_str(), "HH\\nonnull"))  return folly::none;

  return KindOfObject;
}

bool isVecOrArrayType(DataType t) {
  return RuntimeOption::EvalHackArrDVArrs ? isVecType(t) : isArrayType(t);
}

bool isDictOrArrayType(DataType t) {
  return RuntimeOption::EvalHackArrDVArrs ? isDictType(t) : isArrayType(t);
}

///////////////////////////////////////////////////////////////////////////////

}
