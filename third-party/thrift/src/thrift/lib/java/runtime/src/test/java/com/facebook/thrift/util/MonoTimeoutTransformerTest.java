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

package com.facebook.thrift.util;

import com.facebook.thrift.util.resources.RpcResources;
import io.netty.util.concurrent.FastThreadLocalThread;
import java.time.Duration;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.LockSupport;
import org.junit.Assert;
import org.junit.Test;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import reactor.core.scheduler.Schedulers;
import reactor.test.StepVerifier;

public class MonoTimeoutTransformerTest {
  @Test
  public void testMultipleSubscriptionsToSingleTransformer() {
    MonoTimeoutTransformer<Integer> transformer =
        new MonoTimeoutTransformer<>(
            RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS);

    StepVerifier.create(Mono.just(1).transform(transformer)).expectNext(1).verifyComplete();
    StepVerifier.create(Mono.delay(Duration.ofSeconds(1)).then(Mono.just(1)).transform(transformer))
        .verifyError(TimeoutException.class);
    StepVerifier.create(Mono.just(1).transform(transformer)).expectNext(1).verifyComplete();
    StepVerifier.create(Mono.delay(Duration.ofSeconds(1)).then(Mono.just(1)).transform(transformer))
        .verifyError(TimeoutException.class);
  }

  @Test
  public void testMonoTimeoutTransformerWhenFluxIsBlocked() {
    Mono<Object> transform =
        Mono.fromRunnable(LockSupport::park)
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS))
            .subscribeOn(RpcResources.getOffLoopScheduler());

    StepVerifier.create(transform).verifyError(TimeoutException.class);
  }

  @Test
  public void testMonoTimeoutTransformerBeingCalledParallel() {
    int count = 1_000;
    Mono<Integer> reduce =
        Flux.range(0, count)
            .parallel()
            .runOn(Schedulers.parallel())
            .flatMap(
                i -> {
                  if (i % 2 == 0) {
                    return Mono.just(1)
                        .transform(
                            new MonoTimeoutTransformer<>(
                                RpcResources.getClientOffLoopScheduler(),
                                1,
                                TimeUnit.MILLISECONDS));
                  } else {
                    return Mono.delay(Duration.ofSeconds(1))
                        .transform(
                            new MonoTimeoutTransformer<>(
                                RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.MILLISECONDS))
                        .onErrorResume(throwable -> Mono.empty());
                  }
                })
            .sequential()
            .reduce(0, (integer, number) -> integer + 1);

    StepVerifier.create(reduce).expectNext(count / 2).verifyComplete();
  }

  @Test
  public void testShouldNotTimeout() {
    Mono<Integer> transform =
        Flux.range(0, 100_000)
            .ignoreElements()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.DAYS));

    StepVerifier.create(transform).expectComplete().verify();
  }

  @Test
  public void testShouldTimeout() {
    Mono<Object> transform =
        Mono.never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(transform).verifyError(TimeoutException.class);
  }

  @Test
  public void testShouldFallback() {
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.SECONDS,
                    Mono.just("hello")));

    StepVerifier.create(transform).expectNext("hello").verifyComplete();
  }

  @Test
  public void testShouldFallbackMultipleTimes() {
    Mono<String> transform =
        Mono.<String>never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(),
                    1,
                    TimeUnit.SECONDS,
                    Mono.just("hello")));

    Flux<String> flux = Flux.range(0, 3).flatMap(__ -> transform);

    StepVerifier.create(flux).expectNext("hello", "hello", "hello").verifyComplete();
  }

  @Test
  public void testShouldTimeOutWithInfiniteStream() {
    Mono<Long> infinite =
        Flux.interval(Duration.ofMillis(1), Schedulers.elastic())
            .ignoreElements()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(infinite)
        .verifyErrorSatisfies(
            throwable -> {
              Assert.assertTrue(throwable instanceof TimeoutException);
              Thread thread = Thread.currentThread();
              Assert.assertTrue(thread instanceof FastThreadLocalThread);
            });
  }

  @Test(timeout = 5_000)
  public void testShouldTimeOutWhenNeverRequested() {
    Mono<Object> never =
        Mono.never()
            .transform(
                new MonoTimeoutTransformer<>(
                    RpcResources.getClientOffLoopScheduler(), 1, TimeUnit.SECONDS));

    StepVerifier.create(never).verifyError();
  }
}
