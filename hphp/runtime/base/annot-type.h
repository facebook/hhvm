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

#ifndef incl_HPHP_ANNOT_TYPE_H_
#define incl_HPHP_ANNOT_TYPE_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

struct StringData;

///////////////////////////////////////////////////////////////////////////////

/**
 * AnnotMetaType and AnnotType depend on each other, so if you change one
 * of them you must update the other.
 *
 * Setting AnnotMetaType::Precise = 0 makes machine code slightly nicer
 * for some operations, though it's not needed for correctness.
 */
enum class AnnotMetaType : uint8_t {
  Precise = 0,
  Mixed = 1,
  Self = 2,
  Parent = 3,
  Callable = 4,
  Number = 5,
  ArrayKey = 6,
  This = 7,
  VArray = 8,
  DArray = 9,
  VArrOrDArr = 10,
  VecOrDict = 11,
  ArrayLike = 12,
  Nonnull = 13,
  NoReturn = 14,
  Nothing = 15
};

enum class AnnotType : uint16_t {
  Null     = (uint8_t)KindOfNull     | (uint16_t)AnnotMetaType::Precise << 8,
  Bool     = (uint8_t)KindOfBoolean  | (uint16_t)AnnotMetaType::Precise << 8,
  Int      = (uint8_t)KindOfInt64    | (uint16_t)AnnotMetaType::Precise << 8,
  Float    = (uint8_t)KindOfDouble   | (uint16_t)AnnotMetaType::Precise << 8,
  String   = (uint8_t)KindOfString   | (uint16_t)AnnotMetaType::Precise << 8,
  Array    = (uint8_t)KindOfArray    | (uint16_t)AnnotMetaType::Precise << 8,
  Object   = (uint8_t)KindOfObject   | (uint16_t)AnnotMetaType::Precise << 8,
  Resource = (uint8_t)KindOfResource | (uint16_t)AnnotMetaType::Precise << 8,
  Dict     = (uint8_t)KindOfDict     | (uint16_t)AnnotMetaType::Precise << 8,
  Vec      = (uint8_t)KindOfVec      | (uint16_t)AnnotMetaType::Precise << 8,
  Keyset   = (uint8_t)KindOfKeyset   | (uint16_t)AnnotMetaType::Precise << 8,
  Record   = (uint8_t)KindOfRecord   | (uint16_t)AnnotMetaType::Precise << 8,
  // Precise is intentionally excluded
  Mixed    = (uint16_t)AnnotMetaType::Mixed << 8        | (uint8_t)KindOfUninit,
  Nonnull  = (uint16_t)AnnotMetaType::Nonnull << 8      | (uint8_t)KindOfUninit,
  Self     = (uint16_t)AnnotMetaType::Self << 8         | (uint8_t)KindOfUninit,
  Parent   = (uint16_t)AnnotMetaType::Parent << 8       | (uint8_t)KindOfUninit,
  Callable = (uint16_t)AnnotMetaType::Callable << 8     | (uint8_t)KindOfUninit,
  Number   = (uint16_t)AnnotMetaType::Number << 8       | (uint8_t)KindOfUninit,
  ArrayKey = (uint16_t)AnnotMetaType::ArrayKey << 8     | (uint8_t)KindOfUninit,
  This     = (uint16_t)AnnotMetaType::This << 8         | (uint8_t)KindOfUninit,
  VArray   = (uint16_t)AnnotMetaType::VArray << 8       | (uint8_t)KindOfUninit,
  DArray   = (uint16_t)AnnotMetaType::DArray << 8       | (uint8_t)KindOfUninit,
  VArrOrDArr = (uint16_t)AnnotMetaType::VArrOrDArr << 8 | (uint8_t)KindOfUninit,
  VecOrDict  = (uint16_t)AnnotMetaType::VecOrDict << 8  | (uint8_t)KindOfUninit,
  ArrayLike  = (uint16_t)AnnotMetaType::ArrayLike << 8  | (uint8_t)KindOfUninit,
  NoReturn   = (uint16_t)AnnotMetaType::NoReturn << 8   | (uint8_t)KindOfUninit,
  Nothing    = (uint16_t)AnnotMetaType::Nothing << 8    | (uint8_t)KindOfUninit
};

inline AnnotMetaType getAnnotMetaType(AnnotType at) {
  return (AnnotMetaType)((uint16_t)at >> 8);
}

inline DataType getAnnotDataType(AnnotType at) {
  auto const dt = (DataType)(uint8_t)at;
  return dt;
}

inline AnnotType dataTypeToAnnotType(DataType dt) {
  assertx(dt == KindOfBoolean || dt == KindOfInt64 ||
         dt == KindOfDouble || dt == KindOfString || dt == KindOfArray ||
         dt == KindOfVec || dt == KindOfDict || dt == KindOfKeyset ||
         dt == KindOfObject || dt == KindOfResource);
  return (AnnotType)((uint8_t)dt | (uint16_t)AnnotMetaType::Precise << 8);
}

const AnnotType* nameToAnnotType(const StringData* typeName);
const AnnotType* nameToAnnotType(const std::string& typeName);
MaybeDataType nameToMaybeDataType(const StringData* typeName);
MaybeDataType nameToMaybeDataType(const std::string& typeName);

/*
 * Returns true if the interface with the specified name
 * supports any non-object types, false otherwise.
 */
bool interface_supports_non_objects(const StringData* s);

bool interface_supports_int(const StringData* s);
bool interface_supports_double(const StringData* s);
bool interface_supports_string(const StringData* s);
bool interface_supports_array(const StringData* s);
bool interface_supports_shape(const StringData* s);
bool interface_supports_vec(const StringData* s);
bool interface_supports_dict(const StringData* s);
bool interface_supports_keyset(const StringData* s);

bool interface_supports_int(std::string const&);
bool interface_supports_double(std::string const&);
bool interface_supports_string(std::string const&);
bool interface_supports_array(std::string const&);
bool interface_supports_shape(std::string const&);
bool interface_supports_vec(std::string const&);
bool interface_supports_dict(std::string const&);
bool interface_supports_keyset(std::string const&);

TypedValue annotDefaultValue(AnnotType at);

enum class AnnotAction {
  Pass,
  Fail,
  ObjectCheck,
  CallableCheck,
  VArrayCheck,
  DArrayCheck,
  VArrayOrDArrayCheck,
  NonVArrayOrDArrayCheck,
  WarnFunc,
  ConvertFunc,
  WarnClass,
  ConvertClass,
  ClsMethCheck,
  RecordCheck,
};

/*
 * annotCompat() takes a DataType (`dt') and tries to determine if a value
 * with DataType `dt' could be compatiable with a given AnnotType (`at')
 * and class name (`annotClsName').  Note that this function does not have
 * access to the actual value, nor does it do any run time resolution on
 * the annotation's class name.  Here are the possible values that can be
 * returned:
 *
 * Pass: A value with DataType `dt' will always be compatible with the
 * annotation at run time.
 *
 * Fail: A value with DataType `dt' will never be compatible with the
 * annotation at run time.  NOTE that if the annotation is "array" and the
 * value is a collection object, this function will return Fail but the
 * runtime might possibly cast the collection to an array and allow normal
 * execution to continue (see TypeConstraint::verifyFail() for details). In
 * addition in Weak mode verifyFail may coerce certain types allowing
 * execution to continue.
 *
 * CallableCheck: `at' is "callable" and a value with DataType `dt' might
 * be compatible with the annotation, but the caller needs to consult
 * is_callable() to verify the value is actually a valid callable.
 *
 * ObjectCheck: `at' is either (1) a reference to a real non-enum class or
 * interface, (2) an enum, (3) a type alias, or (4) "self" or "parent".
 * The caller needs to perform more checks to determine whether or not a
 * value with DataType `dt' is compatible with the annotation.  NOTE that
 * if dt is not KindOfObject, then we've already checked if the annotation
 * was a direct reference to a "magic" interface that supports non-object
 * types and we've already checked if the annotation was "self" / "parent",
 * but the caller still needs to check if the annotation is a type alias or
 * an enum.
 *
 * VArrayCheck: `dt' is an array on which the caller needs to do a varray check.
 *
 * DArrayCheck: `dt' is an array on which the caller needs to do a darray check.
 *
 * VArrayOrDArrayCheck: `dt' is an array on which the caller needs to do a
 * varray or darray check.
 *
 * NonVArrayOrDArrayCheck: `dt' is an array on which the caller needs to check
 * for non-dvarray-ness.
 *
 * RecordCheck: 'at' and 'dt' are both records and the caller needs to check
 * if the record in the value matches annotation.
 *
 */
inline AnnotAction
annotCompat(DataType dt, AnnotType at, const StringData* annotClsName) {
  assertx(IMPLIES(at == AnnotType::Object, annotClsName != nullptr));

  auto const metatype = getAnnotMetaType(at);
  switch (metatype) {
    case AnnotMetaType::Mixed:
      return AnnotAction::Pass;
    case AnnotMetaType::Nonnull:
      return (dt == KindOfNull) ? AnnotAction::Fail : AnnotAction::Pass;
    case AnnotMetaType::Number:
      return (isIntType(dt) || isDoubleType(dt))
        ? AnnotAction::Pass : AnnotAction::Fail;
    case AnnotMetaType::ArrayKey:
      return (isIntType(dt) || isStringType(dt))
        ? AnnotAction::Pass : AnnotAction::Fail;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
      // For "self" and "parent", if `dt' is not an object we know
      // it's not compatible, otherwise more checks are required
      return (dt == KindOfObject)
        ? AnnotAction::ObjectCheck : AnnotAction::Fail;
    case AnnotMetaType::This:
      return (dt == KindOfObject)
        ? AnnotAction::ObjectCheck
        : AnnotAction::Fail;
    case AnnotMetaType::Callable:
      // For "callable", if `dt' is not string/array/object/func we know
      // it's not compatible, otherwise more checks are required
      return (isStringType(dt) || isArrayType(dt) || isVecType(dt) ||
              isFuncType(dt) || dt == KindOfObject || isClsMethType(dt))
        ? AnnotAction::CallableCheck : AnnotAction::Fail;
    case AnnotMetaType::VArray:
      if (isClsMethType(dt)) {
        return RuntimeOption::EvalHackArrDVArrs ?
          AnnotAction::Fail : AnnotAction::ClsMethCheck;
      }
      if (!isArrayType(dt)) return AnnotAction::Fail;
      return UNLIKELY(RuntimeOption::EvalHackArrCompatTypeHintNotices)
        ? AnnotAction::VArrayCheck
        : AnnotAction::Pass;
    case AnnotMetaType::DArray:
      if (isClsMethType(dt)) {
        return RuntimeOption::EvalHackArrDVArrs ?
          AnnotAction::Fail : AnnotAction::ClsMethCheck;
      }
      if (!isArrayType(dt)) return AnnotAction::Fail;
      return UNLIKELY(RuntimeOption::EvalHackArrCompatTypeHintNotices)
        ? AnnotAction::DArrayCheck
        : AnnotAction::Pass;
    case AnnotMetaType::VArrOrDArr:
      if (isClsMethType(dt)) {
        return RuntimeOption::EvalHackArrDVArrs ?
          AnnotAction::Fail : AnnotAction::ClsMethCheck;
      }
      if (!isArrayType(dt)) return AnnotAction::Fail;
      return UNLIKELY(RuntimeOption::EvalHackArrCompatTypeHintNotices)
        ? AnnotAction::VArrayOrDArrayCheck
        : AnnotAction::Pass;
    case AnnotMetaType::VecOrDict:
      if (isClsMethType(dt)) {
        return !RuntimeOption::EvalHackArrDVArrs ?
          AnnotAction::Fail : AnnotAction::ClsMethCheck;
      }
      return (isVecType(dt) || isDictType(dt))
        ? AnnotAction::Pass
        : AnnotAction::Fail;
    case AnnotMetaType::ArrayLike:
      if (isClsMethType(dt)) {
        return AnnotAction::ClsMethCheck;
      }
      return (isArrayType(dt) || isVecType(dt) ||
              isDictType(dt) || isKeysetType(dt))
        ? AnnotAction::Pass
        : AnnotAction::Fail;
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
      return AnnotAction::Fail;
    case AnnotMetaType::Precise:
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatTypeHintNotices) &&
          at == AnnotType::Array && isArrayType(dt)) {
        return AnnotAction::NonVArrayOrDArrayCheck;
      }
      break;
  }

  assertx(metatype == AnnotMetaType::Precise);
  if (at == AnnotType::String && dt == KindOfFunc) {
    return RuntimeOption::EvalStringHintNotices
      ? AnnotAction::WarnFunc : AnnotAction::ConvertFunc;
  }
  if (at == AnnotType::String && dt == KindOfClass) {
    return RuntimeOption::EvalStringHintNotices
      ? AnnotAction::WarnClass : AnnotAction::ConvertClass;
  }
  if (isClsMethType(dt)) {
    if ((at == AnnotType::VArray) || (at == AnnotType::Array) ||
        (at == AnnotType::ArrayLike)) {
      return RuntimeOption::EvalHackArrDVArrs ?
        AnnotAction::Fail : AnnotAction::ClsMethCheck;
    }
    if ((at == AnnotType::Vec) || (at == AnnotType::ArrayLike)) {
      return !RuntimeOption::EvalHackArrDVArrs ?
        AnnotAction::Fail : AnnotAction::ClsMethCheck ;
    }
  }

  if (at == AnnotType::Record) {
    return dt == KindOfRecord ? AnnotAction::RecordCheck : AnnotAction::Fail;
  }

  if (at != AnnotType::Object) {
    // If `at' is "bool", "int", "float", "string", "array", or "resource",
    // then equivDataTypes() can definitively tell us whether or not `dt'
    // is compatible.
    return equivDataTypes(getAnnotDataType(at), dt)
      ? AnnotAction::Pass : AnnotAction::Fail;
  }

  // If `dt' is not an object, check for "magic" interfaces that
  // support non-object datatypes
  if (dt != KindOfObject && interface_supports_non_objects(annotClsName)) {
    switch (dt) {
      case KindOfInt64:
        return interface_supports_int(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfDouble:
        return interface_supports_double(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentString:
      case KindOfString:
        return interface_supports_string(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray:
        return interface_supports_array(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentVec:
      case KindOfVec:
        return interface_supports_vec(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentDict:
      case KindOfDict:
        return interface_supports_dict(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        return interface_supports_keyset(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfFunc:
        if (interface_supports_string(annotClsName)) {
          return RuntimeOption::EvalStringHintNotices
            ? AnnotAction::WarnFunc : AnnotAction::ConvertFunc;
        }
        return AnnotAction::Fail;
      case KindOfClass:
        if (interface_supports_string(annotClsName)) {
          return RuntimeOption::EvalStringHintNotices
            ? AnnotAction::WarnClass : AnnotAction::ConvertClass;
        }
        return AnnotAction::Fail;
      case KindOfClsMeth:
        if (RuntimeOption::EvalHackArrDVArrs) {
          return interface_supports_vec(annotClsName) ?
            AnnotAction::ClsMethCheck : AnnotAction::Fail;
        } else {
          return interface_supports_array(annotClsName) ?
            AnnotAction::ClsMethCheck : AnnotAction::Fail;
        }
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfResource:
      case KindOfRecord:
        return AnnotAction::Fail;
      case KindOfObject:
        not_reached();
        break;
    }
  }

  return AnnotAction::ObjectCheck;
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
