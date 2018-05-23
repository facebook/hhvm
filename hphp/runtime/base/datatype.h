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
 * Beware if you change the order, as we may have a few type checks in the code
 * that depend on the order.  Also beware of adding to the number of bits
 * needed to represent this.
 */
#define DATATYPES \
                              /*      Hack array bit        */ \
                              /*      |PHP array bit        */ \
                              /*      ||string bit          */ \
                              /*      |||uncounted init bit */ \
                              /*      ||||                  */ \
  DT(Uninit,           0x00)  /*  00000000 */ \
  DT(Null,             0x01)  /*  00000001 */ \
  DT(Int64,            0x11)  /*  00010001 */ \
  DT(PersistentVec,    0x19)  /*  00011001 */ \
  DT(Boolean,          0x21)  /*  00100001 */ \
  DT(PersistentString, 0x23)  /*  00100011 */ \
  DT(PersistentDict,   0x29)  /*  00101001 */ \
  DT(Double,           0x31)  /*  00110001 */ \
  DT(PersistentArray,  0x35)  /*  00110101 */ \
  DT(PersistentKeyset, 0x39)  /*  00111001 */ \
  DT(Object,           0x40)  /*  01000000 */ \
  DT(Resource,         0x50)  /*  01010000 */ \
  DT(Vec,              0x58)  /*  01011000 */ \
  DT(String,           0x62)  /*  01100010 */ \
  DT(Dict,             0x68)  /*  01101000 */ \
  DT(Ref,              0x70)  /*  01110000 */ \
  DT(Array,            0x74)  /*  01110100 */ \
  DT(Keyset,           0x78)  /*  01111000 */

enum class DataType : int8_t {
  // Any code that static_asserts about the value of KindOfNull may also depend
  // on there not being any values between KindOfUninit and KindOfNull.
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
constexpr DataType kInvalidDataType      = static_cast<DataType>(-1);
constexpr DataType kExtraInvalidDataType = static_cast<DataType>(-2);

/*
 * DataType limits.
 */
auto constexpr kMinDataType = dt_t(KindOfUninit);
auto constexpr kMaxDataType = dt_t(KindOfKeyset);

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

/*
 * All DataTypes are expressible in seven bits.
 */
constexpr unsigned kDataTypeMask = 0x7f;

/*
 * KindOfStringBit must be set in KindOfPersistentString and KindOfString,
 * and it must be 0 in any other DataType.
 */
constexpr int KindOfStringBit = 0x02;

/*
 * KindOfArrayBit must be set in KindOfPersistentArray and KindOfArray, and
 * it must be 0 in any other DataType.
 */
constexpr int KindOfArrayBit = 0x04;

/*
 * KindOfHackArrayBit must be set in KindOfPersistentVec, KindOfVec,
 * KindOfPersistentDict, KindOfDict, KindOfPersistentKeyset, and KindOfKeyset,
 * and it must be 0 in any other DataType.
 */
constexpr int KindOfHackArrayBit = 0x08;

/*
 * The result of ANDing KindOfArrayLikeMask against KindOfPersistentVec,
 * KindOfVec, KindOfPersistentDict, KindOfDict, KindOfPersistentKeyset,
 * KindOfKeyset, KindOfPersistentArray, or KindOfArray must be non-zero, and 0
 * against any other DataType.
 */
constexpr int KindOfArrayLikeMask = KindOfArrayBit | KindOfHackArrayBit;

/*
 * KindOfUncountedInitBit must be set for Null, Boolean, Int64, Double,
 * PersistentString, PersistentArray, and it must be 0 for any other DataType.
 */
constexpr int KindOfUncountedInitBit = 0x01;

/*
 * One of KindOfHashPersistentBits must be set for KindOfString,
 * KindOfPersistentString, KindOfArray, KindOfPersistentArray, KindOfVec,
 * KindOfPersistentVec, KindOfPersistentKeyset, KindOfKeyset, KindOfDict, and
 * KindOfPersistentDict. It signifies the type has both persistent and
 * non-persistent variants.
 */
constexpr int KindOfHasPersistentBits =
  KindOfStringBit | KindOfArrayBit | KindOfHackArrayBit;

/*
 * The result of ANDing kDataTypeEquivalentMask against KindOf[Persistent]Array,
 * KindOf[Persistent]String, KindOf[Persistent]Vec, KindOf[Persistent]Dict,
 * KindOf[Persistent]Keyset, or KindOfNull/KindOfUninit yields some unspecified
 * value which is the same for each persistent/non-persistent pair, and
 * different for all else. Used to check for equivalency between persistent and
 * non-persistent DataTypes.
 */
constexpr int kDataTypeEquivalentMask = 0x3e;

/*
 * The result of ANDing kDataTypeEquivalentMask against KindOfPersistentVec and
 * KindOfVec results in KindOfHackArrayVecType. ANDing against
 * KindOfPersistentDict and KindOfDict results in
 * KindOfHackArrayDictType. ANDing against KindOfPersistentKeyset and
 * KindOfKeyset results in KindOfHackArrayKeysetType. For any other DataType,
 * some other value other than KindOfHackArrayVecType or KindOfHackArrayDictType
 * is the result.
 */
constexpr int KindOfHackArrayVecType = 0x18;
constexpr int KindOfHackArrayDictType = 0x28;
constexpr int KindOfHackArrayKeysetType = 0x38;

/*
 * All DataTypes greater than this value are refcounted.
 */
constexpr DataType KindOfRefCountThreshold = KindOfPersistentKeyset;


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
constexpr int kShiftDataTypeToDestrIndex = 2;
constexpr int kDestrTableSize = 31;

constexpr unsigned typeToDestrIdx(DataType t) {
  // t must be a refcounted type, but we can't actually assert that and still
  // be constexpr.
  return dt_t(t) >> kShiftDataTypeToDestrIndex;
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
  return t > KindOfRefCountThreshold;
}

constexpr bool isUncountedInitType(DataType t) {
  return dt_t(t) & KindOfUncountedInitBit;
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
                KindOfNull == static_cast<DataType>(1),
                "isNullType requires Uninit and Null to be 0 and 1");
  return t <= KindOfNull;
}

/*
 * Whether a type is any kind of string or array.
 */
constexpr bool isStringType(DataType t) {
  return dt_t(t) & KindOfStringBit;
}
inline bool isStringType(MaybeDataType t) {
  return t && isStringType(*t);
}

constexpr bool isArrayLikeType(DataType t) {
  return dt_t(t) & KindOfArrayLikeMask;
}
inline bool isArrayLikeType(MaybeDataType t) {
  return t && isArrayLikeType(*t);
}

constexpr bool isArrayType(DataType t) {
  return dt_t(t) & KindOfArrayBit;
}
inline bool isArrayType(MaybeDataType t) {
  return t && isArrayType(*t);
}

constexpr bool isHackArrayType(DataType t) {
  return dt_t(t) & KindOfHackArrayBit;
}
inline bool isHackArrayType(MaybeDataType t) {
  return t && isHackArrayType(*t);
}

constexpr bool isVecType(DataType t) {
  return (dt_t(t) & kDataTypeEquivalentMask) == KindOfHackArrayVecType;
}
inline bool isVecType(MaybeDataType t) {
  return t && isVecType(*t);
}

constexpr bool isDictType(DataType t) {
  return (dt_t(t) & kDataTypeEquivalentMask) == KindOfHackArrayDictType;
}
inline bool isDictType(MaybeDataType t) {
  return t && isDictType(*t);
}

constexpr bool isKeysetType(DataType t) {
  return (dt_t(t) & kDataTypeEquivalentMask) == KindOfHackArrayKeysetType;
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

/*
 * Return whether two DataTypes for primitive types are "equivalent" as far as
 * user-visible PHP types are concerned (i.e. ignoring different types of
 * strings, arrays, and Hack arrays). Note that KindOfUninit and KindOfNull are
 * not considered equivalent.
 */
constexpr bool equivDataTypes(DataType t1, DataType t2) {
  return
    t1 == t2 ||
    ((dt_t(t1) & dt_t(t2) & KindOfHasPersistentBits) &&
     ((dt_t(t1) & kDataTypeEquivalentMask) ==
      (dt_t(t2) & kDataTypeEquivalentMask)));
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
  case KindOfPersistentKeyset

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
