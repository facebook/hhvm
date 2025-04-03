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

#include <glog/logging.h>

#include <folly/portability/SysTime.h>

#include <thrift/lib/cpp/thrift_config.h>

namespace apache::thrift::concurrency {

int64_t Util::currentTime() {
#if defined(THRIFT_HAVE_CLOCK_GETTIME)
  timespec now;
  int ret = clock_gettime(CLOCK_REALTIME, &now);
  auto subsec = now.tv_nsec;
  int64_t oldTicksPerSec = NS_PER_S;
#else
  timeval now;
  int ret = gettimeofday(&now, nullptr);
  auto subsec = now.tv_usec;
  int64_t oldTicksPerSec = US_PER_S;
#endif // defined(THRIFT_HAVE_CLOCK_GETTIME)

  DCHECK(ret == 0);
  int64_t result;
  toTicks(result, now.tv_sec, subsec, oldTicksPerSec, MS_PER_S);
  return result;
}

} // namespace apache::thrift::concurrency
