/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "thrift/lib/cpp/concurrency/Util.h"

#include "thrift/lib/cpp/config.h"
#include "glog/logging.h"

#if defined(HAVE_CLOCK_GETTIME)
#include <time.h>
#elif defined(HAVE_GETTIMEOFDAY)
#include <sys/time.h>
#endif // defined(HAVE_CLOCK_GETTIME)
#include <errno.h>
#include <assert.h>

namespace apache { namespace thrift { namespace concurrency {

const int64_t Util::currentTimeTicks(int64_t ticksPerSec) {
  int64_t result;

#if defined(HAVE_CLOCK_GETTIME)
  struct timespec now;
  int ret = clock_gettime(CLOCK_REALTIME, &now);
  DCHECK(ret == 0);
  toTicks(result, now, ticksPerSec);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timeval now;
  int ret = gettimeofday(&now, nullptr);
  assert(ret == 0);
  toTicks(result, now, ticksPerSec);
#else
#error "No high-precision clock is available."
#endif // defined(HAVE_CLOCK_GETTIME)

  return result;
}

const int64_t Util::monotonicTimeTicks(int64_t ticksPerSec) {
#if defined(HAVE_CLOCK_GETTIME)
  static bool useRealtime;
  if (useRealtime) {
    return currentTimeTicks(ticksPerSec);
  }

  struct timespec now;
  int ret = clock_gettime(CLOCK_MONOTONIC, &now);
  if (ret != 0) {
    // CLOCK_MONOTONIC is probably not supported on this system
    assert(errno == EINVAL);
    useRealtime = true;
    return currentTimeTicks(ticksPerSec);
  }

  int64_t result;
  toTicks(result, now, ticksPerSec);
  return result;
#else
  return currentTimeTicks(ticksPerSec);
#endif // defined(HAVE_CLOCK_GETTIME)
}

}}} // apache::thrift::concurrency
