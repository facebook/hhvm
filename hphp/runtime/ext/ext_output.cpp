/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_output.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/hardware-counter.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


bool f_ob_start(const Variant& callback /* = uninit_null() */,
                int chunk_size /* = 0 */, bool erase /* = true */) {
  // ignoring chunk_size and erase

  if (!callback.isNull()) {
    CallCtx ctx;
    vm_decode_function(callback, nullptr, false, ctx);
    if (!ctx.func) {
      return false;
    }
  }
  g_context->obStart(callback);
  return true;
}
void f_ob_clean() {
  g_context->obClean();
}
void f_ob_flush() {
  g_context->obFlush();
}
bool f_ob_end_clean() {
  g_context->obClean();
  return g_context->obEnd();
}
bool f_ob_end_flush() {
  bool ret = g_context->obFlush();
  g_context->obEnd();
  return ret;
}
void f_flush() {
  g_context->flush();
}
Variant f_ob_get_contents() {
  if (f_ob_get_level() == 0) {
    return false;
  }
  return g_context->obCopyContents();
}
Variant f_ob_get_clean() {
  String output = f_ob_get_contents();
  if (!f_ob_end_clean()) {
    return false;
  }
  return output;
}
Variant f_ob_get_flush() {
  String output = g_context->obCopyContents();
  if (!f_ob_end_flush()) {
    return false;
  }
  return output;
}
int64_t f_ob_get_length() {
  return g_context->obGetContentLength();
}
int64_t f_ob_get_level() {
  return g_context->obGetLevel();
}
Array f_ob_get_status(bool full_status /* = false */) {
  return g_context->obGetStatus(full_status);
}
void f_ob_implicit_flush(bool flag /* = true */) {
  g_context->obSetImplicitFlush(flag);
}
Array f_ob_list_handlers() {
  return g_context->obGetHandlers();
}
bool f_output_add_rewrite_var(const String& name, const String& value) {
  throw NotSupportedException(__func__, "bad coding style");
}
bool f_output_reset_rewrite_vars() {
  throw NotSupportedException(__func__, "bad coding style");
}

void f_hphp_crash_log(const String& name, const String& value) {
  StackTraceNoHeap::AddExtraLogging(name.data(), value.data());
}

void f_hphp_stats(const String& name, int64_t value) {
  ServerStats::Log(name.data(), value);
}
int64_t f_hphp_get_stats(const String& name) {
  return ServerStats::Get(name.data());
}
Array f_hphp_get_status() {
  std::string out;
  ServerStats::ReportStatus(out, ServerStats::Format::JSON);
  return HHVM_FN(json_decode)(String(out)).toArray();
}
Array f_hphp_get_iostatus() {
  return ServerStats::GetThreadIOStatuses();
}
void f_hphp_set_iostatus_address(const String& name) {
  return ServerStats::SetThreadIOStatusAddress(name.c_str());
}




static double ts_float(const timespec &ts) {
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000;
}

static String ts_microtime(const timespec &ts) {
  char ret[100];
  snprintf(ret, 100, "%.8F %ld", (double)ts.tv_nsec / 1000000000, ts.tv_sec);
  return String(ret, CopyString);
}

const StaticString
  s_queue("queue"),
  s_process_wall("process-wall"),
  s_process_cpu("process-cpu"),
  s_process_inst("process-inst");

const StaticString
  s_process_sleep_time("process-sleep-time"),
  s_process_usleep_time("process-usleep-time"),
  s_process_nsleep_time("process-nanosleep-time");

Variant f_hphp_get_timers(bool get_as_float /* = true */) {
  Transport *transport = g_context->getTransport();
  if (transport == NULL) {
    return false;
  }

  const timespec &tsQueue = transport->getQueueTime();
  const timespec &tsWall = transport->getWallTime();
  const timespec &tsCpu = transport->getCpuTime();
  const int64_t &instStart = transport->getInstructions();
  const int64_t &usleep_time = transport->getuSleepTime();
  const int64_t &sleep_time = transport->getSleepTime();
  const int64_t &nsleep_time_s = transport->getnSleepTimeS();
  const int32_t &nsleep_time_n = transport->getnSleepTimeN();

  ArrayInit ret(7);
  if (get_as_float) {
    ret.set(s_queue,        ts_float(tsQueue));
    ret.set(s_process_wall, ts_float(tsWall));
    ret.set(s_process_cpu,  ts_float(tsCpu));
  } else {
    ret.set(s_queue,        ts_microtime(tsQueue));
    ret.set(s_process_wall, ts_microtime(tsWall));
    ret.set(s_process_cpu,  ts_microtime(tsCpu));
  }
  ret.set(s_process_inst, instStart);
  ret.set(s_process_sleep_time, sleep_time);
  ret.set(s_process_usleep_time, (double)usleep_time/1000000);
  ret.set(s_process_nsleep_time, nsleep_time_s +
          (double)nsleep_time_n / 1000000000);
  return ret.create();
}

Variant f_hphp_output_global_state(bool serialize /* = true */) {
  Array r = Array();
  if (serialize) {
    return f_serialize(r);
  } else {
    return r;
  }
}

int64_t f_hphp_instruction_counter(void) {
  return HardwareCounter::GetInstructionCount();
}

Variant f_hphp_get_hardware_counters(void) {
  Array ret;

  HardwareCounter::GetPerfEvents(ret);
  return ret;
}

bool f_hphp_set_hardware_events(const String& events /* = null */) {
  return HardwareCounter::SetPerfEvents(events);
}

void f_hphp_clear_hardware_events() {
  HardwareCounter::ClearPerfEvents();
}

///////////////////////////////////////////////////////////////////////////////
}
