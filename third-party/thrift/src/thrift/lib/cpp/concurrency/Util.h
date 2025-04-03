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

#ifndef _THRIFT_CONCURRENCY_UTIL_H_
#define _THRIFT_CONCURRENCY_UTIL_H_ 1

#include <cstdint>

#include <folly/portability/SysTime.h>

namespace apache::thrift::concurrency {

// DEPRECATED! Use chrono instead.
class Util {
 public:
  /**
   * Get current time as milliseconds from epoch
   */
  static int64_t currentTime();
};

} // namespace apache::thrift::concurrency

#endif // #ifndef _THRIFT_CONCURRENCY_UTIL_H_
