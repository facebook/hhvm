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
 * needed to represent this.  (Known dependency in unwind-x64.h.)
 */
enum DataType : int8_t {
  // Values below zero are not PHP values, but runtime-internal.
  KindOfClass         = -8,   // 11111000

  // Any code that static_asserts about the value of KindOfNull may also depend
  // on there not being any values between KindOfUninit and KindOfNull.

                                 //       array bit
                                 //       |string bit
                                 //       ||uncounted init bit
                                 //       |||
  KindOfUninit          = 0x00,  //  00000000
  KindOfNull            = 0x01,  //  00000001
  KindOfBoolean         = 0x09,  //  00001001
  KindOfInt64           = 0x11,  //  00010001
  KindOfDouble          = 0x19,  //  00011001
  KindOfPersistentString = 0x1b, //  00011011
  KindOfPersistentArray = 0x1d,  //  00011101
  KindOfString          = 0x22,  //  00100010
  KindOfArray           = 0x34,  //  00110100
  KindOfObject          = 0x40,  //  01000000
  KindOfResource        = 0x50,  //  01010000
  KindOfRef             = 0x60,  //  01100000
};

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
 * Function parameter types for users of DataType that want a representation
 * for top and bottom types.
 *
 * These are intended to be used for, e.g., discriminating constructors; they
 * should never be munged into a DataType data member.
 */
enum class KindOfNone {};
enum class KindOfAny  {};

/*
 * Optional DataType.
 *
 * Used for (DataType|KindOfNoneType) or (DataType|KindOfAnyType), depending on
 * context.  Users who wish to use (DataType|KindOfNoneType|KindOfAnyType)
 * should consider dying in a fire.
 */
using MaybeDataType = folly::Optional<DataType>;


///////////////////////////////////////////////////////////////////////////////

/*
 * All DataTypes are expressible in seven bits.
 */
constexpr unsigned kDataTypeMask = 0x7f;

/*
 * DataType limits.
 */
constexpr int8_t kMinDataType  = KindOfClass;
constexpr int8_t kMaxDataType  = KindOfRef;

/*
 * KindOfStringBit must be set in KindOfPersistentString and KindOfString,
 * and it must be 0 in any other DataType.
 */
constexpr int KindOfStringBit = 0x02;

/*
 * KindOfArrayBit must be set in KindOfPersistentArray and KindOfArray, and it
 * must be 0 in any other DataType.
 */
constexpr int KindOfArrayBit = 0x04;

/*
 * KindOfUncountedInitBit must be set for Null, Boolean, Int64, Double,
 * PersistentString, PersistentArray, and it must be 0 for any other DataType.
 */
constexpr int KindOfUncountedInitBit = 0x01;

/*
 * For a given DataType dt >= 0, this mask can be used to test if dt is
 * KindOfPersistentArray, KindOfArray, KindOfObject, KindOfResource, or
 * KindOfRef.
 */
constexpr unsigned kNotConstantValueTypeMask = 0x44;

/*
 * All DataTypes greater than this value are refcounted.
 */
constexpr DataType KindOfRefCountThreshold = KindOfPersistentArray;


///////////////////////////////////////////////////////////////////////////////
// DataTypeCategory

// These must be kept in order from least to most specific.
#define DT_CATEGORIES(func)                     \
  func(Generic)                                 \
  func(Countness)                               \
  func(CountnessInit)                           \
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
// Static asserts.

static_assert(KindOfString        & KindOfStringBit, "");
static_assert(KindOfPersistentString  & KindOfStringBit, "");
static_assert(!(KindOfUninit      & KindOfStringBit), "");
static_assert(!(KindOfNull        & KindOfStringBit), "");
static_assert(!(KindOfBoolean     & KindOfStringBit), "");
static_assert(!(KindOfInt64       & KindOfStringBit), "");
static_assert(!(KindOfDouble      & KindOfStringBit), "");
static_assert(!(KindOfPersistentArray & KindOfStringBit), "");
static_assert(!(KindOfArray       & KindOfStringBit), "");
static_assert(!(KindOfObject      & KindOfStringBit), "");
static_assert(!(KindOfResource    & KindOfStringBit), "");
static_assert(!(KindOfRef         & KindOfStringBit), "");
static_assert(!(KindOfClass       & KindOfStringBit), "");

static_assert(KindOfArray          & KindOfArrayBit, "");
static_assert(KindOfPersistentArray    & KindOfArrayBit, "");
static_assert(!(KindOfUninit       & KindOfArrayBit), "");
static_assert(!(KindOfNull         & KindOfArrayBit), "");
static_assert(!(KindOfBoolean      & KindOfArrayBit), "");
static_assert(!(KindOfInt64        & KindOfArrayBit), "");
static_assert(!(KindOfDouble       & KindOfArrayBit), "");
static_assert(!(KindOfPersistentString & KindOfArrayBit), "");
static_assert(!(KindOfString       & KindOfArrayBit), "");
static_assert(!(KindOfObject       & KindOfArrayBit), "");
static_assert(!(KindOfResource     & KindOfArrayBit), "");
static_assert(!(KindOfRef          & KindOfArrayBit), "");
static_assert(!(KindOfClass        & KindOfArrayBit), "");

static_assert(KindOfNull         & KindOfUncountedInitBit, "");
static_assert(KindOfBoolean      & KindOfUncountedInitBit, "");
static_assert(KindOfInt64        & KindOfUncountedInitBit, "");
static_assert(KindOfDouble       & KindOfUncountedInitBit, "");
static_assert(KindOfPersistentString & KindOfUncountedInitBit, "");
static_assert(KindOfPersistentArray  & KindOfUncountedInitBit, "");
static_assert(!(KindOfUninit     & KindOfUncountedInitBit), "");
static_assert(!(KindOfString     & KindOfUncountedInitBit), "");
static_assert(!(KindOfArray      & KindOfUncountedInitBit), "");
static_assert(!(KindOfObject     & KindOfUncountedInitBit), "");
static_assert(!(KindOfResource   & KindOfUncountedInitBit), "");
static_assert(!(KindOfRef        & KindOfUncountedInitBit), "");
static_assert(!(KindOfClass      & KindOfUncountedInitBit), "");

static_assert(KindOfUninit == 0,
              "Several things assume this tag is 0, especially RDS");

static_assert(kMaxDataType <= kDataTypeMask, "");

static_assert((kNotConstantValueTypeMask & KindOfPersistentArray) != 0  &&
              (kNotConstantValueTypeMask & KindOfArray) != 0  &&
              (kNotConstantValueTypeMask & KindOfObject) != 0 &&
              (kNotConstantValueTypeMask & KindOfResource) != 0 &&
              (kNotConstantValueTypeMask & KindOfRef) != 0,
              "DataType & kNotConstantValueTypeMask must be non-zero for "
              "Array, Object and Ref types");
static_assert(!(kNotConstantValueTypeMask &
                (KindOfNull|KindOfBoolean|KindOfInt64|KindOfDouble|
                 KindOfPersistentString|KindOfString)),
              "DataType & kNotConstantValueTypeMask must be zero for "
              "null, bool, int, double and string types");


///////////////////////////////////////////////////////////////////////////////
// Names.

inline std::string tname(DataType t) {
  switch (t) {
#define CS(name) \
    case KindOf ## name: return std::string(#name);
    CS(Uninit)
    CS(Null)
    CS(Boolean)
    CS(Int64)
    CS(Double)
    CS(PersistentString)
    CS(PersistentArray)
    CS(String)
    CS(Array)
    CS(Object)
    CS(Resource)
    CS(Ref)
    CS(Class)
#undef CS

    default: {
       if (t == kInvalidDataType) {
         return std::string("Invalid");
       }
      char buf[128];
      sprintf(buf, "Unknown:%d", t);
      return std::string(buf);
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
 * These are used in type-variant.cpp and mc-generator.cpp.
 */
constexpr int kShiftDataTypeToDestrIndex = 4;
constexpr int kDestrTableSize = 7;

constexpr unsigned typeToDestrIdx(DataType t) {
  // t must be a refcounted type, but we can't actually assert that and still
  // be constexpr.
  return t >> kShiftDataTypeToDestrIndex;
}


///////////////////////////////////////////////////////////////////////////////
// Is-a macros.

/*
 * Whether a type is valid.
 */
constexpr bool isRealType(DataType t) {
  return (t >= KindOfUninit && t <= kMaxDataType) || t == KindOfClass;
}

/*
 * Whether a type is refcounted.
 */
constexpr bool isRefcountedType(DataType t) {
  return t > KindOfRefCountThreshold;
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
 * Whether a type is KindOfUninit or KindOfNull.
 */
constexpr bool isNullType(DataType t) {
  return unsigned(t) <= KindOfNull;
}

/*
 * Whether a type is any kind of string or array.
 */
constexpr bool isStringType(DataType t) {
  return t & KindOfStringBit;
}
inline bool isStringType(MaybeDataType t) {
  return t && isStringType(*t);
}

constexpr bool isArrayType(DataType t) {
  return t & KindOfArrayBit;
}
inline bool isArrayType(MaybeDataType t) {
  return t && isArrayType(*t);
}

/*
 * Other type-check functions.
 */
constexpr bool isIntType(DataType t) { return t == KindOfInt64; }
constexpr bool isBoolType(DataType t) { return t == KindOfBoolean; }
constexpr bool isDoubleType(DataType t) { return t == KindOfDouble; }

constexpr bool isIntKeyType(DataType t) {
  return t <= KindOfInt64;
}

/*
 * Return whether two DataTypes for primitive types are "equivalent" as far as
 * user-visible PHP types are concerned (i.e. ignoring different types of
 * strings, arrays, or nulls).
 *
 * Pre: t1 and t2 must both be DataTypes that represent PHP-types.
 * (non-internal KindOfs.)
 */
constexpr bool equivDataTypes(DataType t1, DataType t2) {
  return
    (t1 == t2) ||
    (isStringType(t1) && isStringType(t2)) ||
    (isArrayType(t1) && isArrayType(t2)) ||
    (isNullType(t1) && isNullType(t2));
}


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
  case KindOfPersistentArray

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace folly {
template<> struct FormatValue<HPHP::DataTypeCategory> {
  explicit FormatValue(HPHP::DataTypeCategory val) : m_val(val) {}

  template<typename Callback>
  void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(typeCategoryName(m_val), arg, cb);
  }

 private:
  HPHP::DataTypeCategory m_val;
};

template<> struct FormatValue<HPHP::DataType> {
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
