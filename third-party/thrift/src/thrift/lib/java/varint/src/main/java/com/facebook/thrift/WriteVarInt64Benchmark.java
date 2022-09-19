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

package com.facebook.thrift;

import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Fork;
import org.openjdk.jmh.annotations.Measurement;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.OutputTimeUnit;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.Warmup;
import org.openjdk.jmh.infra.Blackhole;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.RunnerException;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;

@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@State(Scope.Benchmark)
@Fork(
    value = 1,
    jvmArgs = {"-Xms2G", "-Xmx2G"})
@Warmup(time = 1, timeUnit = TimeUnit.SECONDS, iterations = 20, batchSize = 1)
@Measurement(time = 1, timeUnit = TimeUnit.SECONDS, iterations = 5, batchSize = 1)
public class WriteVarInt64Benchmark {
  public static void main(String[] args) throws RunnerException {

    Options opt =
        new OptionsBuilder().include(WriteVarInt64Benchmark.class.getSimpleName()).forks(1).build();

    new Runner(opt).run();
  }

  ByteBuffer buffer;

  Blackhole bh;

  @Param({"0", "7", "14", "28", "35", "42", "49", "56", "63"})
  long shift;

  long i;

  @Setup()
  public void setup(Blackhole bh) {
    this.buffer = ByteBuffer.allocate(32);
    buffer.mark();
    this.i = (1L << shift) - 1;
    this.bh = bh;
  }

  @Benchmark
  public void benchmarkWrite() {
    buffer.reset();
    VarIntUtils.writeVarInt64(i, buffer);
  }
}
