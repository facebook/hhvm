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

#ifndef incl_HPHP_SERVER_MEMORY_PROTECTOR_H_
#define incl_HPHP_SERVER_MEMORY_PROTECTOR_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>
#include <boost/container/flat_set.hpp>

#include "hphp/runtime/base/config.h"
#include "hphp/util/async-func.h"
#include "hphp/util/health-monitor-types.h"

namespace HPHP {

// This class must be used as a singleton.
struct HostHealthMonitor {
  void subscribe(IHostHealthObserver* observer) {
    assert(observer != nullptr);
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
  void notifyObservers(HealthLevel newStatus);
  void monitor();

  std::vector<IHealthMonitorMetric*> m_metrics;
  boost::container::flat_set<IHostHealthObserver*> m_observers;
  std::mutex m_lock;                    // protects metrics/observers
  HealthLevel m_status{HealthLevel::Bold};
  std::mutex m_stopped_lock;
  std::condition_variable m_condition;
  bool m_stopped{true};
  std::unique_ptr<AsyncFunc<HostHealthMonitor>> m_monitor_func;
};

}

#endif
