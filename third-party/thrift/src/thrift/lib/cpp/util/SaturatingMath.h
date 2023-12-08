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

#include <cmath>
#include <limits>
#include <type_traits>

namespace apache::thrift::util {

/**
 * TODO: implement multiplication and devision operations. Improve performance
 *
 * A simple wrapper class to provide saturated math operations for integral and
 * floating point numnbers
 */

template <typename T>
constexpr T add_saturating(T lhs, T rhs) {
  if constexpr (!std::is_integral_v<T>) {
    if (std::isnan(lhs) || std::isnan(rhs)) {
      return NAN;
    }

    if (std::isinf(lhs) || std::isinf(rhs)) {
      if (std::isinf(lhs) && std::isinf(rhs) && (rhs < 0) != (lhs < 0)) {
        return NAN;
      }
      return lhs + rhs;
    }
  }

  if (rhs > 0 && std::numeric_limits<T>::max() - rhs < lhs) {
    return std::numeric_limits<T>::max();
  } else if (rhs < 0 && std::numeric_limits<T>::lowest() - rhs > lhs) {
    return std::numeric_limits<T>::min();
  }
  return lhs + rhs;
}
} // namespace apache::thrift::util
