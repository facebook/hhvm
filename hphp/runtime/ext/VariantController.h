#ifndef VARIANTCONTROLLER_H
#define VARIANTCONTROLLER_H

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

/**
 * Hphp datatype conforming to datatype requirements in FBSerialize.h
 */
struct VariantController {
  typedef Variant VariantType;
  typedef Array MapType;
  typedef Array VectorType;
  typedef String StringType;

  // variant accessors
  static HPHP::serialize::Type type(const VariantType& obj) {
    switch (obj.getType()) {
      case KindOfUninit:
      case KindOfNull:       return HPHP::serialize::Type::NULLT;
      case KindOfBoolean:    return HPHP::serialize::Type::BOOL;
      case KindOfDouble:     return HPHP::serialize::Type::DOUBLE;
      case KindOfInt64:      return HPHP::serialize::Type::INT64;
      case KindOfArray:      return HPHP::serialize::Type::MAP;
      case KindOfStaticString:
      case KindOfString:     return HPHP::serialize::Type::STRING;
      case KindOfObject:     return HPHP::serialize::Type::OBJECT;
      default:
        throw HPHP::serialize::SerializeError(
          "don't know how to serialize HPHP Variant");
    }
  }
  static int64_t asInt64(const VariantType& obj) { return obj.toInt64(); }
  static bool asBool(const VariantType& obj) { return obj.toInt64() != 0; }
  static double asDouble(const VariantType& obj) { return obj.toDouble(); }
  static const String& asString(const VariantType& obj) {
    return obj.toCStrRef();
  }
  static CArrRef asMap(const VariantType& obj) { return obj.toCArrRef(); }
  static CArrRef asVector(const VariantType& obj) { return obj.toCArrRef(); }

  // variant creators
  static VariantType createNull() { return null_variant; }
  static VariantType fromInt64(int64_t val) { return val; }
  static VariantType fromBool(bool val) { return val; }
  static VariantType fromDouble(double val) { return val; }
  static VariantType fromString(const StringType& str) { return str; }
  static VariantType fromMap(const MapType& map) { return map; }
  static VariantType fromVector(const VectorType& vec) { return vec; }

  // map methods
  static MapType createMap() { return Array::Create(); }
  static HPHP::serialize::Type mapKeyType(CVarRef k) {
    return type(k);
  }
  static int64_t mapKeyAsInt64(CVarRef k) { return k.toInt64(); }
  static const String& mapKeyAsString(CVarRef k) {
    return k.toCStrRef();
  }
  template <typename Key>
  static void mapSet(MapType& map, Key&& k, VariantType&& v) {
    map.set(std::move(k), std::move(v));
  }
  static int64_t mapSize(const MapType& map) { return map.size(); }
  static ArrayIter mapIterator(const MapType& map) {
    return ArrayIter(map);
  }
  static bool mapNotEnd(const MapType& map, ArrayIter& it) {
    return !it.end();
  }
  static void mapNext(ArrayIter& it) { ++it; }
  static Variant mapKey(ArrayIter& it) { return it.first(); }
  static const VariantType& mapValue(ArrayIter& it) { return it.secondRef(); }

  // vector methods
  static VectorType createVector() { return Array::Create(); }
  static void vectorAppend(VectorType& vec, const VariantType& v) {
    vec.append(v);
  }
  static ArrayIter vectorIterator(const VectorType& vec) {
    return ArrayIter(vec);
  }
  static bool vectorNotEnd(const VectorType& vec, ArrayIter& it) {
    return !it.end();
  }
  static void vectorNext(ArrayIter& it) { ++it; }
  static const VariantType& vectorValue(ArrayIter& it) {
    return it.secondRef();
  }

  // string methods
  static StringType createMutableString(size_t n) {
    String ret(n, ReserveString);
    ret.setSize(n);
    return ret;
  }
  static char* getMutablePtr(StringType& s) {
    return s.bufferSlice().ptr;
  }
  static void shrinkString(String& s, size_t length) {
    s.shrink(length);
  }
  static StringType stringFromData(const char* src, int n) {
    return StringData::Make(src, n, CopyString);
  }
  static size_t stringLen(const StringType& str) { return str.size(); }
  static const char* stringData(const StringType& str) {
    return str.data();
  }
};

}

#endif
