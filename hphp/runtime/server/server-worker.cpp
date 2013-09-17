/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/server/server-worker.h"
#include "hphp/util/timer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// ServerJob

ServerJob::ServerJob() {
  Timer::GetMonotonicTime(start);
}

void ServerJob::stopTimer(const struct timespec &reqStart) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    // This measures [enqueue:dequeue]
    timespec end;
    Timer::GetMonotonicTime(end);
    time_t dsec = end.tv_sec - start.tv_sec;
    long dnsec = end.tv_nsec - start.tv_nsec;
    int64_t dusec = dsec * 1000000 + dnsec / 1000;
    ServerStats::Log("page.wall.queuing", dusec);

    // This measures [request start:dequeue]
    dsec = start.tv_sec - reqStart.tv_sec;
    dnsec = start.tv_nsec - reqStart.tv_nsec;
    dusec = dsec * 1000000 + dnsec / 1000;
    ServerStats::Log("page.wall.request_read_time", dusec);
  }
}

}
