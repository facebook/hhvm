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

#include "thrift/lib/cpp/concurrency/PosixThreadFactory.h"
#include "thrift/lib/cpp/concurrency/Exception.h"
#include "thrift/lib/cpp/concurrency/Mutex.h"
#include "thrift/lib/cpp/concurrency/SpinLock.h"
#include "thrift/lib/cpp/config.h"

#if GOOGLE_PERFTOOLS_REGISTER_THREAD
#  include "base/Profiler.h"
#endif

#include <assert.h>
#include <pthread.h>
#include <sys/resource.h>

#include <iostream>

#include <boost/weak_ptr.hpp>

namespace apache { namespace thrift { namespace concurrency {

using std::shared_ptr;
using std::weak_ptr;

const PosixThreadFactory::POLICY PosixThreadFactory::kDefaultPolicy;
const PosixThreadFactory::PRIORITY PosixThreadFactory::kDefaultPriority;
const int PosixThreadFactory::kDefaultStackSizeMB;

// global set to track the ids of live threads.
std::set<pthread_t> liveThreadIds;
Mutex liveThreadIdMutex;

void addLiveThreadId(pthread_t tid) {
  Guard g(liveThreadIdMutex);
  liveThreadIds.insert(tid);
}

void removeLiveThreadId(pthread_t tid) {
  Guard g(liveThreadIdMutex);
  liveThreadIds.erase(tid);
}

void getLiveThreadIds(std::set<pthread_t>* tids) {
  Guard g(liveThreadIdMutex);
  for (std::set<pthread_t>::const_iterator it = liveThreadIds.begin();
       it != liveThreadIds.end(); ++it) {
    tids->insert(*it);
  }
}

#if (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 12))
bool setPosixThreadName(pthread_t id, const std::string& name) {
  return 0 == pthread_setname_np(id, name.substr(0, 15).c_str());
}
#else
bool setPosixThreadName(pthread_t id, const std::string& name) {
  return false;
}
#endif

/**
 * The POSIX thread class.
 *
 * @version $Id:$
 */
class PthreadThread: public Thread {
 public:

  enum STATE {
    uninitialized,
    starting,
    started,
    stopping,
    stopped
  };

  static const int MB = 1024 * 1024;

  static void* threadMain(void* arg);

 private:
  pthread_t pthread_;
  STATE state_;
  int policy_;
  int priority_;
  int stackSize_;
  weak_ptr<PthreadThread> self_;
  bool detached_;
  SpinLock stateLock_;

 public:

  PthreadThread(int policy, int priority, int stackSize, bool detached, shared_ptr<Runnable> runnable) :
    pthread_(0),
    state_(uninitialized),
    policy_(policy),
    priority_(priority),
    stackSize_(stackSize),
    detached_(detached) {

    this->Thread::runnable(runnable);
  }

  ~PthreadThread() {
    /* Nothing references this thread, if is is not detached, do a join
       now, otherwise the thread-id and, possibly, other resources will
       be leaked. */
    if(!detached_) {
      try {
        join();
      } catch(...) {
        // We're really hosed.
      }
    }
    removeLiveThreadId(pthread_);
  }

  void start() {
    stateLock_.lock();

    if (state_ != uninitialized) {
      return;
    }

    pthread_attr_t thread_attr;
    if (pthread_attr_init(&thread_attr) != 0) {
        throw SystemResourceException("pthread_attr_init failed");
    }

    if(pthread_attr_setdetachstate(&thread_attr,
                                   detached_ ?
                                   PTHREAD_CREATE_DETACHED :
                                   PTHREAD_CREATE_JOINABLE) != 0) {
        throw SystemResourceException("pthread_attr_setdetachstate failed");
    }

    // Set thread stack size
    if (pthread_attr_setstacksize(&thread_attr, MB * stackSize_) != 0) {
      throw SystemResourceException("pthread_attr_setstacksize failed");
    }

    // Create reference
    shared_ptr<PthreadThread>* selfRef = new shared_ptr<PthreadThread>();
    *selfRef = self_.lock();

    state_ = starting;
    stateLock_.unlock();

    if (pthread_create(&pthread_, &thread_attr, threadMain, (void*)selfRef) != 0) {
      throw SystemResourceException("pthread_create failed");
    }
    addLiveThreadId(pthread_);
  }

  void join() {
    stateLock_.lock();
    STATE join_state = state_;
    stateLock_.unlock();

    if (!detached_ && join_state != uninitialized) {
      void* ignore;
      /* XXX
         If join fails it is most likely due to the fact
         that the last reference was the thread itself and cannot
         join.  This results in leaked threads and will eventually
         cause the process to run out of thread resources.
         We're beyond the point of throwing an exception.  Not clear how
         best to handle this. */
      int res = pthread_join(pthread_, &ignore);
      detached_ = (res == 0);
      if (res != 0) {
        GlobalOutput.printf("PthreadThread::join(): fail with code %d", res);
      }
    } else {
      GlobalOutput.printf("PthreadThread::join(): detached thread");
    }
  }

  Thread::id_t getId() {
    return (Thread::id_t)pthread_;
  }

  shared_ptr<Runnable> runnable() const { return Thread::runnable(); }

  void runnable(shared_ptr<Runnable> value) { Thread::runnable(value); }

  void weakRef(shared_ptr<PthreadThread> self) {
    assert(self.get() == this);
    self_ = weak_ptr<PthreadThread>(self);
  }

  bool setName(const std::string& name) {
    if (!pthread_) {
      return false;
    }
    return setPosixThreadName(pthread_, name);
  }
};

void* PthreadThread::threadMain(void* arg) {
  shared_ptr<PthreadThread> thread = *(shared_ptr<PthreadThread>*)arg;
  delete reinterpret_cast<shared_ptr<PthreadThread>*>(arg);

  if (thread == nullptr) {
    return (void*)0;
  }

  thread->stateLock_.lock();

  if (thread->state_ != starting) {
    return (void*)0;
  }

#if GOOGLE_PERFTOOLS_REGISTER_THREAD
  ProfilerRegisterThread();
#endif
  // Using pthread_attr_setschedparam() at thread creation doesn't actually
  // change the new thread's priority for some reason... Other people on the
  // 'net also complain about it.  The solution is to set priority inside the
  // new thread's threadMain.
  if (thread->policy_ == SCHED_FIFO || thread->policy_ == SCHED_RR) {
    struct sched_param sched_param;
    sched_param.sched_priority = thread->priority_;
    int err =
      pthread_setschedparam(pthread_self(), thread->policy_, &sched_param);
    if (err != 0) {
      GlobalOutput.printf("pthread_setschedparam failed (are you root?) "
        "with error %d: %s", err, strerror(err));
    }
  } else if (thread->policy_ == SCHED_OTHER) {
    if (setpriority(PRIO_PROCESS, 0, thread->priority_) != 0) {
      GlobalOutput.printf("setpriority failed (are you root?) "
        "with error %d: %s", errno, strerror(errno));
    }
  }

  thread->state_ = starting;
  thread->runnable()->run();
  if (thread->state_ != stopping && thread->state_ != stopped) {
    thread->state_ = stopping;
  }

  thread->stateLock_.unlock();

  return (void*)0;
}

/**
 * POSIX Thread factory implementation
 */
class PosixThreadFactory::Impl {

 private:
  POLICY policy_;
  PRIORITY priority_;
  int stackSize_;
  DetachState detached_;

  /**
   * Converts generic posix thread schedule policy enums into pthread
   * API values.
   */
  static int toPthreadPolicy(POLICY policy) {
    switch (policy) {
    case OTHER:
      return SCHED_OTHER;
    case FIFO:
      return SCHED_FIFO;
    case ROUND_ROBIN:
      return SCHED_RR;
    }
    return SCHED_OTHER;
  }

  /**
   * Converts relative thread priorities to absolute value based on posix
   * thread scheduler policy
   *
   *  The idea is simply to divide up the priority range for the given policy
   * into the corresponding relative priority level (lowest..highest) and
   * then pro-rate accordingly.
   */
  static int toPthreadPriority(POLICY policy, PRIORITY priority) {
    int pthread_policy = toPthreadPolicy(policy);
    int min_priority = 0;
    int max_priority = 0;
    if (pthread_policy == SCHED_FIFO || pthread_policy == SCHED_RR) {
#ifdef HAVE_SCHED_GET_PRIORITY_MIN
      min_priority = sched_get_priority_min(pthread_policy);
#endif
#ifdef HAVE_SCHED_GET_PRIORITY_MAX
      max_priority = sched_get_priority_max(pthread_policy);
#endif
    } else if (pthread_policy == SCHED_OTHER) {
      min_priority = 19;
      max_priority = -20;
    }
    int quanta = HIGHEST - LOWEST;
    float stepsperquanta = (float)(max_priority - min_priority) / quanta;

    if (priority <= HIGHEST) {
      return (int)(min_priority + stepsperquanta * priority);
    } else {
      // should never get here for priority increments.
      assert(false);
      return (int)(min_priority + stepsperquanta * NORMAL);
    }
  }

 public:

  Impl(POLICY policy, PRIORITY priority, int stackSize, DetachState detached) :
    policy_(policy),
    priority_(priority),
    stackSize_(stackSize),
    detached_(detached) {}

  /**
   * Creates a new POSIX thread to run the runnable object
   *
   * @param runnable A runnable object
   */
  shared_ptr<Thread> newThread(const shared_ptr<Runnable>& runnable,
                               DetachState detachState) const {
    shared_ptr<PthreadThread> result = shared_ptr<PthreadThread>(
        new PthreadThread(toPthreadPolicy(policy_),
                          toPthreadPriority(policy_, priority_), stackSize_,
                          detachState == DETACHED, runnable));
    result->weakRef(result);
    runnable->thread(result);
    return result;
  }

  int getStackSize() const { return stackSize_; }

  void setStackSize(int value) { stackSize_ = value; }

  PRIORITY getPriority() const { return priority_; }

  /**
   * Sets priority.
   *
   *  XXX
   *  Need to handle incremental priorities properly.
   */
  void setPriority(PRIORITY value) { priority_ = value; }

  DetachState getDetachState() const { return detached_; }

  void setDetachState(DetachState value) { detached_ = value; }

  Thread::id_t getCurrentThreadId() const {
    return static_cast<Thread::id_t>(pthread_self());
  }

};

PosixThreadFactory::PosixThreadFactory(POLICY policy, PRIORITY priority,
                                       int stackSize, bool detached)
  : impl_(new PosixThreadFactory::Impl(policy, priority, stackSize,
                                       detached ? DETACHED : ATTACHED)) {
}

PosixThreadFactory::PosixThreadFactory(DetachState detached)
  : impl_(new PosixThreadFactory::Impl(kDefaultPolicy, kDefaultPriority,
                                       kDefaultStackSizeMB, detached)) {
}

shared_ptr<Thread> PosixThreadFactory::newThread(
    const shared_ptr<Runnable>& runnable) const {
  return impl_->newThread(runnable, impl_->getDetachState());
}

shared_ptr<Thread> PosixThreadFactory::newThread(
    const shared_ptr<Runnable>& runnable,
    DetachState detachState) const {
  return impl_->newThread(runnable, detachState);
}

int PosixThreadFactory::getStackSize() const { return impl_->getStackSize(); }

void PosixThreadFactory::setStackSize(int value) { impl_->setStackSize(value); }

PosixThreadFactory::PRIORITY PosixThreadFactory::getPriority() const { return impl_->getPriority(); }

void PosixThreadFactory::setPriority(PosixThreadFactory::PRIORITY value) { impl_->setPriority(value); }

bool PosixThreadFactory::isDetached() const {
  return impl_->getDetachState() == DETACHED;
}

void PosixThreadFactory::setDetached(bool value) {
  impl_->setDetachState(value ? DETACHED : ATTACHED);
}

void PosixThreadFactory::setDetached(DetachState value) {
  impl_->setDetachState(value);
}

Thread::id_t PosixThreadFactory::getCurrentThreadId() const { return impl_->getCurrentThreadId(); }

}}} // apache::thrift::concurrency
