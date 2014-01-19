/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef HPHP_THRIFT_CONCURRENCY_THREADMANAGER_H
#define HPHP_THRIFT_CONCURRENCY_THREADMANAGER_H 1

#include <memory>
#include <tr1/functional>
#include <sys/types.h>
#include <array>
#include <unistd.h>

namespace apache { namespace thrift { namespace concurrency {

class Runnable;
class ThreadFactory;

/**
 * ThreadManager class
 *
 * This class manages a pool of threads. It uses a ThreadFactory to create
 * threads. It never actually creates or destroys worker threads, rather
 * It maintains statistics on number of idle threads, number of active threads,
 * task backlog, and average wait and service times and informs the PoolPolicy
 * object bound to instances of this manager of interesting transitions. It is
 * then up the PoolPolicy object to decide if the thread pool size needs to be
 * adjusted and call this object addWorker and removeWorker methods to make
 * changes.
 *
 * This design allows different policy implementations to used this code to
 * handle basic worker thread management and worker task execution and focus on
 * policy issues. The simplest policy, StaticPolicy, does nothing other than
 * create a fixed number of threads.
 */
class ThreadManager {

 protected:
  ThreadManager() {}

 public:
  class Task;
  typedef std::tr1::function<void(std::shared_ptr<Runnable>)> ExpireCallback;
  typedef std::tr1::function<void()> InitCallback;

  virtual ~ThreadManager() {}

  /**
   * Starts the thread manager. Verifies all attributes have been properly
   * initialized, then allocates necessary resources to begin operation
   */
  virtual void start() = 0;

  /**
   * Stops the thread manager. Aborts all remaining unprocessed task, shuts
   * down all created worker threads, and releases all allocated resources.
   * This method blocks for all worker threads to complete, thus it can
   * potentially block forever if a worker thread is running a task that
   * won't terminate.
   */
  virtual void stop() = 0;

  /**
   * Joins the thread manager. This is the same as stop, except that it will
   * wait until all the tasks have finished, rather than aborting the tasks.
   */
  virtual void join() = 0;

  enum STATE {
    UNINITIALIZED,
    STARTING,
    STARTED,
    JOINING,
    STOPPING,
    STOPPED
  };

  virtual const STATE state() const = 0;

  virtual std::shared_ptr<ThreadFactory> threadFactory() const = 0;

  virtual void threadFactory(std::shared_ptr<ThreadFactory> value) = 0;

  virtual void setNamePrefix(const std::string& name) = 0;

  virtual void addWorker(size_t value=1) = 0;

  virtual void removeWorker(size_t value=1) = 0;

  /**
   * Gets the current number of idle worker threads
   */
  virtual size_t idleWorkerCount() const = 0;

  /**
   * Gets the current number of total worker threads
   */
  virtual size_t workerCount() const = 0;

  /**
   * Gets the current number of pending tasks
   */
  virtual size_t pendingTaskCount() const  = 0;

  /**
   * Gets the current number of pending and executing tasks
   */
  virtual size_t totalTaskCount() const = 0;

  /**
   * Gets the maximum pending task count.  0 indicates no maximum
   */
  virtual size_t pendingTaskCountMax() const = 0;

  /**
   * Gets the number of tasks which have been expired without being run.
   */
  virtual size_t expiredTaskCount() = 0;

  /**
   * Adds a task to be executed at some time in the future by a worker thread.
   *
   * This method will block if pendingTaskCountMax() in not zero and
   * pendingTaskCount() is greater than or equal to pendingTaskCountMax().  If
   * this method is called in the context of a ThreadManager worker thread it
   * will throw a TooManyPendingTasksException
   *
   * @param task  The task to queue for execution
   *
   * @param timeout Time to wait in milliseconds to add a task when a
   * pending-task-count is specified. Specific cases:
   * timeout = 0  : Wait forever to queue task.
   * timeout = -1 : Return immediately if pending task count exceeds specified
   * max
   * @param expiration when nonzero, the number of milliseconds the task is
   * valid to be run; if exceeded, the task will be dropped off the queue and
   * not run.
   *
   * @throws TooManyPendingTasksException Pending task count exceeds max
   * pending task count
   */
  virtual void add(std::shared_ptr<Runnable>task,
                   int64_t timeout=0LL,
                   int64_t expiration=0LL) = 0;

  /**
   * Removes a pending task
   */
  virtual void remove(std::shared_ptr<Runnable> task) = 0;

  /**
   * Remove the next pending task which would be run.
   *
   * @return the task removed.
   */
  virtual std::shared_ptr<Runnable> removeNextPending() = 0;

  /**
   * Set a callback to be called when a task is expired and not run.
   *
   * @param expireCallback a function called with the shared_ptr<Runnable> for
   * the expired task.
   */
  virtual void setExpireCallback(ExpireCallback expireCallback) = 0;

  /**
   * Set a callback to be called when a worker thread is created.
   */
  virtual void setThreadInitCallback(InitCallback initCallback) = 0;

  static std::shared_ptr<ThreadManager> newThreadManager();

  /**
   * Creates a simple thread manager the uses count number of worker threads
   * and has a pendingTaskCountMax maximum pending tasks. The default, 0,
   * specified no limit on pending tasks. maxQueueLen is the maximum total
   * number of non-completed tasks ThreadManager can handle. 0 means a large
   * default.
   */
  static std::shared_ptr<ThreadManager>
    newSimpleThreadManager(size_t count = 4,
                           size_t pendingTaskCountMax = 0,
                           bool enableTaskStats = false,
                           size_t maxQueueLen = 0);

  /**
   * Get an internal statistics.
   *
   * @param waitTimeUs - average time (us) task spent in a queue
   * @param runTimeUs - average time (us) task spent running
   * @param maxItems - max items collected for stats
   */
  virtual void getStats(int64_t& waitTimeUs, int64_t& runTimeUs,
                        int64_t maxItems) {
    waitTimeUs = 0;
    runTimeUs = 0;
  }

  class Worker;

  class Impl;
};

/**
 * PriorityThreadManager class
 *
 * This class extends ThreadManager by adding priorities to tasks.
 * It is up to the specific implementation to define how worker threads
 * are assigned to run tasks of various priorities. The current
 * implementation, PriorityImpl, runs a task on a thread with exactly
 * the same priority.
 *
 * The interface of this class was kept as close to ThreadManager's
 * as possible, diverging only where the original interface doesn't
 * make sense in the priority world.
 */
class PriorityThreadManager : public ThreadManager {
public:
  enum PRIORITY {
    HIGH_IMPORTANT,
    HIGH,
    IMPORTANT,
    NORMAL,
    BEST_EFFORT,
    N_PRIORITIES
  };

  using ThreadManager::addWorker;
  virtual void addWorker(PRIORITY priority, size_t value) = 0;

  using ThreadManager::removeWorker;
  virtual void removeWorker(PRIORITY priority, size_t value) = 0;

  using ThreadManager::add;
  virtual void add(PRIORITY priority,
                   std::shared_ptr<Runnable>task,
                   int64_t timeout=0LL,
                   int64_t expiration=0LL) = 0;

  /**
   * Creates a priority-aware thread manager that uses counts[X]
   * worker threads for priority X.
   */
  static std::shared_ptr<PriorityThreadManager>
    newPriorityThreadManager(std::array<size_t, N_PRIORITIES> counts,
                             bool enableTaskStats = false);

  /**
   * Creates a priority-aware thread manager that uses normalThreadsCount
   * threads of NORMAL priority, and an implementation-defined number of threads
   * for other priorities. Useful when the vast majority of tasks are NORMAL,
   * but occasionally a non-NORMAL task may arrive.
   *
   * @param normalThreadsCount - number of threads of NORMAL priority, defaults
   *          to the number of CPUs on the system
   */
  static std::shared_ptr<PriorityThreadManager>
    newPriorityThreadManager(size_t normalThreadsCount
                                      = sysconf(_SC_NPROCESSORS_ONLN),
                             bool enableTaskStats = false);

  class PriorityImpl;
};

}}} // apache::thrift::concurrency

#endif // #ifndef HPHP_THRIFT_CONCURRENCY_THREADMANAGER_H
