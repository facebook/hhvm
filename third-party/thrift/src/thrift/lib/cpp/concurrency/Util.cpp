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

#include <thrift/lib/cpp/concurrency/Util.h>

#include <ctime>
#include <ratio>

#include <glog/logging.h>

#include <folly/portability/SysTime.h>

#include <thrift/lib/cpp/thrift_config.h>

namespace apache::thrift::concurrency {

static void toTicks(
    int64_t& result,
    int64_t secs,
    int64_t oldTicks,
    int64_t oldTicksPerSec,
    int64_t newTicksPerSec) {
  result = secs * newTicksPerSec;
  result += oldTicks * newTicksPerSec / oldTicksPerSec;

  int64_t oldPerNew = oldTicksPerSec / newTicksPerSec;
  if (oldPerNew && ((oldTicks % oldPerNew) >= (oldPerNew / 2))) {
    ++result;
  }
}

int64_t Util::currentTime() {
#if defined(THRIFT_HAVE_CLOCK_GETTIME)
  timespec now;
  int ret = clock_gettime(CLOCK_REALTIME, &now);
  auto subsec = now.tv_nsec;
  int64_t oldTicksPerSec = std::nano::den;
#else
  timeval now;
  int ret = gettimeofday(&now, nullptr);
  auto subsec = now.tv_usec;
  int64_t oldTicksPerSec = std::micro::den;
#endif // defined(THRIFT_HAVE_CLOCK_GETTIME)

  DCHECK(ret == 0);
  int64_t result;
  toTicks(result, now.tv_sec, subsec, oldTicksPerSec, std::milli::den);
  return result;
}

} // namespace apache::thrift::concurrency
