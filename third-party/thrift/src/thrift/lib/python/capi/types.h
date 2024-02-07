/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <Python.h>
#include <folly/Preprocessor.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

#define __CAPI_LOCATED_ERROR_IMPL(MESSAGE, LINE) \
  __FILE__ ":" FOLLY_PP_STRINGIZE(LINE) ": " MESSAGE
#define CAPI_LOCATED_ERROR(MESSAGE) __CAPI_LOCATED_ERROR_IMPL(MESSAGE, __LINE__)

template <typename R, typename RValue = std::remove_reference_t<R>>
constexpr bool is_optional_maybe_boxed_field_ref_v =
    apache::thrift::detail::is_optional_field_ref_v<RValue> ||
    apache::thrift::detail::is_optional_boxed_field_ref_v<RValue>;

template <typename Field>
struct is_union_field_ref : std::false_type {};
template <typename FieldT>
struct is_union_field_ref<::apache::thrift::union_field_ref<FieldT>>
    : std::true_type {};

/*
 * RAII wrapper around PyObject* representing a strong reference, i.e.,
 * a PyObject* returned by a C api function that returns a *new* reference.
 * The StrongRef will decrement when dropped, so it can safely encapsulate a
 * a temporarily used object or one that may be dropped in an error case.
 */
struct StrongRef {
  StrongRef() : obj_(nullptr) {}
  StrongRef(StrongRef&& other) : obj_(nullptr) { std::swap(obj_, other.obj_); }
  StrongRef& operator=(StrongRef&& other);

  PyObject* obj_;
  // Constructor "steals" a reference so that object, so it should onl be
  // constructed from "new" reference function returns
  explicit StrongRef(PyObject* o) : obj_(o) {}
  // When StrongRef goes out of scope, the contained PyObject* will have
  // a refcount one less than when it was constructed, unless it is released.
  ~StrongRef() { Py_XDECREF(obj_); }
  // Return a borrowed reference to the contained object.
  PyObject* operator*() const { return obj_; }
  PyObject* operator->() const { return obj_; }
  operator bool() const { return obj_ != nullptr; }
  // Release ownership to function that "steals" reference
  PyObject* release() &&;
  bool isNone() const;
};

template <typename T>
struct native {
  using type = T;
};

template <typename T>
using native_t = typename native<T>::type;

/**
 * There are two separate text data types in Python (bytes and str) that are
 * backed by the same type in C++ (std::string).
 * We use two 'template' type aliases for std::string to distinguish between
 * the two target python types, and same for std::string_view.
 */
struct Bytes {};
struct String {};
struct BytesView {};
struct StringView {};

template <>
struct native<Bytes> {
  using type = std::string;
};

template <>
struct native<String> {
  using type = std::string;
};

template <>
struct native<BytesView> {
  using type = std::string_view;
};

template <>
struct native<StringView> {
  using type = std::string_view;
};

/**
 * Used to denote Extractors that need to apply
 * Adapter::fromThrift to Extractor result to obtain cpp type,
 * and Constructors that need to apply Adapter::toThrift before
 * constructing from cpp type.
 */
template <
    typename Adapter,
    typename ThriftT,
    typename CppT = std::remove_const_t<std::remove_reference_t<
        decltype(Adapter::fromThrift(std::declval<native_t<ThriftT>>()))>>>
struct AdaptedThrift;

template <typename Adapter, typename ThriftT, typename CppT>
struct native<AdaptedThrift<Adapter, ThriftT, CppT>> {
  using type = CppT;
};

template <typename Adapter, typename AdaptedType>
using CircularlyAdaptedThrift = AdaptedThrift<
    Adapter,
    native_t<std::remove_const_t<std::remove_reference_t<
        decltype(Adapter::toThrift(std::declval<AdaptedType>()))>>>,
    AdaptedType>;

template <typename T>
struct ComposedEnum {};

template <typename T>
struct native<ComposedEnum<T>> {
  using type = T;
};

template <typename T>
struct ComposedStruct {};

template <typename T>
struct native<ComposedStruct<T>> {
  using type = T;
};

// T is the element type, CppT is the full type
template <typename T, typename CppT = std::vector<native_t<T>>>
struct list {};

// T is the element type, CppT is the full type
template <typename T, typename CppT = std::set<native_t<T>>>
struct set {};

template <
    typename K,
    typename V,
    typename CppT = std::map<native_t<K>, native_t<V>>>
struct map {};

template <typename T, typename CppT>
struct native<list<T, CppT>> {
  using type = CppT;
};

template <typename T, typename CppT>
struct native<set<T, CppT>> {
  using type = CppT;
};

template <typename K, typename V, typename CppT>
struct native<map<K, V, CppT>> {
  using type = CppT;
};

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
