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

#ifndef incl_HPHP_HEALTH_MONITOR_EXTERNAL_CLIENT_SHIM_H_
#define incl_HPHP_HEALTH_MONITOR_EXTERNAL_CLIENT_SHIM_H_

#include "hphp/runtime/ext/apache/ext_apache.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/util/health-monitor-types.h"

namespace HPHP {

struct ExternalClientShim : IHostHealthObserver {
  ExternalClientShim():
    m_status(HealthLevel::Bold) {
  }
  virtual void notifyNewStatus(HealthLevel newStatus) final {
    m_status = newStatus;

    // Push (asynchronuously if necessary) new system health status to
    // external clients, i.e., fb303, ODS, HealthMon, etc.

    // push to ServerStats
    ServerStats::SetServerHealthLevel(m_status);

    // push to ApacheExtension
    ApacheExtension::UpdateHealthLevel(m_status);

  }

  virtual HealthLevel getStatus() {
    return m_status;
  }
 private:
  HealthLevel m_status;
};

}

#endif
