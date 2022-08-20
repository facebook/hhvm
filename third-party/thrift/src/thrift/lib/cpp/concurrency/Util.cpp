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

#include <thrift/lib/cpp/concurrency/Util.h>

#include <cassert>
#include <cerrno>
#include <ctime>

#include <glog/logging.h>

#include <folly/portability/SysTime.h>

#include <thrift/lib/cpp/thrift_config.h>

namespace apache {
namespace thrift {
namespace concurrency {

int64_t Util::currentTimeTicks(int64_t ticksPerSec) {
#if defined(THRIFT_HAVE_CLOCK_GETTIME)
  struct timespec now;
  int ret = clock_gettime(CLOCK_REALTIME, &now);
#else
  struct timeval now;
  int ret = gettimeofday(&now, nullptr);
#endif // defined(THRIFT_HAVE_CLOCK_GETTIME)

  DCHECK(ret == 0);
  int64_t result;
  toTicks(result, now, ticksPerSec);
  return result;
}

} // namespace concurrency
} // namespace thrift
} // namespace apache
