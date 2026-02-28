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

package com.facebook.thrift.util.resources;

import static com.facebook.thrift.util.resources.ResourceConfiguration.capForkJoinThreads;

import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.TimeUnit;

/*
 * Java 11 introduced an advance constructor for the ForkJoinPool that allows us to configure the
 * max pool size. We want to use this constructor and set maximumPoolSize = parallelism to avoid threads
 * expanding to MAX_CAP = 0x7fff. This has been seen in some corner cases where tasks block and the
 * thread pool is expanded up to 32767 or 0x7fff threads. This causes hosts to become effectively
 * unresponsive. This can be removed once Java 8 is deprecated.
 */
public class ForkJoinPoolFactory {
  public static ForkJoinPool create(
      int parallelism,
      ForkJoinPool.ForkJoinWorkerThreadFactory workerThreadFactory,
      Thread.UncaughtExceptionHandler uncaughtExceptionHandler,
      boolean capForkJoinThreads) {
    if (capForkJoinThreads) {
      return new ForkJoinPool(
          parallelism,
          workerThreadFactory,
          uncaughtExceptionHandler,
          true,
          0,
          parallelism,
          1,
          null,
          60L,
          TimeUnit.SECONDS);
    } else {
      return new ForkJoinPool(parallelism, workerThreadFactory, uncaughtExceptionHandler, true);
    }
  }
}
