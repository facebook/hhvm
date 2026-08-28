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

#include <thrift/conformance/stresstest/server/StressTestServerStats.h>

#include <optional>
#include <set>
#include <utility>

#include <glog/logging.h>
#include <folly/io/async/IoUringBackend.h>
#include <folly/system/ThreadId.h>

namespace apache::thrift::stress {

#if FOLLY_HAS_LIBURING
class StressTestServerStats::IoUringStatsTimer : public folly::AsyncTimeout {
 public:
  IoUringStatsTimer(
      folly::EventBase* evb,
      folly::IoUringBackend* backend,
      size_t evbIdx,
      uint32_t intervalSeconds)
      : folly::AsyncTimeout(evb),
        backend_(backend),
        evbIdx_(evbIdx),
        intervalMs_(intervalSeconds * 1000) {}

  void timeoutExpired() noexcept override {
    collect();
    logStats();
    scheduleTimeout(std::chrono::milliseconds(intervalMs_));
  }

  void collect() noexcept {
    auto stats = backend_->getStats();

    if (backend_->zcBufferPool()) {
      auto& prevZcrx = stats_.zcrx;
      auto zcrx = stats.zcrx;
      prevZcrx.copyFallbackCount += zcrx.copyFallbackCount;
      prevZcrx.copyFallbackBytes += zcrx.copyFallbackBytes;
      prevZcrx.noBufferCount += zcrx.noBufferCount;
    } else {
      auto prov = stats.providedBuffer;
      stats_.providedBuffer.enobufCount += prov.enobufCount;
      stats_.providedBuffer.utilPct = prov.utilPct;
    }
  }

  void logStats() const {
    if (backend_->zcBufferPool()) {
      const auto& zcrx = stats_.zcrx;
      LOG(INFO) << "Zero copy stats for evb " << evbIdx_ << ":" << std::endl
                << " - copyFallbackCount : " << zcrx.copyFallbackCount
                << std::endl
                << " - copyFallbackBytes: " << zcrx.copyFallbackBytes
                << std::endl
                << " - noBufferCount: " << zcrx.noBufferCount;
    } else {
      const auto& providedBuffer = stats_.providedBuffer;
      LOG(INFO) << "Provided buffer stats for evb " << evbIdx_ << ":"
                << std::endl
                << " - enobufCount: " << providedBuffer.enobufCount << std::endl
                << " - utilPct: " << providedBuffer.utilPct;
    }
  }

  std::optional<folly::IoUringZeroCopyBufferPool::Stats> zcrxStats()
      const noexcept {
    if (!backend_->zcBufferPool()) {
      return std::nullopt;
    }
    return stats_.zcrx;
  }

 private:
  folly::IoUringBackend* backend_;
  size_t evbIdx_;
  uint32_t intervalMs_;
  folly::IoUringBackend::IoUringStats stats_{};
};

void StressTestServerStats::init(
    std::vector<folly::Executor::KeepAlive<folly::EventBase>>& evbs,
    uint32_t intervalSeconds) {
  timers_.reserve(evbs.size());
  for (size_t i = 0; i < evbs.size(); i++) {
    auto* backend = dynamic_cast<folly::IoUringBackend*>(evbs[i]->getBackend());
    if (!backend) {
      continue;
    }

    auto timer = std::make_shared<IoUringStatsTimer>(
        evbs[i].get(), backend, i, intervalSeconds);

    std::weak_ptr<IoUringStatsTimer> weakTimer{timer};
    if (intervalSeconds > 0) {
      evbs[i]->runInEventBaseThread([weakTimer, intervalSeconds] {
        if (auto timer = weakTimer.lock()) {
          timer->scheduleTimeout(
              std::chrono::milliseconds(intervalSeconds * 1000));
        }
      });
    }
    evbs[i]->runOnDestruction([weakTimer] {
      if (auto timer = weakTimer.lock()) {
        timer->cancelTimeout();
      }
    });
    timers_.push_back({evbs[i].get(), std::move(timer)});
  }
}

void StressTestServerStats::collectFinal() {
  int64_t eventBaseCount = 0;
  std::set<int64_t> ioThreadIds;
  folly::IoUringZeroCopyBufferPool::Stats zcrxStats;
  for (const auto& [eventBase, timer] : timers_) {
    eventBase->runInEventBaseThreadAndWait([&] {
      timer->cancelTimeout();
      timer->collect();
      const auto stats = timer->zcrxStats();
      if (!stats) {
        return;
      }
      ++eventBaseCount;
      ioThreadIds.insert(static_cast<int64_t>(folly::getOSThreadID()));
      zcrxStats.copyFallbackCount += stats->copyFallbackCount;
      zcrxStats.copyFallbackBytes += stats->copyFallbackBytes;
      zcrxStats.noBufferCount += stats->noBufferCount;
    });
  }
  zcrxCounters_.eventBaseCount() = eventBaseCount;
  zcrxCounters_.ioThreadIds() = std::move(ioThreadIds);
  zcrxCounters_.copyFallbackCount() =
      static_cast<int64_t>(zcrxStats.copyFallbackCount);
  zcrxCounters_.copyFallbackBytes() =
      static_cast<int64_t>(zcrxStats.copyFallbackBytes);
  zcrxCounters_.noBufferCount() = static_cast<int64_t>(zcrxStats.noBufferCount);
}
#else
void StressTestServerStats::init(
    std::vector<folly::Executor::KeepAlive<folly::EventBase>>&,
    uint32_t /* intervalSeconds */) {}

void StressTestServerStats::collectFinal() {}
#endif

void StressTestServerStats::recordConnection(ConnectionMode mode) {
  auto connectionState = connectionState_.wlock();
  switch (mode) {
    case ConnectionMode::Plaintext:
      ++connectionState->plaintextConnections;
      break;
    case ConnectionMode::Tls:
      ++connectionState->tlsConnections;
      break;
    case ConnectionMode::StopTlsV2:
      ++connectionState->stopTlsV2Connections;
      break;
    case ConnectionMode::Unknown:
      ++connectionState->unknownConnections;
      break;
  }
}

ServerResult StressTestServerStats::publish(ResultMetadata metadata) const {
  auto connectionState = connectionState_.rlock();
  ServerResult result;
  result.metadata() = std::move(metadata);
  result.connections() = {
      {ConnectionMode::Unknown, connectionState->unknownConnections},
      {ConnectionMode::Plaintext, connectionState->plaintextConnections},
      {ConnectionMode::Tls, connectionState->tlsConnections},
      {ConnectionMode::StopTlsV2, connectionState->stopTlsV2Connections},
  };
  result.zcrx() = zcrxCounters_;
  return result;
}

} // namespace apache::thrift::stress
