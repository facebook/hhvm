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

import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.SettableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import org.junit.Assert;
import org.junit.Test;
import org.reactivestreams.Subscription;
import reactor.core.CoreSubscriber;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Hooks;
import reactor.core.publisher.Mono;
import reactor.core.publisher.Operators;
import reactor.core.publisher.Sinks;
import reactor.core.publisher.Sinks.One;
import reactor.core.scheduler.Scheduler;
import reactor.core.scheduler.Schedulers;
import reactor.test.StepVerifier;

public class FutureUtilTest {
  @Test
  public void testShouldScheduleWorkOnDifferentThreadWithSynchronousFuture() {
    Scheduler myThread =
        Schedulers.fromExecutor(
            Executors.newSingleThreadExecutor(
                r -> {
                  Thread t = new Thread(r);
                  t.setDaemon(true);
                  t.setName("MyThread");
                  return t;
                }));

    Mono<String> stringMono = FutureUtil.toScheduledMono(this::hi, myThread);

    StepVerifier.create(stringMono)
        .assertNext(
            s -> {
              Thread thread = Thread.currentThread();
              Assert.assertTrue(thread.getName().contains("MyThread"));
            })
        .verifyComplete();
  }

  private ListenableFuture<String> hi() {
    SettableFuture<String> future = SettableFuture.create();
    future.set("Hi");
    return future;
  }

  @Test
  public void testOnNextDropped() throws Exception {
    CountDownLatch latch = new CountDownLatch(1);
    Hooks.onNextDropped(o -> latch.countDown());
    One<Object> one = Sinks.one();
    ListenableFuture<Object> future = FutureUtil.toListenableFuture(one.asMono());
    future.cancel(true);
    one.tryEmitValue(one);
    latch.await();
  }

  @Test
  public void testOnErrorDropped() throws Exception {
    CountDownLatch latch = new CountDownLatch(1);
    Hooks.onErrorDropped(o -> latch.countDown());
    One<Object> one = Sinks.one();
    ListenableFuture<Object> future = FutureUtil.toListenableFuture(one.asMono());
    future.cancel(true);
    one.tryEmitError(new RuntimeException());
    latch.await();
  }

  @Test
  public void testMonoToFutureUtil() throws Exception {
    Mono<Long> count = Flux.range(1, 100).count();

    ListenableFuture<Long> future = FutureUtil.toListenableFuture(count);
    Long aLong = future.get();
    Assert.assertEquals(100L, aLong.longValue());
  }

  @Test
  public void testMonoEmpty() throws Exception {
    ListenableFuture future = FutureUtil.toListenableFuture(Mono.empty());
    Object o = future.get();
    Assert.assertNull(o);
  }

  @Test
  public void testMonoThatDoesNotEmit() throws Exception {
    Mono m =
        new Mono() {
          @Override
          public void subscribe(CoreSubscriber actual) {
            actual.onSubscribe(Operators.emptySubscription());
            actual.onComplete();
          }
        };

    ListenableFuture future = FutureUtil.toListenableFuture(m);
    Object o = future.get();
    Assert.assertNull(o);
  }

  @Test
  public void testWithScalarSubscription() throws Exception {
    Mono m =
        new Mono() {
          @Override
          public void subscribe(CoreSubscriber actual) {
            Subscription subscription = Operators.scalarSubscription(actual, 1);
            actual.onSubscribe(subscription);
          }
        };

    ListenableFuture future = FutureUtil.toListenableFuture(m);
    int i = (int) future.get();
    Assert.assertEquals(1, i);
  }

  @Test
  public void testWithScalarMono() throws Exception {
    ListenableFuture future = FutureUtil.toListenableFuture(Mono.just(1));
    int i = (int) future.get();
    Assert.assertEquals(1, i);
  }

  @Test(expected = ExecutionException.class)
  public void testWithScalarMonoThatEmitsException() throws Exception {
    ListenableFuture future = FutureUtil.toListenableFuture(Mono.error(new RuntimeException()));
    future.get();
  }
}
