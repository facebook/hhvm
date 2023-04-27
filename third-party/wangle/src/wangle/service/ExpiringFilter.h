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

#pragma once

#include <wangle/service/Service.h>

namespace wangle {
/**
 * A service filter that expires the self service after a certain
 * amount of idle time, or after a maximum amount of time total.
 * Idle timeout is cancelled when any requests are outstanding.
 */

template <typename Req, typename Resp = Req>
class ExpiringFilter : public ServiceFilter<Req, Resp> {
 public:
  explicit ExpiringFilter(
      std::shared_ptr<Service<Req, Resp>> service,
      std::chrono::milliseconds idleTimeoutTime = std::chrono::milliseconds(0),
      std::chrono::milliseconds maxTime = std::chrono::milliseconds(0),
      folly::Timekeeper* timekeeper = nullptr)
      : ServiceFilter<Req, Resp>(service),
        idleTimeoutTime_(idleTimeoutTime),
        maxTime_(maxTime),
        timekeeper_(timekeeper) {
    if (maxTime_ > std::chrono::milliseconds(0)) {
      maxTimeout_ = folly::futures::sleepUnsafe(maxTime_, timekeeper_);
      std::move(maxTimeout_).thenValue([this](auto&&) { this->close(); });
    }
    startIdleTimer();
  }

  ~ExpiringFilter() override {
    if (!idleTimeout_.isReady()) {
      idleTimeout_.cancel();
    }
    if (!maxTimeout_.isReady()) {
      maxTimeout_.cancel();
    }
  }

  void startIdleTimer() {
    if (requests_ != 0) {
      return;
    }
    if (idleTimeoutTime_ > std::chrono::milliseconds(0)) {
      idleTimeout_ = folly::futures::sleepUnsafe(idleTimeoutTime_, timekeeper_);
      std::move(idleTimeout_).thenValue([this](auto&&) { this->close(); });
    }
  }

  folly::Future<Resp> operator()(Req req) override {
    if (!idleTimeout_.isReady()) {
      idleTimeout_.cancel();
    }
    requests_++;
    return (*this->service_)(std::move(req)).ensure([this]() {
      requests_--;
      startIdleTimer();
    });
  }

 private:
  folly::Future<folly::Unit> idleTimeout_;
  folly::Future<folly::Unit> maxTimeout_;
  std::chrono::milliseconds idleTimeoutTime_{0};
  std::chrono::milliseconds maxTime_{0};
  folly::Timekeeper* timekeeper_;
  uint32_t requests_{0};
};

} // namespace wangle
