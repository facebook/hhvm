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

#include <thrift/lib/cpp2/server/AdaptiveConcurrency.h>

#include <folly/portability/GTest.h>

#include <folly/experimental/observer/SimpleObservable.h>

using namespace std::literals::chrono_literals;
using namespace apache::thrift;

using Clock = std::chrono::steady_clock;

namespace apache::thrift {
class AdaptiveConcurrencyTestHelper {
 public:
  explicit AdaptiveConcurrencyTestHelper(AdaptiveConcurrencyController& c)
      : c_(c) {}
  auto samplingPeriodStart() const { return c_.samplingPeriodStart(); }
  auto samplingPeriodStart(Clock::time_point v) { c_.samplingPeriodStart(v); }
  auto rttRecalcStart() const { return c_.rttRecalcStart(); }
  auto nextRttRecalcStart() const { return c_.nextRttRecalcStart(); }
  auto nextRttRecalcStart(Clock::time_point t) { c_.nextRttRecalcStart(t); }

 private:
  apache::thrift::AdaptiveConcurrencyController& c_;
};
} // namespace apache::thrift

// helper to call private methods on the controller
AdaptiveConcurrencyTestHelper p(AdaptiveConcurrencyController& c) {
  return AdaptiveConcurrencyTestHelper(c);
}

template <size_t MinConcurrency = 5>
class AdaptiveConcurrencyBase : public testing::Test {
 protected:
  static constexpr Clock::duration kMinRtt = 1ms;
  static constexpr Clock::duration kIdealRtt = 10ms;
  static constexpr uint32_t minConcurrency = MinConcurrency;

  AdaptiveConcurrencyBase(uint32_t maxRequests = 0)
      : oMaxRequests(maxRequests) {}

  void setConfig(
      size_t concurrency,
      double jitter = 0.0,
      std::chrono::milliseconds targetRttFixed = {},
      std::chrono::milliseconds minTargetRtt = {},
      double targetRttPercentile = 0.95) {
    oConfig.setValue(makeConfig(
        concurrency,
        jitter,
        targetRttFixed,
        minTargetRtt,
        targetRttPercentile));
    folly::observer_detail::ObserverManager::waitForAllUpdates();
  }

  static auto makeConfig(
      size_t concurrency,
      double jitter = 0.0,
      std::chrono::milliseconds targetRttFixed = {},
      std::chrono::milliseconds minTargetRtt = {},
      double targetRttPercentile = 0.95) {
    AdaptiveConcurrencyController::Config config;
    config.minConcurrency = concurrency;
    config.recalcPeriodJitter = jitter;
    config.targetRttFixed = targetRttFixed;
    config.minTargetRtt = minTargetRtt;
    config.targetRttPercentile = targetRttPercentile;
    return config;
  }

  folly::observer::SimpleObservable<AdaptiveConcurrencyController::Config>
      oConfig{makeConfig(MinConcurrency)};
  folly::observer::SimpleObservable<uint32_t> oMaxRequests{0u};

  AdaptiveConcurrencyController controller{
      oConfig.getObserver(), oMaxRequests.getObserver()};

  void makeRequest(Clock::duration latency = kIdealRtt) {
    auto now = Clock::now();
    controller.requestStarted(now);
    controller.requestFinished(now, now + latency);
  }

  enum class EndState {
    SamplingScheduled,
    RecalcScheduled,
  };

  void doSamplingRequests(
      Clock::duration latency = kIdealRtt,
      EndState endState = EndState::SamplingScheduled) {
    // execute 200 samples to collect stats, new ideal latency is recomputed
    // at the end
    auto before = Clock::now();

    for (int i = 0; i < 200; i++) {
      EXPECT_GT(p(controller).samplingPeriodStart(), Clock::time_point{});
      EXPECT_LT(p(controller).samplingPeriodStart(), Clock::now());
      makeRequest(latency);
    }
    switch (endState) {
      case EndState::RecalcScheduled:
        EXPECT_EQ(
            p(controller).rttRecalcStart(), p(controller).nextRttRecalcStart());
        break;
      case EndState::SamplingScheduled:
        EXPECT_GT(p(controller).samplingPeriodStart(), before + 500ms);
        EXPECT_LT(p(controller).samplingPeriodStart(), Clock::now() + 500ms);
        break;
    }
  }

  void performSampling(
      Clock::duration latency = kIdealRtt,
      EndState endState = EndState::SamplingScheduled) {
    auto concurrency = controller.getMaxRequests();
    p(controller).samplingPeriodStart(Clock::now());
    doSamplingRequests(latency, endState);
    double slope = 1.0 * controller.targetRtt() / latency;
    uint32_t expectConcurrency =
        slope * concurrency + std::sqrt(slope * concurrency);
    auto maxRequests = **oMaxRequests.getObserver();
    expectConcurrency = std::min(
        expectConcurrency,
        (maxRequests == 0) ? 1000u : std::min(1000u, maxRequests));
    expectConcurrency = std::max(minConcurrency, expectConcurrency);
    EXPECT_EQ(controller.getMaxRequests(), expectConcurrency);
    // targetRtt is computed to be 2x the observed p95 latency
    EXPECT_EQ(controller.targetRtt(), 2 * kIdealRtt);
  }
};

class AdaptiveConcurrency : public AdaptiveConcurrencyBase<5> {};
class AdaptiveConcurrencyDisabled : public AdaptiveConcurrencyBase<0> {};

TEST_F(AdaptiveConcurrency, IdealRttCalc) {
  // first request primes the system to start ideal rtt calibration
  EXPECT_EQ(p(controller).samplingPeriodStart(), Clock::time_point{});
  {
    auto now = Clock::now();
    makeRequest();
    EXPECT_GT(p(controller).samplingPeriodStart(), now);
  }

  // test latency computations w/ changing input
  p(controller).samplingPeriodStart(Clock::now());
  for (int i = 0; i < 200; i++) {
    makeRequest(std::chrono::milliseconds(i) + 100ms);
  }
  // target should now be twice the p95 of the sampled latencies
  EXPECT_EQ(controller.targetRtt(), 2 * 290ms);
}

class AdaptiveConcurrencyP : public AdaptiveConcurrencyBase<5>,
                             public ::testing::WithParamInterface<uint32_t> {
 public:
  AdaptiveConcurrencyP() : AdaptiveConcurrencyBase<5>{GetParam()} {}
};

TEST_P(AdaptiveConcurrencyP, Sampling) {
  // calibrate ideal rtt
  makeRequest();
  doSamplingRequests(kIdealRtt);

  uint32_t maxRequests = **oMaxRequests.getObserver();
  maxRequests = maxRequests == 0 ? 1000u : std::min(maxRequests, 1000u);

  // imitate low request latencies
  // concurrency limit grows until reaching max of 1000 (or less if server
  // provided maxRequests is smaller than 1000)
  auto lowLatency = kIdealRtt;
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 13);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 31);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 69);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 149);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 315);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 655);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);

  // imitate higher latencies
  // concurrency limit drops until reaching min of 5
  auto highLatency = 4 * kIdealRtt;
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  maxRequests = maxRequests / 2.0 + std::sqrt(maxRequests / 2.0);
  EXPECT_EQ(controller.getMaxRequests(), maxRequests);
  performSampling(highLatency);
  EXPECT_EQ(controller.getMaxRequests(), 7);
  performSampling(highLatency);
  EXPECT_EQ(controller.getMaxRequests(), 5);
  performSampling(highLatency);
  EXPECT_EQ(controller.getMaxRequests(), 5);
  performSampling(highLatency);
  EXPECT_EQ(controller.getMaxRequests(), 5);

  // validate making requests before sampling periods does not affect
  // the sampling
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 13);
  for (int i = 0; i < 200; ++i) {
    makeRequest(highLatency);
  }
  performSampling(lowLatency);
  EXPECT_EQ(controller.getMaxRequests(), 31);
}

INSTANTIATE_TEST_CASE_P(
    AdaptiveConcurrencySequence,
    AdaptiveConcurrencyP,
    testing::Values(0, 900, 1000, 5000));

TEST_F(AdaptiveConcurrency, RttRecalibration) {
  // calibrate ideal rtt
  setConfig(5, 0.2);

  auto before = Clock::now();
  makeRequest();
  auto after = Clock::now();
  doSamplingRequests(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_GT(p(controller).nextRttRecalcStart(), before + 4min);
  EXPECT_LT(p(controller).nextRttRecalcStart(), after + 6min);

  performSampling(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_EQ(controller.getMaxRequests(), 13);

  // force entering calibration mode after the next sampling round
  // and perform sampling round
  p(controller).nextRttRecalcStart(Clock::now());
  performSampling(kIdealRtt, EndState::RecalcScheduled);

  // next request will cause concurrency to reset to the minimum
  // also resets next recalibration target time
  EXPECT_EQ(controller.getMaxRequests(), 31);
  before = Clock::now();
  EXPECT_LT(p(controller).nextRttRecalcStart(), before);
  makeRequest();
  EXPECT_GT(p(controller).nextRttRecalcStart(), before + 4min);
  EXPECT_LT(p(controller).nextRttRecalcStart(), Clock::now() + 6min);
  EXPECT_EQ(controller.getMaxRequests(), 5);

  // run recalibration, at the end of the calibration concurrency
  // should be return to pre-calibration value
  doSamplingRequests(kIdealRtt * 2);
  EXPECT_EQ(controller.getMaxRequests(), 31);

  // targetRtt is computed to be 2x the observed p95 latency
  EXPECT_EQ(controller.targetRtt(), 4 * kIdealRtt);
}

TEST_F(AdaptiveConcurrency, FixedTargetRtt) {
  // use fixed target rtt
  setConfig(
      5,
      0.2,
      std::chrono::duration_cast<std::chrono::milliseconds>(kIdealRtt * 2));

  makeRequest();
  doSamplingRequests(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_EQ(p(controller).nextRttRecalcStart(), Clock::time_point{});
  EXPECT_EQ(controller.getMaxRequests(), 5);

  performSampling(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_EQ(controller.getMaxRequests(), 13);
  EXPECT_EQ(p(controller).nextRttRecalcStart(), Clock::time_point{});

  setConfig(5, 0.2);

  auto before = Clock::now();
  makeRequest();
  auto after = Clock::now();
  doSamplingRequests(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_GT(p(controller).nextRttRecalcStart(), before + 4min);
  EXPECT_LT(p(controller).nextRttRecalcStart(), after + 6min);
  EXPECT_EQ(controller.getMaxRequests(), 5);

  setConfig(
      5,
      0.2,
      std::chrono::duration_cast<std::chrono::milliseconds>(kIdealRtt * 2));

  makeRequest();
  doSamplingRequests(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_EQ(p(controller).nextRttRecalcStart(), Clock::time_point{});
  EXPECT_EQ(controller.getMaxRequests(), 5);

  performSampling(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_EQ(controller.getMaxRequests(), 13);
  EXPECT_EQ(p(controller).nextRttRecalcStart(), Clock::time_point{});
}

class TargetRttPercentileTestP : public AdaptiveConcurrencyBase<5>,
                                 public ::testing::WithParamInterface<double> {
 public:
  TargetRttPercentileTestP() : targetRttPct_(GetParam()) {}

 protected:
  double targetRttPct_{0.95};
  // Range of pct values to be tested. Note that values out side
  // of the range (0.0, 1.0) (eclusive) are invalid.
  std::map<double, std::chrono::milliseconds> pctToTarget_{
      {std::numeric_limits<double>::lowest(), 2 * 100ms},
      {0.0, 2 * 100ms},
      {0.25, 2 * 150ms},
      {0.50, 2 * 200ms},
      {0.75, 2 * 250ms},
      {0.95, 2 * 290ms},
      {1.0, 2 * 299ms},
      {2.0, 2 * 299ms},
      {std::numeric_limits<double>::max(), 2 * 299ms}};
};

TEST_P(TargetRttPercentileTestP, TargetRttPercentileTest) {
  // Validate that changing the targetRTT percentile will result in
  // the appropriate targetRTT being selected during the sampling period.
  // The test basically configures a range of targetRTT pct values
  // and then verifies that the correct target latency is picked.
  EXPECT_EQ(p(controller).samplingPeriodStart(), Clock::time_point{});
  setConfig(
      5,
      0.2,
      std::chrono::milliseconds{},
      std::chrono::milliseconds{},
      targetRttPct_);
  {
    auto now = Clock::now();
    makeRequest();
    EXPECT_GT(p(controller).samplingPeriodStart(), now);
  }

  p(controller).samplingPeriodStart(Clock::now());
  for (int i = 0; i < 200; i++) {
    makeRequest(std::chrono::milliseconds(i) + 100ms);
  }
  // target should be 2 x pctTargetRtt of the sampled latencies
  EXPECT_EQ(controller.targetRtt(), pctToTarget_[targetRttPct_]);
}

INSTANTIATE_TEST_CASE_P(
    TargetRttPercentileTestSequence,
    TargetRttPercentileTestP,
    testing::Values(
        std::numeric_limits<double>::lowest(),
        0.0,
        0.25,
        0.50,
        0.75,
        0.95,
        1.0,
        2.0,
        std::numeric_limits<double>::max()));

TEST_F(AdaptiveConcurrency, MinTargetRtt) {
  // Validate that the minimum target RTT setting is observed if specified.
  Clock::duration minTargetRtt = 5ms;
  EXPECT_EQ(controller.minTargetRtt(), std::chrono::microseconds{});

  setConfig(
      5,
      0.2,
      std::chrono::milliseconds{},
      std::chrono::duration_cast<std::chrono::milliseconds>(minTargetRtt));

  // The stored min target RTT value is updated upon recalibration.
  makeRequest();
  doSamplingRequests(kMinRtt);

  EXPECT_EQ(
      controller.minTargetRtt(),
      std::chrono::duration_cast<std::chrono::microseconds>(minTargetRtt));
  EXPECT_EQ(controller.getMaxRequests(), 5);
  // The target RTT should be floored at 5ms (minTargetRtt) even though the
  // sampled RTT times will be smaller (1 ms).
  EXPECT_EQ(controller.targetRtt(), minTargetRtt);
  EXPECT_EQ(controller.sampledRtt(), kMinRtt);

  // Recalibrate again by bumping up the sampled RTT latencies to kIdealRtt.
  // The minTargetRtt shouldn't change after this but the sampled RTT should.
  p(controller).samplingPeriodStart(Clock::now());
  doSamplingRequests(kIdealRtt);
  EXPECT_EQ(controller.getMaxRequests(), 5);
  EXPECT_EQ(controller.targetRtt(), minTargetRtt);
  EXPECT_EQ(controller.sampledRtt(), kIdealRtt);

  // Reset the minTargetRTT to a value higher than kIdealRtt and redo the same
  // checks.
  minTargetRtt = 30ms;
  setConfig(
      5,
      0.2,
      std::chrono::milliseconds{},
      std::chrono::duration_cast<std::chrono::milliseconds>(minTargetRtt));
  makeRequest();
  p(controller).samplingPeriodStart(Clock::now());
  doSamplingRequests(kIdealRtt);
  // Concurrency shouldn't be bumped up since the minimum limit is higher
  // than the sampled latencies. So even though requests are getting processed
  // faster, we still adhere to the configured lower limit.
  EXPECT_EQ(controller.getMaxRequests(), 5);
  EXPECT_EQ(controller.targetRtt(), minTargetRtt);
  EXPECT_EQ(controller.sampledRtt(), kIdealRtt);

  // Now remove the minimum latency limit and redo the checks.
  setConfig(5);
  makeRequest();
  p(controller).samplingPeriodStart(Clock::now());
  doSamplingRequests(kIdealRtt, EndState::SamplingScheduled);
  EXPECT_EQ(controller.minTargetRtt(), std::chrono::microseconds{});
  EXPECT_EQ(controller.getMaxRequests(), 5);
  EXPECT_EQ(controller.targetRtt(), 2 * kIdealRtt);
  EXPECT_EQ(controller.sampledRtt(), kIdealRtt);

  // Use a smaller sampled latency value and verify the target RTT
  // goes below minTargetRtt (since it's been disabled).
  setConfig(5);
  makeRequest();
  p(controller).samplingPeriodStart(Clock::now());
  doSamplingRequests(kMinRtt);
  EXPECT_EQ(controller.getMaxRequests(), 5);
  EXPECT_EQ(controller.targetRtt(), 2 * kMinRtt);
  EXPECT_EQ(controller.sampledRtt(), kMinRtt);
}

TEST_F(AdaptiveConcurrencyDisabled, Enabling) {
  makeRequest();

  EXPECT_EQ(controller.getMaxRequests(), 0);
  EXPECT_EQ(p(controller).samplingPeriodStart(), Clock::time_point{});

  setConfig(5);

  // first request primes the system to start ideal rtt calibration
  EXPECT_EQ(p(controller).samplingPeriodStart(), Clock::time_point{});
  {
    auto now = Clock::now();
    makeRequest();
    EXPECT_GT(p(controller).samplingPeriodStart(), now);
  }
  EXPECT_EQ(controller.getMaxRequests(), 5);
}

TEST_F(AdaptiveConcurrency, Disabling) {
  makeRequest();

  // disable controller
  setConfig(0);
  EXPECT_EQ(controller.getMaxRequests(), 0);

  // re-enable controller
  setConfig(minConcurrency);
  EXPECT_EQ(controller.getMaxRequests(), 0);
  makeRequest();
  EXPECT_EQ(controller.getMaxRequests(), 5);

  doSamplingRequests(kIdealRtt, EndState::SamplingScheduled);

  // disable controller again
  setConfig(0);
  EXPECT_EQ(controller.getMaxRequests(), 0);
}
