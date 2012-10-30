/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_output.h>
#include <runtime/ext/ext_json.h>
#include <runtime/base/runtime_option.h>
#include <util/hardware_counter.h>
#include <util/lock.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void f_hphp_crash_log(CStrRef name, CStrRef value) {
  StackTraceNoHeap::AddExtraLogging(name.data(), value.data());
}

Array f_hphp_get_status() {
  std::string out;
  ServerStats::ReportStatus(out, ServerStats::JSON);
  return f_json_decode(String(out));
}

static double ts_float(const timespec &ts) {
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000;
}

static String ts_microtime(const timespec &ts) {
  char ret[100];
  snprintf(ret, 100, "%.8F %ld", (double)ts.tv_nsec / 1000000000, ts.tv_sec);
  return String(ret, CopyString);
}

Variant f_hphp_get_timers(bool get_as_float /* = true */) {
  Transport *transport = g_context->getTransport();
  if (transport == NULL) {
    return false;
  }

  const timespec &tsQueue = transport->getQueueTime();
  const timespec &tsWall = transport->getWallTime();
  const timespec &tsCpu = transport->getCpuTime();
  const int64 &instStart = transport->getInstructions();

  Array ret;
  if (get_as_float) {
    ret.set("queue",        ts_float(tsQueue));
    ret.set("process-wall", ts_float(tsWall));
    ret.set("process-cpu",  ts_float(tsCpu));
  } else {
    ret.set("queue",        ts_microtime(tsQueue));
    ret.set("process-wall", ts_microtime(tsWall));
    ret.set("process-cpu",  ts_microtime(tsCpu));
  }
  ret.set("process-inst", instStart);
  return ret;
}

Variant f_hphp_output_global_state(bool serialize /* = true */) {
  Array r(get_global_state());
  if (serialize) {
    return f_serialize(r);
  } else {
    return r;
  }
}

int64 f_hphp_instruction_counter(void) {
  return Util::HardwareCounter::GetInstructionCount();
}

Variant f_hphp_get_hardware_counters(void) {
  Array ret;

  Util::HardwareCounter::GetPerfEvents(ret);
  return ret;
}

bool f_hphp_set_hardware_events(CStrRef events /* = null */) {
  return Util::HardwareCounter::SetPerfEvents(events);
}

void f_hphp_clear_hardware_events() {
  Util::HardwareCounter::ClearPerfEvents();
}

///////////////////////////////////////////////////////////////////////////////
}
