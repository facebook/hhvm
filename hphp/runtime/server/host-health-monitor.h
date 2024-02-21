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

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <boost/container/flat_set.hpp>

#include "hphp/util/assertions.h"
#include "hphp/util/health-monitor-types.h"
#include "hphp/util/service-data.h"

namespace HPHP {

// This class must be used as a singleton.
struct HostHealthMonitor {
  void subscribe(IHostHealthObserver* observer) {
    assertx(observer);
    std::lock_guard<std::mutex> g(m_lock);
    m_observers.insert(observer);
  }
  bool unsubscribe(IHostHealthObserver* observer) {
    std::lock_guard<std::mutex> g(m_lock);
    return !!m_observers.erase(observer);
  }
  void addMetric(IHealthMonitorMetric* metric);

  void start();
  void stop();
  void waitForEnd();

 private:
  HealthLevel evaluate();
  // true if the monitoring thread should wake up immediately, used with the
  // condition variable.
  bool shouldWakeup() const {
    return m_stopped.load(std::memory_order_acquire);
  }
  void notifyObservers(HealthLevel newStatus);
  void monitor();

  std::vector<IHealthMonitorMetric*> m_metrics;
  boost::container::flat_set<IHostHealthObserver*> m_observers;
  std::mutex m_lock;                    // protects metrics/observers
  HealthLevel m_status{HealthLevel::Bold};
  std::chrono::steady_clock::time_point m_statusTime;
  ServiceData::ExportedTimeSeries* m_healthLevelCounter{nullptr};
  ServiceData::ExportedTimeSeries* m_illnessLevelCounter{nullptr};
  std::mutex m_condvar_lock;
  std::condition_variable m_condition;
  std::atomic_bool m_stopped{true};
  std::unique_ptr<std::thread> m_monitor_thread;
};

}

