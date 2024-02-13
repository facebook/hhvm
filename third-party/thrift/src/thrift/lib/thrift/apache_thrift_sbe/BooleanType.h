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

/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_BOOLEANTYPE_CXX_H_
#define _APACHE_THRIFT_SBE_BOOLEANTYPE_CXX_H_

#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS 1
#endif

#include <cstdint>
#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()
#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()
#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()
#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()
#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()
#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()
#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()
#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()

namespace apache {
namespace thrift {
namespace sbe {

class BooleanType {
 public:
  enum Value {
    FALSE = static_cast<std::uint8_t>(0),
    TRUE = static_cast<std::uint8_t>(1),
    NULL_VALUE = static_cast<std::uint8_t>(255)
  };

  static BooleanType::Value get(const std::uint8_t value) {
    switch (value) {
      case static_cast<std::uint8_t>(0):
        return FALSE;
      case static_cast<std::uint8_t>(1):
        return TRUE;
      case static_cast<std::uint8_t>(255):
        return NULL_VALUE;
    }

    throw std::runtime_error("unknown value for enum BooleanType [E103]");
  }

  static const char* c_str(const BooleanType::Value value) {
    switch (value) {
      case FALSE:
        return "FALSE";
      case TRUE:
        return "TRUE";
      case NULL_VALUE:
        return "NULL_VALUE";
    }

    throw std::runtime_error("unknown value for enum BooleanType [E103]:");
  }

  template <typename CharT, typename Traits>
  friend std::basic_ostream<CharT, Traits>& operator<<(
      std::basic_ostream<CharT, Traits>& os, BooleanType::Value m) {
    return os << BooleanType::c_str(m);
  }
};

} // namespace sbe
} // namespace thrift
} // namespace apache

#endif
