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

#include <string>
#include <string_view>
#include <vector>
#include <folly/Preprocessor.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

#define __CAPI_LOCATED_ERROR_IMPL(MESSAGE, LINE) \
  __FILE__ ":" FOLLY_PP_STRINGIZE(LINE) ": " MESSAGE
#define CAPI_LOCATED_ERROR(MESSAGE) __CAPI_LOCATED_ERROR_IMPL(MESSAGE, __LINE__)

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

// T is the element type, CppT is the full type
template <typename T, typename CppT = std::vector<native_t<T>>>
struct list {};

template <typename T, typename CppT>
struct native<list<T, CppT>> {
  using type = CppT;
};

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
