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
  Self = 2,
  Parent = 3,
  Callable = 4,
  Number = 5,
  ArrayKey = 6,
  This = 7,
  VecOrDict = 8,
  ArrayLike = 9,
  Nonnull = 10,
  NoReturn = 11,
  Nothing = 12,
  Classname = 13,
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
  VecOrDict  = (uint16_t)AnnotMetaType::VecOrDict << 8  | (uint8_t)KindOfUninit,
  ArrayLike  = (uint16_t)AnnotMetaType::ArrayLike << 8  | (uint8_t)KindOfUninit,
  NoReturn   = (uint16_t)AnnotMetaType::NoReturn << 8   | (uint8_t)KindOfUninit,
  Nothing    = (uint16_t)AnnotMetaType::Nothing << 8    | (uint8_t)KindOfUninit,
  Classname  = (uint16_t)AnnotMetaType::Classname << 8  | (uint8_t)KindOfUninit
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

enum class AnnotAction {
  Pass,
  Fail,
  ObjectCheck,
  CallableCheck,
  WarnClass,
  ConvertClass,
  WarnLazyClass,
  ConvertLazyClass,
  ClsMethCheck,
  RecordCheck,
  WarnClassname,
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
 * RecordCheck: 'at' and 'dt' are both records and the caller needs to check
 * if the record in the value matches annotation.
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
