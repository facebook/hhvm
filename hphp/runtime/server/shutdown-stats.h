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

#pragma once

#include "hphp/util/assertions.h"
#include "hphp/util/process.h"
#include <time.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * The list of events for which we record stats.
 */
enum ShutdownEvent {
  SHUTDOWN_PREPARE = 0,
  SHUTDOWN_INITIATED,
  SHUTDOWN_DRAIN_READS,
  SHUTDOWN_DRAIN_DISPATCHER,
  SHUTDOWN_DRAIN_WRITES,
  SHUTDOWN_DONE,
  kNumEvents
};

constexpr char* s_shutdownEventNames[ShutdownEvent::kNumEvents] = {
  "prepare",
  "initiate",
  "drain_reads",
  "drain_dispatcher",
  "drain_writes",
  "done"
};

/**
 * System stats at a time point during shutdown.
 */
struct ShutdownStat {
  ShutdownEvent event;
  time_t time;
  MemInfo memUsage;
  int64_t rss;
  int64_t requestsServed;

  const char* eventName() const {
    assertx(event >= 0 && event < ShutdownEvent::kNumEvents);
    return s_shutdownEventNames[event];
  }
};

///////////////////////////////////////////////////////////////////////////////
}
