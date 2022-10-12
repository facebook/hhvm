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

package com.facebook.thrift.runner;

import com.facebook.thrift.util.resources.RpcResources;
import java.net.SocketAddress;
import java.time.Duration;
import java.util.concurrent.TimeUnit;
import java.util.function.BiFunction;
import org.HdrHistogram.Histogram;
import org.HdrHistogram.Recorder;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Schedulers;

/**
 * Base class for a simple ping / pong test. Creates a number of clients based on 2 * the number of
 * available processors. Use Flux.parallel to create a number of rails equal to the CONCURRENCY
 * value.
 *
 * <p>This class uses HDR Histogram to capture latency metrics.
 *
 * @param <C> the type of client to use to run the test
 */
public abstract class AbstractClient<C> {
  protected static int CONCURRENCY = Runtime.getRuntime().availableProcessors() * 2 + 8;

  protected abstract C getClient(SocketAddress address);

  protected abstract Mono<?> getResponse(C c, String message);

  private Flux<Long> test(SocketAddress address) {
    final C c = getClient(address);
    return Flux.range(0, Integer.MAX_VALUE)
        .flatMap(
            __ -> {
              long start = System.nanoTime();
              String message = Long.toString(System.currentTimeMillis());
              return getResponse(c, message).map(response -> System.nanoTime() - start);
            })
        .subscribeOn(Schedulers.fromExecutor(RpcResources.getEventLoopGroup().next()));
  }

  private void test(String label, SocketAddress address, Duration runDuration, Recorder histogram) {
    Flux.range(0, CONCURRENCY)
        .flatMap(c -> test(address))
        .scan(
            0L,
            new BiFunction<Long, Long, Long>() {
              long dotTs = 0;
              long restTs = 0;
              long dotTimeout = TimeUnit.MILLISECONDS.toNanos(500);
              long restTimeout = TimeUnit.SECONDS.toNanos(5);

              @Override
              public Long apply(Long total, Long duration) {
                histogram.recordValue(duration);

                if (System.nanoTime() - restTs > restTimeout) {
                  System.out.print("\r" + label);
                  restTs = System.nanoTime();
                }

                if (System.nanoTime() - dotTs > dotTimeout) {
                  System.out.print(".");
                  dotTs = System.nanoTime();
                }

                return total + 1;
              }
            })
        .take(runDuration)
        .blockLast();
    System.out.println();

    Histogram intervalHistogram = histogram.getIntervalHistogram();
    intervalHistogram.outputPercentileDistribution(System.out, 5, 1_000_000.0, false);

    double count = intervalHistogram.getTotalCount();
    double qps = count / (double) runDuration.getSeconds();
    System.out.println(label + " qps -> " + qps);
  }

  /**
   * Runs a simple ping post using the client provided by the getClient method. This method creates
   * two sequential runs. The first run is a warmup run to all the JVM JIT the code. The second run
   * is the actually test.
   *
   * @param address address to run the test against
   * @param warmupDuration the duration of the warmup run
   * @param benchmarkSeconds the duration of the benchmark run
   */
  protected void runTest(
      SocketAddress address, Duration warmupDuration, Duration benchmarkSeconds) {
    final Recorder warmupHistogram = new Recorder(3600000000000L, 3);
    test("warming up", address, warmupDuration, warmupHistogram);

    final Recorder histogram = new Recorder(3600000000000L, 3);
    test("benchmarking", address, benchmarkSeconds, histogram);

    System.exit(0);
  }
}
