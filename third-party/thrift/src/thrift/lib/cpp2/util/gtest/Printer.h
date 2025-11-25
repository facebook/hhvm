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

#include <ostream>

#include <gtest/gtest.h>

// portability/GTest must be imported before any other gtest header
#include <gtest/gtest-spi.h>

#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

namespace apache::thrift {

template <typename T>
void PrintTo(field_ref<T> obj, std::ostream* os) {
  *os << "field_ref holding " << testing::PrintToString(*obj);
}

template <typename T>
void PrintTo(optional_field_ref<T> obj, std::ostream* os) {
  if (!obj) {
    *os << "empty optional_field_ref";
    return;
  }
  *os << "optional_field_ref holding " << testing::PrintToString(*obj);
}

template <typename T>
  requires(
      is_thrift_class_v<std::remove_cvref_t<T>> ||
      is_thrift_exception_v<std::remove_cvref_t<T>>)
void PrintTo(const T& obj, std::ostream* os) {
  *os << debugStringViaEncode(obj);
}

} // namespace apache::thrift
