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

import io.airlift.stats.Distribution;
import io.netty.util.concurrent.FastThreadLocalThread;

/**
 * Thread used by a thread pool to caputre execution for the entire pool, and execution for a single
 * thread. The {@link DisposableExecutionMeasuringRunnable} uses this thread to record is execution
 * duration.
 */
final class ExecutionRecordingThread extends FastThreadLocalThread {
  private final Distribution perThreadExecutionTime;
  private final Distribution executionTime;

  public ExecutionRecordingThread(
      Runnable r, Distribution perThreadExecutionTime, Distribution executionTime) {
    super(r);
    this.perThreadExecutionTime = perThreadExecutionTime;
    this.executionTime = executionTime;
  }

  public void recordExecutionTimeNanos(long duration) {
    perThreadExecutionTime.add(duration);
    executionTime.add(duration);
  }
}
