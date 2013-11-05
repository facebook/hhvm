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
#include "hphp/util/job-queue.h"

#include <condition_variable>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace HPHP {

DEFINE_string(workload_input, "", "input file for request latency.");

class Tickable {
 public:
  virtual ~Tickable() {}
  virtual void tick() = 0;
};

class TickingClock {
 public:
  TickingClock() : m_ticks(0) {
  }

  ~TickingClock() {
  }

  void tick() {
    ++m_ticks;
    for (auto x : m_tickables) {
      x->tick();
    }
  }

  void registerTickable(Tickable* tickable) {
    m_tickables.push_back(tickable);
  }

  std::atomic<int> m_ticks;

 private:
  std::vector<Tickable*> m_tickables;
};

class TickRequest {
 public:
  enum class State {
    IN_QUEUE,
    PROCESSING,
    COMPLETED,
    ABORTED
  };

  explicit TickRequest(int duration) : m_duration(duration),
                                       m_state(State::IN_QUEUE) {}

  int duration() const {
    return m_duration;
  }

  State getState() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_state;
  }

  void setState(State state) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_state = state;
    if (isFinishedNoLock()) {
      m_cv.notify_all();
    }
  }

  bool inFlight() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return !isFinishedNoLock();
  }

  void waitUntilDone() {
    std::unique_lock<std::mutex> lk(m_mutex);
    if (isFinishedNoLock()) {
      return;
    }
    m_cv.wait(lk);
  }

  const int m_duration;

 private:
  bool isFinishedNoLock() const {
    return m_state == State::COMPLETED || m_state == State::ABORTED;
  }
  std::mutex m_mutex;
  std::condition_variable m_cv;
  State m_state;
};
typedef std::shared_ptr<TickRequest> TickRequestPtr;

class TickRequestFactory {
 public:
  TickRequestFactory() {}

  TickRequestPtr newRequest(int duration) {
    auto request = std::make_shared<TickRequest>(duration);
    m_requests.push_back(request);
    return request;
  }

  std::vector<TickRequestPtr> m_requests;
};

class TickWorker : public JobQueueWorker<TickRequestPtr, TickingClock*, false,
                                         true>,
                   public Tickable {
 public:
  TickWorker() : m_ticks(0) {}
  virtual ~TickWorker() {}

  virtual void onThreadEnter() {
    m_context->registerTickable(this);
  }
  virtual void doJob(TickRequestPtr job) {
    job->setState(TickRequest::State::PROCESSING);
    std::unique_lock<std::mutex> lk(m_mutex);
    m_job = job;
    m_ticks = 0;
    m_cv.wait(lk);
    m_job->setState(TickRequest::State::COMPLETED);
    m_job.reset();
  }
  virtual void abortJob(TickRequestPtr job) {
    m_job->setState(TickRequest::State::ABORTED);
  }
  virtual void tick() {
    std::lock_guard<std::mutex> lk(m_mutex);
    ++m_ticks;
    if (!m_job) {
      return;
    }
    if (m_ticks > m_job->m_duration) {
      m_cv.notify_all();
    }
  }

  int getTicks() {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_ticks;
  }

 private:
  int m_ticks;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  TickRequestPtr m_job;
};

class JobQueueTest : public testing::Test {
 public:
  virtual void SetUp() {
    m_clock.reset(new TickingClock());
    m_dispatcher.reset(
      new JobQueueDispatcher<TickWorker>(180, false, 0, true, m_clock.get()));
    m_factory.reset(new TickRequestFactory());
  }

  std::unique_ptr<TickingClock> m_clock;
  std::unique_ptr<JobQueueDispatcher<TickWorker>> m_dispatcher;
  std::unique_ptr<TickRequestFactory> m_factory;
};

TEST_F(JobQueueTest, ClockTest) {
  TickWorker t1, t2;
  m_clock->registerTickable(&t1);
  m_clock->registerTickable(&t2);
  EXPECT_EQ(0, t1.getTicks());
  EXPECT_EQ(0, t2.getTicks());
  m_clock->tick();
  EXPECT_EQ(1, t1.getTicks());
  EXPECT_EQ(1, t2.getTicks());
  m_clock->tick();
  EXPECT_EQ(2, t1.getTicks());
  EXPECT_EQ(2, t2.getTicks());
}

TEST_F(JobQueueTest, SanityTest) {
  m_dispatcher->enqueue(m_factory->newRequest(3));
  m_dispatcher->enqueue(m_factory->newRequest(1));
  EXPECT_EQ(2, m_dispatcher->getQueuedJobs());
  m_dispatcher->start();
  m_clock->tick();
  EXPECT_TRUE(m_factory->m_requests[0]->inFlight());
  m_clock->tick();
  m_clock->tick();
  EXPECT_TRUE(m_factory->m_requests[0]->inFlight());
  m_factory->m_requests[1]->waitUntilDone();
  EXPECT_FALSE(m_factory->m_requests[1]->inFlight());
  m_clock->tick();
  m_clock->tick();
  m_dispatcher->waitEmpty();
  EXPECT_FALSE(m_factory->m_requests[0]->inFlight());
  EXPECT_EQ(0, m_dispatcher->getQueuedJobs());
}

TEST_F(JobQueueTest, WorkloadTest) {
  if (FLAGS_workload_input.empty()) {
    LOG(INFO) << "skipped.";
    return;
  }
  std::vector<int> latencies;
  std::string line;
  std::ifstream fin(FLAGS_workload_input.c_str());
  while (std::getline(fin, line)) {
    latencies.push_back(folly::to<int>(line));
  }
  LOG(INFO) << "Loaded " << latencies.size() << " lines.";
  for (auto l : latencies) {
    m_dispatcher->enqueue(m_factory->newRequest(l));
  }
  m_dispatcher->start();
  while (!m_dispatcher->pollEmpty()) {
    m_clock->tick();
  }

  for (auto request : m_factory->m_requests) {
    while (request->inFlight()) {
      m_clock->tick();
    }
  }
  LOG(INFO) << "Processed " << latencies.size() << " requests in "
            << m_clock->m_ticks.load() << " ticks";
}

} // namespace HPHP

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  google::ParseCommandLineFlags(&argc, &argv, false);
  return RUN_ALL_TESTS();
}
