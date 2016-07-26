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

#include <thread>
#include <folly/Singleton.h>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/health-monitor-types.h"
#include "hphp/util/logger.h"

namespace HPHP {

namespace {
bool Enabled;
int32_t UpdateFreq;

struct HostHealthMonitorExtension final : public Extension {
  HostHealthMonitorExtension() : Extension("hosthealthmonitor", "1.0") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf globalConfig) override {
    Config::Bind(Enabled, ini, globalConfig,
                 "HealthMonitor.EnableHealthMonitor", true);
    Config::Bind(UpdateFreq, ini, globalConfig,
                 "HealthMonitor.UpdateFreq", 1000 /* miliseconds */);
  }
} s_host_health_monitor_extension;

folly::Singleton<HostHealthMonitor> s_health_monitor;

}

void HostHealthMonitor::addMetric(IHealthMonitorMetric* metric) {
  assert(metric != nullptr);
  std::lock_guard<std::mutex> g(m_lock);
  m_metrics.push_back(metric);
}

void IHealthMonitorMetric::registerSelf() {
  folly::Singleton<HostHealthMonitor>::try_get()->addMetric(this);
}

void HostHealthMonitor::start() {
  if (!Enabled || !m_stopped) return;
  if (UpdateFreq < 10) UpdateFreq = 10;
  if (UpdateFreq > 10000) UpdateFreq = 10000;

  m_monitor_func = folly::make_unique<AsyncFunc<HostHealthMonitor>>(
    this,
    &HostHealthMonitor::monitor
  );
  m_monitor_func->start();
  // Make sure the thread is gone after hphp_process_exit().  The node
  // is intentionally leaked.
  new InitFiniNode(
    [] { folly::Singleton<HostHealthMonitor>::try_get()->waitForEnd(); },
    InitFiniNode::When::ProcessExit
  );
}

void HostHealthMonitor::stop() {
  notifyObservers(HealthLevel::Bold);
  std::unique_lock<std::mutex> guard(m_stopped_lock);
  m_stopped = true;
  m_condition.notify_one();
}

void HostHealthMonitor::waitForEnd() {
  if (!m_stopped) stop();
  if (m_monitor_func) {
    m_monitor_func->waitForEnd();
    m_monitor_func.reset();
  }
}

void HostHealthMonitor::monitor() {
  Logger::Info("Host health monitor starts working.");
  std::unique_lock<std::mutex> guard(m_stopped_lock);
  m_stopped = false;

  std::chrono::milliseconds dura(UpdateFreq);
  while (!m_stopped) {
    HealthLevel newStatus = evaluate();
    notifyObservers(newStatus);
    m_condition.wait_for(guard, dura, [this] { return m_stopped; });
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
  }
}

}
