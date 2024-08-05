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

package com.facebook.thrift.metrics.benchmark;

import com.facebook.thrift.metrics.distribution.MultiWindowDistribution;
import com.facebook.thrift.metrics.distribution.SingleWindowDistribution;
import com.facebook.thrift.metrics.distribution.Utils;
import com.facebook.titan.stats.MultiWindowTDigest;
import io.airlift.stats.Distribution;
import org.apache.commons.lang3.RandomUtils;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.TearDown;
import org.openjdk.jmh.annotations.Threads;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.RunnerException;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;

@State(Scope.Benchmark)
public class StatsBenchmark {

  MultiWindowDistribution multiWindowDist;
  SingleWindowDistribution singleWindowDist;
  Distribution airliftDist;
  MultiWindowTDigest multiWindowTDigest;

  @Param({"1", "1000"})
  int size;

  long[] samples;

  @Setup
  public void setup() {
    multiWindowDist = new MultiWindowDistribution();
    singleWindowDist = new SingleWindowDistribution();
    airliftDist = new Distribution();
    multiWindowTDigest = new MultiWindowTDigest();

    samples = new long[size];
    for (int c = 0; c < size; c++) {
      samples[c] = RandomUtils.nextLong();
    }
  }

  @TearDown
  public void tearDown() {
    Utils.shutdownExecutorService();
  }

  @Benchmark
  public void FastStat_MultiWindowDistribution() {
    for (int i = 0; i < size; i++) {
      multiWindowDist.add(samples[i]);
    }
  }

  @Benchmark
  public void FastStat_SingleWindowDistribution() {
    for (int i = 0; i < size; i++) {
      singleWindowDist.add(samples[i]);
    }
  }

  @Benchmark
  public void Airlift_Distribution() {
    for (int i = 0; i < size; i++) {
      airliftDist.add(samples[i]);
    }
  }

  @Benchmark
  public void TDigest_MultiWindowDistribution() {
    for (int i = 0; i < size; i++) {
      multiWindowTDigest.add(samples[i]);
    }
  }

  @Benchmark
  @Threads(Threads.MAX)
  public void FastStat_MultiWindowDistribution_multiThreaded() {
    for (int i = 0; i < size; i++) {
      multiWindowDist.add(samples[i]);
    }
  }

  @Benchmark
  @Threads(Threads.MAX)
  public void FastStat_SingleWindowDistribution_MultiThreaded() {
    for (int i = 0; i < size; i++) {
      singleWindowDist.add(samples[i]);
    }
  }

  @Benchmark
  @Threads(Threads.MAX)
  public void Airlift_Distribution_MultiThreaded() {
    for (int i = 0; i < size; i++) {
      airliftDist.add(samples[i]);
    }
  }

  @Benchmark
  @Threads(Threads.MAX)
  public void TDigest_MultiWindowDistribution_MultiThreaded() {
    for (int i = 0; i < size; i++) {
      multiWindowTDigest.add(samples[i]);
    }
  }

  public static void main(String[] args) throws RunnerException {
    Options opt =
        new OptionsBuilder()
            .include(Benchmark.class.getSimpleName())
            .mode(Mode.Throughput)
            .forks(1)
            .build();

    new Runner(opt).run();
  }
}
