/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/std/ext_std_output.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int64_t k_PHP_OUTPUT_HANDLER_CONT = 0;
const int64_t k_PHP_OUTPUT_HANDLER_WRITE = 0;
const int64_t k_PHP_OUTPUT_HANDLER_START = 1;
const int64_t k_PHP_OUTPUT_HANDLER_CLEAN = 2;
const int64_t k_PHP_OUTPUT_HANDLER_FLUSH = 4;
const int64_t k_PHP_OUTPUT_HANDLER_END = 8;
const int64_t k_PHP_OUTPUT_HANDLER_FINAL = 8;
const int64_t k_PHP_OUTPUT_HANDLER_CLEANABLE = 16;
const int64_t k_PHP_OUTPUT_HANDLER_FLUSHABLE = 32;
const int64_t k_PHP_OUTPUT_HANDLER_REMOVABLE = 64;
const int64_t k_PHP_OUTPUT_HANDLER_STDFLAGS =
  k_PHP_OUTPUT_HANDLER_CLEANABLE | k_PHP_OUTPUT_HANDLER_FLUSHABLE |
  k_PHP_OUTPUT_HANDLER_REMOVABLE;

bool HHVM_FUNCTION(ob_start, const Variant& callback /* = null */,
                             int chunk_size /* = 0 */,
                             int flags /* = k_PHP_OUTPUT_HANDLER_STDFLAGS */) {
  // ignoring flags for now

  if (!callback.isNull()) {
    CallCtx ctx;
    vm_decode_function(callback, nullptr, false, ctx);
    if (!ctx.func) {
      return false;
    }
  }
  g_context->obStart(callback, chunk_size);
  return true;
}
void HHVM_FUNCTION(ob_clean) {
  // PHP_OUTPUT_HANDLER_START is included by PHP5
  g_context->obClean(k_PHP_OUTPUT_HANDLER_START | k_PHP_OUTPUT_HANDLER_CLEAN);
}
void HHVM_FUNCTION(ob_flush) {
  g_context->obFlush();
}
bool HHVM_FUNCTION(ob_end_clean) {
  g_context->obClean(k_PHP_OUTPUT_HANDLER_START |
                     k_PHP_OUTPUT_HANDLER_CLEAN |
                     k_PHP_OUTPUT_HANDLER_END);
  return g_context->obEnd();
}
bool HHVM_FUNCTION(ob_end_flush) {
  bool ret = g_context->obFlush();
  g_context->obEnd();
  return ret;
}
void HHVM_FUNCTION(flush) {
  g_context->flush();
}
Variant HHVM_FUNCTION(ob_get_contents) {
  if (HHVM_FN(ob_get_level)() == 0) {
    return false;
  }
  return g_context->obCopyContents();
}
Variant HHVM_FUNCTION(ob_get_clean) {
  String output = HHVM_FN(ob_get_contents)();
  if (!HHVM_FN(ob_end_clean)()) {
    return false;
  }
  return output;
}
Variant HHVM_FUNCTION(ob_get_flush) {
  String output = g_context->obCopyContents();
  if (!HHVM_FN(ob_end_flush)()) {
    return false;
  }
  return output;
}
int64_t HHVM_FUNCTION(ob_get_length) {
  return g_context->obGetContentLength();
}
int64_t HHVM_FUNCTION(ob_get_level) {
  return g_context->obGetLevel();
}
Array HHVM_FUNCTION(ob_get_status, bool full_status /* = false */) {
  return g_context->obGetStatus(full_status);
}
void HHVM_FUNCTION(ob_implicit_flush, bool flag /* = true */) {
  g_context->obSetImplicitFlush(flag);
}
Array HHVM_FUNCTION(ob_list_handlers) {
  return g_context->obGetHandlers();
}
bool HHVM_FUNCTION(output_add_rewrite_var, const String& name,
                                           const String& value) {
  throw_not_supported(__func__, "bad coding style");
}
bool HHVM_FUNCTION(output_reset_rewrite_vars) {
  throw_not_supported(__func__, "bad coding style");
}

void HHVM_FUNCTION(hphp_crash_log, const String& name, const String& value) {
  StackTraceNoHeap::AddExtraLogging(name.data(), value.data());
}

void HHVM_FUNCTION(hphp_stats, const String& name, int64_t value) {
  ServerStats::Log(name.data(), value);
}
int64_t HHVM_FUNCTION(hphp_get_stats, const String& name) {
  return ServerStats::Get(name.data());
}
Array HHVM_FUNCTION(hphp_get_status) {
  std::string out;
  ServerStats::ReportStatus(out, Writer::Format::JSON);
  return HHVM_FN(json_decode)(String(out)).toArray();
}
Array HHVM_FUNCTION(hphp_get_iostatus) {
  return ServerStats::GetThreadIOStatuses();
}
void HHVM_FUNCTION(hphp_set_iostatus_address, const String& name) {
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

Variant HHVM_FUNCTION(hphp_get_timers, bool get_as_float /* = true */) {
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

  ArrayInit ret(7, ArrayInit::Map{});
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
  return ret.toVariant();
}

Variant HHVM_FUNCTION(hphp_output_global_state, bool serialize /* = true */) {
  Array r = Array();
  if (serialize) {
    return f_serialize(r);
  } else {
    return r;
  }
}

int64_t HHVM_FUNCTION(hphp_instruction_counter) {
  return HardwareCounter::GetInstructionCount();
}

Variant HHVM_FUNCTION(hphp_get_hardware_counters) {
  Array ret;

  HardwareCounter::GetPerfEvents(
    [](const std::string& key, int64_t value, void* data) {
      Array& ret = *reinterpret_cast<Array*>(data);
      ret.set(String(key), value);
    },
    &ret);

  jit::mcg->getPerfCounters(ret);
  return ret;
}

bool HHVM_FUNCTION(hphp_set_hardware_events,
                   const Variant& events /* = null */) {
  auto ev = events.toString();
  ev = url_decode(ev.data(), ev.size());
  return HardwareCounter::SetPerfEvents(ev.slice());
}

void HHVM_FUNCTION(hphp_clear_hardware_events) {
  HardwareCounter::ClearPerfEvents();
}

// __SystemLib\print_hashbang
void HHVM_FUNCTION(SystemLib_print_hashbang, const String& hashbang) {
  CallerFrame cf;
  auto ar = cf();

  if (ar->m_func->name()->empty() && RuntimeOption::ClientExecutionMode()) {
    // If run in cli mode, print nothing in the lowest pseudomain
    if (!g_context->getPrevFunc(ar)) return;
  }

  g_context->write(hashbang);
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::initOutput() {
  HHVM_FE(ob_start);
  HHVM_FE(ob_clean);
  HHVM_FE(ob_flush);
  HHVM_FE(ob_end_clean);
  HHVM_FE(ob_end_flush);
  HHVM_FE(flush);
  HHVM_FE(ob_get_contents);
  HHVM_FE(ob_get_clean);
  HHVM_FE(ob_get_flush);
  HHVM_FE(ob_get_length);
  HHVM_FE(ob_get_level);
  HHVM_FE(ob_get_status);
  HHVM_FE(ob_implicit_flush);
  HHVM_FE(ob_list_handlers);
  HHVM_FE(output_add_rewrite_var);
  HHVM_FE(output_reset_rewrite_vars);
  HHVM_FE(hphp_crash_log);
  HHVM_FE(hphp_stats);
  HHVM_FE(hphp_get_stats);
  HHVM_FE(hphp_get_status);
  HHVM_FE(hphp_get_iostatus);
  HHVM_FE(hphp_set_iostatus_address);
  HHVM_FE(hphp_get_timers);
  HHVM_FE(hphp_output_global_state);
  HHVM_FE(hphp_instruction_counter);
  HHVM_FE(hphp_get_hardware_counters);
  HHVM_FE(hphp_set_hardware_events);
  HHVM_FE(hphp_clear_hardware_events);
  HHVM_FALIAS(__SystemLib\\print_hashbang, SystemLib_print_hashbang);

#define INTCONST(v) Native::registerConstant<KindOfInt64> \
                  (makeStaticString(#v), k_##v);
  INTCONST(PHP_OUTPUT_HANDLER_CONT);
  INTCONST(PHP_OUTPUT_HANDLER_WRITE);
  INTCONST(PHP_OUTPUT_HANDLER_START);
  INTCONST(PHP_OUTPUT_HANDLER_CLEAN);
  INTCONST(PHP_OUTPUT_HANDLER_FLUSH);
  INTCONST(PHP_OUTPUT_HANDLER_END);
  INTCONST(PHP_OUTPUT_HANDLER_FINAL);
  INTCONST(PHP_OUTPUT_HANDLER_CLEANABLE);
  INTCONST(PHP_OUTPUT_HANDLER_FLUSHABLE);
  INTCONST(PHP_OUTPUT_HANDLER_REMOVABLE);
  INTCONST(PHP_OUTPUT_HANDLER_STDFLAGS);
#undef INTCONST

  loadSystemlib("std_output");
}

}
