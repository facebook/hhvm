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

#include <type_traits>

#include <Python.h>
#include <folly/Expected.h>
#include <folly/Preprocessor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/python/capi/types.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

// Using an ExtractorResult lets us accommodate two different error handling
// strategies:
//   1. If an error has occurred, avoid making further capi calls
//      and throw C++ exception at C++ entrypoint
//   2. If an error has occurred, avoid making further capi calls and let
//      cython raise python error at cython-binding return to python.
//   The unexpected std::string_view arms lets us capture the C++ file and line
//   to augment the python error raised by capi.
template <typename T>
using ExtractorResult = folly::Expected<native_t<T>, std::string_view>;

template <typename T>
ExtractorResult<T> extractorError(std::string_view msg) {
  return folly::makeUnexpected(msg);
}

#define EXTRACTOR_ERROR(TYPE, MESSAGE) \
  extractorError<TYPE>(CAPI_LOCATED_ERROR(MESSAGE))

template <typename T>
ExtractorResult<T> extractorSuccess(T&& val) {
  static_assert(std::is_same_v<T, native_t<T>>);
  return folly::makeExpected<std::string_view>(std::forward<T>(val));
}

template <typename T>
struct Extractor;

/*
 * Base class provides a conditional extract(PyObject*, bool&). The bool
 * represents whether any python error has occurred. The base extractor
 * calls the derived Extractor operator (PyObject*) if no error has
 * occurred; otherwise it returns Unexpected variant to avoid cpython api
 * calls that are undefined behavior.
 */
template <typename T>
struct BaseExtractor {
  ExtractorResult<T> extract(PyObject* obj, bool& extractError) {
    if (extractError) {
      return EXTRACTOR_ERROR(T, "extractError already set");
    }
    auto res = (*static_cast<Extractor<T>*>(this))(obj);
    extractError = res.hasError();
    return res;
  }

  template <typename FieldRef>
  std::enable_if_t<
      !::apache::thrift::detail::is_shared_or_unique_ptr_v<FieldRef>>
  extractInto(
      FieldRef ref, PyObject* obj, std::optional<std::string_view>& error) {
    if (error) {
      return;
    }
    if constexpr (is_optional_maybe_boxed_field_ref_v<FieldRef>) {
      if (obj == Py_None) {
        return;
      }
    }
    auto extractResult = (*static_cast<Extractor<T>*>(this))(obj);
    if (extractResult.hasError()) {
      error = extractResult.error();
    } else {
      ref = std::move(*extractResult);
    }
  }
  template <typename S>
  void extractInto(
      std::unique_ptr<S>& ref, /* RefType.Unique */
      PyObject* obj,
      std::optional<std::string_view>& error) {
    // Ref may be optional so always check if None
    if (error || obj == Py_None) {
      return;
    }
    auto extractResult = (*static_cast<Extractor<T>*>(this))(obj);
    if (extractResult.hasError()) {
      error = extractResult.error();
    } else {
      ref = std::make_unique<S>(std::move(*extractResult));
    }
  }
  template <typename S>
  void extractInto(
      std::shared_ptr<S>& ref, /* RefType.Shared, RefType.SharedMutable */
      PyObject* obj,
      std::optional<std::string_view>& error) {
    // Ref may be optional so always check if None
    if (error || obj == Py_None) {
      return;
    }
    auto extractResult = (*static_cast<Extractor<T>*>(this))(obj);
    if (extractResult.hasError()) {
      error = extractResult.error();
    } else {
      ref = std::make_shared<S>(std::move(*extractResult));
    }
  }
};

/**
 * Series of functions to convert a Python object into a native type T.
 *
 * Some important notes:
 * 1. The Extractor borrows a reference to `obj`: It's intended to be used as a
 *    temporary.
 * 2: The extractor will leave the python indicator set if a python
 *    exception is encountered during the extraction. Be sure to check the error
 *    indicator before calling back into python!
 */
template <typename T>
struct Extractor : public BaseExtractor<T> {
  template <typename U>
  struct Unspecialized : std::false_type {};
  static_assert(Unspecialized<T>{}, "No extractor defined");
  ExtractorResult<T> operator()(/* Source */ PyObject* obj);
  int typeCheck(PyObject* obj); /* returns 0/1 bool or -1 on error */
};

// Concrete type specializations
#define SPECIALIZE_SCALAR(type)                         \
  template <>                                           \
  struct Extractor<type> : public BaseExtractor<type> { \
    ExtractorResult<type> operator()(PyObject* obj);    \
    int typeCheck(PyObject* obj);                       \
  }

SPECIALIZE_SCALAR(signed char);
SPECIALIZE_SCALAR(signed short);
SPECIALIZE_SCALAR(signed int);
SPECIALIZE_SCALAR(signed long);
SPECIALIZE_SCALAR(signed long long);
SPECIALIZE_SCALAR(unsigned char);
SPECIALIZE_SCALAR(unsigned short);
SPECIALIZE_SCALAR(unsigned int);
SPECIALIZE_SCALAR(unsigned long);
SPECIALIZE_SCALAR(unsigned long long);
SPECIALIZE_SCALAR(bool);
SPECIALIZE_SCALAR(float);
SPECIALIZE_SCALAR(double);
SPECIALIZE_SCALAR(Bytes);
SPECIALIZE_SCALAR(String);
SPECIALIZE_SCALAR(BytesView);
SPECIALIZE_SCALAR(StringView);
SPECIALIZE_SCALAR(std::unique_ptr<folly::IOBuf>);
SPECIALIZE_SCALAR(folly::IOBuf);

#undef SPECIALIZE_SCALAR

template <typename Adapter, typename ThriftT, typename CppT>
struct Extractor<AdaptedThrift<Adapter, ThriftT, CppT>>
    : public BaseExtractor<AdaptedThrift<Adapter, ThriftT, CppT>> {
  ExtractorResult<CppT> operator()(PyObject* obj) {
    auto result = Extractor<ThriftT>{}(obj);
    if (result.hasError()) {
      return extractorError<CppT>(result.error());
    }
    return Adapter::fromThrift(std::move(*result));
  }
};

template <typename T>
struct Extractor<ComposedEnum<T>> : public BaseExtractor<ComposedEnum<T>> {
  ExtractorResult<T> operator()(PyObject* obj) {
    auto fbthrift_value = Extractor<int32_t>{}(obj);
    if (fbthrift_value.hasError()) {
      return extractorError<T>(fbthrift_value.error());
    }
    return static_cast<T>(*fbthrift_value);
  }
  int typeCheck(PyObject* obj) { return Extractor<int32_t>::typeCheck(obj); }
};

template <typename C>
using reserve_method_t =
    decltype(std::declval<C&>().reserve(std::declval<size_t>()));
template <typename C>
constexpr bool has_reserve_v =
    folly::is_detected_v<reserve_method_t, std::remove_reference_t<C>>;

template <typename ElemT, typename CppT>
struct Extractor<list<ElemT, CppT>> : public BaseExtractor<list<ElemT, CppT>> {
  using list_t = list<ElemT, CppT>;
  ExtractorResult<list_t> operator()(PyObject* obj) {
    if constexpr (folly::kIsDebug) {
      if (PyTuple_Check(obj) != 1) {
        PyErr_SetString(PyExc_TypeError, CAPI_LOCATED_ERROR("Not a tuple"));
        return EXTRACTOR_ERROR(list_t, "Not a tuple");
      }
    }
    CppT ret;
    Py_ssize_t size = PyTuple_GET_SIZE(obj);
    if (size == 0) {
      return ret;
    }

    if constexpr (has_reserve_v<CppT>) {
      ret.reserve(size);
    }
    Extractor<ElemT> extractor;
    for (Py_ssize_t i = 0; i < size; ++i) {
      // Note PyTuple_GET_ITEM returns a borrowed reference
      auto extracted = extractor(PyTuple_GET_ITEM(obj, i));
      if (!extracted.hasValue()) {
        // Extraction failed. Abort
        return extractorError<list_t>(extracted.error());
      }
      ret.push_back(std::move(*extracted));
    }

    return ret;
  }

  int typeCheck(PyObject* obj) {
    if (PyTuple_Check(obj) != 1) {
      return 0;
    }
    const Py_ssize_t size = PyTuple_GET_SIZE(obj);
    // Just check the first element given type checking inherent in
    // thrift-python struct construction; thrift consts
    if (size == 0) {
      return 1;
    }
    PyObject* front_obj = PyTuple_GET_ITEM(obj, 0);
    if (!front_obj) {
      return 0;
    }
    return Extractor<ElemT>{}.typeCheck(front_obj);
  }
};

template <typename ElemT, typename CppT>
struct Extractor<set<ElemT, CppT>> : public BaseExtractor<set<ElemT, CppT>> {
  using set_t = set<ElemT, CppT>;
  ExtractorResult<set_t> operator()(PyObject* obj) {
    StrongRef iter(PyObject_GetIter(obj));
    if (!iter) {
      return EXTRACTOR_ERROR(set_t, "GetIter failed");
    }
    CppT ret;
    if constexpr (has_reserve_v<CppT>) {
      Py_ssize_t len = PySet_Size(obj);
      if (len == -1) {
        return EXTRACTOR_ERROR(set_t, "len() failed");
      } else if (len == 0) {
        return ret;
      }
      ret.reserve(static_cast<size_t>(len));
    }
    Extractor<ElemT> extractor;
    while (auto item = StrongRef(PyIter_Next(*iter))) {
      auto extracted = extractor(*item);
      if (!extracted.hasValue()) {
        return EXTRACTOR_ERROR(set_t, "extractor error while iterating");
      }
      ret.insert(std::move(*extracted));
    }
    return ret;
  }
  int typeCheck(PyObject* obj) {
    if (PySet_Check(obj) != 1) {
      return 0;
    }
    const Py_ssize_t size = PySet_Size(obj);
    // Just check the first element given type checking inherent in
    // thrift-python struct construction; thrift consts
    if (size == 0) {
      return 1;
    }
    StrongRef iter(PyObject_GetIter(obj));
    if (!iter) {
      PyErr_Clear();
      return 0;
    }
    StrongRef front_obj(PyIter_Next(*iter));
    if (!front_obj) {
      PyErr_Clear();
      return 0;
    }
    return Extractor<ElemT>{}.typeCheck(*front_obj);
  }
};

template <typename M, typename K, typename V>
using emplace_method_t =
    decltype(std::declval<M&>().emplace(std::declval<K>(), std::declval<V>()));
template <typename C, typename K, typename V>
constexpr bool has_emplace_v =
    folly::is_detected_v<emplace_method_t, std::remove_reference_t<C>, K, V>;

template <typename KeyT, typename ValT, typename CppT>
struct Extractor<map<KeyT, ValT, CppT>>
    : public BaseExtractor<map<KeyT, ValT, CppT>> {
  using map_t = map<KeyT, ValT, CppT>;
  ExtractorResult<map_t> operator()(PyObject* obj) {
    if constexpr (folly::kIsDebug) {
      if (PyTuple_Check(obj) != 1) {
        PyErr_SetString(PyExc_TypeError, CAPI_LOCATED_ERROR("Not a tuple"));
        return EXTRACTOR_ERROR(map_t, "Not a tuple");
      }
    }
    CppT ret;
    Py_ssize_t size = PyTuple_GET_SIZE(obj);
    if (size == 0) {
      return ret;
    }

    if constexpr (has_reserve_v<CppT>) {
      ret.reserve(size);
    }
    Extractor<KeyT> key_extractor;
    Extractor<ValT> val_extractor;
    for (Py_ssize_t i = 0; i < size; ++i) {
      // Note PyTuple_GET_ITEM returns a borrowed reference
      auto kv_tuple = PyTuple_GET_ITEM(obj, i);
      auto key = key_extractor(PyTuple_GET_ITEM(kv_tuple, 0));
      if (!key.hasValue()) {
        // Extraction failed. Abort
        return extractorError<map_t>(key.error());
      }
      auto val = val_extractor(PyTuple_GET_ITEM(kv_tuple, 1));
      if (!val.hasValue()) {
        // Extraction failed. Abort
        return extractorError<map_t>(val.error());
      }
      if constexpr (has_emplace_v<CppT, KeyT, ValT>) {
        ret.emplace(std::move(*key), std::move(*val));
      } else {
        ret.try_emplace(std::move(*key), std::move(*val));
      }
    }

    return ret;
  }

  int typeCheck(PyObject* obj) {
    if (PyTuple_Check(obj) != 1) {
      return 0;
    }
    const Py_ssize_t size = PyTuple_GET_SIZE(obj);
    // Just check the first element given type checking inherent in
    // thrift-python struct construction; thrift consts
    if (size == 0) {
      return 1;
    }
    PyObject* front_kv = PyTuple_GET_ITEM(obj, 0);
    if (!front_kv) {
      return 0;
    }
    return Extractor<KeyT>{}.typeCheck(PyTuple_GET_ITEM(front_kv, 0)) &&
        Extractor<ValT>{}.typeCheck(PyTuple_GET_ITEM(front_kv, 1));
  }
};

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
