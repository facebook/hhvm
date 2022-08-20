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
#include <utility>

#include <thrift/lib/cpp/Field.h>

namespace apache::thrift::test {

struct TestFieldInterceptor {
  inline static int count = 0;

  template <typename T, typename Struct, int16_t FieldId>
  static void interceptThriftFieldAccess(
      T&&, apache::thrift::FieldContext<Struct, FieldId>&&) {
    count++;
  }
};

} // namespace apache::thrift::test
