/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#ifndef VARIANTCONTROLLER_H
#define VARIANTCONTROLLER_H

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/runtime-error.h"

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/fb/FBSerialize/FBSerialize.h"

#include <algorithm>

namespace HPHP {

inline void mapSetAndConvertStaticKeys(Array& map,
                                       StringData* str,
                                       Variant&& v) {
  int64_t i;
  if (map->convertKey<IntishCast::Cast>(str, i)) {
    map.set(i, *v.asTypedValue());
  } else if (auto sstr = str->isStatic()
             ? str
             : lookupStaticString(str)) {
    map.set(String::attach(sstr), *v.asTypedValue());
  } else {
    map.set(String(str), *v.asTypedValue());
  }
}

enum class VariantControllerHackArraysMode {
  // Do not serialize Hack arrays and unserialize as PHP arrays
  OFF,
  // Do serialize Hack arrays and unserialize as Hack arrays.
  // Does not support serializing keysets.
  ON,
  // (Un)serialize varrays / darrays: this will accept / emit Hack arrays if
  // HackArrDVArrs is set.
  MIGRATORY,
  // Do serialize Hack arrays and unserialize as Hack arrays.
  // Supports serializing keysets.
  ON_AND_KEYSET,

};

/**
 * Hphp datatype conforming to datatype requirements in FBSerialize.h
 */
template <VariantControllerHackArraysMode HackArraysMode>
struct VariantControllerImpl {
  typedef Variant VariantType;
  typedef Array MapType;
  typedef Array VectorType;
  typedef Array SetType;
  typedef String StringType;

  // variant accessors
  static HPHP::serialize::Type type(const_variant_ref obj) {
    switch (obj.getType()) {
      case KindOfUninit:
      case KindOfNull:       return HPHP::serialize::Type::NULLT;
      case KindOfBoolean:    return HPHP::serialize::Type::BOOL;
      case KindOfDouble:     return HPHP::serialize::Type::DOUBLE;
      case KindOfInt64:      return HPHP::serialize::Type::INT64;
      case KindOfFunc:
      case KindOfClass:
      case KindOfPersistentString:
      case KindOfString:     return HPHP::serialize::Type::STRING;
      case KindOfObject:     return HPHP::serialize::Type::OBJECT;
      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray:
        if (HackArraysMode == VariantControllerHackArraysMode::MIGRATORY) {
          return obj.asCArrRef().isVecOrVArray()
            ? HPHP::serialize::Type::LIST
            : HPHP::serialize::Type::MAP;
        }
        return HPHP::serialize::Type::MAP;
      case KindOfPersistentDict:
      case KindOfDict: {
        if (HackArraysMode == VariantControllerHackArraysMode::ON ||
            HackArraysMode == VariantControllerHackArraysMode::ON_AND_KEYSET) {
          return HPHP::serialize::Type::MAP;
        }
        if (RuntimeOption::EvalHackArrCompatFBSerializeHackArraysNotices) {
          raise_hackarr_compat_notice(
              "attempted to fb_serialize dict without "
              "FB_SERIALIZE_HACK_ARRAYS flag");
        }
        throw HPHP::serialize::HackArraySerializeError{};
      }
      case KindOfPersistentVec:
      case KindOfVec: {
        if (HackArraysMode == VariantControllerHackArraysMode::ON ||
            HackArraysMode == VariantControllerHackArraysMode::ON_AND_KEYSET) {
          return HPHP::serialize::Type::LIST;
        }
        if (RuntimeOption::EvalHackArrCompatFBSerializeHackArraysNotices) {
          raise_hackarr_compat_notice(
              "attempted to fb_serialize vec without "
              "FB_SERIALIZE_HACK_ARRAYS flag");
        }
        throw HPHP::serialize::HackArraySerializeError{};
      }
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        if (HackArraysMode == VariantControllerHackArraysMode::ON_AND_KEYSET) {
          return HPHP::serialize::Type::SET;
        }

        throw HPHP::serialize::KeysetSerializeError{};

      case KindOfClsMeth:
        if (HackArraysMode == VariantControllerHackArraysMode::MIGRATORY) {
          return HPHP::serialize::Type::LIST;
        } else {
          return HPHP::serialize::Type::MAP;
        }

      case KindOfResource:
      case KindOfRecord: // TODO(T41025646): implement serialization for records
        throw HPHP::serialize::SerializeError(
          "don't know how to serialize HPHP Variant");
    }
    not_reached();
  }
  static int64_t asInt64(const_variant_ref obj) { return obj.toInt64(); }
  static bool asBool(const_variant_ref obj) { return obj.toInt64() != 0; }
  static double asDouble(const_variant_ref obj) { return obj.toDouble(); }
  static String asString(const_variant_ref obj) { return obj.toString(); }
  static Array asMap(const_variant_ref obj) { return obj.toArray(); }
  static Array asVector(const_variant_ref obj) { return obj.toArray(); }
  static Array asSet(const_variant_ref obj) { return obj.toArray(); }

  // variant creators
  static VariantType createNull() { return init_null(); }
  static VariantType fromInt64(int64_t val) { return val; }
  static VariantType fromBool(bool val) { return val; }
  static VariantType fromDouble(double val) { return val; }
  static VariantType fromString(const StringType& str) { return str; }
  static VariantType fromMap(const MapType& map) { return map; }
  static VariantType fromVector(const VectorType& vec) { return vec; }
  static VariantType fromSet(const SetType& set) { return set; }

  // map methods
  static MapType createMap() {
    switch (HackArraysMode) {
      case VariantControllerHackArraysMode::ON:
      case VariantControllerHackArraysMode::ON_AND_KEYSET:
        return empty_dict_array();
      case VariantControllerHackArraysMode::OFF:
        return empty_darray();
      case VariantControllerHackArraysMode::MIGRATORY:
        return RuntimeOption::EvalHackArrDVArrs
          ? empty_dict_array()
          : empty_darray();
    }
  }
  static MapType createMap(DArrayInit&& map) {
    auto arrayData = map.toArray().detach();
    switch (HackArraysMode) {
      case VariantControllerHackArraysMode::ON:
      case VariantControllerHackArraysMode::ON_AND_KEYSET:
        return Array::attach(arrayData->toDict(false));
      case VariantControllerHackArraysMode::OFF:
        return Array::attach(arrayData->toDArray(false));
      case VariantControllerHackArraysMode::MIGRATORY:
        return Array::attach(arrayData->toDArray(false));
    }
    not_reached(); // not sure why I need this here and not in createMap()
  }
  static DArrayInit reserveMap(size_t n) {
    DArrayInit res(n, CheckAllocation{});
    return res;
  }
  static MapType getStaticEmptyMap() {
    ArrayData* empty;
    switch (HackArraysMode) {
      case VariantControllerHackArraysMode::ON:
      case VariantControllerHackArraysMode::ON_AND_KEYSET:
        empty = ArrayData::CreateDict();
        break;
      case VariantControllerHackArraysMode::OFF:
        empty = ArrayData::CreateDArray();
        break;
      case VariantControllerHackArraysMode::MIGRATORY:
        empty = RuntimeOption::EvalHackArrDVArrs
          ? ArrayData::CreateDict()
          : ArrayData::CreateDArray();
        break;
    }
    return MapType(empty);
  }
  static HPHP::serialize::Type mapKeyType(const Variant& k) {
    return type(k);
  }
  static int64_t mapKeyAsInt64(const Variant& k) { return k.toInt64(); }
  static String mapKeyAsString(const Variant& k) {
    return k.toString();
  }

  static void mapSet(MapType& map, StringType&& k, VariantType&& v) {
    mapSetAndConvertStaticKeys(map, k.get(), std::forward<VariantType>(v));
  }

  static void mapSet(MapType& map, int64_t k, VariantType&& v) {
    map.set(k, std::move(v));
  }

  template <typename Key>
  static void mapSet(DArrayInit& map, Key&& k, VariantType&& v) {
    map.setUnknownKey<IntishCast::Cast>(std::move(k), std::move(v));
  }
  static int64_t mapSize(const MapType& map) { return map.size(); }
  static ArrayIter mapIterator(const MapType& map) {
    return ArrayIter(map);
  }
  static bool mapNotEnd(const MapType& /*map*/, ArrayIter& it) {
    return !it.end();
  }
  static void mapNext(ArrayIter& it) { ++it; }
  static Variant mapKey(ArrayIter& it) { return it.first(); }
  static Variant mapValue(ArrayIter& it) { return it.second(); }

  // vector methods
  static VectorType createVector() {
    switch (HackArraysMode) {
      case VariantControllerHackArraysMode::ON:
      case VariantControllerHackArraysMode::ON_AND_KEYSET:
        return empty_vec_array();
      case VariantControllerHackArraysMode::OFF:
        return empty_darray();
      case VariantControllerHackArraysMode::MIGRATORY:
        return RuntimeOption::EvalHackArrDVArrs
          ? empty_vec_array()
          : empty_varray();
    }
  }
  static int64_t vectorSize(const VectorType& vec) {
    return vec.size();
  }
  static void vectorAppend(VectorType& vec, const VariantType& v) {
    vec.append(v);
  }
  static ArrayIter vectorIterator(const VectorType& vec) {
    return ArrayIter(vec);
  }
  static bool vectorNotEnd(const VectorType& /*vec*/, ArrayIter& it) {
    return !it.end();
  }
  static void vectorNext(ArrayIter& it) { ++it; }
  static Variant vectorValue(ArrayIter& it) { return it.second(); }

  // set methods
  static SetType createSet() {
    return empty_keyset();
  }
  static int64_t setSize(const SetType& set) {
    return set.size();
  }
  static void setAppend(SetType& set, const VariantType& v) {
    auto value_type = type(v);
    if (value_type != HPHP::serialize::Type::INT64 &&
        value_type != HPHP::serialize::Type::STRING) {
      throw HPHP::serialize::UnserializeError(
          "Unsupported keyset element of type " +
          folly::to<std::string>(value_type));
    }
    set.append(v);
  }
  static ArrayIter setIterator(const VectorType& set) {
    return ArrayIter(set);
  }
  static bool setNotEnd(const SetType& /*set*/, ArrayIter& it) {
    return !it.end();
  }
  static void setNext(ArrayIter& it) { ++it; }
  static Variant setValue(ArrayIter& it) { return it.second(); }

  // string methods
  static StringType createMutableString(size_t n) {
    String ret(n, ReserveString);
    ret.setSize(n);
    return ret;
  }
  static StringType createStaticString(const char* str, size_t len) {
    String ret = String(makeStaticString(str, len));
    return ret;
  }
  static StringType getStaticEmptyString() {
    return empty_string();
  }
  static char* getMutablePtr(StringType& s) {
    return s.mutableData();
  }
  static void shrinkString(String& s, size_t length) {
    s.shrink(length);
  }
  static StringType stringFromData(const char* src, int n) {
    return StringType::attach(StringData::Make(src, n, CopyString));
  }
  static StringType stringFromMutableString(StringType&& s) {
    return std::move(s);
  }
  static size_t stringLen(const StringType& str) { return str.size(); }
  static const char* stringData(const StringType& str) {
    return str.data();
  }

  /* called by FBSerializer before serializing each item,
     useful to instrument the serialization process if needed */
  ALWAYS_INLINE
  static void traceSerialization(const_variant_ref thing) {
    if (LIKELY(!RuntimeOption::EvalLogArrayProvenance)) return;
    if (HackArraysMode != VariantControllerHackArraysMode::ON &&
        HackArraysMode != VariantControllerHackArraysMode::ON_AND_KEYSET) {
      return;
    }
    if (!thing.isArray()) return;
    auto const ad = thing.getArrayData();

    if (arrprov::arrayWantsTag(ad) &&
        (thing.isVecArray() || ad->isVArray())) {
      raise_array_serialization_notice(SerializationSite::FBSerialize,
                                       thing.asCArrRef().get());
    }
  }
};

using VariantController =
  VariantControllerImpl<VariantControllerHackArraysMode::OFF>;
using VariantControllerUsingHackArrays =
  VariantControllerImpl<VariantControllerHackArraysMode::ON>;
using VariantControllerUsingHackArraysAndKeyset =
  VariantControllerImpl<VariantControllerHackArraysMode::ON_AND_KEYSET>;
using VariantControllerUsingVarrayDarray =
  VariantControllerImpl<VariantControllerHackArraysMode::MIGRATORY>;
}


#endif
