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

import java.util.concurrent.ForkJoinPool;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ForkJoinPoolFactory {
  private static final Logger LOG = LoggerFactory.getLogger(ForkJoinPoolFactory.class);

  public static ForkJoinPool create(
      int parallelism,
      ForkJoinPool.ForkJoinWorkerThreadFactory workerThreadFactory,
      Thread.UncaughtExceptionHandler uncaughtExceptionHandler,
      boolean capForkJoinThreads) {
    if (capForkJoinThreads) {
      LOG.error(
          "Capping ForkJoinPool concurrency is only supported for Java 11+, concurrency will NOT be"
              + " limited!");
    }
    return new ForkJoinPool(parallelism, workerThreadFactory, uncaughtExceptionHandler, true);
  }
}
