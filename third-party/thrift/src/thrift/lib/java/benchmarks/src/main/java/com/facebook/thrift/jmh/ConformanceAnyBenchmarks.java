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

package com.facebook.thrift.jmh;

import java.util.concurrent.TimeUnit;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;
import org.openjdk.jmh.runner.options.TimeValue;

public class ConformanceAnyBenchmarks {
  public static void main(String... args) throws Exception {
    Options opt =
        new OptionsBuilder()
            .include(ConformanceLazyAnyBenchmarks.class.getSimpleName())
            .mode(Mode.Throughput)
            .forks(1)
            .warmupIterations(20)
            .warmupTime(new TimeValue(1000, TimeUnit.MILLISECONDS))
            .measurementIterations(20)
            .measurementTime(new TimeValue(2000, TimeUnit.MILLISECONDS))
            .addProfiler("gc")
            .jvmArgsAppend(
                "-Xms4g",
                "-Xmx4g",
                "-Dio.netty.leakDetectionLevel=disabled",
                "--add-opens=java.base/jdk.internal.misc=ALL-UNNAMED",
                "-Dio.netty.tryReflectionSetAccessible=true",
                "-Dio.netty.buffer.checkBounds=false",
                "-Dio.netty.buffer.checkAccessible=false",
                "-Dio.netty.allocator.directMemoryCacheAlignment=8",
                "-Dthrift.force-execution-off-eventloop=false")
            .build();
    new Runner(opt).run();
  }
}
