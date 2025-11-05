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

#include <string_view>
#include <vector>

namespace apache::thrift::metadata {
class ThriftConstStruct;
namespace detail {

class OnlyDefinedInCython;

#ifdef CYTHON_HEX_VERSION
// for thrift-py3 compatibility.
class OnlyDefinedInCython {};
#endif

template <bool b = true>
void ensureInsideCython() {
  // The next line won't compile outside Cython. Note we can't check
  // CYTHON_HEX_VERSION directly here since that violates ODR.
  std::enable_if_t<b, OnlyDefinedInCython>{};
}

template <class T>
class LimitedVector : public std::vector<T> {
 public:
  decltype(auto) at(std::size_t idx) const {
    ensureInsideCython();
    return std::vector<T>::at(idx);
  }

  decltype(auto) front() const {
    ensureInsideCython();
    return std::vector<T>::front();
  }
};

} // namespace detail

const ThriftConstStruct* findStructuredAnnotation(
    const detail::LimitedVector<ThriftConstStruct>& annotations,
    std::string_view name);
const ThriftConstStruct& findStructuredAnnotationOrThrow(
    const detail::LimitedVector<ThriftConstStruct>& annotations,
    std::string_view name);
} // namespace apache::thrift::metadata
