/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
};

enum class AnnotType : uint16_t {
  Uninit   = (uint8_t)KindOfUninit   | (uint16_t)AnnotMetaType::Precise << 8,
  Null     = (uint8_t)KindOfNull     | (uint16_t)AnnotMetaType::Precise << 8,
  Bool     = (uint8_t)KindOfBoolean  | (uint16_t)AnnotMetaType::Precise << 8,
  Int      = (uint8_t)KindOfInt64    | (uint16_t)AnnotMetaType::Precise << 8,
  Float    = (uint8_t)KindOfDouble   | (uint16_t)AnnotMetaType::Precise << 8,
  String   = (uint8_t)KindOfString   | (uint16_t)AnnotMetaType::Precise << 8,
  Array    = (uint8_t)KindOfArray    | (uint16_t)AnnotMetaType::Precise << 8,
  Object   = (uint8_t)KindOfObject   | (uint16_t)AnnotMetaType::Precise << 8,
  Resource = (uint8_t)KindOfResource | (uint16_t)AnnotMetaType::Precise << 8,
  // Precise is intentionally excluded
  Mixed    = (uint16_t)AnnotMetaType::Mixed << 8    | (uint8_t)KindOfUninit,
  Self     = (uint16_t)AnnotMetaType::Self << 8     | (uint8_t)KindOfUninit,
  Parent   = (uint16_t)AnnotMetaType::Parent << 8   | (uint8_t)KindOfUninit,
  Callable = (uint16_t)AnnotMetaType::Callable << 8 | (uint8_t)KindOfUninit,
  Number   = (uint16_t)AnnotMetaType::Number << 8   | (uint8_t)KindOfUninit,
  ArrayKey = (uint16_t)AnnotMetaType::ArrayKey << 8 | (uint8_t)KindOfUninit,
};

inline AnnotMetaType getAnnotMetaType(AnnotType at) {
  return (AnnotMetaType)((uint16_t)at >> 8);
}

inline DataType getAnnotDataType(AnnotType at) {
  auto const dt = (DataType)(uint8_t)at;
  return dt;
}

inline AnnotType dataTypeToAnnotType(DataType dt) {
  assert(dt == KindOfUninit || dt == KindOfBoolean || dt == KindOfInt64 ||
         dt == KindOfDouble || dt == KindOfString || dt == KindOfArray ||
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

bool interface_supports_int(std::string const&);
bool interface_supports_double(std::string const&);
bool interface_supports_string(std::string const&);
bool interface_supports_array(std::string const&);

enum class AnnotAction { Pass, Fail, ObjectCheck, CallableCheck };

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
 */
inline AnnotAction
annotCompat(DataType dt, AnnotType at, const StringData* annotClsName) {
  assert(dt != KindOfRef && dt != KindOfClass);
  assert(IMPLIES(at == AnnotType::Object, annotClsName != nullptr));

  auto const metatype = getAnnotMetaType(at);
  switch (metatype) {
    case AnnotMetaType::Mixed:
      return AnnotAction::Pass;
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
    case AnnotMetaType::Callable:
      // For "callable", if `dt' is not string/array/object we know
      // it's not compatible, otherwise more checks are required
      return (isStringType(dt) || isArrayType(KindOfArray) ||
              dt == KindOfObject)
        ? AnnotAction::CallableCheck : AnnotAction::Fail;
    case AnnotMetaType::Precise:
      break;
  }

  assert(metatype == AnnotMetaType::Precise);
  if (at != AnnotType::Object) {
    // If `at' is "bool", "int", "float", "string", "array", or "resource",
    // then equivDataTypes() can definitively tell us whether or not `dt'
    // is compatible. Uninit, to which 'HH\noreturn' maps, is special-cased
    // because uninit and null are equivalent due to isNullType.
    return equivDataTypes(getAnnotDataType(at), dt) && (at != AnnotType::Uninit)
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
      case KindOfPersistentArray:
      case KindOfArray:
        return interface_supports_array(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfResource:
        return AnnotAction::Fail;
      case KindOfObject:
      case KindOfRef:
      case KindOfClass:
        not_reached();
        break;
    }
  }

  return AnnotAction::ObjectCheck;
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
