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

#include "thrift/lib/cpp/concurrency/Monitor.h"
#include "thrift/lib/cpp/concurrency/Exception.h"
#include "thrift/lib/cpp/concurrency/Util.h"
#include "glog/logging.h"

#include <boost/scoped_ptr.hpp>

#include <errno.h>

#include <iostream>

#include <pthread.h>

namespace apache { namespace thrift { namespace concurrency {

using boost::scoped_ptr;

/**
 * Monitor implementation using the POSIX pthread library
 *
 * @version $Id:$
 */
class Monitor::Impl {

 public:

  Impl()
     : ownedMutex_(new Mutex()),
       mutex_(nullptr),
       condInitialized_(false) {
    init(ownedMutex_.get());
  }

  Impl(Mutex* mutex)
     : mutex_(nullptr),
       condInitialized_(false) {
    init(mutex);
  }

  Impl(Monitor* monitor)
     : mutex_(nullptr),
       condInitialized_(false) {
    init(&(monitor->mutex()));
  }

  ~Impl() { cleanup(); }

  Mutex& mutex() { return *mutex_; }
  void lock() { mutex().lock(); }
  void unlock() { mutex().unlock(); }

  /**
   * Exception-throwing version of waitForTimeRelative(), called simply
   * wait(int64) for historical reasons.  Timeout is in milliseconds.
   *
   * If the condition occurs,  this function returns cleanly; on timeout or
   * error an exception is thrown.
   */
  void wait(int64_t timeout_ms) const {
    int result = waitForTimeRelative(timeout_ms);
    if (result == ETIMEDOUT) {
      // pthread_cond_timedwait has been observed to return early on
      // various platforms, so comment out this assert.
      //assert(Util::currentTime() >= (now + timeout));
      throw TimedOutException();
    } else if (result != 0) {
      throw TLibraryException(
        "pthread_cond_wait() or pthread_cond_timedwait() failed");
    }
  }

  /**
   * Waits until the specified timeout in milliseconds for the condition to
   * occur, or waits forever if timeout_ms == 0.
   *
   * Returns 0 if condition occurs, ETIMEDOUT on timeout, or an error code.
   */
  int waitForTimeRelative(int64_t timeout_ms) const {
    if (timeout_ms == 0LL) {
      return waitForever();
    }

    struct timespec abstime;
    Util::toTimespec(abstime, Util::currentTime() + timeout_ms);
    return waitForTime(&abstime);
  }

  /**
   * Waits until the absolute time specified using struct timespec.
   * Returns 0 if condition occurs, ETIMEDOUT on timeout, or an error code.
   */
  int waitForTime(const timespec* abstime) const {
    // The caller must lock the mutex before calling waitForTime()
    DCHECK(mutex_);
    DCHECK(mutex_->isLocked());

    pthread_mutex_t* mutexImpl =
      reinterpret_cast<pthread_mutex_t*>(mutex_->getUnderlyingImpl());
    DCHECK(mutexImpl);

    return pthread_cond_timedwait(&pthread_cond_,
                                  mutexImpl,
                                  abstime);
  }

  /**
   * Waits forever until the condition occurs.
   * Returns 0 if condition occurs, or an error code otherwise.
   */
  int waitForever() const {
    // The caller must lock the mutex before calling waitForever()
    DCHECK(mutex_);
    DCHECK(mutex_->isLocked());

    pthread_mutex_t* mutexImpl =
      reinterpret_cast<pthread_mutex_t*>(mutex_->getUnderlyingImpl());
    DCHECK(mutexImpl);
    return pthread_cond_wait(&pthread_cond_, mutexImpl);
  }


  void notify() {
    // The caller must lock the mutex before calling notify()
    DCHECK(mutex_ && mutex_->isLocked());

    int iret = pthread_cond_signal(&pthread_cond_);
    DCHECK(iret == 0);
  }

  void notifyAll() {
    // The caller must lock the mutex before calling notify()
    DCHECK(mutex_ && mutex_->isLocked());

    int iret = pthread_cond_broadcast(&pthread_cond_);
    DCHECK(iret == 0);
  }

 private:

  void init(Mutex* mutex) {
    mutex_ = mutex;

    if (pthread_cond_init(&pthread_cond_, nullptr) == 0) {
      condInitialized_ = true;
    }

    if (!condInitialized_) {
      cleanup();
      throw SystemResourceException();
    }
  }

  void cleanup() {
    if (condInitialized_) {
      condInitialized_ = false;
      int iret = pthread_cond_destroy(&pthread_cond_);
      DCHECK(iret == 0);
    }
  }

  scoped_ptr<Mutex> ownedMutex_;
  Mutex* mutex_;

  mutable pthread_cond_t pthread_cond_;
  mutable bool condInitialized_;
};

Monitor::Monitor() : impl_(new Monitor::Impl()) {}
Monitor::Monitor(Mutex* mutex) : impl_(new Monitor::Impl(mutex)) {}
Monitor::Monitor(Monitor* monitor) : impl_(new Monitor::Impl(monitor)) {}

Monitor::~Monitor() { delete impl_; }

Mutex& Monitor::mutex() const { return impl_->mutex(); }

void Monitor::lock() const { impl_->lock(); }

void Monitor::unlock() const { impl_->unlock(); }

void Monitor::wait(int64_t timeout) const { impl_->wait(timeout); }

int Monitor::waitForTime(const timespec* abstime) const {
  return impl_->waitForTime(abstime);
}

int Monitor::waitForTimeRelative(int64_t timeout_ms) const {
  return impl_->waitForTimeRelative(timeout_ms);
}

int Monitor::waitForever() const {
  return impl_->waitForever();
}

void Monitor::notify() const { impl_->notify(); }

void Monitor::notifyAll() const { impl_->notifyAll(); }

}}} // apache::thrift::concurrency
