/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/thirdparty/jansson/jansson.h"

/**

`watchman::serde` provides convenient mapping between JSON or BSER serialization
and standard and custom C++ data types.

Brief API overview:

`serde::encode(value)` converts `value` into its JSON representation.

`serde::decode<T>(json)` decodes a JSON value into T, or throws an exception
if not possible.

To define how a struct is mapped, derive from `serde::Object` and implement a
`map` template function. See the `serde::Object` documentation.

To define how a fixed-width tuple is mapped, derive from
`serde::Array<Required, T...>`, where `Required` is the minimum number of array
elements required.


# Protocol Evolution

This API allows protocol evolution. New fields can be added to structs or
tuples, and unrecognized fields are never an error.

*/
namespace watchman::serde {

// TODO: introduce a proper exception hierarchy
using MissingKey = std::domain_error;

namespace detail {

class FieldEncoder {
 public:
  explicit FieldEncoder(std::unordered_map<w_string, json_ref>& map)
      : map_{map} {}

  template <size_t N, typename T>
  void operator()(const char (&name)[N], const T& field);

  template <size_t N, typename T>
  void required(const char (&name)[N], const T& field);

  template <size_t N, typename T, typename P>
  void skip_if(const char (&name)[N], const T& field, P&& predicate);

  template <size_t N, typename T>
  void skip_if_default(const char (&name)[N], const T& field);

 private:
  std::unordered_map<w_string, json_ref>& map_;
};

class FieldDecoder {
 public:
  explicit FieldDecoder(const std::unordered_map<w_string, json_ref>& map)
      : map_{map} {}

  template <size_t N, typename T>
  void operator()(const char (&name)[N], T& field);

  template <size_t N, typename T>
  void required(const char (&name)[N], T& field);

  template <size_t N, typename T, typename P>
  void skip_if(const char (&name)[N], T& field, P&& predicate);

  template <size_t N, typename T>
  void skip_if_default(const char (&name)[N], T& field);

 private:
  const std::unordered_map<w_string, json_ref>& map_;
};

} // namespace detail

/**
 * Derive from Object to indicate that a struct has a mapping to and from a JSON
 * or BSER object.
 *
 * Implement a `map` member function that provides the name of each field.
 *
 * template <typename X>
 * void map(X& x) {
 *   x("field", field);
 * }
 *
 * X is an opaque type that defines the mapping between JSON object elements and
 * struct members.
 *
 * Calling `x` defines a field mapping with the default rules:
 * When deserializing, if present, it must have the correct type.
 * If not present, the field is default-initialized.
 * The field is always serialized on output.
 *
 * Calling `x.required` fails deserialization if the key is not present.
 *
 * Calling `x.skip_if_default` registers a mapping with default rules except
 * that, if the value compares equal to the default value for that type, it will
 * not be serialized.
 *
 * `x.skip_if` is the same as `x.skip_if_default` except that it takes an
 * arbitrary predicate.
 *
 * Do not conditionally change the field mappings at runtime.
 */
struct Object {};

/**
 * Derive from Array to indicate conversion to and from roughly fixed-sized
 * arrays. Encoding produces an array of length sizeof...(E) and decoding will
 * succeed as long as the array has 'Required' values or more.
 */
template <size_t Required, typename... E>
struct Array : std::tuple<E...> {};

/**
 * Specializations provide two static members: toJson and fromJson.
 *
 * `static json_ref toJson(T value)`
 * returns a JSON encoding of the given value.
 *
 * `static T fromJson(const json_ref& v)`
 * returns a decoded value, or throws an exception if `v` has the wrong type.
 */
template <typename T>
struct Serde {
  static_assert(
      std::is_base_of_v<Object, T>,
      "T must either derive Object or provide a Serde specialization");

  static json_ref toJson(const T& v) {
    std::unordered_map<w_string, json_ref> o;
    detail::FieldEncoder encoder{o};
    // The const_cast is gross, but allowing `map` to run in both read and write
    // contexts would otherwise require an additional template parameter.
    // FieldEncoder::operator() takes a const field, so there isn't much real
    // risk here.
    const_cast<T&>(v).map(encoder);
    return json_object(std::move(o));
  }

  static T fromJson(const json_ref& v) {
    // TODO: throw a nicer error when v is not an object
    auto& o = v.object();
    if (JSON_OBJECT != v.type()) {
      // throw a nice error
    }

    detail::FieldDecoder decoder{o};
    T rv;
    rv.map(decoder);
    return rv;
  }
};

// Primitive Serde Instances
// TODO: flesh this out

template <>
struct Serde<json_ref> {
  static json_ref toJson(json_ref v) {
    return v;
  }
  static json_ref fromJson(json_ref v) {
    return v;
  }
};

template <>
struct Serde<bool> {
  static json_ref toJson(bool v) {
    return json_boolean(v);
  }

  static bool fromJson(const json_ref& v) {
    // TODO: error checking
    return v.asBool();
  }
};

template <>
struct Serde<json_int_t> {
  static json_ref toJson(json_int_t i) {
    return json_integer(i);
  }

  static json_int_t fromJson(const json_ref& v) {
    // TODO: error checking
    return v.asInt();
  }
};

template <>
struct Serde<w_string> {
  static json_ref toJson(const w_string& v) {
    return w_string_to_json(v);
  }

  static w_string fromJson(const json_ref& v) {
    // TODO: error checking
    return v.asString();
  }
};

template <>
struct Serde<std::string> {
  static json_ref toJson(const std::string& v) {
    return typed_string_to_json(v);
  }

  static std::string fromJson(const json_ref& v) {
    return v.asString().string();
  }
};

/**
 * Convert any supported C++ value to its JSON representation.
 */
template <typename T>
json_ref encode(T&& v) {
  using Base = std::remove_const_t<std::remove_reference_t<T>>;
  if constexpr (std::is_integral_v<Base> && !std::is_same_v<Base, bool>) {
    // TODO: json_int_t is a signed 64-bit integer. It would be nice to
    // bounds-check here. It should only be necessary if Base is uint64_t and
    // the high bit is set.
    return Serde<json_int_t>::toJson(v);
  } else {
    return Serde<Base>::toJson(std::forward<T>(v));
  }
}

/**
 * Attempt to decode a C++ value from a JSON value. Throws if decoding fails.
 */
template <typename T>
T decode(const json_ref& j) {
  using Base = std::remove_const_t<std::remove_reference_t<T>>;
  if constexpr (std::is_integral_v<Base> && !std::is_same_v<Base, bool>) {
    // TODO: bounds-checking
    return Serde<json_int_t>::fromJson(j);
  } else {
    return Serde<Base>::fromJson(j);
  }
}

// Compound Serde Instances
// TODO: add new specializations as necessary

namespace detail {

template <size_t I, size_t Required, typename... E>
struct PushTuple {
  static size_t push(
      std::vector<json_ref>& array,
      const std::tuple<E...>& tuple,
      size_t lastNonDefault) {
    if constexpr (I == sizeof...(E)) {
      return lastNonDefault;
    } else {
      auto& element = std::get<I>(tuple);
      array.push_back(encode(element));

      return PushTuple<I + 1, Required, E...>::push(
          array,
          tuple,
          (I >= Required && decltype(element){} == element) ? lastNonDefault
                                                            : (I + 1));
    }
  }
};

template <size_t I, typename... E>
struct WriteTuple {
  static void write(
      std::tuple<E...>& tuple,
      const std::vector<json_ref>& array) {
    if constexpr (I == sizeof...(E)) {
      return;
    } else if (I >= array.size()) {
      return;
    } else {
      auto& element = std::get<I>(tuple);
      element = decode<std::remove_reference_t<decltype(element)>>(array[I]);
      WriteTuple<I + 1, E...>::write(tuple, array);
    }
  }
};

} // namespace detail

template <size_t Required, typename... E>
struct Serde<Array<Required, E...>> {
  static_assert(Required <= sizeof...(E));
  using Tuple = Array<Required, E...>;

  static json_ref toJson(const Tuple& tuple) {
    std::vector<json_ref> array;

    // TODO: We could scan for non-default values and compute a precise size to
    // reserve. But the worst-case allocation should be fine. After all,
    // json_ref is only pointer-sized, and optional tuple elements are small and
    // rare.
    array.reserve(sizeof...(E));
    auto size = detail::PushTuple<0, Required, E...>::push(array, tuple, 0);
    array.erase(array.begin() + size, array.end());
    return json_array(std::move(array));
  }

  static Tuple fromJson(const json_ref& j) {
    Tuple tuple{};

    if (!j.isArray()) {
      // TODO: throw a nice error
    }
    auto& array = j.array();
    if (array.size() < Required) {
      // TODO: make a better exception type
      throw std::domain_error("array must have at least N elements");
    }

    detail::WriteTuple<0, E...>::write(tuple, array);

    return tuple;
  }
};

template <typename T>
struct Serde<std::optional<T>> {
  static json_ref toJson(const std::optional<T>& o) {
    return o ? Serde<T>::toJson(*o) : json_null();
  }

  static std::optional<T> fromJson(const json_ref& j) {
    if (j.isNull()) {
      return std::nullopt;
    } else {
      return Serde<T>::fromJson(j);
    }
  }
};

template <typename T>
struct Serde<std::vector<T>> {
  static json_ref toJson(const std::vector<T>& v) {
    std::vector<json_ref> arr;
    arr.reserve(v.size());
    for (const auto& element : v) {
      arr.push_back(encode(element));
    }
    return json_array(std::move(arr));
  }

  static std::vector<T> fromJson(const json_ref& j) {
    auto& array = j.array();

    std::vector<T> result;
    result.reserve(array.size());
    for (auto& element : array) {
      result.push_back(decode<T>(element));
    }
    return result;
  }
};

template <typename V>
struct Serde<std::map<w_string, V>> {
  static json_ref toJson(const std::map<w_string, V>& m) {
    std::unordered_map<w_string, json_ref> o;
    o.reserve(m.size());
    for (auto& [name, value] : m) {
      o.insert_or_assign(name, encode(value));
    }
    return json_object(std::move(o));
  }

  static std::map<w_string, V> fromJson(const json_ref& j) {
    auto& hashmap = j.object();

    std::map<w_string, V> result;
    for (auto& [key, value] : hashmap) {
      result.emplace(key, decode<V>(value));
    }
    return result;
  }
};

/**
 * Type that serializes to null and deserialized from anything.
 */
struct Nothing {};

template <>
struct Serde<Nothing> {
  static json_ref toJson(Nothing) {
    return json_null();
  }

  static Nothing fromJson(const json_ref&) {
    return {};
  }
};

namespace detail {

template <size_t N, typename T>
void FieldEncoder::operator()(const char (&name)[N], const T& field) {
  // TODO: per-field allocation, yikes
  map_.insert_or_assign(w_string(name, W_STRING_UNICODE), serde::encode(field));
}

template <size_t N, typename T>
void FieldEncoder::required(const char (&name)[N], const T& field) {
  // TODO: per-field allocation, yikes
  map_.insert_or_assign(w_string(name, W_STRING_UNICODE), serde::encode(field));
}

template <size_t N, typename T, typename P>
void FieldEncoder::skip_if(
    const char (&name)[N],
    const T& field,
    P&& predicate) {
  // To prevent callers from accidentally mutating the field or capturing other
  // fields, ensure the predicate has a non-capturing signature.
  bool (*p)(const T&) = predicate;

  if (p(field)) {
    return;
  }
  (*this)(name, field);
}

template <size_t N, typename T>
void FieldEncoder::skip_if_default(const char (&name)[N], const T& field) {
  if (T{} == field) {
    return;
  }
  (*this)(name, field);
}

template <size_t N, typename T>
void FieldDecoder::operator()(const char (&name)[N], T& field) {
  // TODO: per-field allocation, yikes
  auto iter = map_.find(w_string{name});
  if (iter == map_.end()) {
    field = T{};
  } else {
    field = serde::decode<T>(iter->second);
  }
}

template <size_t N, typename T>
void FieldDecoder::required(const char (&name)[N], T& field) {
  // TODO: per-field allocation, yikes
  auto iter = map_.find(w_string{name});
  if (iter == map_.end()) {
    throw MissingKey{"key is missing"};
  } else {
    field = serde::decode<T>(iter->second);
  }
}

template <size_t N, typename T, typename P>
void FieldDecoder::skip_if(const char (&name)[N], T& field, P&&) {
  (*this)(name, field);
}

template <size_t N, typename T>
void FieldDecoder::skip_if_default(const char (&name)[N], T& field) {
  (*this)(name, field);
}

} // namespace detail

} // namespace watchman::serde
