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

#pragma once

#include <cstdint>

#include <boost/operators.hpp>

namespace apache {
namespace thrift {
namespace test {

struct CppHasANumber : private boost::totally_ordered<CppHasANumber> {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(number);

  std::int32_t number{};
  CppHasANumber() {}
  /* implicit */ CppHasANumber(std::int32_t number_) : number(number_) {}
  bool operator==(CppHasANumber that) const { return number == that.number; }
  bool operator<(CppHasANumber that) const { return number < that.number; }
};
} // namespace test
} // namespace thrift
} // namespace apache
