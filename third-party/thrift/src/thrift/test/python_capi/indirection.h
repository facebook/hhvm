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

#include <thrift/lib/cpp2/FieldRef.h>

#pragma once

namespace thrift::test::python_capi {

// forward declaration of thrift struct
class TWrapped;

template <class T = TWrapped>
class CppWrapperImpl {
 public:
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(inner());
  CppWrapperImpl() = default;

  const T& inner() const { return inner_; }

  T& inner() { return inner_; }

 private:
  T inner_;
};

using CppWrapperT = CppWrapperImpl<>;

} // namespace thrift::test::python_capi
