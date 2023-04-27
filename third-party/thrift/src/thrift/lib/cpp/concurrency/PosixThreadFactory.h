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

#ifndef THRIFT_CONCURRENCY_POSIXTHREADFACTORY_H_
#define THRIFT_CONCURRENCY_POSIXTHREADFACTORY_H_ 1

#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>

#include <folly/portability/PThread.h>

#include <thrift/lib/cpp/concurrency/Thread.h>

namespace apache {
namespace thrift {
namespace concurrency {

class PthreadThread : public Thread {
 public:
  enum STATE {
    uninitialized,
    starting,
  };

  static const int MB = 1024 * 1024;

  static void* threadMain(void* arg);

 protected:
  pthread_t pthread_;
  STATE state_;
  int policy_;
  int priority_;
  std::optional<int> stackSize_;
  std::weak_ptr<PthreadThread> self_;
  bool detached_;
  std::mutex stateLock_;
  std::string name_;

  // push our given name upstream into pthreads
  bool updateName();

 public:
  PthreadThread(
      int policy,
      int priority,
      std::optional<int> stackSize,
      bool detached,
      std::shared_ptr<Runnable> runnable);
  ~PthreadThread() override;

  void start() override;
  void join() override;
  Thread::id_t getId() override;
  std::shared_ptr<Runnable> runnable() const override;
  void runnable(std::shared_ptr<Runnable> value) override;
  void weakRef(std::shared_ptr<PthreadThread> self);
  bool setName(const std::string& name) override;
  std::chrono::nanoseconds usedCpuTime() const override;
};

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
    ROUND_ROBIN,
  };

  /**
   * POSIX Thread scheduler relative priorities,
   *
   * Absolute priority is determined by scheduler policy and OS. This
   * enumeration specifies relative priorities such that one can specify a
   * priority within a giving scheduler policy without knowing the absolute
   * value of the priority.
   */
  enum THREAD_PRIORITY {
    LOWEST_PRI = 0,
    LOWER_PRI = 1,
    LOW_PRI = 2,
    NORMAL_PRI = 3,
    HIGH_PRI = 4,
    HIGHER_PRI = 5,
    HIGHEST_PRI = 6,

    // Inherit priority of caller thread (supported only for POLICY::OTHER).
    INHERITED_PRI = 9,
  };

  static const POLICY kDefaultPolicy = OTHER;
  static const THREAD_PRIORITY kDefaultPriority = NORMAL_PRI;

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
  explicit PosixThreadFactory(
      POLICY policy = kDefaultPolicy,
      THREAD_PRIORITY priority = kDefaultPriority,
      std::optional<int> stackSize = std::nullopt,
      bool detached = true);

  explicit PosixThreadFactory(DetachState detached);

  // From ThreadFactory;
  std::shared_ptr<Thread> newThread(
      const std::shared_ptr<Runnable>& runnable) const override;
  std::shared_ptr<Thread> newThread(
      const std::shared_ptr<Runnable>& runnable,
      DetachState detachState) const override;

  // From ThreadFactory;
  Thread::id_t getCurrentThreadId() const override;

  /**
   * Sets stack size for created threads
   *
   * @param value size in megabytes
   */
  virtual void setStackSize(int value);

  /**
   * Gets priority relative to current policy
   */
  virtual THREAD_PRIORITY getPriority() const;

  /**
   * Sets priority relative to current policy
   */
  virtual void setPriority(THREAD_PRIORITY priority);

  /**
   * Sets detached mode of threads
   */
  virtual void setDetached(bool detached);
  virtual void setDetached(DetachState detached);

  /**
   * Gets current detached mode
   */
  virtual bool isDetached() const;

  class Impl {
   protected:
    POLICY policy_;
    THREAD_PRIORITY priority_;
    std::optional<int> stackSize_;
    DetachState detached_;

    /**
     * Converts generic posix thread schedule policy enums into pthread
     * API values.
     */
    static int toPthreadPolicy(POLICY policy);

   public:
    /**
     * Converts relative thread priorities to absolute value based on posix
     * thread scheduler policy
     *
     *  The idea is simply to divide up the priority range for the given policy
     * into the corresponding relative priority level (lowest..highest) and
     * then pro-rate accordingly.
     */
    static int toPthreadPriority(POLICY policy, THREAD_PRIORITY priority);

    Impl(
        POLICY policy,
        THREAD_PRIORITY priority,
        std::optional<int> stackSize,
        DetachState detached);
    virtual ~Impl() {}

    /**
     * Creates a new POSIX thread to run the runnable object
     *
     * @param runnable A runnable object
     */
    virtual std::shared_ptr<Thread> newThread(
        const std::shared_ptr<Runnable>& runnable,
        DetachState detachState) const;

    void setStackSize(int value);
    THREAD_PRIORITY getPriority() const;

    /**
     * Sets priority.
     *
     *  XXX
     *  Need to handle incremental priorities properly.
     */
    void setPriority(THREAD_PRIORITY value);

    DetachState getDetachState() const;
    void setDetachState(DetachState value);
    Thread::id_t getCurrentThreadId() const;
  };

 protected:
  std::shared_ptr<Impl> impl_;
};

} // namespace concurrency
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_CONCURRENCY_POSIXTHREADFACTORY_H_
