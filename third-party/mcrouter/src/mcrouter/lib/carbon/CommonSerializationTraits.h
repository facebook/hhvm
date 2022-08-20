/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <folly/Optional.h>
#include <folly/Traits.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/carbon/Fields.h"
#include "mcrouter/lib/carbon/Keys.h"

namespace carbon {

// SerializationTraits specialization for Keys<>
template <class Storage>
struct SerializationTraits<Keys<Storage>> {
  static_assert(
      carbon::detail::TypeToField<Storage>::fieldType ==
          carbon::FieldType::Binary,
      "Keys can only be templated by a binary type.");

  static constexpr carbon::FieldType kWireType = carbon::FieldType::Binary;

  template <class Reader>
  static Keys<Storage> read(Reader&& reader) {
    return Keys<Storage>(reader.template readRaw<Storage>());
  }

  template <class Writer>
  static void write(const Keys<Storage>& key, Writer&& writer) {
    writer.writeRaw(key.raw());
  }

  static bool isEmpty(const Keys<Storage>& key) {
    return key.empty();
  }
};

template <class T>
struct SerializationTraits<folly::Optional<T>> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::Struct;

  using value_type = typename folly::Optional<T>::value_type;

  template <class Reader>
  static folly::Optional<T> read(Reader&& reader) {
    folly::Optional<T> opt;
    reader.readStructBegin();
    while (true) {
      const auto pr = reader.readFieldHeader();
      const auto fieldType = pr.first;
      const auto fieldId = pr.second;

      if (fieldType == carbon::FieldType::Stop) {
        break;
      }

      switch (fieldId) {
        case 1: {
          reader.readField(opt, fieldType);
          break;
        }
        default: {
          reader.skip(fieldType);
          break;
        }
      }
    }
    reader.readStructEnd();
    return opt;
  }

  template <class Writer>
  static void write(const folly::Optional<T>& opt, Writer&& writer) {
    writer.writeStructBegin();
    writer.writeField(1 /* field id */, opt);
    writer.writeFieldStop();
    writer.writeStructEnd();
  }

  static bool isEmpty(const folly::Optional<T>& opt) {
    return !opt.hasValue();
  }
};

template <class T>
struct SerializationTraits<
    T,
    typename std::enable_if<
        folly::IsOneOf<T, std::string, folly::IOBuf>::value>::type> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::Binary;

  template <class Reader>
  static T read(Reader&& reader) {
    return reader.template readRaw<T>();
  }

  template <class Writer>
  static void write(const T& t, Writer&& writer) {
    writer.writeRaw(t);
  }

  static bool isEmpty(const T& t) {
    return t.empty();
  }
};

template <class T>
struct SerializationTraits<std::vector<T>> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::List;

  using inner_type = T;

  static size_t size(const std::vector<T>& vec) {
    return vec.size();
  }

  static bool emplace(std::vector<T>& vec, T&& t) {
    vec.emplace_back(std::move(t));
    return true;
  }

  static void clear(std::vector<T>& vec) {
    vec.clear();
  }

  static void reserve(std::vector<T>& vec, size_t len) {
    vec.reserve(len);
  }

  static auto begin(const std::vector<T>& vec) -> decltype(vec.begin()) {
    return vec.begin();
  }

  static auto end(const std::vector<T>& vec) -> decltype(vec.end()) {
    return vec.end();
  }
};

template <class T>
struct SerializationTraits<
    T,
    typename std::enable_if<folly::IsOneOf<
        T,
        std::set<typename T::key_type>,
        std::unordered_set<typename T::key_type>,
        folly::F14FastSet<typename T::key_type>,
        folly::F14NodeSet<typename T::key_type>,
        folly::F14ValueSet<typename T::key_type>,
        folly::F14VectorSet<typename T::key_type>>::value>::type> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::Set;

  using inner_type = typename T::key_type;

  static size_t size(const T& set) {
    return set.size();
  }

  template <class... Args>
  static bool emplace(T& set, Args&&... args) {
    return set.emplace(std::forward<Args>(args)...).second;
  }

  static void clear(T& set) {
    set.clear();
  }

  static void reserve(std::set<inner_type>& /* set */, size_t /* len */) {}

  static void reserve(std::unordered_set<inner_type>& set, size_t len) {
    set.reserve(len);
  }

  static void reserve(folly::F14FastSet<inner_type>& set, size_t len) {
    set.reserve(len);
  }

  static void reserve(folly::F14NodeSet<inner_type>& set, size_t len) {
    set.reserve(len);
  }

  static void reserve(folly::F14ValueSet<inner_type>& set, size_t len) {
    set.reserve(len);
  }

  static void reserve(folly::F14VectorSet<inner_type>& set, size_t len) {
    set.reserve(len);
  }

  static auto begin(const T& set) -> decltype(set.begin()) {
    return set.begin();
  }

  static auto end(const T& set) -> decltype(set.end()) {
    return set.end();
  }
};

template <class T>
struct SerializationTraits<
    T,
    typename std::enable_if<folly::IsOneOf<
        T,
        std::map<typename T::key_type, typename T::mapped_type>,
        std::unordered_map<typename T::key_type, typename T::mapped_type>,
        folly::F14FastMap<typename T::key_type, typename T::mapped_type>,
        folly::F14NodeMap<typename T::key_type, typename T::mapped_type>,
        folly::F14ValueMap<typename T::key_type, typename T::mapped_type>,
        folly::F14VectorMap<typename T::key_type, typename T::mapped_type>>::
                                value>::type> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::Map;

  using key_type = typename T::key_type;
  using mapped_type = typename T::mapped_type;

  static size_t size(const T& map) {
    return map.size();
  }

  static void clear(T& map) {
    map.clear();
  }

  static void reserve(
      std::map<key_type, mapped_type>& /* map */,
      size_t /* len */) {}

  static void reserve(
      std::unordered_map<key_type, mapped_type>& map,
      size_t len) {
    map.reserve(len);
  }

  static void reserve(
      folly::F14FastMap<key_type, mapped_type>& map,
      size_t len) {
    map.reserve(len);
  }

  static void reserve(
      folly::F14NodeMap<key_type, mapped_type>& map,
      size_t len) {
    map.reserve(len);
  }

  static void reserve(
      folly::F14ValueMap<key_type, mapped_type>& map,
      size_t len) {
    map.reserve(len);
  }

  static void reserve(
      folly::F14VectorMap<key_type, mapped_type>& map,
      size_t len) {
    map.reserve(len);
  }

  template <class... Args>
  static auto emplace(T& map, Args&&... args)
      -> decltype(map.emplace(std::forward<Args>(args)...)) {
    return map.emplace(std::forward<Args>(args)...);
  }

  static auto begin(const T& map) -> decltype(map.begin()) {
    return map.begin();
  }

  static auto end(const T& map) -> decltype(map.end()) {
    return map.end();
  }
};

} // namespace carbon
