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

package com.facebook.thrift.metrics.distribution;

import com.facebook.thrift.metrics.common.Clock;
import java.util.concurrent.TimeUnit;

public class TestClock implements Clock {
  private long now = 0l;

  @Override
  public long tickMilis() {
    return TimeUnit.NANOSECONDS.toMillis(now);
  }

  @Override
  public long tickNanos() {
    return now;
  }

  public void incrementNs(long nanos) {
    this.now += nanos;
  }

  public void incrementMs(long ms) {
    this.now += TimeUnit.MILLISECONDS.toNanos(ms);
  }

  public void incrementSec(long sec) {
    this.now += TimeUnit.SECONDS.toNanos(sec);
  }
}
