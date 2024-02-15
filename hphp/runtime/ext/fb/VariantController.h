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
#include "hphp/runtime/base/bespoke-runtime.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/datatype.h"

#include "hphp/runtime/ext/extension.h"
#include "common/serialize/FBSerialize.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace HPHP {

template <IntishCast IC>
inline void mapSetConvertStatic(Array& map, StringData* str, Variant&& v) {
  int64_t i;
  if (IC == IntishCast::Cast && str->isStrictlyInteger(i)) {
    map.set(i, *v.asTypedValue());
  } else if (auto sstr = str->isStatic() ? str : lookupStaticString(str)) {
    map.set(String::attach(sstr), *v.asTypedValue());
  } else {
    map.set(String(str), *v.asTypedValue());
  }
}

enum class VariantControllerHackArraysMode {
  // Serialize vecs and dicts both as dicts. Intish-cast dict keys. This mode
  // does not preserve types or values, and only exists because vecs and dicts
  // were once a single PHP array type with the intish-cast behavior.
  OFF,
  // Serialize Hack arrays (excluding keysets), and unserialize them as the
  // same Hack arrays, except for one case: unserialize marked vecs as dicts.
  ON,
  // (Un)serialize varrays / darrays: this will accept / emit Hack arrays.
  // Intish-cast dict keys.
  MIGRATORY,
  // Serialize Hack arrays (including keysets), and unserialize them as the
  // same Hack arrays, except for one case: unserialize marked vecs as dicts.
  ON_AND_KEYSET,
  // The "best" mode, with the fewest number of legacy array behaviors.
  //
  // Serialize Hack arrays (including keysets), and unserialize them as the
  // same Hack array in all cases. Ignore legacy array marks.
  POST_MIGRATION,
};

/**
 * Hphp datatype conforming to datatype requirements in FBSerialize.h
 */
template <VariantControllerHackArraysMode HackArraysMode>
struct VariantControllerImpl {
  using VariantType = Variant;
  using MapType = Array;
  using VectorType = Array;
  using SetType = Array;
  using StringType = String;
  using StructHandle = RuntimeStruct*;

  // variant accessors
  static HPHP::serialize::Type type(const_variant_ref obj) {
    switch (obj.getType()) {
      case KindOfUninit:
      case KindOfNull:       return HPHP::serialize::Type::NULLT;
      case KindOfBoolean:    return HPHP::serialize::Type::BOOL;
      case KindOfDouble:     return HPHP::serialize::Type::DOUBLE;
      case KindOfInt64:      return HPHP::serialize::Type::INT64;
      case KindOfFunc:
        if (obj.toFuncVal()->isMethCaller()) {
          throw HPHP::serialize::MethCallerSerializeError();
        }
      case KindOfClass:
      case KindOfLazyClass:
      case KindOfPersistentString:
      case KindOfString:     return HPHP::serialize::Type::STRING;
      case KindOfObject:
        if (RO::EvalForbidMethCallerHelperSerialize &&
            obj.asCObjRef().get()->getVMClass() ==
              SystemLib::getMethCallerHelperClass()) {
          if (RO::EvalForbidMethCallerHelperSerialize == 1) {
            raise_warning("Serializing MethCallerHelper");
          } else {
            throw HPHP::serialize::MethCallerSerializeError();
          }
        }
        return HPHP::serialize::Type::OBJECT;

      case KindOfPersistentDict:
      case KindOfDict: {
        return HPHP::serialize::Type::MAP;
      }

      case KindOfPersistentVec:
      case KindOfVec: {
        switch (HackArraysMode) {
          case VariantControllerHackArraysMode::OFF:
            return HPHP::serialize::Type::MAP;
          case VariantControllerHackArraysMode::MIGRATORY:
          case VariantControllerHackArraysMode::POST_MIGRATION:
            return HPHP::serialize::Type::LIST;
          case VariantControllerHackArraysMode::ON:
          case VariantControllerHackArraysMode::ON_AND_KEYSET: {
            auto const legacy = obj.rval().val().parr->isLegacyArray();
            return legacy ? HPHP::serialize::Type::MAP
                          : HPHP::serialize::Type::LIST;
          }
        }
      }

      case KindOfPersistentKeyset:
      case KindOfKeyset:
        if constexpr (
            HackArraysMode == VariantControllerHackArraysMode::ON_AND_KEYSET ||
            HackArraysMode == VariantControllerHackArraysMode::POST_MIGRATION) {
          return HPHP::serialize::Type::SET;
        }
        throw HPHP::serialize::KeysetSerializeError{};

      case KindOfClsMeth:
        if constexpr (
            HackArraysMode == VariantControllerHackArraysMode::MIGRATORY ||
            HackArraysMode == VariantControllerHackArraysMode::POST_MIGRATION) {
          return HPHP::serialize::Type::LIST;
        } else {
          return HPHP::serialize::Type::MAP;
        }

      case KindOfRClsMeth:
        throw HPHP::serialize::SerializeError(
          "Unable to serialize reified class method pointer"
        );

      case KindOfResource:
      case KindOfEnumClassLabel:
        throw HPHP::serialize::SerializeError(
          "don't know how to serialize HPHP Variant");
      case KindOfRFunc:
        throw HPHP::serialize::SerializeError(
          "Unable to serialize reified function pointer"
        );
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
    return Array::CreateDict();
  }
  static MapType createMap(DictInit&& map) {
    return map.toArray();
  }
  static MapType createMap(StructDictInit&& map) {
    return map.toArray();
  }
  static DictInit reserveMap(size_t n) {
    DictInit res(n, CheckAllocation{});
    return res;
  }
  static StructDictInit reserveMap(StructHandle handle, size_t n) {
    StructDictInit res(handle, n);
    return res;
  }
  static MapType getStaticEmptyMap() {
    return Array::CreateDict();
  }
  static HPHP::serialize::Type mapKeyType(const Variant& k) {
    return type(k);
  }
  static int64_t mapKeyAsInt64(const Variant& k) { return k.toInt64(); }
  static String mapKeyAsString(const Variant& k) {
    return k.toString();
  }
  static bool mapExistsStringKey(const MapType& map, const StringType& key) {
    return map.exists(key);
  }
  static VariantType mapAtStringKey(
      const MapType& map,
      const StringType& key) {
    return map[key];
  }
  static void mapSet(MapType& map, StringType&& k, VariantType&& v) {
    auto constexpr IC = [&]{
      switch (HackArraysMode) {
        case VariantControllerHackArraysMode::ON:
        case VariantControllerHackArraysMode::ON_AND_KEYSET:
        case VariantControllerHackArraysMode::POST_MIGRATION:
          return IntishCast::None;
        case VariantControllerHackArraysMode::OFF:
        case VariantControllerHackArraysMode::MIGRATORY:
          return IntishCast::Cast;
      }
    }();
    mapSetConvertStatic<IC>(map, k.get(), std::forward<VariantType>(v));
  }

  static void mapSet(MapType& map, int64_t k, VariantType&& v) {
    map.set(k, std::move(v));
  }

  static void mapSet(DictInit& map, const StringType& k, VariantType&& v) {
    map.setUnknownKey<IntishCast::Cast>(k.asTypedValue(), std::move(v));
  }

  static void mapSet(DictInit& map, int64_t k, VariantType&& v) {
    map.setUnknownKey<IntishCast::Cast>(make_tv<KindOfInt64>(k), std::move(v));
  }

  static void mapSet(StructDictInit& map, size_t idx, const StringType& k,
                     VariantType&& v) {
    map.setIntishCast(idx, k, std::move(v));
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
      case VariantControllerHackArraysMode::MIGRATORY:
      case VariantControllerHackArraysMode::POST_MIGRATION:
        return empty_vec_array();
      case VariantControllerHackArraysMode::OFF:
        return empty_dict_array();
    }
  }
  static int64_t vectorSize(const VectorType& vec) {
    return vec.size();
  }
  static void vectorAppend(VectorType& vec, const VariantType& v) {
    if constexpr (HackArraysMode == VariantControllerHackArraysMode::OFF) {
      vec.set(safe_cast<int64_t>(vec.size()), v);
    } else {
      vec.append(v);
    }
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
    if (!v.isInteger() && !v.isString()) {
      throw HPHP::serialize::UnserializeError(
        "Keysets can only contain integers or strings"
      );
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

  static StructHandle registerStruct(
      const String& stableIdentifier,
      const std::vector<std::pair<size_t, StringType>>& fields) {
    auto const runtimeStruct =
      RuntimeStruct::registerRuntimeStruct(stableIdentifier, fields);
    return runtimeStruct;
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
using VariantControllerPostHackArrayMigration =
  VariantControllerImpl<VariantControllerHackArraysMode::POST_MIGRATION>;

}


#endif
