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

#include <limits>

#include <thrift/test/python_capi/gen-cpp2/thrift_dep_types.h>

namespace thrift::test::lib {
python_capi::DepEnum enumSource() noexcept {
  return python_capi::DepEnum::Arm2;
}

python_capi::DepStruct structSource() noexcept {
  python_capi::DepStruct s;
  s.s() = "Hello";
  s.i() = 42;
  return s;
}

python_capi::DepUnion unionSource() noexcept {
  python_capi::DepUnion u;
  u.s_ref() = "World";
  return u;
}

int unpackEnum(python_capi::DepEnum e) noexcept {
  return static_cast<int>(e);
}

std::string unpackStruct(python_capi::DepStruct&& s) noexcept {
  std::string ret = *s.s();
  return ret + ": " + std::to_string(*s.i());
}

std::string unpackUnion(python_capi::DepUnion&& u) noexcept {
  if (u.s_ref().has_value()) {
    return *u.s_ref();
  } else if (u.i_ref().has_value()) {
    return std::to_string(*u.i_ref());
  }
  return "";
}

} // namespace thrift::test::lib
