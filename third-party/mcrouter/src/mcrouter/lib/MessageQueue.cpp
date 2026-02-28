/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MessageQueue.h"

namespace facebook {
namespace memcache {

Notifier::Notifier(
    size_t noNotifyRate,
    int64_t waitThreshold,
    NowUsecFunc nowFunc,
    std::function<bool(bool)> postDrainCallback) noexcept
    : noNotifyRate_(noNotifyRate),
      waitThreshold_(waitThreshold),
      nowFunc_(nowFunc),
      postDrainCallback_(std::move(postDrainCallback)),
      lastTimeUsec_(nowFunc_()),
      state_(State::EMPTY) {}

bool Notifier::shouldNotifyRelaxed() noexcept {
  if (waitThreshold_ && nowFunc_() - waitStart_ > waitThreshold_) {
    return shouldNotify();
  }

  auto period = period_.load();
  if (!period || ((++counter_) % period == 0)) {
    return shouldNotify();
  }

  return false;
}

void Notifier::maybeUpdatePeriod() noexcept {
  if (noNotifyRate_ == 0) {
    return;
  }

  auto now = nowFunc_();
  if (now - lastTimeUsec_ > kUpdatePeriodUsec) {
    auto secElapsed = (double)(now - lastTimeUsec_) / 1000000.0;
    size_t notifyEvery = 1000000;
    double msgPerSec = (double)curMessages_ / secElapsed;
    double p = 1.0 - msgPerSec / noNotifyRate_;
    if (p > 0.0) {
      notifyEvery = 1.0 / p;
    }
    /* Notify on every request is worse than the default notification
       logic, which is more efficient */
    if (notifyEvery == 1) {
      notifyEvery = 0;
    }
    period_ = notifyEvery;
    lastTimeUsec_ = now;
    curMessages_ = 0;
  }
}

} // namespace memcache
} // namespace facebook
