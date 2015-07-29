/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
  KindOfClass         = -13,

  // Any code that static_asserts about the value of KindOfNull may also depend
  // on there not being any values between KindOfUninit and KindOfNull.

                               //      uncounted init bit
                               //      |string bit
                               //      ||
  KindOfUninit        = 0x00,  //  00000000
  KindOfNull          = 0x08,  //  00001000
  KindOfBoolean       = 0x09,  //  00001001
  KindOfInt64         = 0x0a,  //  00001010
  KindOfDouble        = 0x0b,  //  00001011
  KindOfStaticString  = 0x0c,  //  00001100
  KindOfString        = 0x14,  //  00010100
  KindOfArray         = 0x20,  //  00100000
  KindOfObject        = 0x30,  //  00110000
  KindOfResource      = 0x40,  //  01000000
  KindOfRef           = 0x50,  //  01010000
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
constexpr int    kNumDataTypes = 12;
constexpr int8_t kMinDataType  = KindOfClass;
constexpr int8_t kMaxDataType  = KindOfRef;

/*
 * KindOfStringBit must be set in KindOfStaticString and KindOfString, and it
 * must be 0 in any other DataType.
 */
constexpr int KindOfStringBit = 0x04;

/*
 * KindOfUncountedInitBit must be set for Null, Boolean, Int64, Double, and
 * StaticString, and it must be 0 for any other DataType.
 */
constexpr int KindOfUncountedInitBit = 0x08;

/*
 * For a given DataType dt >= 0, this mask can be used to test if dt is
 * KindOfArray, KindOfObject, KindOfResource, or KindOfRef.
 */
constexpr unsigned kNotConstantValueTypeMask = 0x60;

/*
 * All DataTypes greater than this value are refcounted.
 */
constexpr DataType KindOfRefCountThreshold = KindOfStaticString;


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

static_assert(KindOfString       & KindOfStringBit, "");
static_assert(KindOfStaticString & KindOfStringBit, "");
static_assert(!(KindOfUninit     & KindOfStringBit), "");
static_assert(!(KindOfNull       & KindOfStringBit), "");
static_assert(!(KindOfBoolean    & KindOfStringBit), "");
static_assert(!(KindOfInt64      & KindOfStringBit), "");
static_assert(!(KindOfDouble     & KindOfStringBit), "");
static_assert(!(KindOfArray      & KindOfStringBit), "");
static_assert(!(KindOfObject     & KindOfStringBit), "");
static_assert(!(KindOfResource   & KindOfStringBit), "");
static_assert(!(KindOfRef        & KindOfStringBit), "");
static_assert(!(KindOfClass      & KindOfStringBit), "");

static_assert(KindOfNull         & KindOfUncountedInitBit, "");
static_assert(KindOfBoolean      & KindOfUncountedInitBit, "");
static_assert(KindOfInt64        & KindOfUncountedInitBit, "");
static_assert(KindOfDouble       & KindOfUncountedInitBit, "");
static_assert(KindOfStaticString & KindOfUncountedInitBit, "");
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

static_assert((kNotConstantValueTypeMask & KindOfArray) != 0  &&
              (kNotConstantValueTypeMask & KindOfObject) != 0 &&
              (kNotConstantValueTypeMask & KindOfResource) != 0 &&
              (kNotConstantValueTypeMask & KindOfRef) != 0,
              "DataType & kNotConstantValueTypeMask must be non-zero for "
              "Array, Object and Ref types");
static_assert(!(kNotConstantValueTypeMask &
                (KindOfNull|KindOfBoolean|KindOfInt64|KindOfDouble|
                 KindOfStaticString|KindOfString)),
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
    CS(StaticString)
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


///////////////////////////////////////////////////////////////////////////////
// Indices.

inline int getDataTypeIndex(DataType type) {
  switch (type) {
    case KindOfUninit       : return 0;
    case KindOfNull         : return 1;
    case KindOfBoolean      : return 2;
    case KindOfInt64        : return 3;
    case KindOfDouble       : return 4;
    case KindOfStaticString : return 5;
    case KindOfString       : return 6;
    case KindOfArray        : return 7;
    case KindOfObject       : return 8;
    case KindOfResource     : return 9;
    case KindOfRef          : return 10;
    case KindOfClass        : break;  // Not a "real" DT.
  }
  not_reached();
}

inline DataType getDataTypeValue(unsigned index) {
  switch (index) {
    case 0:  return KindOfUninit;
    case 1:  return KindOfNull;
    case 2:  return KindOfBoolean;
    case 3:  return KindOfInt64;
    case 4:  return KindOfDouble;
    case 5:  return KindOfStaticString;
    case 6:  return KindOfString;
    case 7:  return KindOfArray;
    case 8:  return KindOfObject;
    case 9:  return KindOfResource;
    case 10: return KindOfRef;
    default: not_reached();
  }
}

/*
 * These are used in type_variant.cpp and mc-generator.cpp.
 */
constexpr int kShiftDataTypeToDestrIndex = 4;
constexpr int kDestrTableSize = 6;

constexpr unsigned typeToDestrIdx(DataType t) {
  //assert(t == KindOfString || t == KindOfArray || t == KindOfObject ||
         //t == KindOfResource || t == KindOfRef);
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
 * Whether a type is any kind of string.
 */
constexpr bool isStringType(DataType t) {
  return (t & ~0x18) == KindOfStringBit;
}
inline bool isStringType(MaybeDataType t) {
  return t && isStringType(*t);
}
static_assert(KindOfStaticString == 0x0c, "");
static_assert(KindOfString       == 0x14, "");

/*
 * Other type-check functions.
 */
constexpr bool isIntType(DataType t) { return t == KindOfInt64; }
constexpr bool isBoolType(DataType t) { return t == KindOfBoolean; }
constexpr bool isDoubleType(DataType t) { return t == KindOfDouble; }
constexpr bool isArrayType(DataType t) { return t == KindOfArray; }

constexpr bool isIntKeyType(DataType t) {
  return t <= KindOfInt64;
}

/*
 * Return whether two DataTypes for primitive types are "equivalent" as far as
 * user-visible PHP types are concerned (i.e. ignoring different types of
 * strings or different types of nulls).
 *
 * Pre: t1 and t2 must both be DataTypes that represent PHP-types.
 * (non-internal KindOfs.)
 */
constexpr bool equivDataTypes(DataType t1, DataType t2) {
  return
    (t1 == t2) ||
    (isStringType(t1) && isStringType(t2)) ||
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
  case KindOfStaticString

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
