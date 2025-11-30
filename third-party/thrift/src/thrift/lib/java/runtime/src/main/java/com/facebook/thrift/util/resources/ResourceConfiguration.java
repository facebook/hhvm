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

  /**
   * System property to control the type of off loop scheduler used to execute thrift requests.
   *
   * <ol>
   *   <li>thread-pool (default)
   *   <li>fork-join-pool
   *   <li>virtual-thread
   * </ol>
   *
   * <p>Thread pool scheduler, backed by ThreadPoolExecutor, has the lowest scalability of all
   * options but is the default for legacy compatibly reasons. Enqueued work is serviced by a single
   * LinkedBlockingQueue for tasks which increase contention across threads.
   *
   * <p>Fork Join Pool Scheduler, backed by a fork join pool has increased scalability due to work
   * stealing algorithm where each thread has its own queue but can steal from other queues when its
   * own is empty. This reduces contention across threads.
   *
   * <p>Virtual Thread Scheduler, backed by a Virtual Thread Per Task Executor is available on Java
   * 19+. This can increase scalability over the fork join pool and thread pool executor by allowing
   * progress even when blocking (network/io) or locking. Virtual threads are psychically backed by
   * a system-wide fork join pool, but are able to unmount when blocking/locking allowing other work
   * to be done.
   */
  static final String thriftOffLoopScheduler =
      System.getProperty("thrift.off-loop-scheduler", "thread-pool").toLowerCase();

  /**
   * When true enables us to limit the concurrency of the Virtual Thread Scheduler. By default,
   * virtual threads are run on a system-wide fork join pool that has `n` platform threads, one for
   * each CPU core. To avoid having to tune the JVM's system-wide fork join pool we'll employ a
   * semaphore for all work submitted to the thrift scheduler. This is a lightweight way of limiting
   * how much work can be done concurrently.
   */
  static final boolean limitVirtualThreadConcurrency =
      System.getProperty("thrift.virtual-thread-limit-concurrency", "false")
          .equalsIgnoreCase("true");

  static final boolean enableOperatorFusion =
      System.getProperty("thrift.operator-fusion", "true").equalsIgnoreCase("true");

  static final boolean forceExecutionOffEventLoop =
      System.getProperty("thrift.force-execution-off-eventloop", "true").equalsIgnoreCase("true");

  static final boolean forceClientExecutionOffEventLoop =
      System.getProperty("thrift.force-client-execution-off-eventloop", "true")
          .equalsIgnoreCase("true");

  static final int maxPendingTasksForOffLoop =
      Math.max(
          Queues.SMALL_BUFFER_SIZE,
          Integer.getInteger("thrift.pending-tasks.count", Integer.MAX_VALUE));

  static final int numThreadsForEventLoop =
      Math.max(
          1,
          Integer.getInteger(
              "thrift.eventloop-threads.count", Runtime.getRuntime().availableProcessors()));

  /**
   * Target concurrency level for the off loop scheduler, will cap the number of threads used by
   * either the thread pool or fork join pool, can be used to limit concurrency of the virtual
   * thread scheduler if used with -Dthrift.virtual-thread-limit-concurrency=true
   */
  static final int targetConcurrency =
      Math.max(
          numThreadsForEventLoop,
          Integer.getInteger(
              "thrift.scheduler-concurrency.count",
              Runtime.getRuntime().availableProcessors() * 4));

  /**
   * Target concurrency level for the client off loop scheduler, will cap the number of threads used
   * by either the thread pool or fork join pool
   *
   * <p>Note: Virtual threads will only use one scheduler and not intended ot be used with client
   * scheduler, thus we don't check for limitVirtualThreadConcurrency
   */
  static final int targetClientConcurrency =
      Math.max(
          numThreadsForEventLoop,
          Integer.getInteger(
              "thrift.client-scheduler-concurrency.count",
              Runtime.getRuntime().availableProcessors() * 4));

  static final boolean capForkJoinThreads =
      System.getProperty("thrift.enforce-forkjoin-parallelism-limit", "true")
          .equalsIgnoreCase("true");

  static final int minNumThreadsForOffLoop = numThreadsForEventLoop;

  static final int minPendingTasksBeforeNewThread =
      Math.max(
          1, Integer.getInteger("thrift.min-pending-before-new-thread", Queues.XS_BUFFER_SIZE));

  static final String eventLoopGroupThreadPrefix = "thrift-eventloop";

  static final boolean separateOffLoopScheduler =
      System.getProperty("thrift.separate-offloop-scheduler", "false").equalsIgnoreCase("true");
}
