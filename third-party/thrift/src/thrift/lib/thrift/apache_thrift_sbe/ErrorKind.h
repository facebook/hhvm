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
#ifndef _APACHE_THRIFT_SBE_ERRORKIND_CXX_H_
#define _APACHE_THRIFT_SBE_ERRORKIND_CXX_H_

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

class ErrorKind {
 public:
  enum Value {
    UNSPECIFIED = static_cast<std::uint8_t>(0),
    TRANSIENT = static_cast<std::uint8_t>(1),
    STATEFUL = static_cast<std::uint8_t>(2),
    PERMANENT = static_cast<std::uint8_t>(3),
    NULL_VALUE = static_cast<std::uint8_t>(255)
  };

  static ErrorKind::Value get(const std::uint8_t value) {
    switch (value) {
      case static_cast<std::uint8_t>(0):
        return UNSPECIFIED;
      case static_cast<std::uint8_t>(1):
        return TRANSIENT;
      case static_cast<std::uint8_t>(2):
        return STATEFUL;
      case static_cast<std::uint8_t>(3):
        return PERMANENT;
      case static_cast<std::uint8_t>(255):
        return NULL_VALUE;
    }

    throw std::runtime_error("unknown value for enum ErrorKind [E103]");
  }

  static const char* c_str(const ErrorKind::Value value) {
    switch (value) {
      case UNSPECIFIED:
        return "UNSPECIFIED";
      case TRANSIENT:
        return "TRANSIENT";
      case STATEFUL:
        return "STATEFUL";
      case PERMANENT:
        return "PERMANENT";
      case NULL_VALUE:
        return "NULL_VALUE";
    }

    throw std::runtime_error("unknown value for enum ErrorKind [E103]:");
  }

  template <typename CharT, typename Traits>
  friend std::basic_ostream<CharT, Traits>& operator<<(
      std::basic_ostream<CharT, Traits>& os, ErrorKind::Value m) {
    return os << ErrorKind::c_str(m);
  }
};

} // namespace sbe
} // namespace thrift
} // namespace apache

#endif
