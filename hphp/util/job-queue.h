/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_UTIL_JOB_QUEUE_H_
#define incl_HPHP_UTIL_JOB_QUEUE_H_

#include <time.h>
#include <vector>
#include <set>
#include "hphp/util/alloc.h"
#include <boost/range/adaptors.hpp>
#include "hphp/util/async-func.h"
#include "hphp/util/atomic.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/exception.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/synchronizable-multi.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A queue-based multi-threaded job processing facility. Internally, we have a
 * queue of jobs and a list of workers, each of which runs in its own thread.
 * Job queue can take new jobs on the fly and workers will continue to pull
 * jobs off the queue and work on it.
 *
 * To use it, simply define your own job and worker class like this,
 *
 *   class MyJob {
 *     public:
 *       // storing job data
 *   };
 *
 *   class MyWorker : public JobQueueWorker<MyJob*> {
 *     public:
 *       virtual void doJob(MyJob *job) {
 *         // process the job
 *         delete job; // if it was new-ed
 *       }
 *   };
 *
 * Now, use JobQueueDispatcher to start the whole thing,
 *
 *   JobQueueDispatcher<MyJob*, MyWorker> dispatcher(40, NULL); // 40 threads
 *   dispatcher.start();
 *   ...
 *   dispatcher.enqueue(new MyJob(...));
 *   ...
 *   dispatcher.stop();
 *
 * Note this class is different from JobListDispatcher that uses a vector to
 * store prepared jobs. With JobQueueDispatcher, job queue is normally empty
 * initially and new jobs are pushed into the queue over time. Also, workers
 * can be stopped individually.
 *
 * Job process ordering
 * ====================
 * By default, requests are processed in FIFO order.
 *
 * In addition, we support an option where the request processing order can flip
 * between FIFO or LIFO based on the length of the queue. This can be enabled by
 * setting the 'lifoSwitchThreshold' parameter. If the job queue is configured
 * to be in FIFO mode, and the current queue length exceeds
 * lifoSwitchThreshold, then the workers will begin work on requests in LIFO
 * order until the queue size is below the threshold in which case we resume in
 * FIFO order. Setting the queue to be in LIFO mode initially will have the
 * opposite behavior. This is useful when we are in a loaded situation and we
 * want to prioritize the newest requests.
 *
 * You can configure a LIFO ordered queue by setting lifoSwitchThreshold to 0.
 */

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  struct NoDropCachePolicy { static void dropCache() {} };
}

/**
 * A job queue that's suitable for multiple threads to work on.
 */
template<typename TJob,
         bool waitable = false,
         class DropCachePolicy = detail::NoDropCachePolicy>
class JobQueue : public SynchronizableMulti {
public:
  // trivial class for signaling queue stop
  class StopSignal {};

public:
  /**
   * Constructor.
   */
  JobQueue(int threadCount, bool threadRoundRobin, int dropCacheTimeout,
           bool dropStack, int lifoSwitchThreshold=INT_MAX,
           int maxJobQueuingMs=-1, int numPriorities=1, int groups = 1)
      : SynchronizableMulti(threadRoundRobin ? 1 : threadCount, groups),
        m_jobCount(0), m_stopped(false), m_workerCount(0),
        m_dropCacheTimeout(dropCacheTimeout), m_dropStack(dropStack),
        m_lifoSwitchThreshold(lifoSwitchThreshold),
        m_maxJobQueuingMs(maxJobQueuingMs),
        m_jobReaperId(-1) {
    m_jobQueues.resize(numPriorities);
  }

  /**
   * Put a job into the queue and notify a worker to pick it up.
   */
  void enqueue(TJob job, int priority=0) {
    assert(priority >= 0);
    assert(priority < m_jobQueues.size());
    timespec enqueueTime;
    Timer::GetMonotonicTime(enqueueTime);
    Lock lock(this);
    m_jobQueues[priority].emplace_back(job, enqueueTime);
    ++m_jobCount;
    notify();
  }

  /**
   * Grab a job from the queue for processing. Since the job was not created
   * by this queue class, it's up to a worker class on whether to deallocate
   * the job object correctly.
   */
  TJob dequeueMaybeExpired(int id, int q, bool inc, bool* expired) {
    if (id == m_jobReaperId.load()) {
      *expired = true;
      return dequeueOnlyExpiredImpl(id, q, inc);
    }
    timespec now;
    Timer::GetMonotonicTime(now);
    return dequeueMaybeExpiredImpl(id, q, inc, now, expired);
  }

  /**
   * Purely for making sure no new jobs are queued when we are stopping.
   */
  void stop() {
    Lock lock(this);
    m_stopped = true;
    notifyAll(); // so all waiting threads can find out queue is stopped
  }

  void waitEmpty() {}
  void signalEmpty() {}

  /**
   * Keeps track of how many active workers are working on the queue.
   */
  void incActiveWorker() {
    ++m_workerCount;
  }
  int decActiveWorker() {
    return --m_workerCount;
  }
  int getActiveWorker() {
    return m_workerCount;
  }

  /**
   * Keep track of how many jobs are queued, but not yet been serviced.
   */
  int getQueuedJobs() {
    return m_jobCount;
  }

  /**
   * One worker can be designated as the job reaper. The job reaper's job is to
   * check if the oldest job on the queue has expired and if so, terminate that
   * job without processing it. When the job reaper work calls
   * dequeueMaybeExpired(), it'll only return the oldest job and only if it's
   * expired. Otherwise dequeueMaybeExpired() will block until a job expires.
   */
  void setJobReaperId(int id) {
    assert(m_maxJobQueuingMs > 0);
    m_jobReaperId.store(id);
  }

  int getJobReaperId() const {
    return m_jobReaperId.load();
  }

 private:
  friend class JobQueue_Expiration_Test;
  TJob dequeueMaybeExpiredImpl(int id, int q, bool inc, const timespec& now,
                               bool* expired) {
    *expired = false;
    Lock lock(this);
    bool flushed = false;
    while (m_jobCount == 0) {
      if (m_stopped) {
        throw StopSignal();
      }
      if (m_dropCacheTimeout <= 0 || flushed) {
        wait(id, q, false);
      } else if (!wait(id, q, true, m_dropCacheTimeout)) {
        // since we timed out, maybe we can turn idle without holding memory
        if (m_jobCount == 0) {
          ScopedUnlock unlock(this);
          Util::flush_thread_caches();
          if (m_dropStack && Util::s_stackLimit) {
            Util::flush_thread_stack();
          }
          DropCachePolicy::dropCache();
          flushed = true;
        }
      }
    }
    if (inc) incActiveWorker();
    --m_jobCount;

    // look across all our queues from highest priority to lowest.
    for (auto& jobs : boost::adaptors::reverse(m_jobQueues)) {
      if (jobs.empty()) {
        continue;
      }

      // peek at the beginning of the queue to see if the request has already
      // timed out.
      if (m_maxJobQueuingMs > 0 &&
          gettime_diff_us(jobs.front().second, now) >
          m_maxJobQueuingMs * 1000) {
        *expired = true;
        TJob job = jobs.front().first;
        jobs.pop_front();
        return job;
      }


      if (m_jobCount >= m_lifoSwitchThreshold) {
        TJob job = jobs.back().first;
        jobs.pop_back();
        return job;
      }
      TJob job = jobs.front().first;
      jobs.pop_front();
      return job;
    }
    assert(false);
    return TJob();  // make compiler happy.
  }

  TJob dequeueOnlyExpiredImpl(int id, int q, bool inc) {
    Lock lock(this);
    assert(m_maxJobQueuingMs > 0);
    while(!m_stopped) {
      long waitTimeUs = m_maxJobQueuingMs * 1000;

      for (auto& jobs : boost::adaptors::reverse(m_jobQueues)) {
        if (!jobs.empty()) {
          timespec now;
          Timer::GetMonotonicTime(now);
          int64_t queuedTimeUs = gettime_diff_us(jobs.front().second, now);
          if (queuedTimeUs > m_maxJobQueuingMs * 1000) {
            if (inc) incActiveWorker();
            --m_jobCount;

            TJob job = jobs.front().first;
            jobs.pop_front();
            return job;
          }
          // oldest job hasn't expired yet. wake us up when it will.
          long waitTimeForQueue = m_maxJobQueuingMs * 1000 - queuedTimeUs;
          waitTimeUs = ((waitTimeUs < waitTimeForQueue) ?
                        waitTimeUs :
                        waitTimeForQueue);
        }
      }
      if (wait(id, q, false, waitTimeUs / 1000000, waitTimeUs % 1000000)) {
        // We got woken up by somebody calling notify (as opposed to timeout),
        // then some work might be on the queue. We only expire things here,
        // so let's notify somebody else as well.
        notify();
      }
    }
    throw StopSignal();
  }

  int m_jobCount;
  std::vector<std::deque<std::pair<TJob, timespec>>> m_jobQueues;
  bool m_stopped;
  std::atomic<int> m_workerCount;
  const int m_dropCacheTimeout;
  const bool m_dropStack;
  const int m_lifoSwitchThreshold;
  const int m_maxJobQueuingMs;
  std::atomic<int> m_jobReaperId;
};

template<class TJob, class Policy>
struct JobQueue<TJob,true,Policy> : JobQueue<TJob,false,Policy> {
  JobQueue(int threadCount, bool threadRoundRobin, int dropCacheTimeout,
           bool dropStack, int lifoSwitchThreshold=INT_MAX,
           int maxJobQueuingMs=-1, int numPriorities=1, int groups = 1) :
    JobQueue<TJob,false,Policy>(threadCount,
                                threadRoundRobin,
                                dropCacheTimeout,
                                dropStack,
                                lifoSwitchThreshold,
                                maxJobQueuingMs,
                                numPriorities,
                                groups) {
    pthread_cond_init(&m_cond, nullptr);
  }
  ~JobQueue() {
    pthread_cond_destroy(&m_cond);
  }
  void waitEmpty() {
    Lock lock(this);
    while (this->getActiveWorker() || this->getQueuedJobs()) {
      pthread_cond_wait(&m_cond, &this->getMutex().getRaw());
    }
  }
  bool pollEmpty() {
    Lock lock(this);
    return !(this->getActiveWorker() || this->getQueuedJobs());
  }
  void signalEmpty() {
    pthread_cond_signal(&m_cond);
  }
private:
  pthread_cond_t m_cond;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Base class for a customized worker.
 *
 * DropCachePolicy is an extra callback for specific actions to take
 * when we decide to drop stack/caches.
 */
template<typename TJob,
         typename TContext = void*,
         bool countActive = false,
         bool waitable = false,
         class Policy = detail::NoDropCachePolicy>
class JobQueueWorker {
public:
  typedef TJob JobType;
  typedef TContext ContextType;
  typedef JobQueue<TJob, waitable, Policy> QueueType;
  typedef Policy DropCachePolicy;

  static const bool Waitable = waitable;
  static const bool CountActive = countActive;
  /**
   * Default constructor.
   */
  JobQueueWorker()
      : m_func(nullptr), m_context(), m_stopped(false), m_queue(nullptr) {
  }

  virtual ~JobQueueWorker() {
  }

  /**
   * Two-phase object creation for easier derivation and for JobQueueDispatcher
   * to easily create a vector of workers.
   */
  void create(int id, QueueType* queue, void *func, ContextType context) {
    assert(queue);
    m_id = id;
    m_queue = queue;
    m_func = func;
    m_context = context;
  }

  /**
   * The only functions a subclass needs to implement.
   */
  virtual void doJob(TJob job) = 0;
  virtual void abortJob(TJob job) {
    Logger::Warning("Job dropped by JobQueueDispatcher because of timeout.");
  }
  virtual void onThreadEnter() {}
  virtual void onThreadExit() {}

  /**
   * Start this worker thread.
   */
  void start() {
    assert(m_queue);
    onThreadEnter();
    while (!m_stopped) {
      try {
        bool expired = false;
        TJob job = m_queue->dequeueMaybeExpired(m_id, Util::s_numaNode,
                                                countActive, &expired);
        if (expired) {
          abortJob(job);
        } else {
          doJob(job);
        }
        if (countActive) {
          if (!m_queue->decActiveWorker() && waitable) {
            Lock lock(m_queue);
            if (!m_queue->getActiveWorker() &&
                !m_queue->getQueuedJobs()) {
              m_queue->signalEmpty();
            }
          }
        }
      } catch (const typename QueueType::StopSignal&) {
        m_stopped = true; // queue is empty and stopped, so we are done
      }
    }
    onThreadExit();
  }

  /**
   * Stop this worker thread.
   */
  void stop() {
    m_stopped = true;
  }

protected:
  int m_id;
  void *m_func;
  ContextType m_context;
  bool m_stopped;

private:
  QueueType* m_queue;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Driver class to push through the whole thing.
 */
template<class TWorker>
class JobQueueDispatcher {
public:
  /**
   * Constructor.
   */
  JobQueueDispatcher(int threadCount, bool threadRoundRobin,
                     int dropCacheTimeout, bool dropStack,
                     typename TWorker::ContextType context,
                     int lifoSwitchThreshold = INT_MAX,
                     int maxJobQueuingMs = -1, int numPriorities = 1,
                     int groups = 1)
      : m_stopped(true), m_id(0), m_context(context),
        m_maxThreadCount(threadCount),
        m_queue(threadCount, threadRoundRobin, dropCacheTimeout, dropStack,
                lifoSwitchThreshold, maxJobQueuingMs, numPriorities, groups),
        m_startReaperThread(maxJobQueuingMs > 0) {
    assert(threadCount >= 1);
    if (!TWorker::CountActive) {
      // If TWorker does not support counting the number of
      // active workers, just start all of the workers eagerly
      for (int i = 0; i < threadCount; i++) {
        addWorkerImpl(false);
      }
    }
  }

  ~JobQueueDispatcher() {
    stop();
    for (typename
           std::set<AsyncFunc<TWorker>*>::iterator iter = m_funcs.begin();
         iter != m_funcs.end(); ++iter) {
      delete *iter;
    }
    for (typename
           std::set<TWorker*>::iterator iter = m_workers.begin();
         iter != m_workers.end(); ++iter) {
      delete *iter;
    }
  }

  int getActiveWorker() {
    return m_queue.getActiveWorker();
  }

  int getQueuedJobs() {
    return m_queue.getQueuedJobs();
  }

  int getTargetNumWorkers() {
    if (TWorker::CountActive) {
      int target = getActiveWorker() + getQueuedJobs();
      return (target > m_maxThreadCount) ? m_maxThreadCount : target;
    } else {
      return m_maxThreadCount;
    }
  }

  /**
   * Creates worker threads and start running them. This is non-blocking.
   */
  void start() {
    Lock lock(m_mutex);
    // Spin up more worker threads if appropriate
    int target = getTargetNumWorkers();
    for (int n = m_workers.size(); n < target; ++n) {
      addWorkerImpl(false);
    }
    for (typename
           std::set<AsyncFunc<TWorker>*>::iterator iter = m_funcs.begin();
         iter != m_funcs.end(); ++iter) {
      (*iter)->start();
    }
    m_stopped = false;

    if (m_startReaperThread) {
      // If we have set a max timeout for requests on the queue, start a reaper
      // thread just for expiring off old requests so we guarantee requests are
      // taken off the queue as soon as possible when they expire even if all
      // other worker threads are stalled.
      m_queue.setJobReaperId(addWorkerImpl(true));
    }
  }

  /**
   * Enqueue a new job.
   */
  void enqueue(typename TWorker::JobType job, int priority = 0) {
    m_queue.enqueue(job, priority);
    // Spin up another worker thread if appropriate
    int target = getTargetNumWorkers();
    int n = m_workers.size();
    if (n < target) {
      addWorker();
    }
  }

  /**
   * Add a worker thread on the fly.
   */
  void addWorker() {
    Lock lock(m_mutex);
    if (!m_stopped) {
      addWorkerImpl(true);
    }
  }

  /*
   * Add N new worker threads.
   */
  void addWorkers(int n) {
    Lock lock(m_mutex);
    if (m_stopped) return;
    m_maxThreadCount += n;
    if (!TWorker::CountActive) {
      for (int i = 0; i < n; ++i) {
        addWorkerImpl(true);
      }
    } else {
      while (m_workers.size() < getTargetNumWorkers()) {
        addWorkerImpl(true);
      }
    }
  }

  void getWorkers(std::vector<TWorker*> &workers) {
    Lock lock(m_mutex);
    workers.insert(workers.end(), m_workers.begin(), m_workers.end());
  }

  void waitEmpty(bool stop = true) {
    if (m_stopped) return;
    m_queue.waitEmpty();
    if (stop) this->stop();
  }

  bool pollEmpty() {
    if (m_stopped) return true;
    return m_queue.pollEmpty();
  }

  /**
   * Stop all workers after all jobs are processed. No new jobs should be
   * enqueued at this moment, or this call may block for longer time.
   */
  void stop() {
    if (m_stopped) return;
    m_stopped = true;

    m_queue.stop();
    bool exceptioned = false;
    Exception exception;

    while (true) {
      AsyncFunc<TWorker> *func = nullptr;
      {
        Lock lock(m_mutex);
        if (!m_funcs.empty()) {
          func = *m_funcs.begin();
          m_funcs.erase(func);
        }
      }
      if (func == nullptr) {
        break;
      }
      try {
        func->waitForEnd();
      } catch (Exception &e) {
        exceptioned = true;
        exception = e;
      }
      delete func;
    }
    if (exceptioned) {
      throw exception;
    }
  }

  void run() {
    start();
    stop();
  }

private:
  bool m_stopped;
  int m_id;
  typename TWorker::ContextType m_context;
  int m_maxThreadCount;
  JobQueue<typename TWorker::JobType,
           TWorker::Waitable,
           typename TWorker::DropCachePolicy> m_queue;

  Mutex m_mutex;
  std::set<TWorker*> m_workers;
  std::set<AsyncFunc<TWorker> *> m_funcs;
  const bool m_startReaperThread;

  // return the id for the worker.
  int addWorkerImpl(bool start) {
    TWorker *worker = new TWorker();
    AsyncFunc<TWorker> *func = new AsyncFunc<TWorker>(worker, &TWorker::start);
    m_workers.insert(worker);
    m_funcs.insert(func);
    int id = m_id++;
    worker->create(id, &m_queue, func, m_context);

    if (start) {
      func->start();
    }
    return id;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
