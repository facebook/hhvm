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

#include <thrift/lib/cpp/concurrency/TimerManager.h>

#include <assert.h>
#include <iostream>
#include <set>

#include <thrift/lib/cpp/concurrency/Exception.h>
#include <thrift/lib/cpp/concurrency/Util.h>

namespace apache {
namespace thrift {
namespace concurrency {

using std::shared_ptr;

/**
 * TimerManager class
 *
 * @version $Id:$
 */
class TimerManager::Task : public Runnable {
 public:
  enum STATE {
    WAITING,
    EXECUTING,
    CANCELLED,
    COMPLETE,
  };

  Task(shared_ptr<Runnable> runnable) : runnable_(runnable), state_(WAITING) {}

  ~Task() override {}

  void run() override {
    if (state_ == EXECUTING) {
      runnable_->run();
      state_ = COMPLETE;
    }
  }

 private:
  shared_ptr<Runnable> runnable_;
  friend class TimerManager::Dispatcher;
  STATE state_;
};

class TimerManager::Dispatcher : public Runnable {
 public:
  Dispatcher(TimerManager* manager) : manager_(manager) {}

  ~Dispatcher() override {}

  /**
   * Dispatcher entry point
   *
   * As long as dispatcher thread is running, pull tasks off the task taskMap_
   * and execute.
   */
  void run() override {
    {
      std::unique_lock<std::mutex> l(manager_->mutex_);
      if (manager_->state_ == TimerManager::STARTING) {
        manager_->state_ = TimerManager::STARTED;
        manager_->cond_.notify_all();
      }
    }

    do {
      std::set<shared_ptr<TimerManager::Task>> expiredTasks;
      {
        std::unique_lock<std::mutex> l(manager_->mutex_);
        task_iterator expiredTaskEnd;
        int64_t now = Util::currentTime();
        while (manager_->state_ == TimerManager::STARTED &&
               (expiredTaskEnd = manager_->taskMap_.upper_bound(now)) ==
                   manager_->taskMap_.begin()) {
          int64_t timeout = 0LL;
          if (!manager_->taskMap_.empty()) {
            timeout = manager_->taskMap_.begin()->first - now;
          }
          auto count = manager_->taskCount_.load(std::memory_order_relaxed);
          (void)count;
          assert((timeout != 0 && count > 0) || (timeout == 0 && count == 0));
          if (timeout == 0) {
            manager_->cond_.wait(l);
          } else {
            manager_->cond_.wait_for(l, std::chrono::milliseconds(timeout));
          }
          now = Util::currentTime();
        }

        if (manager_->state_ == TimerManager::STARTED) {
          for (task_iterator ix = manager_->taskMap_.begin();
               ix != expiredTaskEnd;
               ix++) {
            shared_ptr<TimerManager::Task> task = ix->second;
            expiredTasks.insert(task);
            if (task->state_ == TimerManager::Task::WAITING) {
              task->state_ = TimerManager::Task::EXECUTING;
            }
            auto count = manager_->taskCount_.load(std::memory_order_relaxed);
            manager_->taskCount_.store(count - 1, std::memory_order_relaxed);
          }
          manager_->taskMap_.erase(manager_->taskMap_.begin(), expiredTaskEnd);
        }
      }

      for (std::set<shared_ptr<Task>>::iterator ix = expiredTasks.begin();
           ix != expiredTasks.end();
           ix++) {
        (*ix)->run();
      }

    } while (manager_->state_ == TimerManager::STARTED);

    {
      std::unique_lock<std::mutex> l(manager_->mutex_);
      if (manager_->state_ == TimerManager::STOPPING) {
        manager_->state_ = TimerManager::STOPPED;
        manager_->cond_.notify_one();
      }
    }
    return;
  }

 private:
  TimerManager* manager_;
  friend class TimerManager;
};

TimerManager::TimerManager()
    : taskCount_(0),
      state_(TimerManager::UNINITIALIZED),
      dispatcher_(shared_ptr<Dispatcher>(new Dispatcher(this))) {}

TimerManager::~TimerManager() {
  // If we haven't been explicitly stopped, do so now.  We don't need to grab
  // the monitor here, since stop already takes care of reentrancy.

  if (state_ != STOPPED) {
    stop();
  }
}

void TimerManager::start() {
  bool doStart = false;
  {
    std::unique_lock<std::mutex> l(mutex_);
    if (threadFactory_ == nullptr) {
      throw InvalidArgumentException();
    }
    if (state_ == TimerManager::UNINITIALIZED) {
      state_ = TimerManager::STARTING;
      doStart = true;
    }
  }

  if (doStart) {
    dispatcherThread_ = threadFactory_->newThread(dispatcher_);
    dispatcherThread_->start();
  }

  {
    std::unique_lock<std::mutex> l(mutex_);
    while (state_ == TimerManager::STARTING) {
      cond_.wait(l);
    }
    assert(state_ != TimerManager::STARTING);
  }
}

void TimerManager::stop() {
  bool doStop = false;
  {
    std::unique_lock<std::mutex> l(mutex_);
    if (state_ == TimerManager::UNINITIALIZED) {
      state_ = TimerManager::STOPPED;
    } else if (state_ != STOPPING && state_ != STOPPED) {
      doStop = true;
      state_ = STOPPING;
      cond_.notify_all();
    }
    while (state_ != STOPPED) {
      cond_.wait(l);
    }
  }

  if (doStop) {
    // Clean up any outstanding tasks
    taskMap_.clear();

    // Remove dispatcher's reference to us.
    dispatcher_->manager_ = nullptr;
  }
}

shared_ptr<const ThreadFactory> TimerManager::threadFactory() const {
  std::unique_lock<std::mutex> l(mutex_);
  return threadFactory_;
}

void TimerManager::threadFactory(shared_ptr<const ThreadFactory> value) {
  std::unique_lock<std::mutex> l(mutex_);
  threadFactory_ = value;
}

size_t TimerManager::taskCount() const {
  return taskCount_.load(std::memory_order_relaxed);
}

void TimerManager::add(shared_ptr<Runnable> task, int64_t timeout) {
  int64_t now = Util::currentTime();
  timeout += now;

  {
    std::unique_lock<std::mutex> l(mutex_);
    if (state_ != TimerManager::STARTED) {
      throw IllegalStateException();
    }

    // If the task map was empty, or if we have an expiration that is earlier
    // than any previously seen, kick the dispatcher so it can update its
    // timeout. Do this before inserting to limit comparisons to current tasks
    const auto count = taskCount_.load(std::memory_order_relaxed);
    if (count == 0 || timeout < taskMap_.begin()->first) {
      cond_.notify_one();
    }

    taskCount_.store(count + 1, std::memory_order_relaxed);
    taskMap_.insert({timeout, std::make_shared<Task>(std::move(task))});
  }
}

void TimerManager::add(
    shared_ptr<Runnable> task, const struct timespec& value) {
  int64_t expiration;
  Util::toMilliseconds(expiration, value);

  int64_t now = Util::currentTime();

  if (expiration < now) {
    throw InvalidArgumentException();
  }

  add(task, expiration - now);
}

void TimerManager::remove(shared_ptr<Runnable> /*task*/) {
  std::unique_lock<std::mutex> l(mutex_);
  if (state_ != TimerManager::STARTED) {
    throw IllegalStateException();
  }
}

TimerManager::STATE TimerManager::state() const {
  return state_;
}

} // namespace concurrency
} // namespace thrift
} // namespace apache
