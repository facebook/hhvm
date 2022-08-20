/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#include <thrift/lib/cpp/concurrency/TimerManager.h>
#include <thrift/lib/cpp/concurrency/Util.h>

#include <assert.h>
#include <condition_variable>
#include <iostream>
#include <mutex>

namespace apache {
namespace thrift {
namespace concurrency {
namespace test {

/**
 * ThreadManagerTests class
 *
 * @version $Id:$
 */
class TimerManagerTests {
 public:
  static const double ERROR;

  class Task : public Runnable {
   public:
    Task(std::mutex& mutex, std::condition_variable& cond, int64_t timeout)
        : _timeout(timeout),
          _startTime(Util::currentTime()),
          _mutex(mutex),
          _cond(cond),
          _success(false),
          _done(false) {}

    ~Task() override { std::cerr << this << std::endl; }

    void run() override {
      _endTime = Util::currentTime();

      // Figure out error percentage

      int64_t delta = _endTime - _startTime;

      delta = delta > _timeout ? delta - _timeout : _timeout - delta;

      float error = delta / _timeout;

      if (error < ERROR) {
        _success = true;
      }

      _done = true;

      std::cout << "\t\t\tTimerManagerTests::Task[" << this << "] done"
                << std::endl; // debug

      {
        std::unique_lock<std::mutex> l(_mutex);
        _cond.notify_all();
      }
    }

    int64_t _timeout;
    int64_t _startTime;
    int64_t _endTime;
    std::mutex& _mutex;
    std::condition_variable& _cond;
    bool _success;
    bool _done;
  };

  /**
   * This test creates two tasks and waits for the first to expire within 10%
   * of the expected expiration time. It then verifies that the timer manager
   * properly clean up itself and the remaining orphaned timeout task when the
   * manager goes out of scope and its destructor is called.
   */
  bool test00(int64_t timeout = 1000LL) {
    shared_ptr<TimerManagerTests::Task> orphanTask =
        shared_ptr<TimerManagerTests::Task>(
            new TimerManagerTests::Task(_mutex, _cond, 10 * timeout));

    {
      TimerManager timerManager;

      timerManager.threadFactory(
          shared_ptr<PosixThreadFactory>(new PosixThreadFactory()));

      timerManager.start();

      assert(timerManager.state() == TimerManager::STARTED);

      shared_ptr<TimerManagerTests::Task> task =
          shared_ptr<TimerManagerTests::Task>(
              new TimerManagerTests::Task(_mutex, _cond, timeout));

      {
        std::unique_lock<std::mutex> l(_mutex);

        timerManager.add(orphanTask, 10 * timeout);

        timerManager.add(task, timeout);

        _cond.wait(l);
      }

      assert(task->_done);

      std::cout << "\t\t\t" << (task->_success ? "Success" : "Failure") << "!"
                << std::endl;
    }

    // timerManager.stop(); This is where it happens via destructor

    assert(!orphanTask->_done);

    return true;
  }

  friend class TestTask;

  std::mutex _mutex;
  std::condition_variable _cond;
};

const double TimerManagerTests::ERROR = .20;

} // namespace test
} // namespace concurrency
} // namespace thrift
} // namespace apache
