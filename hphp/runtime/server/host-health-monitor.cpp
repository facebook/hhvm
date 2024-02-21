/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/server/host-health-monitor.h"

#include <folly/Singleton.h>
#include <folly/system/ThreadName.h>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/alloc-defs.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/health-monitor-types.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"

namespace HPHP {

namespace {
bool Enabled;
int32_t MaxUpdatePeriod;
auto DampenTime = std::chrono::milliseconds{0};
int32_t ProcStatusUpdateSeconds;

struct HostHealthMonitorExtension final : Extension {
  HostHealthMonitorExtension() : Extension("hosthealthmonitor", "1.0", NO_ONCALL_YET) {}

  void moduleLoad(const IniSetting::Map& ini, Hdf globalConfig) override {
    Config::Bind(Enabled, ini, globalConfig,
                 "HealthMonitor.EnableHealthMonitor", true);
    Config::Bind(MaxUpdatePeriod, ini, globalConfig,
                 "HealthMonitor.MaxUpdatePeriod", 100 /* miliseconds */);
    auto const dampenMs =
      Config::GetInt32(ini, globalConfig, "HealthMonitor.DampenTimeMs", 0);
    if (dampenMs > 0) DampenTime = std::chrono::milliseconds(dampenMs);
    Config::Bind(ProcStatusUpdateSeconds, ini, globalConfig,
                 "HealthMonitor.ProcStatusUpdateSeconds", 1);
  }
  std::vector<std::string> hackFiles() const override {
    return {};
  }
} s_host_health_monitor_extension;

folly::Singleton<HostHealthMonitor> s_health_monitor;

}

void HostHealthMonitor::addMetric(IHealthMonitorMetric* metric) {
  assertx(metric != nullptr);
  std::lock_guard<std::mutex> g(m_lock);
  m_metrics.push_back(metric);
}

void IHealthMonitorMetric::registerSelf() {
  folly::Singleton<HostHealthMonitor>::try_get()->addMetric(this);
}

void HostHealthMonitor::start() {
  if (!Enabled || !m_stopped.load(std::memory_order_acquire)) return;
  if (MaxUpdatePeriod < 10) MaxUpdatePeriod = 10;
  if (MaxUpdatePeriod > 10000) MaxUpdatePeriod = 10000;

  m_monitor_thread = std::make_unique<std::thread>([] {
    folly::setThreadName("HostHealthMonitor");
    folly::Singleton<HostHealthMonitor>::try_get()->monitor();
  });

  // Make sure the thread is gone after hphp_process_exit().  The node
  // is intentionally leaked.
  new InitFiniNode(
    [] { folly::Singleton<HostHealthMonitor>::try_get()->waitForEnd(); },
    InitFiniNode::When::ProcessExit
  );
}

void HostHealthMonitor::stop() {
  if (m_stopped.exchange(true, std::memory_order_acq_rel)) {
    return;                             // already stopped.
  }
  notifyObservers(HealthLevel::Bold);
  std::unique_lock<std::mutex> guard(m_condvar_lock);
  m_condition.notify_one();
}

void HostHealthMonitor::waitForEnd() {
  stop();
  if (m_monitor_thread) {
    m_monitor_thread->join();
    m_monitor_thread.reset();
  }
}

void HostHealthMonitor::monitor() {
  Logger::Info("Host health monitor starts working.");
  m_healthLevelCounter =
    ServiceData::createTimeSeries("health.level",
                                  {ServiceData::StatsType::AVG},
                                  {std::chrono::seconds(5),
                                   std::chrono::seconds(60)});
  // This is invert of health.level, going from 0 to 100
  m_illnessLevelCounter =
    ServiceData::createTimeSeries("illness.level",
                                  {ServiceData::StatsType::AVG},
                                  {std::chrono::seconds(5),
                                   std::chrono::seconds(60)});
  m_stopped.store(false, std::memory_order_relaxed);
  std::unique_lock<std::mutex> guard(m_condvar_lock);
  std::chrono::milliseconds dura(MaxUpdatePeriod);
  auto next = std::chrono::steady_clock::now();
  while (!m_stopped.load(std::memory_order_acquire)) {
    HealthLevel newStatus = evaluate();
    bool notify = false;
    if (newStatus > m_status) {
      // Server has become less healthy. Respond immediately.
      notify = true;
    } else if (newStatus < m_status) {
      // Server is healthier than before. Don't jump in health levels; instead
      // try to stay at each intermediate level for DampenTime as a slow start,
      // unless current status is BackOff, which we do want to get out of
      // immediately.
      if (m_status == HealthLevel::BackOff ||
          std::chrono::steady_clock::now() >= m_statusTime + DampenTime) {
        newStatus = static_cast<HealthLevel>(static_cast<int>(m_status) - 1);
        notify = true;
      }
    }
    if (notify) notifyObservers(newStatus);
    const auto healthLevel = healthLevelToInt(m_status);
    m_healthLevelCounter->addValue(healthLevel);
    m_illnessLevelCounter->addValue(100 /* max health */ - healthLevel);
    ProcStatus::checkUpdate(ProcStatusUpdateSeconds);
    next += dura;
    auto const now = std::chrono::steady_clock::now();
    if (next <= now) {                  // already late, update immediately
      next = now;
      continue;
    }
    // Make sure we wait at most dura before the next update
    next = std::min(next, now + dura);
    m_condition.wait_until(guard, next, [this] { return shouldWakeup(); });
  }
  Logger::Info("Host health monitor exits.");
}

HealthLevel HostHealthMonitor::evaluate() {
  HealthLevel res = HealthLevel::Bold;
  std::lock_guard<std::mutex> g(m_lock);
  for (auto metric : m_metrics) {
    res = std::max(res, metric->evaluate());
  }
  return res;
}

void HostHealthMonitor::notifyObservers(HealthLevel newStatus) {
  if (newStatus != m_status) {
    Logger::Warning("Health level (lower is better) changes from %d to %d.",
                    static_cast<int>(m_status), static_cast<int>(newStatus));

    std::lock_guard<std::mutex> g(m_lock);
    for (auto observer : m_observers) {
      observer->notifyNewStatus(newStatus);
    }
    m_status = newStatus;
    m_statusTime = std::chrono::steady_clock::now();
  }
}

}
