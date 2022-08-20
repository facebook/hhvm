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
import java.util.concurrent.Executors;
import org.junit.Assert;
import org.junit.Test;
import reactor.core.publisher.Mono;
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
}
