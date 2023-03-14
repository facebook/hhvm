/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxyDestinationBase.h"

#include <chrono>

#include <folly/io/async/AsyncTimeout.h>

#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyDestinationBase.h"
#include "mcrouter/TkoLog.h"
#include "mcrouter/TkoTracker.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

// Probe settings.
constexpr double kProbeExponentialFactor = 1.5;
constexpr double kProbeJitterMin = 0.05;
constexpr double kProbeJitterMax = 0.5;
constexpr double kProbeJitterDelta = kProbeJitterMax - kProbeJitterMin;

static_assert(
    kProbeJitterMax >= kProbeJitterMin,
    "ProbeJitterMax should be greater or equal tham ProbeJitterMin");

} // anonymous namespace

void ProxyDestinationBase::updateTkoStats(GlobalTkoUpdateType type) {
  switch (type) {
    case GlobalTkoUpdateType::INC_SOFT_TKOS:
      proxy_.stats().increment(num_soft_tko_count_stat);
      proxy_.stats().setValue(
          max_num_tko_stat,
          std::max(
              proxy_.stats().getValue(max_num_tko_stat),
              proxy_.stats().getValue(num_soft_tko_count_stat) +
                  proxy_.stats().getValue(num_hard_tko_count_stat)));
      break;
    case GlobalTkoUpdateType::DEC_SOFT_TKOS:
      proxy_.stats().decrement(num_soft_tko_count_stat);
      break;
    case GlobalTkoUpdateType::INC_HARD_TKOS:
      proxy_.stats().increment(num_hard_tko_count_stat);
      proxy_.stats().setValue(
          max_num_tko_stat,
          std::max(
              proxy_.stats().getValue(max_num_tko_stat),
              proxy_.stats().getValue(num_soft_tko_count_stat) +
                  proxy_.stats().getValue(num_hard_tko_count_stat)));
      break;
    case GlobalTkoUpdateType::DEC_HARD_TKOS:
      proxy_.stats().decrement(num_hard_tko_count_stat);
      break;
    case GlobalTkoUpdateType::ENTER_FAIL_OPEN:
      proxy_.stats().increment(num_fail_open_state_entered_stat);
      break;
    case GlobalTkoUpdateType::EXIT_FAIL_OPEN:
      proxy_.stats().increment(num_fail_open_state_exited_stat);
      break;
  }
}

ProxyDestinationBase::ProxyDestinationBase(
    ProxyBase& proxy,
    std::shared_ptr<const AccessPoint> ap,
    std::chrono::milliseconds timeout,
    uint32_t qosClass,
    uint32_t qosPath,
    uint32_t idx)
    : proxy_(proxy),
      accessPoint_(std::move(ap)),
      shortestConnectTimeout_(timeout),
      shortestWriteTimeout_(timeout),
      qosClass_(qosClass),
      qosPath_(qosPath),
      idx_(idx) {
  proxy_.stats().increment(num_servers_new_stat);
  proxy_.stats().increment(num_servers_stat);
  if (accessPoint()->useSsl()) {
    proxy_.stats().increment(num_ssl_servers_new_stat);
    proxy_.stats().increment(num_ssl_servers_stat);
  }
}

ProxyDestinationBase::~ProxyDestinationBase() {
  if (tracker_->removeDestination(this)) {
    onTkoEvent(TkoLogEvent::RemoveFromConfig, carbon::Result::OK);
    stopSendingProbes();
  }
}

void ProxyDestinationBase::updateShortestTimeout(
    std::chrono::milliseconds connectTimeout,
    std::chrono::milliseconds writeTimeout) {
  if (connectTimeout.count() == 0 && writeTimeout.count() == 0) {
    return;
  }
  if (shortestConnectTimeout_.count() == 0 ||
      shortestConnectTimeout_ > connectTimeout ||
      shortestWriteTimeout_.count() == 0 ||
      shortestWriteTimeout_ > writeTimeout) {
    shortestConnectTimeout_ = std::min(shortestConnectTimeout_, connectTimeout);
    shortestWriteTimeout_ = std::min(shortestWriteTimeout_, writeTimeout);
    updateTransportTimeoutsIfShorter(
        shortestConnectTimeout_, shortestWriteTimeout_);
  }
}

bool ProxyDestinationBase::maySend(carbon::Result& tkoReason) const {
  if (tracker_->isTko()) {
    // There's a small race window here, but as the tkoReason is used for
    // logging/debugging purposes only, it's ok to eventually return an
    // outdated value.
    tkoReason = tracker_->tkoReason();
    return false;
  }
  return true;
}

void ProxyDestinationBase::onTkoEvent(TkoLogEvent event, carbon::Result result)
    const {
  auto logUtil = [this, result](folly::StringPiece eventStr) {
    VLOG(2) << accessPoint_.load()->toHostPortString() << " " << eventStr
            << ". Total hard TKOs: " << tracker_->globalTkos().hardTkos
            << "; soft TKOs: " << tracker_->globalTkos().softTkos
            << ". Reply: " << carbon::resultToString(result);
  };

  switch (event) {
    case TkoLogEvent::MarkHardTko:
      logUtil("marked hard TKO");
      break;
    case TkoLogEvent::MarkSoftTko:
      logUtil("marked soft TKO");
      break;
    case TkoLogEvent::UnMarkTko:
      logUtil("unmarked TKO");
      break;
    case TkoLogEvent::RemoveFromConfig:
      logUtil("was TKO, removed from config");
      break;
  }

  TkoLog tkoLog(*accessPoint_.load(), tracker_->globalTkos());
  tkoLog.event = event;
  tkoLog.isHardTko = tracker_->isHardTko();
  tkoLog.isSoftTko = tracker_->isSoftTko();
  tkoLog.avgLatency = stats_.avgLatency.value();
  tkoLog.probesSent = stats_.probesSent;
  tkoLog.result = result;

  logTkoEvent(proxy(), tkoLog);
}

void ProxyDestinationBase::handleTko(
    const carbon::Result result,
    bool isProbeRequest) {
  if (proxy().router().opts().disable_tko_tracking) {
    return;
  }

  if (isErrorResult(result)) {
    if (isHardTkoErrorResult(result)) {
      if (tracker_->recordHardFailure(this, result)) {
        onTkoEvent(TkoLogEvent::MarkHardTko, result);
        startSendingProbes();
      }
    } else if (isSoftTkoErrorResult(result)) {
      if (tracker_->recordSoftFailure(this, result)) {
        onTkoEvent(TkoLogEvent::MarkSoftTko, result);
        startSendingProbes();
      }
    }
    return;
  }

  if (tracker_->isTko()) {
    if (isProbeRequest && tracker_->recordSuccess(this)) {
      onTkoEvent(TkoLogEvent::UnMarkTko, result);
      stopSendingProbes();
    }
    return;
  }

  tracker_->recordSuccess(this);
}

void ProxyDestinationBase::scheduleNextProbe() {
  assert(!proxy().router().opts().disable_tko_tracking);

  int delayMs = probeDelayNextMs;
  if (probeDelayNextMs < 2) {
    // int(1 * 1.5) == 1, so advance it to 2 first
    probeDelayNextMs = 2;
  } else {
    probeDelayNextMs *= kProbeExponentialFactor;
  }
  if (probeDelayNextMs > proxy().router().opts().probe_delay_max_ms) {
    probeDelayNextMs = proxy().router().opts().probe_delay_max_ms;
  }

  // Calculate random jitter
  double r = (double)rand() / (double)RAND_MAX;
  double tmo_jitter_pct = r * kProbeJitterDelta + kProbeJitterMin;
  delayMs = (double)delayMs * (1.0 + tmo_jitter_pct);
  assert(delayMs > 0);

  if (!probeTimer_->scheduleTimeout(delayMs)) {
    MC_LOG_FAILURE(
        proxy().router().opts(),
        failure::Category::kSystemError,
        "failed to schedule probe timer for ProxyDestination");
  }
}

void ProxyDestinationBase::startSendingProbes() {
  probeDelayNextMs = proxy().router().opts().probe_delay_initial_ms;
  probeTimer_ =
      folly::AsyncTimeout::make(proxy().eventBase(), [this]() noexcept {
        // Note that the previous probe might still be in flight
        if (!probeInflight_) {
          probeInflight_ = true;
          ++stats_.probesSent;
          proxy().fiberManager().addTask([selfPtr = selfPtr()]() mutable {
            auto pdstn = selfPtr.lock();
            if (pdstn == nullptr) {
              return;
            }
            pdstn->markAsActive();
            auto result = pdstn->sendProbe();
            pdstn->handleTko(result, /* isProbeRequest */ true);
            pdstn->probeInflight_ = false;
          });
        }
        scheduleNextProbe();
      });
  scheduleNextProbe();
}

void ProxyDestinationBase::stopSendingProbes() {
  stats_.probesSent = 0;
  probeTimer_.reset();
}

void ProxyDestinationBase::updateConnectionClosedInternalStat() {
  if (stats().inactiveConnectionClosedTimestampUs != 0) {
    std::chrono::microseconds timeClosed(
        nowUs() - stats().inactiveConnectionClosedTimestampUs);
    proxy().stats().inactiveConnectionClosedIntervalSec().insertSample(
        std::chrono::duration_cast<std::chrono::seconds>(timeClosed).count());

    // resets inactiveConnectionClosedTimestampUs, as we just want to take
    // into account connections that were closed due to lack of activity
    stats().inactiveConnectionClosedTimestampUs = 0;
  }
}

void ProxyDestinationBase::setPoolStatsIndex(int32_t index) {
  proxy().eventBase().runInEventBaseThread([selfPtr = selfPtr(), index]() {
    auto pdstn = selfPtr.lock();
    if (!pdstn) {
      return;
    }
    pdstn->stats().poolStatIndex_ = index;
    if (pdstn->stats().state == State::Up) {
      if (auto* poolStats = pdstn->proxy().stats().getPoolStats(index)) {
        poolStats->updateConnections(1);
      }
    }
  });
}

void ProxyDestinationBase::updatePoolStatConnections(bool connected) {
  if (auto poolStats = proxy().stats().getPoolStats(stats().poolStatIndex_)) {
    poolStats->updateConnections(connected ? 1 : -1);
  }
}

void ProxyDestinationBase::onTransitionToState(State st) {
  onTransitionImpl(st, true /* to */);
}

void ProxyDestinationBase::onTransitionFromState(State st) {
  onTransitionImpl(st, false /* to */);
}

void ProxyDestinationBase::onTransitionImpl(State st, bool to) {
  const int64_t delta = to ? 1 : -1;
  const bool ssl = accessPoint()->useSsl();

  switch (st) {
    case ProxyDestinationBase::State::New: {
      proxy().stats().increment(num_servers_new_stat, delta);
      if (ssl) {
        proxy().stats().increment(num_ssl_servers_new_stat, delta);
      }
      break;
    }
    case ProxyDestinationBase::State::Up: {
      proxy().stats().increment(num_servers_up_stat, delta);
      if (ssl) {
        proxy().stats().increment(num_ssl_servers_up_stat, delta);
      }
      break;
    }
    case ProxyDestinationBase::State::Closed: {
      proxy().stats().increment(num_servers_closed_stat, delta);
      if (ssl) {
        proxy().stats().increment(num_ssl_servers_closed_stat, delta);
      }
      break;
    }
    case ProxyDestinationBase::State::Down: {
      proxy().stats().increment(num_servers_down_stat, delta);
      if (ssl) {
        proxy().stats().increment(num_ssl_servers_down_stat, delta);
      }
      break;
    }
    case ProxyDestinationBase::State::NumStates: {
      CHECK(false);
      break;
    }
  }
}

void ProxyDestinationBase::setState(State newState) {
  if (stats().state == newState) {
    return;
  }

  auto logUtil = [this](const char* s) {
    VLOG(3) << "server " << ProxyDestinationKey(*this).str() << " " << s << " ("
            << proxy().stats().getValue(num_servers_up_stat) << " of "
            << proxy().stats().getValue(num_servers_stat) << ")";
  };

  onTransitionFromState(stats().state);
  onTransitionToState(newState);
  stats().state = newState;

  switch (stats().state) {
    case State::Up:
      logUtil("up");
      break;
    case State::Closed:
      logUtil("closed");
      break;
    case State::Down:
      logUtil("down");
      break;
    case State::New:
    case State::NumStates:
      assert(false);
      break;
  }
}

void ProxyDestinationBase::markAsActive() {
  proxy().destinationMap()->markAsActive(*this);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
