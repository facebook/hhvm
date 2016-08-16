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
  KindOfClass            = -16,      //  11110000

  // Any code that static_asserts about the value of KindOfNull may also depend
  // on there not being any values between KindOfUninit and KindOfNull.

                                     //      Hack array bit
                                     //      |PHP array bit
                                     //      ||string bit
                                     //      |||uncounted init bit
                                     //      ||||
  KindOfUninit           = 0x00,     //  00000000
  KindOfNull             = 0x01,     //  00000001
  KindOfInt64            = 0x11,     //  00010001
//KindOfPersistentVec    = 0x19,     //  00011001
  KindOfBoolean          = 0x21,     //  00100001
  KindOfPersistentString = 0x23,     //  00100011
//KindOfPersistentDict   = 0x29,     //  00101001
  KindOfDouble           = 0x31,     //  00110001
  KindOfPersistentArray  = 0x35,     //  00110101
//KindOfPersistentKeyset = 0x39,     //  00111001
  KindOfObject           = 0x40,     //  01000000
  KindOfResource         = 0x50,     //  01010000
//KindOfVec              = 0x58,     //  01011000
  KindOfString           = 0x62,     //  01100010
//KindOfDict             = 0x68,     //  01101000
  KindOfRef              = 0x70,     //  01110000
  KindOfArray            = 0x74,     //  01110100
//KindOfKeyset           = 0x78,     //  01111000
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
constexpr int8_t kMaxDataType  = KindOfArray;

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
 * One of KindOfHashPersistentBits must be set for KindOfString,
 * KindOfPersistentString, KindOfArray, or KindOfPersistentArray. It signifies
 * the type has both persistent and non-persistent variants.
 */
constexpr int KindOfHasPersistentBits = KindOfStringBit | KindOfArrayBit;

/*
 * The result of ANDing kDataTypeEquivalentMask against KindOf[Persistent]Array,
 * KindOf[Persistent]String, or KindOfNull/KindOfUninit yields some unspecified
 * value which is the same for each persistent/non-persistent pair, and
 * different for all else. Used to check for equivalency between persistent and
 * non-persistent DataTypes.
 */
constexpr int kDataTypeEquivalentMask = 0x3e;

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
constexpr int kShiftDataTypeToDestrIndex = 2;
constexpr int kDestrTableSize = 31;

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
    ((t1 & t2 & KindOfHasPersistentBits) &&
     ((t1 & kDataTypeEquivalentMask) == (t2 & kDataTypeEquivalentMask))) ||
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
static_assert(!isStringType(KindOfObject),          "");
static_assert(!isStringType(KindOfResource),        "");
static_assert(!isStringType(KindOfRef),             "");
static_assert(!isStringType(KindOfClass),           "");

static_assert(isArrayType(KindOfArray),             "");
static_assert(isArrayType(KindOfPersistentArray),   "");
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
static_assert(!isArrayType(KindOfClass),            "");

static_assert(isNullType(KindOfUninit),            "");
static_assert(isNullType(KindOfNull),              "");
static_assert(!isNullType(KindOfArray),            "");
static_assert(!isNullType(KindOfPersistentArray),  "");
static_assert(!isNullType(KindOfBoolean),          "");
static_assert(!isNullType(KindOfInt64),            "");
static_assert(!isNullType(KindOfDouble),           "");
static_assert(!isNullType(KindOfPersistentString), "");
static_assert(!isNullType(KindOfString),           "");
static_assert(!isNullType(KindOfObject),           "");
static_assert(!isNullType(KindOfResource),         "");
static_assert(!isNullType(KindOfRef),              "");
static_assert(!isNullType(KindOfClass),            "");

static_assert(isRefcountedType(KindOfString),            "");
static_assert(isRefcountedType(KindOfObject),            "");
static_assert(isRefcountedType(KindOfResource),          "");
static_assert(isRefcountedType(KindOfRef),               "");
static_assert(isRefcountedType(KindOfArray),             "");
static_assert(!isRefcountedType(KindOfUninit),           "");
static_assert(!isRefcountedType(KindOfNull),             "");
static_assert(!isRefcountedType(KindOfBoolean),          "");
static_assert(!isRefcountedType(KindOfInt64),            "");
static_assert(!isRefcountedType(KindOfDouble),           "");
static_assert(!isRefcountedType(KindOfPersistentString), "");
static_assert(!isRefcountedType(KindOfPersistentArray),  "");
static_assert(!isRefcountedType(KindOfClass),            "");

static_assert(isIntKeyType(KindOfUninit),            "");
static_assert(isIntKeyType(KindOfNull),              "");
static_assert(isIntKeyType(KindOfInt64),             "");
static_assert(!isIntKeyType(KindOfString),           "");
static_assert(!isIntKeyType(KindOfObject),           "");
static_assert(!isIntKeyType(KindOfResource),         "");
static_assert(!isIntKeyType(KindOfRef),              "");
static_assert(!isIntKeyType(KindOfArray),            "");
static_assert(!isIntKeyType(KindOfBoolean),          "");
static_assert(!isIntKeyType(KindOfDouble),           "");
static_assert(!isIntKeyType(KindOfPersistentString), "");
static_assert(!isIntKeyType(KindOfPersistentArray),  "");

static_assert(equivDataTypes(KindOfNull, KindOfUninit),             "");
static_assert(equivDataTypes(KindOfArray, KindOfPersistentArray),   "");
static_assert(equivDataTypes(KindOfString, KindOfPersistentString), "");
static_assert(!equivDataTypes(KindOfNull, KindOfString),            "");
static_assert(!equivDataTypes(KindOfNull, KindOfInt64),             "");
static_assert(!equivDataTypes(KindOfBoolean, KindOfInt64),          "");
static_assert(!equivDataTypes(KindOfUninit, KindOfArray),           "");
static_assert(!equivDataTypes(KindOfObject, KindOfResource),        "");
static_assert(!equivDataTypes(KindOfArray, KindOfString),           "");
static_assert(!equivDataTypes(KindOfArray, KindOfPersistentString), "");
static_assert(!equivDataTypes(KindOfArray, KindOfObject),           "");

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

static_assert(KindOfString       & KindOfHasPersistentBits, "");
static_assert(KindOfArray        & KindOfHasPersistentBits, "");
static_assert(KindOfPersistentString & KindOfHasPersistentBits, "");
static_assert(KindOfPersistentArray  & KindOfHasPersistentBits, "");
static_assert(!(KindOfNull       & KindOfHasPersistentBits), "");
static_assert(!(KindOfBoolean    & KindOfHasPersistentBits), "");
static_assert(!(KindOfInt64      & KindOfHasPersistentBits), "");
static_assert(!(KindOfDouble     & KindOfHasPersistentBits), "");
static_assert(!(KindOfUninit     & KindOfHasPersistentBits), "");
static_assert(!(KindOfObject     & KindOfHasPersistentBits), "");
static_assert(!(KindOfResource   & KindOfHasPersistentBits), "");
static_assert(!(KindOfRef        & KindOfHasPersistentBits), "");
static_assert(!(KindOfClass      & KindOfHasPersistentBits), "");

static_assert(KindOfUninit == 0,
              "Several things assume this tag is 0, especially RDS");

static_assert(kMaxDataType <= kDataTypeMask, "");

///////////////////////////////////////////////////////////////////////////////

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
