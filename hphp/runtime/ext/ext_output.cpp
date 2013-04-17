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
#include <runtime/base/hardware_counter.h>
#include <util/lock.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////


bool f_ob_start(CVarRef output_callback /* = uninit_null() */,
                int chunk_size /* = 0 */, bool erase /* = true */) {
  // ignoring chunk_size and erase
  g_context->obStart(output_callback);
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
String f_ob_get_contents() {
  return g_context->obCopyContents();
}
String f_ob_get_clean() {
  String output = f_ob_get_contents();
  f_ob_end_clean();
  return output;
}
String f_ob_get_flush() {
  String output = g_context->obCopyContents();
  g_context->obFlush();
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
String f_ob_gzhandler(CStrRef buffer, int mode) {
  throw NotSupportedException(__func__, "something that's in transport layer");
}
void f_ob_implicit_flush(bool flag /* = true */) {
  g_context->obSetImplicitFlush(flag);
}
Array f_ob_list_handlers() {
  return g_context->obGetHandlers();
}
bool f_output_add_rewrite_var(CStrRef name, CStrRef value) {
  throw NotSupportedException(__func__, "bad coding style");
}
bool f_output_reset_rewrite_vars() {
  throw NotSupportedException(__func__, "bad coding style");
}

void f_hphp_crash_log(CStrRef name, CStrRef value) {
  StackTraceNoHeap::AddExtraLogging(name.data(), value.data());
}

void f_hphp_stats(CStrRef name, int64_t value) {
  ServerStats::Log(name.data(), value);
}
int64_t f_hphp_get_stats(CStrRef name) {
  return ServerStats::Get(name.data());
}
Array f_hphp_get_status() {
  std::string out;
  ServerStats::ReportStatus(out, ServerStats::JSON);
  return f_json_decode(String(out));
}
Array f_hphp_get_iostatus() {
  return ServerStats::GetThreadIOStatuses();
}
void f_hphp_set_iostatus_address(CStrRef name) {
  return ServerStats::SetThreadIOStatusAddress(name);
}




static double ts_float(const timespec &ts) {
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000;
}

static String ts_microtime(const timespec &ts) {
  char ret[100];
  snprintf(ret, 100, "%.8F %ld", (double)ts.tv_nsec / 1000000000, ts.tv_sec);
  return String(ret, CopyString);
}

static const StaticString s_queue("queue");
static const StaticString s_process_wall("process-wall");
static const StaticString s_process_cpu("process-cpu");
static const StaticString s_process_inst("process-inst");

Variant f_hphp_get_timers(bool get_as_float /* = true */) {
  Transport *transport = g_context->getTransport();
  if (transport == NULL) {
    return false;
  }

  const timespec &tsQueue = transport->getQueueTime();
  const timespec &tsWall = transport->getWallTime();
  const timespec &tsCpu = transport->getCpuTime();
  const int64_t &instStart = transport->getInstructions();

  ArrayInit ret(4);
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

bool f_hphp_set_hardware_events(CStrRef events /* = null */) {
  return HardwareCounter::SetPerfEvents(events);
}

void f_hphp_clear_hardware_events() {
  HardwareCounter::ClearPerfEvents();
}

///////////////////////////////////////////////////////////////////////////////
}
