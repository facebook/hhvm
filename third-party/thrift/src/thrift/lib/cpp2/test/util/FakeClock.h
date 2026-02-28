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

#include <chrono>

namespace apache::thrift {

class FakeClock {
 public:
  using rep = uint64_t;
  using period = std::ratio<1L, 1000000000L>;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<FakeClock>;

  static void advance(const duration& d) noexcept { now_us_ += d; }

  static void reset_to_epoch() noexcept { now_us_ -= (now_us_ - time_point()); }

  static time_point now() noexcept { return now_us_; }

  static constexpr bool is_steady = true;

 private:
  FakeClock() = delete;

  static time_point now_us_;
};

} // namespace apache::thrift
