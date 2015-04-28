/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SERVER_MEMORY_PROTECTOR_H_
#define incl_HPHP_SERVER_MEMORY_PROTECTOR_H_

#include <algorithm>
#include <chrono>
#include <memory>
#include <thread>
#include <time.h>
#include <vector>
#include <condition_variable>

#include "hphp/runtime/server/health-metrics.h"
#include "hphp/runtime/base/config.h"
#include "hphp/util/async-func.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

namespace HPHP {

struct HostHealthMonitor {
  HostHealthMonitor();

  void subscribe(IHostHealthObserver* observer) {
    assert(observer != nullptr);
    m_observers.push_back(observer);
  }

  void start();
  void stop() {
    {
      std::unique_lock<std::mutex> guard(m_stopped_lock);
      m_stopped = true;
      m_condition.notify_one();
    }
    m_monitor_func->waitForEnd();
  }

 private:
  HealthLevel evaluate();
  void notifyObservers(HealthLevel newStatus);
  void loadMetrics();
  void monitor();

  void registerMetric(IHealthMonitorMetric* metric) {
    assert(metric != nullptr);
    m_metrics.push_back(metric);
  }

  std::vector<IHealthMonitorMetric*> m_metrics;
  std::vector<IHostHealthObserver*> m_observers;
  HealthLevel m_status;
  std::mutex m_stopped_lock;
  std::condition_variable m_condition;
  bool m_stopped;
  std::unique_ptr<AsyncFunc<HostHealthMonitor>> m_monitor_func;
};

}

#endif
