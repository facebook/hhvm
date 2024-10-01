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

package com.facebook.thrift.metrics.distribution;

import com.facebook.thrift.metrics.common.Clock;
import com.facebook.thrift.metrics.common.NanoClock;
import com.google.common.annotations.VisibleForTesting;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class Utils {
  private Utils() {}

  private static volatile ScheduledExecutorService executorService;
  private static final Clock clock = new NanoClock();

  public static Clock getClock() {
    return clock;
  }

  public static ScheduledExecutorService getExecutorService() {
    if (executorService == null) {
      synchronized (Utils.class) {
        if (executorService == null) {
          executorService =
              Executors.newSingleThreadScheduledExecutor(
                  new ThreadFactoryBuilder().setDaemon(true).build());
        }
      }
    }
    return executorService;
  }

  @VisibleForTesting
  public static void setExecutorService(ScheduledExecutorService executorService) {
    Utils.executorService = executorService;
  }

  /** Allow clean shutdown for testing purposes */
  public static void shutdownExecutorService() {
    if (executorService != null) {
      executorService.shutdown();
      // Disable new tasks from being submitted
      try {
        // Wait a while for existing tasks to terminate
        if (!executorService.awaitTermination(60, TimeUnit.SECONDS)) {
          executorService.shutdownNow();
          // Wait a while for tasks to respond to being cancelled
          if (!executorService.awaitTermination(10, TimeUnit.SECONDS)) {
            System.err.println("Pool did not terminate");
          }
        }
      } catch (InterruptedException e) {
        executorService.shutdownNow(); // Preserve interrupt status
        Thread.currentThread().interrupt();
      }
    }
  }
}
