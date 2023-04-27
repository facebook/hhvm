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

#include <folly/Range.h>

namespace HPHP {

struct StringData;
struct TypedValue;

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
  Callable = 2,
  Number = 3,
  ArrayKey = 4,
  This = 5,
  VecOrDict = 6,
  ArrayLike = 7,
  Nonnull = 8,
  NoReturn = 9,
  Nothing = 10,
  Classname = 11,
  Unresolved = 12,
};

enum class AnnotType : uint16_t {
  Null     = (uint8_t)KindOfNull     | (uint16_t)AnnotMetaType::Precise << 8,
  Bool     = (uint8_t)KindOfBoolean  | (uint16_t)AnnotMetaType::Precise << 8,
  Int      = (uint8_t)KindOfInt64    | (uint16_t)AnnotMetaType::Precise << 8,
  Float    = (uint8_t)KindOfDouble   | (uint16_t)AnnotMetaType::Precise << 8,
  String   = (uint8_t)KindOfString   | (uint16_t)AnnotMetaType::Precise << 8,
  Object   = (uint8_t)KindOfObject   | (uint16_t)AnnotMetaType::Precise << 8,
  Resource = (uint8_t)KindOfResource | (uint16_t)AnnotMetaType::Precise << 8,
  Dict     = (uint8_t)KindOfDict     | (uint16_t)AnnotMetaType::Precise << 8,
  Vec      = (uint8_t)KindOfVec      | (uint16_t)AnnotMetaType::Precise << 8,
  Keyset   = (uint8_t)KindOfKeyset   | (uint16_t)AnnotMetaType::Precise << 8,
  // Precise is intentionally excluded
  Mixed    = (uint16_t)AnnotMetaType::Mixed << 8        | (uint8_t)KindOfUninit,
  Nonnull  = (uint16_t)AnnotMetaType::Nonnull << 8      | (uint8_t)KindOfUninit,
  Callable = (uint16_t)AnnotMetaType::Callable << 8     | (uint8_t)KindOfUninit,
  Number   = (uint16_t)AnnotMetaType::Number << 8       | (uint8_t)KindOfUninit,
  ArrayKey = (uint16_t)AnnotMetaType::ArrayKey << 8     | (uint8_t)KindOfUninit,
  This     = (uint16_t)AnnotMetaType::This << 8         | (uint8_t)KindOfUninit,
  VecOrDict  = (uint16_t)AnnotMetaType::VecOrDict << 8  | (uint8_t)KindOfUninit,
  ArrayLike  = (uint16_t)AnnotMetaType::ArrayLike << 8  | (uint8_t)KindOfUninit,
  NoReturn   = (uint16_t)AnnotMetaType::NoReturn << 8   | (uint8_t)KindOfUninit,
  Nothing    = (uint16_t)AnnotMetaType::Nothing << 8    | (uint8_t)KindOfUninit,
  Classname  = (uint16_t)AnnotMetaType::Classname << 8  | (uint8_t)KindOfUninit,
  Unresolved = (uint16_t)AnnotMetaType::Unresolved << 8 | (uint8_t)KindOfUninit,
};

inline AnnotMetaType getAnnotMetaType(AnnotType at) {
  return (AnnotMetaType)((uint16_t)at >> 8);
}

inline DataType getAnnotDataType(AnnotType at) {
  auto const dt = (DataType)(uint8_t)at;
  return dt;
}

inline AnnotType enumDataTypeToAnnotType(DataType dt) {
  assertx(dt == KindOfInt64 || dt == KindOfString);
  return (AnnotType)((uint8_t)dt | (uint16_t)AnnotMetaType::Precise << 8);
}

const char* annotName(AnnotType);

const AnnotType* nameToAnnotType(const StringData* typeName);
const AnnotType* nameToAnnotType(const std::string& typeName);
MaybeDataType nameToMaybeDataType(const StringData* typeName);
MaybeDataType nameToMaybeDataType(const std::string& typeName);

bool interface_supports_non_objects(const StringData* s);
bool interface_supports_int(const StringData* s);
bool interface_supports_double(const StringData* s);
bool interface_supports_string(const StringData* s);
bool interface_supports_arrlike(const StringData* s);

bool interface_supports_non_objects(folly::StringPiece s);
bool interface_supports_int(folly::StringPiece s);
bool interface_supports_double(folly::StringPiece s);
bool interface_supports_string(folly::StringPiece s);
bool interface_supports_arrlike(folly::StringPiece s);

TypedValue annotDefaultValue(AnnotType at);

inline bool enumSupportsAnnot(AnnotType at) {
  return
    at == AnnotType::String ||
    at == AnnotType::Int ||
    at == AnnotType::ArrayKey ||
    at == AnnotType::Classname;
}

enum class AnnotAction {
  Pass,
  Fail,
  Fallback,
  FallbackCoerce,
  ObjectCheck,
  CallableCheck,
  WarnClass,
  ConvertClass,
  WarnLazyClass,
  ConvertLazyClass,
  WarnClassname,
};

/*
 * annotCompat() takes a DataType (`dt') and tries to determine if a value
 * with DataType `dt' could be compatible with a given AnnotType (`at')
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
 * Fallback: A recommendation to perform a full C++ check. This can happen
 * if `at' is an unresolved reference to either (1) a real non-enum class or
 * interface, (2) an enum, or (3) a type alias and the DataType `dt' is
 * not KindOfObject. FallbackCoerce may require type coercion, Fallback is
 * guaranteed to not require it.
 *
 * CallableCheck: `at' is "callable" and a value with DataType `dt' might
 * be compatible with the annotation, but the caller needs to consult
 * is_callable() to verify the value is actually a valid callable.
 *
 * ObjectCheck: `at' is either (1) a reference to a real non-enum class or
 * interface, (2) an enum, (3) a type alias, or (4) "self" or "parent".
 * The caller needs to perform more checks to determine whether or not a
 * value with the KindOfObject DataType is compatible with the annotation.
 *
 * WarnClassname: 'at' is classname and 'dt' is either a Class or LazyClass
 * and RuntimeOption::ClassnameNotices is on. The 'dt' is compatible with 'at'
 * but raises a notice at runtime.
 *
 */
AnnotAction
annotCompat(DataType dt, AnnotType at, const StringData* annotClsName);

///////////////////////////////////////////////////////////////////////////////

}
