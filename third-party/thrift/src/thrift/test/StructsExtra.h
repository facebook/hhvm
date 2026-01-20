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

#include <folly/small_vector.h>
#include <folly/sorted_vector_types.h>

#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift::test {

template <class T>
struct WrappedType {
  T raw;

  T& rawAccessor() { return raw; }

  const T& rawAccessor() const { return raw; }
};

template <class T>
struct WrappedTypeField : WrappedType<T> {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(raw);
};

template <class T>
struct WrappedTypeMethod : WrappedType<T> {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(rawAccessor());
};

} // namespace apache::thrift::test
