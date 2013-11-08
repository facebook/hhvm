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
#ifndef HPHP_THRIFT_CONCURRENCY_POSIXTHREADFACTORY_H
#define HPHP_THRIFT_CONCURRENCY_POSIXTHREADFACTORY_H 1

#include <set>
#include <string>

#include "thrift/lib/cpp/concurrency/Thread.h"

#include <memory>

namespace apache { namespace thrift { namespace concurrency {

void getLiveThreadIds(std::set<pthread_t>* tids);
/**
 * Wrapper around pthread_setname_np that handles older glibc versions
 */
bool setPosixThreadName(pthread_t id, const std::string& name);

/**
 * A thread factory to create posix threads
 *
 * @version $Id:$
 */
class PosixThreadFactory : public ThreadFactory {

 public:

  /**
   * POSIX Thread scheduler policies
   */
  enum POLICY {
    OTHER,
    FIFO,
    ROUND_ROBIN
  };

  /**
   * POSIX Thread scheduler relative priorities,
   *
   * Absolute priority is determined by scheduler policy and OS. This
   * enumeration specifies relative priorities such that one can specify a
   * priority within a giving scheduler policy without knowing the absolute
   * value of the priority.
   */
  enum PRIORITY {
    LOWEST = 0,
    LOWER = 1,
    LOW = 2,
    NORMAL = 3,
    HIGH = 4,
    HIGHER = 5,
    HIGHEST = 6,
    INCREMENT = 7,
    DECREMENT = 8
  };

  static const POLICY kDefaultPolicy = OTHER;
  static const PRIORITY kDefaultPriority = NORMAL;
  static const int kDefaultStackSizeMB = 1;

  /**
   * Posix thread (pthread) factory.  All threads created by a factory are
   * reference-counted via std::shared_ptr and std::weak_ptr.  The factory
   * guarantees that threads and the Runnable tasks they host will be properly
   * cleaned up once the last strong reference to both is given up.
   *
   * Threads are created with the specified policy, priority, stack-size and
   * detachable-mode detached means the thread is free-running and will release
   * all system resources the when it completes.  A detachable thread is not
   * joinable.  The join method of a detachable thread will return immediately
   * with no error.
   *
   * By default threads are not joinable.
   */
  explicit PosixThreadFactory(POLICY policy=kDefaultPolicy,
                              PRIORITY priority=kDefaultPriority,
                              int stackSize=kDefaultStackSizeMB,
                              bool detached=true);

  explicit PosixThreadFactory(DetachState detached);

  // From ThreadFactory;
  std::shared_ptr<Thread> newThread(
      const std::shared_ptr<Runnable>& runnable) const;
  std::shared_ptr<Thread> newThread(
      const std::shared_ptr<Runnable>& runnable,
      DetachState detachState) const;

  // From ThreadFactory;
  Thread::id_t getCurrentThreadId() const;

  /**
   * Gets stack size for created threads
   *
   * @return int size in megabytes
   */
  virtual int getStackSize() const;

  /**
   * Sets stack size for created threads
   *
   * @param value size in megabytes
   */
  virtual void setStackSize(int value);

  /**
   * Gets priority relative to current policy
   */
  virtual PRIORITY getPriority() const;

  /**
   * Sets priority relative to current policy
   */
  virtual void setPriority(PRIORITY priority);

  /**
   * Sets detached mode of threads
   */
  virtual void setDetached(bool detached);
  virtual void setDetached(DetachState detached);

  /**
   * Gets current detached mode
   */
  virtual bool isDetached() const;

 private:
  class Impl;
  std::shared_ptr<Impl> impl_;
};

}}} // apache::thrift::concurrency

#endif // #ifndef HPHP_THRIFT_CONCURRENCY_POSIXTHREADFACTORY_H
