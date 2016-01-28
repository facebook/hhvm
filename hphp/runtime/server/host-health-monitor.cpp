/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

#include <folly/Singleton.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

namespace HPHP {

namespace {

bool Enabled;
int32_t WaitBeforeStart;
int32_t UpdateFreq;

struct HostHealthMonitorExtension final : public Extension {
  HostHealthMonitorExtension() : Extension("hosthealthmonitor", "1.0") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf globalConfig) override {
    Config::Bind(Enabled, ini, globalConfig,
                 "HealthMonitor.EnableHealthMonitor", true);
    Config::Bind(WaitBeforeStart, ini, globalConfig,
                 "HealthMonitor.WaitBeforeStart", 100000);
    Config::Bind(UpdateFreq, ini, globalConfig, "HealthMonitor.UpdateFreq",
                 3000);
    for (auto const metric : GetHealthMonitorMetrics()) {
      metric->setConfig(ini, globalConfig, "HealthMonitor.Metrics.CPUDelay");
    }
  }
} s_host_health_monitor_extension;

folly::Singleton<HostHealthMonitor> s_health_monitor;

}

HostHealthMonitor::HostHealthMonitor()
    : m_status(HealthLevel::Bold)
    , m_stopped(true)
{}

void HostHealthMonitor::loadMetrics() {
  for (auto const metric : GetHealthMonitorMetrics()) {
    if (metric->enabled()) {
      registerMetric(metric);
    }
  }
}

void HostHealthMonitor::start() {
  if (!Enabled || !m_stopped) return;

  loadMetrics();

  m_monitor_func = folly::make_unique<AsyncFunc<HostHealthMonitor>>(
    this,
    &HostHealthMonitor::monitor
  );
  m_monitor_func->start();
}

void HostHealthMonitor::monitor() {
  // wait hhvm finish bootstrapping phase
  std::chrono::milliseconds bootDura(WaitBeforeStart);
  std::unique_lock<std::mutex> guard(m_stopped_lock);
  m_stopped = false;
  m_condition.wait_for(guard, bootDura, [this] { return m_stopped; });

  if (!m_stopped) {
    Logger::Info("Host health monitor starts working.");
  }

  std::chrono::milliseconds dura(UpdateFreq);

  // periodically update monitored metrics
  while (!m_stopped) {
    HealthLevel newStatus = evaluate();
    notifyObservers(newStatus);
    m_condition.wait_for(guard, dura, [this] { return m_stopped; });
  }
}

HealthLevel HostHealthMonitor::evaluate() {
  HealthLevel res = HealthLevel::Bold;
  for (auto metric : m_metrics) {
    res = std::max(res, metric->evaluate());
  }
  return res;
}

void HostHealthMonitor::notifyObservers(HealthLevel newStatus) {
  if (newStatus != m_status) {
    m_status = newStatus;
    for (auto observer : m_observers) {
      observer->notifyNewStatus(newStatus);
    }
  }
}

}
