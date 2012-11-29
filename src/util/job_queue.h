/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <vector>
#include <set>
#include "util/async_func.h"
#include "util/synchronizable_multi.h"
#include "util/lock.h"
#include "util/atomic.h"
#include "util/alloc.h"
#include "util/exception.h"

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
           bool dropStack, bool lifo)
      : SynchronizableMulti(threadRoundRobin ? 1 : threadCount),
        m_jobCount(0), m_stopped(false), m_workerCount(0),
        m_dropCacheTimeout(dropCacheTimeout), m_dropStack(dropStack),
        m_lifo(lifo) {
  }

  /**
   * Put a job into the queue and notify a worker to pick it up.
   */
  void enqueue(TJob job) {
    Lock lock(this);
    m_jobs.push_back(job);
    m_jobCount = m_jobs.size();
    notify();
  }

  /**
   * Grab a job from the queue for processing. Since the job was not created
   * by this queue class, it's up to a worker class on whether to deallocate
   * the job object correctly.
   */
  TJob dequeue(int id, bool inc = false) {
    Lock lock(this);
    bool flushed = false;
    while (m_jobs.empty()) {
      if (m_stopped) {
        throw StopSignal();
      }
      if (m_dropCacheTimeout <= 0 || flushed) {
        wait(id, false);
      } else if (!wait(id, true, m_dropCacheTimeout)) {
        // since we timed out, maybe we can turn idle without holding memory
        if (m_jobs.empty()) {
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
    m_jobCount = m_jobs.size() - 1;
    if (m_lifo) {
      TJob job = m_jobs.back();
      m_jobs.pop_back();
      return job;
    }
    TJob job = m_jobs.front();
    m_jobs.pop_front();
    return job;
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
    atomic_inc(m_workerCount);
  }
  int decActiveWorker() {
    return atomic_dec(m_workerCount);
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

 private:
  int m_jobCount;
  std::deque<TJob> m_jobs;
  bool m_stopped;
  int m_workerCount;
  int m_dropCacheTimeout;
  bool m_dropStack;
  bool m_lifo;
};

template<class TJob, class Policy>
struct JobQueue<TJob,true,Policy> : JobQueue<TJob,false,Policy> {
  JobQueue(int threadCount, bool threadRoundRobin, int dropCacheTimeout,
           bool dropStack, bool lifo) :
    JobQueue<TJob,false,Policy>(threadCount,
                                threadRoundRobin,
                                dropCacheTimeout,
                                dropStack,
                                lifo) {
    pthread_cond_init(&m_cond, NULL);
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
         bool countActive = false,
         bool waitable = false,
         class Policy = detail::NoDropCachePolicy>
class JobQueueWorker {
public:
  typedef TJob JobType;
  typedef JobQueue<TJob,waitable,Policy> QueueType;
  typedef Policy DropCachePolicy;

  static const bool Waitable = waitable;
  static const bool CountActive = countActive;
  /**
   * Default constructor.
   */
  JobQueueWorker()
      : m_func(NULL), m_opaque(NULL), m_stopped(false), m_queue(NULL) {
  }

  virtual ~JobQueueWorker() {
  }

  /**
   * Two-phase object creation for easier derivation and for JobQueueDispatcher
   * to easily create a vector of workers.
   */
  void create(int id, QueueType* queue, void *func, void *opaque) {
    ASSERT(queue);
    m_id = id;
    m_queue = queue;
    m_func = func;
    m_opaque = opaque;
  }

  /**
   * The only functions a subclass needs to implement.
   */
  virtual void doJob(TJob job) = 0;
  virtual void onThreadEnter() {}
  virtual void onThreadExit() {}

  /**
   * Start this worker thread.
   */
  void start() {
    ASSERT(m_queue);
    onThreadEnter();
    while (!m_stopped) {
      try {
        TJob job = m_queue->dequeue(m_id, countActive);
        doJob(job);
        if (countActive) {
          if (!m_queue->decActiveWorker() && waitable) {
            Lock lock(m_queue);
            if (!m_queue->getActiveWorker() &&
                !m_queue->getQueuedJobs()) {
              m_queue->signalEmpty();
            }
          }
        }
      } catch (typename QueueType::StopSignal) {
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
  void *m_opaque;
  bool m_stopped;

private:
  QueueType* m_queue;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Driver class to push through the whole thing.
 */
template<class TJob, class TWorker>
class JobQueueDispatcher {
public:
  /**
   * Constructor.
   */
  JobQueueDispatcher(int threadCount, bool threadRoundRobin,
                     int dropCacheTimeout, bool dropStack, void *opaque,
                     bool lifo = false)
      : m_stopped(true), m_id(0), m_opaque(opaque),
        m_maxThreadCount(threadCount),
        m_queue(threadCount, threadRoundRobin, dropCacheTimeout, dropStack,
                lifo) {
    ASSERT(threadCount >= 1);
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
  }

  /**
   * Enqueue a new job.
   */
  void enqueue(TJob job) {
    m_queue.enqueue(job);
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

  void getWorkers(std::vector<TWorker*> &workers) {
    Lock lock(m_mutex);
    workers.insert(workers.end(), m_workers.begin(), m_workers.end());
  }

  void waitEmpty(bool stop = true) {
    if (m_stopped) return;
    m_queue.waitEmpty();
    if (stop) this->stop();
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
      AsyncFunc<TWorker> *func = NULL;
      {
        Lock lock(m_mutex);
        if (!m_funcs.empty()) {
          func = *m_funcs.begin();
          m_funcs.erase(func);
        }
      }
      if (func == NULL) {
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
  void *m_opaque;
  int m_maxThreadCount;
  JobQueue<TJob,
           TWorker::Waitable,
           typename TWorker::DropCachePolicy> m_queue;

  Mutex m_mutex;
  std::set<TWorker*> m_workers;
  std::set<AsyncFunc<TWorker> *> m_funcs;

  void addWorkerImpl(bool start) {
    TWorker *worker = new TWorker();
    AsyncFunc<TWorker> *func = new AsyncFunc<TWorker>(worker, &TWorker::start);
    m_workers.insert(worker);
    m_funcs.insert(func);
    worker->create(m_id++, &m_queue, func, m_opaque);

    if (start) {
      func->start();
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
