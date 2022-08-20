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

#include <thrift/lib/cpp/test/loadgen/Controller.h>

#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/test/loadgen/IntervalTimer.h>
#include <thrift/lib/cpp/test/loadgen/Monitor.h>
#include <thrift/lib/cpp/test/loadgen/WorkerIf.h>

using apache::thrift::concurrency::PosixThreadFactory;
using apache::thrift::concurrency::Thread;
using std::shared_ptr;

namespace apache {
namespace thrift {
namespace loadgen {

class Controller::WorkerRunner : public concurrency::Runnable {
 public:
  WorkerRunner(Controller* controller) : controller_(controller) {}

  void run() override {
    shared_ptr<WorkerIf> worker = controller_->createWorker();
    worker->run();
  }

 private:
  Controller* controller_;
};

Controller::Controller(
    WorkerFactory* factory,
    Monitor* monitor,
    std::shared_ptr<LoadConfig> config,
    PosixThreadFactory* threadFactory)
    : numThreads_(0),
      maxThreads_(0),
      workerFactory_(factory),
      monitor_(monitor),
      intervalTimer_(0),
      config_(config),
      threadFactory_(threadFactory) {}

void Controller::run(
    uint32_t numThreads, uint32_t maxThreads, double monitorInterval) {
  maxThreads_ = maxThreads;

  // start all of the worker threads
  startWorkers(numThreads);

  // drive the monitor
  runMonitor(monitorInterval);
}

void Controller::createWorkerThreads(uint32_t numThreads) {
  const PosixThreadFactory& threadFactory =
      threadFactory_ ? *threadFactory_ : PosixThreadFactory();

  for (uint32_t n = 0; n < numThreads; ++n) {
    shared_ptr<WorkerRunner> runner(new WorkerRunner(this));
    shared_ptr<Thread> thread = threadFactory.newThread(runner);
    thread->start();
  }
}

void Controller::startWorkers(uint32_t numThreads) {
  numThreads_ = numThreads;

  intervalTimer_.setRatePerSec(config_->getDesiredQPS());
  intervalTimer_.start();

  // Start all of the WorkerRunners
  createWorkerThreads(numThreads_);

  // Wait for all of the workers to be created.
  //
  // If the monitor starts before all workers are running, the statistics
  // reported for the first interval might be lower than expected.  On the
  // other hand, the monitor correctly handles the case where some workers were
  // already performing operations for some time before the monitor started.
  {
    std::unique_lock<std::mutex> l(initMutex_);
    while (workers_.size() < numThreads_) {
      initCondVar_.wait(l);
    }
  }
}

void Controller::runMonitor(double interval) {
  unsigned long intervalUsec =
      static_cast<unsigned long>(interval * concurrency::Util::US_PER_S);
  unsigned long intervalNsec =
      static_cast<unsigned long>(interval * concurrency::Util::NS_PER_S);
  IntervalTimer itimer(intervalNsec);

  itimer.start();
  monitor_->initializeInfo();

  while (true) {
    itimer.sleep();
    monitor_->redisplay(intervalUsec);

    // Break if all the threads are stopped
    bool allStopped = true;
    for (WorkerVector::const_iterator it = workers_.begin();
         it != workers_.end();
         ++it) {
      if ((*it)->isAlive()) {
        allStopped = false;
        break;
      }
    }
    if (allStopped) {
      break;
    }

    // Adjust worker threads based on current QPS
    if (config_->getDesiredQPS() > 0 && numThreads_ < maxThreads_) {
      uint64_t currentQps = monitor_->getCurrentQps();
      uint64_t desiredQps = config_->getDesiredQPS();

      if (currentQps < desiredQps) {
        uint32_t numNewWorkerThreads =
            (desiredQps - currentQps) * numThreads_ / currentQps;
        numNewWorkerThreads =
            std::min(numNewWorkerThreads, maxThreads_ - numThreads_);

        createWorkerThreads(numNewWorkerThreads);
        numThreads_ += numNewWorkerThreads;

        T_LOG_OPER(
            "Total worker threads: %u (max %u)", numThreads_, maxThreads_);
      }
    }
  }
}

/**
 * createWorker() is called once in each of the worker threads.
 * It creates the actual worker threads, then waits for all of the other
 * threads to create their workers before returning.
 *
 * It holds a lock, so that the threads call monitor_->newScoreBoard() and
 * workerFactory_->newWorker() serially, and the Monitor and WorkerFactory
 * classes don't have to worry about locking.
 *
 * This is performed in the worker threads rather than the main thread, just to
 * ensure that memory allocation for the workers is performed in their
 * respective threads.  Allocating the memory in the main thread can lead to
 * false sharing in some cases, hurting performance.  (i.e., scoreboard data
 * for two different workers may be placed on the same cache line, forcing it
 * to be continually evicted as two different CPUs try to access it very
 * frequently.)  Performing the allocation in the individual threads helps
 * let smarter malloc implementations avoid false sharing.
 */
shared_ptr<WorkerIf> Controller::createWorker() {
  std::unique_lock<std::mutex> l(initMutex_);

  // Create the worker
  int id = workers_.size();
  shared_ptr<ScoreBoard> scoreboard(monitor_->newScoreBoard(id));
  shared_ptr<WorkerIf> worker(
      workerFactory_->newWorker(id, scoreboard, &intervalTimer_));

  // Add the worker to the workers_ array,
  // and notify anyone waiting that we have updated workers_
  workers_.push_back(worker);
  initCondVar_.notify_all();

  // Wait on all of the other workers to be created
  while (workers_.size() != numThreads_) {
    initCondVar_.wait(l);
  }

  return worker;
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
