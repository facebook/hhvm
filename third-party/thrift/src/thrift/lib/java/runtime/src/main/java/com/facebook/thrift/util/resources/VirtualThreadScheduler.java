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

import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;
import reactor.core.Disposable;

/**
 * This is a noop {@link VirtualThreadScheduler} to be used with Java 21. This {@link
 * VirtualThreadScheduler} variant is included when is used with a JDK versions lower than 21, and
 * all methods throw an {@link UnsupportedOperationException}. An alternative variant is available
 * for use on JDK 21+ where virtual threads are supported.
 */
public final class VirtualThreadScheduler implements ThriftScheduler {

  public VirtualThreadScheduler() {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  public VirtualThreadScheduler(boolean limitConcurrency, int maxConcurrency) {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public ExecutorService getExecutor() {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public Worker createWorker() {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public Disposable schedule(Runnable task) {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public Disposable schedule(Runnable task, long delay, TimeUnit unit) {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public Disposable schedulePeriodically(
      Runnable task, long initialDelay, long period, TimeUnit unit) {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public void dispose() {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public boolean isDisposed() {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }

  @Override
  public Map<String, Long> getStats() {
    throw new UnsupportedOperationException(
        "Virtual Threads are not supported in JVM lower than 21");
  }
}
