/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_LIB_CPP_UTIL_FROZENUTIL_H_
#define THRIFT_LIB_CPP_UTIL_FROZENUTIL_H_

#include <folly/File.h>
#include <folly/system/MemoryMapping.h>
#include <thrift/lib/cpp/Frozen.h>

namespace apache {
namespace thrift {
namespace util {

template <class T, class Frozen = typename Freezer<T>::FrozenType>
const Frozen* freezeToFile(const T& value, const folly::MemoryMapping& mapping);

template <class T>
void freezeToFile(const T& value, folly::File file);

template <class T>
void freezeToFile(const T& value, int fd);

template <class T>
void freezeToSparseFile(
    const T& value, folly::File file, size_t sparseSize = 1L << 30 /* 1 GB */);

template <class T, class Frozen = typename Freezer<T>::FrozenType>
const Frozen* mapFrozen(folly::ByteRange mapping);

template <class T, class Frozen = typename Freezer<T>::FrozenType>
const Frozen* mapFrozen(const folly::MemoryMapping& mapping);

} // namespace util
} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp/util/FrozenUtil-inl.h>

#endif // include guard
