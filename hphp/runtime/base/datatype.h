/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// definitions

typedef std::conditional<packed_tv, int8_t, int32_t>::type DataTypeInt;
enum DataType: DataTypeInt {
  KindOfClass            = -13,
  MinDataType            = -13,

  // Values below zero are not PHP values, but runtime-internal.
  KindOfAny              = -8,
  KindOfUncounted        = -7,
  KindOfUncountedInit    = -6,

  KindOfInvalid          = -1,
  KindOfUnknown          = KindOfInvalid,

  /**
   * Beware if you change the order, as we may have a few type checks
   * in the code that depend on the order.  Also beware of adding to
   * the number of bits needed to represent this.  (Known dependency
   * in unwind-x64.h.)
   */
  KindOfUninit           = 0,
  // Any code that static_asserts about the value of KindOfNull may
  // also depend on there not being any values between KindOfUninit
  // and KindOfNull.
  KindOfNull             = 8,     //   0001000    0x08
  KindOfBoolean          = 9,     //   0001001    0x09
  KindOfInt64            = 10,    //   0001010    0x0a
  KindOfDouble           = 11,    //   0001011    0x0b

  KindOfStaticString     = 12,    //   0001100    0x0c
  KindOfString           = 20,    //   0010100    0x14
  KindOfArray            = 32,    //   0100000    0x20
  KindOfObject           = 64,    //   1000000    0x40
  KindOfRef              = 96,    //   1100000    0x60
  KindOfIndirect         = 97,    //   1100001    0x61

  MaxNumDataTypes        = KindOfIndirect + 1, // marker, not a valid type
  MaxNumDataTypesIndex   = 11 + 1,  // 1 + the number of valid DataTypes above

  MaxDataType            = 0x7f, // Allow KindOf* > 11 in HphpArray.
  // Note: KindOfStringBit must be set in KindOfStaticString and KindOfString,
  //       and it must be 0 in any other real DataType.
  KindOfStringBit        = 4,

  // Note: KindOfUncountedInitBit must be set for Null, Boolean, Int64, Double,
  //       and StaticString, and it must be 0 for any other real DataType.
  KindOfUncountedInitBit = 8,
};

const unsigned int kDataTypeMask = 0x7F;
const unsigned int kNotConstantValueTypeMask = KindOfRef;

// All DataTypes greater than this value are refcounted.
const DataType KindOfRefCountThreshold = KindOfStaticString;

enum DataTypeCategory {
  DataTypeGeneric,
  DataTypeCountness,
  DataTypeCountnessInit,
  DataTypeSpecific,
  DataTypeSpecialized
};

///////////////////////////////////////////////////////////////////////////////
// nannies

static_assert(KindOfString       & KindOfStringBit, "");
static_assert(KindOfStaticString & KindOfStringBit, "");
static_assert(!(KindOfUninit     & KindOfStringBit), "");
static_assert(!(KindOfNull       & KindOfStringBit), "");
static_assert(!(KindOfBoolean    & KindOfStringBit), "");
static_assert(!(KindOfInt64      & KindOfStringBit), "");
static_assert(!(KindOfDouble     & KindOfStringBit), "");
static_assert(!(KindOfArray      & KindOfStringBit), "");
static_assert(!(KindOfObject     & KindOfStringBit), "");
static_assert(!(KindOfRef        & KindOfStringBit), "");
static_assert(!(KindOfIndirect   & KindOfStringBit), "");
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
static_assert(!(KindOfRef        & KindOfUncountedInitBit), "");
static_assert(!(KindOfIndirect   & KindOfUncountedInitBit), "");
static_assert(!(KindOfClass      & KindOfUncountedInitBit), "");

// assume KindOfUninit == 0 in ClsCns
static_assert(KindOfUninit == 0,
              "Several things assume this tag is 0, especially target cache");

static_assert(MaxNumDataTypes - 1 <= kDataTypeMask, "");

static_assert((kNotConstantValueTypeMask & KindOfArray) != 0  &&
              (kNotConstantValueTypeMask & KindOfObject) != 0 &&
              (kNotConstantValueTypeMask & KindOfRef) != 0,
              "DataType & kNotConstantValueTypeMask must be non-zero for "
              "Array, Object and Ref types");
static_assert(!(kNotConstantValueTypeMask &
                (KindOfNull|KindOfBoolean|KindOfInt64|KindOfDouble|
                 KindOfStaticString|KindOfString)),
              "DataType & kNotConstantValueTypeMask must be zero for "
              "null, bool, int, double and string types");

///////////////////////////////////////////////////////////////////////////////
// helpers

inline std::string tname(DataType t) {
  switch(t) {
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
    CS(Ref)
    CS(Class)
    CS(Any)
    CS(Uncounted)
    CS(UncountedInit)

#undef CS
    case KindOfInvalid: return std::string("Invalid");

    default: {
      char buf[128];
      sprintf(buf, "Unknown:%d", t);
      return std::string(buf);
    }
  }
}

inline const char* getDataTypeString(DataType t) {
  switch (t) {
    case KindOfUninit:
    case KindOfNull:    return "NULL";
    case KindOfBoolean: return "boolean";
    case KindOfInt64:   return "integer";
    case KindOfDouble:  return "double";
    case KindOfStaticString:
    case KindOfString:  return "string";
    case KindOfArray:   return "array";
    case KindOfObject:  return "object";
    default:
      assert(false);
      break;
  }
  return "";
}

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
    case KindOfRef          : return 9;
    case KindOfIndirect     : return 10;
    default                 : not_reached();
  }
}

inline DataType getDataTypeValue(unsigned index) {
  switch (index) {
    case 0  : return KindOfUninit;
    case 1  : return KindOfNull;
    case 2  : return KindOfBoolean;
    case 3  : return KindOfInt64;
    case 4  : return KindOfDouble;
    case 5  : return KindOfStaticString;
    case 6  : return KindOfString;
    case 7  : return KindOfArray;
    case 8  : return KindOfObject;
    case 9  : return KindOfRef;
    case 10 : return KindOfIndirect;
    default : not_reached();
  }
}

// These are used in type_variant.cpp and translator-x64.cpp
const unsigned int kShiftDataTypeToDestrIndex = 5;
const unsigned int kDestrTableSize = 4;

#define TYPE_TO_DESTR_IDX(t) ((t) >> kShiftDataTypeToDestrIndex)

inline ALWAYS_INLINE unsigned typeToDestrIndex(DataType t) {
  assert(t >= KindOfString && t <= KindOfRef);
  return TYPE_TO_DESTR_IDX(t);
}

// Helper macro for checking if a given type is refcounted
#define IS_REFCOUNTED_TYPE(t) ((t) > KindOfRefCountThreshold)

// Helper macro for checking if a type is KindOfString or KindOfStaticString.
static_assert(KindOfStaticString == 0x0C, "");
static_assert(KindOfString       == 0x14, "");
#define IS_STRING_TYPE(t) (((t) & ~0x18) == KindOfStringBit)

// Check if a type is KindOfUninit or KindOfNull
#define IS_NULL_TYPE(t) (unsigned(t) <= KindOfNull)
// Other type check macros
#define IS_INT_TYPE(t) ((t) == KindOfInt64)
#define IS_ARRAY_TYPE(t) ((t) == KindOfArray)
#define IS_BOOL_TYPE(t) ((t) == KindOfBoolean)
#define IS_DOUBLE_TYPE(t) ((t) == KindOfDouble)

#define IS_REAL_TYPE(t) \
  (((t) >= KindOfUninit && (t) < MaxNumDataTypes) || (t) == KindOfClass)

inline bool IS_INT_KEY_TYPE(DataType t) {
  return t <= KindOfInt64;
}

// typeReentersOnRelease --
//   Returns whether the release helper for a given type can
//   reenter.
inline bool typeReentersOnRelease(DataType type) {
  return IS_REFCOUNTED_TYPE(type) && type != KindOfString;
}

inline DataType typeInitNull(DataType t) {
  return t == KindOfUninit ? KindOfNull : t;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_DATATYPE_H_
