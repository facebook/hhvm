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

#include "folly/Random.h"

#include <gtest/gtest.h>

namespace HPHP {

DEFINE_string(workload_input, "", "input file for request latency.");
DEFINE_int32(time_padding_high, 0,
             "max num of ticks to add to each request's latency "
             "for those that are selected.");
DEFINE_int32(time_padding_low, 0,
             "min num of ticks to add to each request's latency "
             "for those that are selected.");
DEFINE_int32(time_padding_percent, 0,
             "percent of time request is selected for padding");

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
    std::lock_guard<std::mutex> lk(m_mutex);
    ++m_ticks;
    for (auto x : m_tickables) {
      x->tick();
    }
  }

  void registerTickable(Tickable* tickable) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_tickables.push_back(tickable);
  }

  int now() const {
    return m_ticks.load();
  }

  std::atomic<int> m_ticks;

 private:
  std::mutex m_mutex;
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

  TickRequest(int duration, TickingClock* clock)
    : m_duration(duration),
      m_startQueuingTimeStamp(clock->now()),
      m_startProcessingTimeStamp(-1),
      m_finishProcessingTimeStamp(-1),
      m_state(State::IN_QUEUE),
      m_clock(clock) {
  }

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
    if (m_state == State::PROCESSING) {
      m_startProcessingTimeStamp = m_clock->now();
    }
    if (isFinishedNoLock()) {
      m_finishProcessingTimeStamp = m_clock->now();
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

  int getQueuingTime() {
    CHECK(!inFlight());
    return m_startProcessingTimeStamp - m_startQueuingTimeStamp;
  }

  int getWallTime() {
    CHECK(!inFlight());
    return m_finishProcessingTimeStamp - m_startQueuingTimeStamp;
  }

  const int m_duration;
  int m_startQueuingTimeStamp;
  int m_startProcessingTimeStamp;
  int m_finishProcessingTimeStamp;

 private:
  bool isFinishedNoLock() const {
    return m_state == State::COMPLETED || m_state == State::ABORTED;
  }
  std::mutex m_mutex;
  std::condition_variable m_cv;
  State m_state;
  TickingClock* m_clock;
};
typedef std::shared_ptr<TickRequest> TickRequestPtr;

class TickRequestFactory {
 public:
  explicit TickRequestFactory(TickingClock* clock) : m_clock(clock) {}

  TickRequestPtr newRequest(int duration) {
    auto request = std::make_shared<TickRequest>(duration, m_clock);
    m_requests.push_back(request);
    return request;
  }

  struct Stats {
    int max;
    int p99_99;
    int p99_9;
    int p99;
    int p95;
    int p75;
    int p50;
    int p5;
    int mean;
  };

  template <class Getter>
  Stats getStats(Getter getter) {
    vector<int> metrics;
    metrics.reserve(m_requests.size());
    for(auto r : m_requests) {
      metrics.push_back(getter(r));
    }
    std::sort(metrics.begin(), metrics.end());

    Stats stats;
    stats.max = *metrics.rbegin();
    stats.p99_99 = metrics[static_cast<int>(0.9999 * (metrics.size() - 1))];
    stats.p99_9 = metrics[static_cast<int>(0.999 * (metrics.size() - 1))];
    stats.p99 = metrics[static_cast<int>(0.99 * (metrics.size() - 1))];
    stats.p95 = metrics[static_cast<int>(0.95 * (metrics.size() - 1))];
    stats.p75 = metrics[static_cast<int>(0.75 * (metrics.size() - 1))];
    stats.p50 = metrics[static_cast<int>(0.50 * (metrics.size() - 1))];
    stats.p5 = metrics[static_cast<int>(0.05 * (metrics.size() - 1))];
    stats.mean = std::accumulate(metrics.begin(), metrics.end(), 0) /
      metrics.size();

    return stats;
  }

  Stats getQueuingStats() {
    return getStats([=](TickRequestPtr r) { return r->getQueuingTime();});
  }

  Stats getWallTimeStats() {
    return getStats([=](TickRequestPtr r) { return r->getWallTime();});
  }

  std::vector<TickRequestPtr> m_requests;
  TickingClock* m_clock;
};

class TickWorker : public JobQueueWorker<TickRequestPtr, TickingClock*, true,
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

class JobQueueStatsCollector : public Tickable {
 public:
  explicit JobQueueStatsCollector(JobQueueDispatcher<TickWorker>* dispatcher)
      : m_dispatcher(dispatcher),
        m_maxLoad(0),
        m_maxQueued(0) {
  }

  void tick() {
    m_maxQueued = std::max(m_maxQueued, m_dispatcher->getQueuedJobs());
    m_maxLoad = std::max(m_maxLoad, m_dispatcher->getActiveWorker());
  }

  JobQueueDispatcher<TickWorker>* m_dispatcher;
  int m_maxLoad;
  int m_maxQueued;
};

class JobQueueTest : public testing::Test {
 protected:

  virtual void SetUp() {
    m_clock.reset(new TickingClock());
    m_dispatcher.reset(
      new JobQueueDispatcher<TickWorker>(180, false, 0, true, m_clock.get()));
    m_factory.reset(new TickRequestFactory(m_clock.get()));
  }

  void loadRequestFromFile(std::vector<int>* latencies) {
    std::string line;
    std::ifstream fin(FLAGS_workload_input.c_str());
    while (std::getline(fin, line)) {
      int padding = 0;
      if (std::rand() % 100 < FLAGS_time_padding_percent) {
        padding = FLAGS_time_padding_low +
          std::rand() % (FLAGS_time_padding_high - FLAGS_time_padding_low);
      }
      latencies->push_back(folly::to<int>(line) + padding);
    }
    LOG(INFO) << "Loaded " << latencies->size() << " lines.";
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
  ASSERT_EQ(2, m_dispatcher->getQueuedJobs());
  m_dispatcher->start();
  m_clock->tick();
  EXPECT_TRUE(m_factory->m_requests[0]->inFlight());
  while (m_factory->m_requests[1]->inFlight()) {
    m_clock->tick();
  }
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

  vector<int> latencies;
  loadRequestFromFile(&latencies);
  int rps = 150;
  m_dispatcher->start();
  JobQueueStatsCollector stats(m_dispatcher.get());
  m_clock->registerTickable(&stats);

  for (auto request : latencies) {
    do {
      m_clock->tick();
    } while (std::rand() % 1000 > rps);

    m_dispatcher->enqueue(m_factory->newRequest(request));
  }

  while (!m_dispatcher->pollEmpty()) {
    m_clock->tick();
  }

  for (auto request : m_factory->m_requests) {
    while (request->inFlight()) {
      m_clock->tick();
    }
  }

  auto queueStats = m_factory->getQueuingStats();
  auto wallStats = m_factory->getWallTimeStats();

  LOG(INFO) << "Processed " << latencies.size() << " requests in "
            << m_clock->m_ticks.load() << " ticks";
  LOG(INFO) << "max load: " << stats.m_maxLoad
            << " max queued: " << stats.m_maxQueued;
  LOG(INFO) << "max queuing time: " << queueStats.max
            << " max wall time: " << wallStats.max;
  LOG(INFO) << "p99 queuing time: " << queueStats.p99
            << " p99 wall time: " << wallStats.p99;
  LOG(INFO) << "p95 queuing time: " << queueStats.p95
            << " p95 wall time: " << wallStats.p95;
  LOG(INFO) << "p50 queuing time: " << queueStats.p50
            << " p50 wall time: " << wallStats.p50;
  LOG(INFO) << "avg queuing time: " << queueStats.mean
            << " avg wall time: " << wallStats.mean;
}

} // namespace HPHP

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  google::ParseCommandLineFlags(&argc, &argv, false);
  std::srand(folly::randomNumberSeed());
  return RUN_ALL_TESTS();
}
