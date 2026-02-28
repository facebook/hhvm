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

import java.util.concurrent.TimeUnit;
import reactor.core.Disposable;

/** Runnable that will not execute if it's disposed. */
final class DisposableExecutionMeasuringRunnable implements Runnable, Disposable {
  private final Runnable runnable;
  private final long start;
  private volatile int disposed = 0;

  public DisposableExecutionMeasuringRunnable(Runnable runnable) {
    this.runnable = runnable;
    this.start = System.nanoTime();
  }

  @Override
  public void run() {
    if (disposed == 0) {
      final ExecutionRecordingThread t = (ExecutionRecordingThread) Thread.currentThread();
      try {
        runnable.run();
      } finally {
        long duration = System.nanoTime() - start;
        t.recordExecutionTimeNanos(TimeUnit.NANOSECONDS.toMicros(duration));
      }
    }
  }

  @Override
  public void dispose() {
    disposed = 1;
  }

  @Override
  public boolean isDisposed() {
    return disposed == 1;
  }
}
