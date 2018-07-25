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

#ifndef incl_HPHP_DATATYPE_H_
#define incl_HPHP_DATATYPE_H_

#include <cstdint>
#include <cstdio>
#include <string>

#include <folly/Format.h>
#include <folly/Optional.h>

#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * DataType is the type tag for a TypedValue (see typed-value.h).
 *
 * If you want to add a new type, beware of the following restrictions:
 * - KindOfUninit must be 0. Many places rely on zero-initialized memory
 *   being a valid, KindOfUninit TypedValue.
 * - KindOfNull must be 2, and 1 must not be a valid type. This allows for
 *   a fast implementation of isNullType().
 * - The Array and String types are positioned to allow for fast array/string
 *   checks, ignoring persistence (see isArrayType and isStringType).
 * - Refcounted types are odd, and uncounted types are even, to allow fast
 *   countness checks.
 * - Types with persistent and non-persistent versions must be negative, for
 *   equivDataTypes(). Other types may be negative, as long as dropping the low
 *   bit does not give another valid type.
 * - -128 and -127 are used as invalid types and can't be real DataTypes.
 *
 * If you think you need to change any of these restrictions, be prepared to
 * deal with subtle bugs and/or performance regressions while you sort out the
 * consequences. At a minimum, you must:
 * - Audit every helper function in this file.
 * - Audit jit::emitTypeTest().
 */
#define DATATYPES \
  DT(PersistentArray,  -10) \
  DT(Array,             -9) \
  DT(PersistentKeyset,  -8) \
  DT(Keyset,            -7) \
  DT(PersistentDict,    -6) \
  DT(Dict,              -5) \
  DT(PersistentVec,     -4) \
  DT(Vec,               -3) \
  DT(PersistentString,  -2) \
  DT(String,            -1) \
  DT(Uninit,             0) \
  /* isNullType relies on a hole here */ \
  DT(Null,               2) \
  DT(Object,             3) \
  DT(Boolean,            4) \
  DT(Resource,           5) \
  DT(Int64,              6) \
  DT(Ref,                7) \
  DT(Double,             8) \
  DT(Func,              10) \
  DT(Class,             12) \

enum class DataType : int8_t {
#define DT(name, value) name = value,
DATATYPES
#undef DT
};

using data_type_t = typename std::underlying_type<DataType>::type;

// Macro so we can limit its scope to this file. Anyone else doing this cast
// should have to write out the whole thing and think about their life choices.
#define dt_t(t) static_cast<data_type_t>(t)

/*
 * Also define KindOf<Foo> for each type, to avoid having to change thousands
 * of existing usage sites.
 */
#define DT(name, ...) auto constexpr KindOf##name = DataType::name;
DATATYPES
#undef DT

/*
 * Sentinel invalid DataTypes.
 *
 * These values must differ from that of any real DataType.  A live TypedValue
 * should never have these as its type tag, so we keep them out of the enum to
 * keep switches cleaner.
 *
 * These should only be used where MaybeDataType cannot be (e.g., in
 * TypedValues, such as for MixedArray tombstones).
 */
constexpr DataType kInvalidDataType      = static_cast<DataType>(-128);
constexpr DataType kExtraInvalidDataType = static_cast<DataType>(-127);

/*
 * DataType limits.
 */
auto constexpr kMinDataType = dt_t(KindOfPersistentArray);
auto constexpr kMaxDataType = dt_t(KindOfClass);
auto constexpr kMinRefCountedDataType = dt_t(KindOfArray);
auto constexpr kMaxRefCountedDataType = dt_t(KindOfRef);

/*
 * A DataType is a refcounted type if and only if it has this bit set.
 */
constexpr int kRefCountedBit = 0x1;

///////////////////////////////////////////////////////////////////////////////
/*
 * Optional DataType.
 *
 * Used for (DataType|KindOfNoneType) or (DataType|KindOfAnyType), depending on
 * context.  Users who wish to use (DataType|KindOfNoneType|KindOfAnyType)
 * should consider dying in a fire.
 */
using MaybeDataType = folly::Optional<DataType>;

/*
 * Extracts the DataType from the given type
 */
MaybeDataType get_datatype(
  const std::string& name,
  bool can_be_collection,
  bool is_function,
  bool is_xhp,
  bool is_tuple,
  bool is_nullable,
  bool is_soft
);

///////////////////////////////////////////////////////////////////////////////
// DataTypeCategory

// These must be kept in order from least to most specific.
//
// Note that Countness can be relaxed to Generic in optimizeProfiledGuards(), so
// it should only be used to constrain values used by instructions that work
// even in the absence of type information.
#define DT_CATEGORIES(func)                     \
  func(Generic)                                 \
  func(Countness)                               \
  func(BoxAndCountness)                         \
  func(BoxAndCountnessInit)                     \
  func(Specific)                                \
  func(Specialized)

enum class DataTypeCategory : uint8_t {
#define DT(name) name,
  DT_CATEGORIES(DT)
#undef DT
};

#define DT(name) auto constexpr DataType##name = DataTypeCategory::name;
DT_CATEGORIES(DT)
#undef DT

///////////////////////////////////////////////////////////////////////////////
// Names.

inline std::string tname(DataType t) {
  switch (t) {
#define DT(name, ...) case KindOf##name: return #name;
DATATYPES
#undef DT
    default: {
      if (t == kInvalidDataType) return "Invalid";
      return folly::sformat("Unknown:{}", static_cast<int>(t));
    }
  }
}

inline std::string typeCategoryName(DataTypeCategory c) {
  switch (c) {
# define CASE(name) case DataType##name: return "DataType" #name;
  DT_CATEGORIES(CASE)
#undef CASE
  }
  not_reached();
}

/*
 * These are used in type-variant.cpp.
 */
constexpr int kDestrTableSize =
  (kMaxRefCountedDataType - kMinRefCountedDataType) / 2 + 1;

constexpr unsigned typeToDestrIdx(DataType t) {
  // t must be a refcounted type, but we can't actually assert that and still
  // be constexpr.
  return (static_cast<int64_t>(t) - kMinRefCountedDataType) / 2;
}

///////////////////////////////////////////////////////////////////////////////
// Is-a macros.

/*
 * Whether a type is valid.
 */
constexpr bool isRealType(DataType t) {
  return t >= static_cast<DataType>(kMinDataType) &&
    t <= static_cast<DataType>(kMaxDataType);
}

/*
 * Whether a type is refcounted.
 */
constexpr bool isRefcountedType(DataType t) {
  return dt_t(t) & kRefCountedBit;
}

/*
 * Whether a builtin return or param type is not a simple type.
 *
 * This is different from isRefcountedType because builtins can accept and
 * return Variants, and we use folly::none to denote these cases.
 */
inline bool isBuiltinByRef(MaybeDataType t) {
  return t != KindOfNull &&
         t != KindOfBoolean &&
         t != KindOfInt64 &&
         t != KindOfDouble;
}

/*
 * Whether a type's value is an integral value in m_data.num.
 */
constexpr bool hasNumData(DataType t) {
  return t == KindOfBoolean || t == KindOfInt64;
}

/*
 * Whether a type is KindOfUninit or KindOfNull.
 */
constexpr bool isNullType(DataType t) {
  static_assert(KindOfUninit == static_cast<DataType>(0) &&
                KindOfNull == static_cast<DataType>(2),
                "isNullType requires Uninit and Null to be 0 and 2");
  return static_cast<uint8_t>(t) <= static_cast<uint8_t>(KindOfNull);
}

/*
 * Whether a type is any kind of string or array.
 */
constexpr bool isStringType(DataType t) {
  return
    static_cast<uint8_t>(t) >= static_cast<uint8_t>(KindOfPersistentString);
}
inline bool isStringType(MaybeDataType t) {
  return t && isStringType(*t);
}

constexpr bool isArrayLikeType(DataType t) {
  return t <= KindOfVec;
}
inline bool isArrayLikeType(MaybeDataType t) {
  return t && isArrayLikeType(*t);
}

constexpr bool isArrayType(DataType t) {
  return t <= KindOfArray;
}
inline bool isArrayType(MaybeDataType t) {
  return t && isArrayType(*t);
}

constexpr bool isHackArrayType(DataType t) {
  return t >= KindOfPersistentKeyset && t <= KindOfVec;
}
inline bool isHackArrayType(MaybeDataType t) {
  return t && isHackArrayType(*t);
}

constexpr bool isVecType(DataType t) {
  return
    static_cast<DataType>(dt_t(t) & ~kRefCountedBit) == KindOfPersistentVec;
}
inline bool isVecType(MaybeDataType t) {
  return t && isVecType(*t);
}

constexpr bool isDictType(DataType t) {
  return
    static_cast<DataType>(dt_t(t) & ~kRefCountedBit) == KindOfPersistentDict;
}
inline bool isDictType(MaybeDataType t) {
  return t && isDictType(*t);
}

constexpr bool isKeysetType(DataType t) {
  return
    static_cast<DataType>(dt_t(t) & ~kRefCountedBit) == KindOfPersistentKeyset;
}
inline bool isKeysetType(MaybeDataType t) {
  return t && isKeysetType(*t);
}

/*
 * Other type-check functions.
 */
constexpr bool isIntType(DataType t) { return t == KindOfInt64; }
constexpr bool isBoolType(DataType t) { return t == KindOfBoolean; }
constexpr bool isDoubleType(DataType t) { return t == KindOfDouble; }
constexpr bool isObjectType(DataType t) { return t == KindOfObject; }
constexpr bool isResourceType(DataType t) { return t == KindOfResource; }
constexpr bool isRefType(DataType t) { return t == KindOfRef; }
constexpr bool isFuncType(DataType t) { return t == KindOfFunc; }
constexpr bool isClassType(DataType t) { return t == KindOfClass; }

constexpr int kHasPersistentMask = -128;

/*
 * Return whether two DataTypes for primitive types are "equivalent" as far as
 * user-visible PHP types are concerned (i.e. ignoring different types of
 * strings, arrays, and Hack arrays). Note that KindOfUninit and KindOfNull are
 * not considered equivalent.
 */
constexpr bool equivDataTypes(DataType t1, DataType t2) {
  return t1 == t2 ||
    ((dt_t(t1) & dt_t(t2) & kHasPersistentMask) &&
     (dt_t(t1) & ~kRefCountedBit) == (dt_t(t2) & ~kRefCountedBit));
}

/*
 * If you think you need to do any of these operations, you should instead add
 * a helper function up above and call that, to keep any knowledge about the
 * relative values of DataTypes in this file.
 */
bool operator<(DataType, DataType) = delete;
bool operator>(DataType, DataType) = delete;
bool operator<=(DataType, DataType) = delete;
bool operator>=(DataType, DataType) = delete;

#undef dt_t

///////////////////////////////////////////////////////////////////////////////
// Switch case macros.

/*
 * Covers all DataTypes `dt' such that !isRefcountedType(dt) holds.
 */
#define DT_UNCOUNTED_CASE   \
  case KindOfUninit:        \
  case KindOfNull:          \
  case KindOfBoolean:       \
  case KindOfInt64:         \
  case KindOfDouble:        \
  case KindOfPersistentString:  \
  case KindOfPersistentArray:   \
  case KindOfPersistentVec: \
  case KindOfPersistentDict: \
  case KindOfPersistentKeyset: \
  case KindOfFunc:          \
  case KindOfClass
}

///////////////////////////////////////////////////////////////////////////////

namespace folly {
template<> class FormatValue<HPHP::DataTypeCategory> {
 public:
  explicit FormatValue(HPHP::DataTypeCategory val) : m_val(val) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(typeCategoryName(m_val), arg, cb);
  }

 private:
  HPHP::DataTypeCategory m_val;
};

template<> class FormatValue<HPHP::DataType> {
 public:
  explicit FormatValue(HPHP::DataType dt) : m_dt(dt) {}

  template<typename C>
  void format(FormatArg& arg, C& cb) const {
    format_value::formatString(tname(m_dt), arg, cb);
  }

 private:
  HPHP::DataType m_dt;
};
}

///////////////////////////////////////////////////////////////////////////////

#endif
