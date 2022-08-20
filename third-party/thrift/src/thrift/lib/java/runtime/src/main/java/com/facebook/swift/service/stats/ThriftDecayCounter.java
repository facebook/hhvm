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

package com.facebook.swift.service.stats;

import io.airlift.stats.DecayCounter;
import io.airlift.stats.ExponentialDecay;
import io.micrometer.core.instrument.AbstractMeter;
import io.micrometer.core.instrument.Counter;

public class ThriftDecayCounter extends AbstractMeter implements Counter {
  private final DecayCounter value;

  public ThriftDecayCounter(Id id, int seconds) {
    super(id);
    this.value = new DecayCounter(ExponentialDecay.seconds(seconds));
  }

  @Override
  public void increment(double amount) {
    value.add((long) amount);
  }

  @Override
  public double count() {
    return value.getCount();
  }
}
