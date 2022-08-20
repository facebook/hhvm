/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/ThreadPool.h"
#include "watchman/Logging.h"

namespace watchman {

ThreadPool& getThreadPool() {
  static ThreadPool pool;
  return pool;
}

ThreadPool::~ThreadPool() {
  stop();
}

void ThreadPool::start(size_t numWorkers, size_t maxItems) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!workers_.empty()) {
    throw std::runtime_error("ThreadPool already started");
  }
  if (stopping_) {
    throw std::runtime_error("Cannot restart a stopped pool");
  }
  maxItems_ = maxItems;

  for (auto i = 0U; i < numWorkers; ++i) {
    workers_.emplace_back([this, i]() noexcept {
      w_set_thread_name("ThreadPool-", i);
      runWorker();
    });
  }
}

void ThreadPool::runWorker() {
  while (true) {
    folly::Func task;

    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
      if (stopping_ && tasks_.empty()) {
        return;
      }
      task = std::move(tasks_.front());
      tasks_.pop_front();
    }

    task();
  }
}

void ThreadPool::stop(bool join) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    stopping_ = true;
  }
  condition_.notify_all();

  if (join) {
    for (auto& worker : workers_) {
      worker.join();
    }
  }
}

void ThreadPool::add(folly::Func func) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (stopping_) {
      throw std::runtime_error("cannot add tasks after pool has stopped");
    }
    if (tasks_.size() + 1 >= maxItems_) {
      throw std::runtime_error("thread pool queue is full");
    }

    tasks_.emplace_back(std::move(func));
  }

  condition_.notify_one();
}
} // namespace watchman
