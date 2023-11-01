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

import reactor.util.concurrent.Queues;

final class ResourceConfiguration {
  private ResourceConfiguration() {}

  static final boolean enableForkJoinPool =
      System.getProperty("thrift.separate-forkjoin-scheduler", "false").equalsIgnoreCase("true");

  static final boolean enableOperatorFusion =
      System.getProperty("thrift.operator-fusion", "true").equalsIgnoreCase("true");

  static final boolean forceExecutionOffEventLoop =
      System.getProperty("thrift.force-execution-off-eventloop", "true").equalsIgnoreCase("true");

  static final int maxPendingTasksForOffLoop =
      Math.max(
          Queues.SMALL_BUFFER_SIZE,
          Integer.getInteger("thrift.pending-tasks.count", Integer.MAX_VALUE));

  static final int numThreadsForEventLoop =
      Math.max(
          1,
          Integer.getInteger(
              "thrift.eventloop-threads.count", Runtime.getRuntime().availableProcessors()));
  static final int numThreadsForOffLoop =
      Math.max(
          numThreadsForEventLoop,
          Integer.getInteger(
              "thrift.executor-threads.count", Runtime.getRuntime().availableProcessors() * 4));

  static final int forkJoinPoolThreads =
      Math.max(
          numThreadsForEventLoop,
          Integer.getInteger(
              "thrift.forkjoin-threads.count", Runtime.getRuntime().availableProcessors() * 4));

  static final int forkJoinPoolClientThreads =
      Math.max(
          numThreadsForEventLoop,
          Integer.getInteger(
              "thrift.forkjoin-client-threads.count",
              Runtime.getRuntime().availableProcessors() * 4));

  static final int minNumThreadsForOffLoop = numThreadsForEventLoop;

  static final int minPendingTasksBeforeNewThread =
      Math.max(
          1, Integer.getInteger("thrift.min-pending-before-new-thread", Queues.XS_BUFFER_SIZE));

  static final String eventLoopGroupThreadPrefix = "thrift-eventloop";

  static final boolean separateOffLoopScheduler =
      System.getProperty("thrift.separate-offloop-scheduler", "false").equalsIgnoreCase("true");
}
