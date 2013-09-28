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

#ifndef incl_HPHP_ASYNC_JOB_H_
#define incl_HPHP_ASYNC_JOB_H_

#include "hphp/util/base.h"
#include "hphp/util/async-func.h"
#include "hphp/util/lock.h"
#include <algorithm>
#include <sys/time.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helper, skip to JobDispatcher at below to read comments and usages...

template<class TJob, class TWorker>
class JobDispatcher;

template<class TJob>
class WorkerInfo {
 public:
  enum { DoInit = true };
};

template<class TJob, class TWorker>
class WorkerWrapper {
public:
  explicit WorkerWrapper(JobDispatcher<TJob, TWorker> &dispatcher)
    : m_dispatcher(dispatcher)
    , m_func(this, &WorkerWrapper<TJob, TWorker>::doJob)
  {
    if (!WorkerInfo<TJob>::DoInit) {
      m_func.setNoInit();
    }
  }

  TWorker *getWorker() { return &m_worker;}
  AsyncFunc<WorkerWrapper<TJob, TWorker> > &getAsyncFunc() {
    return m_func;
  }

  void doJob() {
    m_worker.onThreadEnter();
    while (true) {
      std::shared_ptr<TJob> job = m_dispatcher.getNextJob();
      if (!job) break;
      m_worker.doJob(job);
    }
    m_worker.onThreadExit();
  }

private:
  JobDispatcher<TJob, TWorker> &m_dispatcher;
  AsyncFunc<WorkerWrapper<TJob, TWorker> > m_func;
  TWorker m_worker;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * The promise is to make job processing cross multi-worker as easy as
 * possible. The final syntax is like,
 *
 * MyJobPtrVec jobs;
 * // prepare job list in "jobs"
 *
 * JobDispatcher<MyJob, MyWorker>(jobs, 10).run();
 *
 * or
 *
 * JobDispatcher<MyJob, MyWorker> dispatcher(jobs, 10);
 * dispatcher.start();
 * // do something else;
 * dispatcher.waitForEnd();
 *
 *
 * The only requirement is MyWorker has to be a class that implements this:
 *
 *  class MyWorker {
 *   public:
 *     void doJob(MyJobPtr job);
 *  };
 */
template<class TJob, class TWorker>
class JobDispatcher {
 public:
  JobDispatcher(std::vector<std::shared_ptr<TJob> > &jobs,
                unsigned int workerCount, bool showStatus = false)
    : m_index(0), m_jobs(jobs), m_showStatus(showStatus), m_lastPercent(0) {
    std::random_shuffle(m_jobs.begin(), m_jobs.end());
    if (workerCount > jobs.size()) {
      workerCount = jobs.size();
    }
    m_workers.resize(workerCount);
    for (unsigned int i = 0; i < m_workers.size(); i++) {
      m_workers[i] = std::shared_ptr<WorkerWrapper<TJob, TWorker> >
        (new WorkerWrapper<TJob, TWorker>(*this));
    }
  }

  /**
   * Start job processing asynchronously.
   */
  void start() {
    gettimeofday(&m_start, 0);
    for (unsigned int i = 0; i < m_workers.size(); i++) {
      m_workers[i]->getAsyncFunc().start();
    }
  }

  /**
   * Wait for all jobs finish running.
   */
  void waitForEnd() {
    for (unsigned int i = 0; i < m_workers.size(); i++) {
      m_workers[i]->getAsyncFunc().waitForEnd();
    }
  }

  /**
   * Start job processing and wait for all jobs finish running.
   */
  void run() {
    if (m_workers.size() == 1) {
      m_workers[0]->doJob();
    } else {
      start();
      waitForEnd();
    }
  }

  unsigned int getWorkerCount() {
    return m_workers.size();
  }

  TWorker *getWorker(unsigned int i) {
    assert(i < m_workers.size());
    return m_workers[i]->getWorker();
  }

  void setStatus(int percent) {
    if (percent > m_lastPercent) {
      struct timeval tv;
      gettimeofday(&tv, 0);
      long diff = (tv.tv_sec  - m_start.tv_sec ) * 1000000 +
        (tv.tv_usec - m_start.tv_usec);
      m_lastPercent = percent;
      int seconds = diff / (percent * 10000);
      printf("%d%% (ETA: %d sec or %d min or %d hours)\n", percent,
             seconds, seconds/60, seconds/3600);
    }
  }

  std::shared_ptr<TJob> getNextJob() {
    Lock lock(m_mutex);
    if (m_index >= m_jobs.size()) {
      return std::shared_ptr<TJob>();
    }
    if (m_showStatus && m_index > m_workers.size()) {
      setStatus((m_index - m_workers.size()) * 100 / m_jobs.size());
    }
    return m_jobs[m_index++];
  }

 private:
  Mutex m_mutex;
  unsigned int m_index;
  std::vector<std::shared_ptr<TJob> > &m_jobs;
  std::vector<std::shared_ptr<WorkerWrapper<TJob, TWorker> > > m_workers;
  bool m_showStatus;
  int m_lastPercent;
  struct timeval m_start;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ASYNC_JOB_H_
