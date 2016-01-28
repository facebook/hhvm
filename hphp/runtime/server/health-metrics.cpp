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

#include "hphp/runtime/server/health-metrics.h"
#include "hphp/runtime/base/config.h"

namespace HPHP {

namespace {

std::vector<IHealthMonitorMetric*>* s_health_metrics;

std::vector<IHealthMonitorMetric*>& get() {
  if (!s_health_metrics) {
    s_health_metrics = new std::vector<IHealthMonitorMetric*>;
  }
  return *s_health_metrics;
}

}

void AddHealthMonitorMetric(IHealthMonitorMetric* metric) {
  get().push_back(metric);
}

const std::vector<IHealthMonitorMetric*>& GetHealthMonitorMetrics() {
  return get();
}

}
