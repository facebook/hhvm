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

#include <thrift/lib/cpp/Thrift.h>

#include <sstream>

#include <folly/String.h>

namespace apache::thrift {
namespace detail {
std::string* makeCheckErrorMessage(
    intmax_t v1,
    intmax_t v2,
    const char* s1,
    const char* s2,
    const char* name) {
  auto n1 = (s1 ? std::string(s1) : std::to_string(v1));
  auto n2 = (s2 ? std::string(s2) : std::to_string(v2));
  std::ostringstream ss;
  ss << name << " (" << n1 << " vs. " << n2 << ")";
  return new std::string(ss.str());
}
} // namespace detail

TLibraryException::TLibraryException(const char* message, int errnoValue) {
  message_ = std::string(message) + ": " + folly::errnoStr(errnoValue);
}

} // namespace apache::thrift
