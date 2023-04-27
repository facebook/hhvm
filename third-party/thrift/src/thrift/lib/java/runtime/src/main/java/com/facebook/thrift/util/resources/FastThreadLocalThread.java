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

import io.netty.util.concurrent.FastThreadLocal;
import io.netty.util.internal.InternalThreadLocalMap;
import io.netty.util.internal.UnstableApi;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.ForkJoinWorkerThread;

public class FastThreadLocalThread extends ForkJoinWorkerThread {

  private static final InternalLogger logger =
      InternalLoggerFactory.getInstance(io.netty.util.concurrent.FastThreadLocalThread.class);

  // This will be set to true if we have a chance to wrap the Runnable.
  private final boolean cleanupFastThreadLocals;

  private InternalThreadLocalMap threadLocalMap;

  public FastThreadLocalThread(ForkJoinPool forkJoinPool) {
    super(forkJoinPool);
    cleanupFastThreadLocals = false;
  }

  /**
   * Returns the internal data structure that keeps the thread-local variables bound to this thread.
   * Note that this method is for internal use only, and thus is subject to change at any time.
   */
  public final InternalThreadLocalMap threadLocalMap() {
    if (this != Thread.currentThread() && logger.isWarnEnabled()) {
      logger.warn(
          new RuntimeException(
              "It's not thread-safe to get 'threadLocalMap' "
                  + "which doesn't belong to the caller thread"));
    }
    return threadLocalMap;
  }

  /**
   * Sets the internal data structure that keeps the thread-local variables bound to this thread.
   * Note that this method is for internal use only, and thus is subject to change at any time.
   */
  public final void setThreadLocalMap(InternalThreadLocalMap threadLocalMap) {
    if (this != Thread.currentThread() && logger.isWarnEnabled()) {
      logger.warn(
          new RuntimeException(
              "It's not thread-safe to set 'threadLocalMap' "
                  + "which doesn't belong to the caller thread"));
    }
    this.threadLocalMap = threadLocalMap;
  }

  /**
   * Returns {@code true} if {@link FastThreadLocal#removeAll()} will be called once {@link #run()}
   * completes.
   */
  @UnstableApi
  public boolean willCleanupFastThreadLocals() {
    return cleanupFastThreadLocals;
  }

  /**
   * Returns {@code true} if {@link FastThreadLocal#removeAll()} will be called once {@link
   * Thread#run()} completes.
   */
  @UnstableApi
  public static boolean willCleanupFastThreadLocals(Thread thread) {
    return thread instanceof io.netty.util.concurrent.FastThreadLocalThread
        && ((io.netty.util.concurrent.FastThreadLocalThread) thread).willCleanupFastThreadLocals();
  }
}
