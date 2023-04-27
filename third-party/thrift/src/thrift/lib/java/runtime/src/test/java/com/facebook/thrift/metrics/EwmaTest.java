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

package com.facebook.thrift.metrics;

import org.junit.Assert;
import org.junit.Test;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Schedulers;
import reactor.test.StepVerifier;

public class EwmaTest {

  @Test
  public void testEwma() {
    Ewma ewma = Ewma.oneHourDecay();
    Ewma ewma2 = Ewma.oneSecondDecay();
    int count = 1_000_000;
    for (int i = 0; i < count; i++) {
      ewma.insert(i);
      ewma2.insert(i);
    }

    Assert.assertTrue(ewma.value() < ewma2.value());
  }

  @Test
  public void testEwmaDecay() throws Exception {
    Ewma ewma = Ewma.oneSecondDecay();
    ewma.insert(100);
    double value = ewma.value();
    Assert.assertEquals(value, 100, 0);
    ewma.insert(1);
    double value1 = ewma.value();
    Assert.assertTrue(value1 < value);
  }

  @Test
  public void testAddingWithThreads() {
    Ewma ewma = Ewma.oneMinuteDecay();
    Mono<Double> sequential =
        Flux.range(0, 1_000_000)
            .parallel()
            .runOn(Schedulers.parallel())
            .doOnNext(__ -> ewma.insert(10000))
            .sequential()
            .last()
            .map(__ -> ewma.value());

    StepVerifier.create(sequential)
        .assertNext(expected -> Assert.assertEquals(expected, 10000, 0))
        .verifyComplete();
  }
}
