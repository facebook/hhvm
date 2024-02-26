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

#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>

#include <cassert>
#include <ctime>
#include <iostream>

#include <glog/logging.h>

#include <folly/String.h>
#include <folly/portability/PThread.h>
#include <folly/portability/SysResource.h>
#include <folly/system/ThreadId.h>
#include <folly/system/ThreadName.h>

#include <thrift/lib/cpp/concurrency/Exception.h>
#include <thrift/lib/cpp/thrift_config.h>

namespace apache {
namespace thrift {
namespace concurrency {

using std::shared_ptr;
using std::weak_ptr;

const PosixThreadFactory::POLICY PosixThreadFactory::kDefaultPolicy;
const PosixThreadFactory::THREAD_PRIORITY PosixThreadFactory::kDefaultPriority;

// push our given name upstream into pthreads
bool PthreadThread::updateName() {
  if (!pthread_ || name_.empty()) {
    return false;
  }
  return folly::setThreadName(pthread_, name_);
}

PthreadThread::PthreadThread(
    int policy,
    int priority,
    std::optional<int> stackSize,
    bool detached,
    shared_ptr<Runnable> runnable)
    : pthread_(),
      state_(uninitialized),
      policy_(policy),
      priority_(priority),
      stackSize_(stackSize),
      detached_(detached) {
  this->Thread::runnable(runnable);
}

PthreadThread::~PthreadThread() {
  /* Nothing references this thread, if is is not detached, do a join
     now, otherwise the thread-id and, possibly, other resources will
     be leaked. */
  if (!detached_) {
    try {
      join();
    } catch (...) {
      // We're really hosed.
    }
  }
}

void PthreadThread::start() {
  std::unique_lock<std::mutex> g(stateLock_);

  if (state_ != uninitialized) {
    return;
  }

  pthread_attr_t thread_attr;
  if (pthread_attr_init(&thread_attr) != 0) {
    throw SystemResourceException("pthread_attr_init failed");
  }

  if (pthread_attr_setdetachstate(
          &thread_attr,
          detached_ ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE) != 0) {
    throw SystemResourceException("pthread_attr_setdetachstate failed");
  }

  // Set thread stack size
  if (stackSize_ &&
      pthread_attr_setstacksize(&thread_attr, MB * *stackSize_) != 0) {
    throw SystemResourceException("pthread_attr_setstacksize failed");
  }

  // Create reference
  shared_ptr<PthreadThread>* selfRef = new shared_ptr<PthreadThread>();
  *selfRef = self_.lock();

  state_ = starting;

  if (pthread_create(&pthread_, &thread_attr, threadMain, (void*)selfRef) !=
      0) {
    throw SystemResourceException("pthread_create failed");
  }

  // Now that we have a thread, we can set the name we've been given, if any.
  updateName();
}

void PthreadThread::join() {
  std::unique_lock<std::mutex> g(stateLock_);
  STATE join_state = state_;

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
      LOG(ERROR) << "PthreadThread::join(): fail with code " << res;
    }
  } else {
    LOG(ERROR) << "PthreadThread::join(): detached thread";
  }
}

Thread::id_t PthreadThread::getId() {
#ifdef _WIN32
  return (Thread::id_t)pthread_getw32threadid_np(pthread_);
#else
  return (Thread::id_t)pthread_;
#endif
}

shared_ptr<Runnable> PthreadThread::runnable() const {
  return Thread::runnable();
}

void PthreadThread::runnable(shared_ptr<Runnable> value) {
  Thread::runnable(value);
}

void PthreadThread::weakRef(shared_ptr<PthreadThread> self) {
  assert(self.get() == this);
  self_ = weak_ptr<PthreadThread>(self);
}

bool PthreadThread::setName(const std::string& name) {
  std::unique_lock<std::mutex> g(stateLock_);
  name_ = name;
  return updateName();
}

std::chrono::nanoseconds PthreadThread::usedCpuTime() const {
  timespec tp{};
#ifdef __linux__
  clockid_t clockid;
  if (!pthread_getcpuclockid(pthread_, &clockid)) {
    clock_gettime(clockid, &tp);
  }
#endif
  return std::chrono::nanoseconds(tp.tv_nsec) + std::chrono::seconds(tp.tv_sec);
}

void* PthreadThread::threadMain(void* arg) {
  shared_ptr<PthreadThread> thread = *(shared_ptr<PthreadThread>*)arg;
  delete reinterpret_cast<shared_ptr<PthreadThread>*>(arg);

  if (thread == nullptr) {
    return (void*)nullptr;
  }

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
      VLOG(1) << "pthread_setschedparam failed (are you root?) with error "
              << err << ": " << folly::errnoStr(err);
    }
  } else if (thread->policy_ == SCHED_OTHER) {
#ifndef _MSC_VER
    if (setpriority(PRIO_PROCESS, 0, thread->priority_) != 0) {
      VLOG(1) << "setpriority failed (are you root?) with error " << errno
              << ": " << folly::errnoStr(errno);
    }
#endif
  }

  thread->runnable()->run();

  return (void*)nullptr;
}

int PosixThreadFactory::Impl::toPthreadPolicy(POLICY policy) {
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

/* static */
int PosixThreadFactory::Impl::toPthreadPriority(
    POLICY policy, THREAD_PRIORITY priority) {
  int pthread_policy = toPthreadPolicy(policy);
  int min_priority = 0;
  int max_priority = 0;
  if (pthread_policy == SCHED_FIFO || pthread_policy == SCHED_RR) {
#ifdef THRIFT_HAVE_SCHED_GET_PRIORITY_MIN
    min_priority = sched_get_priority_min(pthread_policy);
#endif
#ifdef THRIFT_HAVE_SCHED_GET_PRIORITY_MAX
    max_priority = sched_get_priority_max(pthread_policy);
#endif
  } else if (pthread_policy == SCHED_OTHER) {
    min_priority = 19;
    max_priority = -20;
  }
  int quanta = HIGHEST_PRI - LOWEST_PRI;
  float stepsperquanta =
      static_cast<float>(max_priority - min_priority) / quanta;

#ifdef _MSC_VER
  return static_cast<int>(
      min_priority + stepsperquanta * folly::to_underlying(priority));
#else
  if (priority >= LOWEST_PRI && priority <= HIGHEST_PRI) {
    return static_cast<int>(
        min_priority + stepsperquanta * folly::to_underlying(priority));
  } else if (priority == INHERITED_PRI && pthread_policy == SCHED_OTHER) {
    errno = 0;
    int prio = getpriority(PRIO_PROCESS, 0);
    if (prio == -1 && errno != 0) {
      PLOG(WARNING) << "getpriority failed";
    } else {
      return prio;
    }
  } else {
    // Should never get here.
    assert(false);
  }

  return static_cast<int>(
      min_priority + stepsperquanta * folly::to_underlying(NORMAL));
#endif
}

PosixThreadFactory::Impl::Impl(
    POLICY policy,
    THREAD_PRIORITY priority,
    std::optional<int> stackSize,
    DetachState detached)
    : policy_(policy),
      priority_(priority),
      stackSize_(stackSize),
      detached_(detached) {}

shared_ptr<Thread> PosixThreadFactory::Impl::newThread(
    const shared_ptr<Runnable>& runnable,
    PosixThreadFactory::DetachState detachState) const {
  shared_ptr<PthreadThread> result =
      shared_ptr<PthreadThread>(new PthreadThread(
          toPthreadPolicy(policy_),
          toPthreadPriority(policy_, priority_),
          stackSize_,
          detachState == DETACHED,
          runnable));
  result->weakRef(result);
  runnable->thread(result);
  return result;
}

void PosixThreadFactory::Impl::setStackSize(int value) {
  stackSize_ = value;
}

PosixThreadFactory::THREAD_PRIORITY PosixThreadFactory::Impl::getPriority()
    const {
  return priority_;
}

/**
 * Sets priority.
 *
 *  XXX
 *  Need to handle incremental priorities properly.
 */
void PosixThreadFactory::Impl::setPriority(THREAD_PRIORITY value) {
  priority_ = value;
}

PosixThreadFactory::DetachState PosixThreadFactory::Impl::getDetachState()
    const {
  return detached_;
}

void PosixThreadFactory::Impl::setDetachState(DetachState value) {
  detached_ = value;
}

Thread::id_t PosixThreadFactory::Impl::getCurrentThreadId() const {
  return (Thread::id_t)folly::getCurrentThreadID();
}

PosixThreadFactory::PosixThreadFactory(
    POLICY policy,
    THREAD_PRIORITY priority,
    std::optional<int> stackSize,
    bool detached)
    : impl_(new PosixThreadFactory::Impl(
          policy, priority, stackSize, detached ? DETACHED : ATTACHED)) {}

PosixThreadFactory::PosixThreadFactory(DetachState detached)
    : impl_(new PosixThreadFactory::Impl(
          kDefaultPolicy, kDefaultPriority, std::nullopt, detached)) {}

shared_ptr<Thread> PosixThreadFactory::newThread(
    const shared_ptr<Runnable>& runnable) const {
  return impl_->newThread(runnable, impl_->getDetachState());
}

shared_ptr<Thread> PosixThreadFactory::newThread(
    const shared_ptr<Runnable>& runnable, DetachState detachState) const {
  return impl_->newThread(runnable, detachState);
}

void PosixThreadFactory::setStackSize(int value) {
  impl_->setStackSize(value);
}

PosixThreadFactory::THREAD_PRIORITY PosixThreadFactory::getPriority() const {
  return impl_->getPriority();
}

void PosixThreadFactory::setPriority(
    PosixThreadFactory::THREAD_PRIORITY value) {
  impl_->setPriority(value);
}

bool PosixThreadFactory::isDetached() const {
  return impl_->getDetachState() == DETACHED;
}

void PosixThreadFactory::setDetached(bool value) {
  impl_->setDetachState(value ? DETACHED : ATTACHED);
}

void PosixThreadFactory::setDetached(DetachState value) {
  impl_->setDetachState(value);
}

Thread::id_t PosixThreadFactory::getCurrentThreadId() const {
  return impl_->getCurrentThreadId();
}

} // namespace concurrency
} // namespace thrift
} // namespace apache
