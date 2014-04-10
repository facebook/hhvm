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
#include "hphp/util/timer.h"

#include <errno.h>
#include <assert.h>

using HPHP::Timer;

namespace apache { namespace thrift { namespace concurrency {

const int64_t Util::currentTimeTicks(int64_t ticksPerSec) {
  int64_t result;
  struct timespec now;
  Timer::GetRealtimeTime(now);
  toTicks(result, now, ticksPerSec);
  return result;
}

const int64_t Util::monotonicTimeTicks(int64_t ticksPerSec) {
  int64_t result;
  struct timespec now;
  Timer::GetMonotonicTime(now);
  toTicks(result, now, ticksPerSec);
  return result;
}

}}} // apache::thrift::concurrency
