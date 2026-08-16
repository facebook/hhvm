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

#include <thrift/conformance/stresstest/server/StressTestStatsLogger.h>

#include <glog/logging.h>
#include <folly/io/async/IoUringBackend.h>

namespace apache::thrift::stress {

#if FOLLY_HAS_LIBURING
class IoUringStatsTimer : public folly::AsyncTimeout {
 public:
  IoUringStatsTimer(
      folly::EventBase* evb, size_t evbIdx, uint32_t intervalSeconds)
      : folly::AsyncTimeout(evb),
        evb_(evb),
        evbIdx_(evbIdx),
        intervalMs_(intervalSeconds * 1000) {}

  void timeoutExpired() noexcept override {
    auto* backend = dynamic_cast<folly::IoUringBackend*>(evb_->getBackend());
    if (!backend) {
      return;
    }
    auto stats = backend->getStats();

    if (backend->zcBufferPool()) {
      auto& prevZcrx = stats_.zcrx;
      auto zcrx = stats.zcrx;
      prevZcrx.copyFallbackCount += zcrx.copyFallbackCount;
      prevZcrx.copyFallbackBytes += zcrx.copyFallbackBytes;
      prevZcrx.noBufferCount += zcrx.noBufferCount;
      LOG(INFO) << "Zero copy stats for evb " << evbIdx_ << ":" << std::endl
                << " - copyFallbackCount : " << prevZcrx.copyFallbackCount
                << std::endl
                << " - copyFallbackBytes: " << prevZcrx.copyFallbackBytes
                << std::endl
                << " - noBufferCount: " << prevZcrx.noBufferCount;
    } else {
      auto prov = stats.providedBuffer;
      stats_.providedBuffer.enobufCount += prov.enobufCount;

      LOG(INFO) << "Provided buffer stats for evb " << evbIdx_ << ":"
                << std::endl
                << " - enobufCount: " << stats_.providedBuffer.enobufCount
                << std::endl
                << " - utilPct: " << prov.utilPct;
    }

    scheduleTimeout(std::chrono::milliseconds(intervalMs_));
  }

 private:
  folly::EventBase* evb_;
  size_t evbIdx_;
  uint32_t intervalMs_;
  folly::IoUringBackend::IoUringStats stats_{};
};

void StressTestStatsLogger::init(
    std::vector<folly::Executor::KeepAlive<folly::EventBase>>& evbs,
    uint32_t intervalSeconds) {
  if (intervalSeconds == 0) {
    return;
  }
  timers_.reserve(evbs.size());
  for (size_t i = 0; i < evbs.size(); i++) {
    auto* backend = dynamic_cast<folly::IoUringBackend*>(evbs[i]->getBackend());
    if (!backend) {
      continue;
    }

    auto timer =
        std::make_shared<IoUringStatsTimer>(evbs[i].get(), i, intervalSeconds);

    std::weak_ptr<IoUringStatsTimer> weakTimer = timer;
    evbs[i]->runInEventBaseThread([weakTimer, intervalSeconds] {
      if (auto timer = weakTimer.lock()) {
        timer->scheduleTimeout(
            std::chrono::milliseconds(intervalSeconds * 1000));
      }
    });
    evbs[i]->runOnDestruction([weakTimer] {
      if (auto timer = weakTimer.lock()) {
        timer->cancelTimeout();
      }
    });
    timers_.push_back(timer);
  }
}
#else
void StressTestStatsLogger::init(
    std::vector<folly::Executor::KeepAlive<folly::EventBase>>&,
    uint32_t /* intervalSeconds */) {}
#endif

} // namespace apache::thrift::stress
