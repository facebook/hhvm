/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/std/ext_std_misc.h"
#include <limits>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/configs/hacklang.h"
#include "hphp/runtime/base/configs/php7.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/zend-pack.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/server-stats.h"

#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/perf-counters.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/util/current-executable.h"
#include "hphp/util/logger.h"

#include "hphp/zend/zend-math.h"

namespace HPHP {

RDS_LOCAL(std::string, s_misc_display_errors);

const std::string s_1("1"), s_2("2"), s_stdout("stdout"), s_stderr("stderr");

const int64_t k_CONNECTION_NORMAL = 0;
const int64_t k_CONNECTION_ABORTED = 1;
const int64_t k_CONNECTION_TIMEOUT = 2;

namespace {

String HHVM_FUNCTION(server_warmup_status) {
  // Fail if we jitted at least Eval.JitWarmupStatusBytes of code.
  size_t begin, end;
  jit::tc::codeEmittedThisRequest(begin, end);
  auto const diff = end - begin;
  if (diff >= RuntimeOption::EvalJitWarmupStatusBytes) {
    // TODO(13274666) Mismatch between 'to' and 'begin' below.
    return folly::format("Translation cache grew by {} bytes to {} bytes.",
                         diff, begin).str();
  }

  // Fail if we spent more than 0.5ms in the JIT.
  auto const jittime = jit::Timer::CounterValue(jit::Timer::mcg_translate);
  auto constexpr kMaxJitTimeNS = 500000;
  if (jittime.total > kMaxJitTimeNS) {
    return folly::format("Spent {}us in the JIT.", jittime.total / 1000).str();
  }

  if (!isStandardRequest()) {
    return "Warmup is still in progress.";
  }

  if (jit::tc::shouldProfileNewFuncs()) {
    return "PGO profiling translations are still enabled.";
  }

  if (jit::mcgen::retranslateAllPending()) {
    return "Waiting on retranslateAll()";
  }

  auto tpc_diff = jit::rl_perf_counters[jit::tpc_interp_bb] -
                  jit::rl_perf_counters[jit::tpc_interp_bb_force];
  if (tpc_diff) {
    return folly::sformat("Interpreted {} non-forced basic blocks.", tpc_diff);
  }

  if (RuntimeOption::EvalJitSerdesMode == JitSerdesMode::SerializeAndExit) {
    return "JitSerdesMode is SerializeAndExit";
  }

  return empty_string();
}

String HHVM_FUNCTION(server_warmup_status_monotonic) {
  return String(jit::tc::warmupStatusString());
}

void HHVM_FUNCTION(set_endpoint_name, const String& endpoint) {
  ServerStats::SetEndpoint(endpoint.c_str());
}

const StaticString
  s_clisrv("clisrv"),
  s_cli("cli"),
  s_worker("worker");

String HHVM_FUNCTION(execution_context) {
  if (auto t = g_context->getTransport()) return t->describe();
  if (is_cli_server_mode()) return s_clisrv;
  return RO::ServerExecutionMode() ? s_worker : s_cli;
}

}

void StandardExtension::threadInitMisc() {
    IniSetting::Bind(
      this, IniSetting::Mode::Request,
      "display_errors", "stderr",
      IniSetting::SetAndGet<std::string>(
        [](const std::string& value) {
          *s_misc_display_errors = value;

          if (value == s_1 || value == s_stdout) {
            Logger::SetStandardOut(Logger::DEFAULT, stdout);
            return true;
          }
          if (value == s_2 || value == s_stderr) {
            Logger::SetStandardOut(Logger::DEFAULT, stderr);
            return true;
          }
          return false;
        },
        nullptr,
        s_misc_display_errors.get()
      )
    );
  }

#define PHP_MAJOR_VERSION_5 5
#define PHP_MINOR_VERSION_5 6
#define PHP_VERSION_5 "5.6.99-hhvm"
#define PHP_VERSION_ID_5 50699

#define PHP_MAJOR_VERSION_7 7
#define PHP_MINOR_VERSION_7 1
#define PHP_VERSION_7 "7.1.99-hhvm"
#define PHP_VERSION_ID_7 70199

#define PHP_RELEASE_VERSION 99
#define PHP_EXTRA_VERSION "hhvm"

StaticString get_PHP_VERSION() {
  static StaticString v5(PHP_VERSION_5);
  static StaticString v7(PHP_VERSION_7);
  return Cfg::PHP7::Builtins ? v7 : v5;
}


///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(connection_status) {
  // FIXME: WAT?
  return k_CONNECTION_NORMAL;
}

int64_t HHVM_FUNCTION(connection_aborted) {
  return HHVM_FN(connection_status)() == k_CONNECTION_ABORTED;
}

int64_t HHVM_FUNCTION(connection_timeout) {
  return HHVM_FN(connection_status)() == k_CONNECTION_TIMEOUT;
}

static Class* getClassByName(const char* name, int len) {
  String className(name, len, CopyString);
  return Class::load(className.get());
}

Variant HHVM_FUNCTION(constant, const String& name) {
  auto const warning = "constant() is deprecated and subject"
  " to removal from the Hack language";
  switch (Cfg::HackLang::PhpismDisableConstant) {
    case 0:  break;
    case 1:  raise_warning(warning); break;
    default: raise_error(warning);
  }
  if (!name.get()) return init_null();
  const char *data = name.data();
  int len = name.length();

  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    Class* cls = getClassByName(data, classNameLen);
    if (cls) {
      String cnsName(constantName, data + len - constantName, CopyString);
      TypedValue cns = cls->clsCnsGet(cnsName.get());
      if (type(cns) != KindOfUninit) {
        return tvAsCVarRef(cns);
      }
    }
  } else {
    auto const cns = Constant::load(name.get());
    if (type(cns) != KindOfUninit) return Variant::attach(cns);
  }

  raise_warning("constant(): Couldn't find constant %s", data);
  return init_null();
}

bool HHVM_FUNCTION(defined, const String& name, bool autoload /* = true */) {
  if (!name.get()) return false;
  const char *data = name.data();
  int len = name.length();

  char *colon;
  if ((colon = (char*)memchr(data, ':', len)) && colon[1] == ':') {
    // class constant
    int classNameLen = colon - data;
    char *constantName = colon + 2;
    Class* cls = getClassByName(data, classNameLen);
    if (cls) {
      String cnsName(constantName, data + len - constantName, CopyString);
      return cls->clsCnsGet(cnsName.get()).m_type != KindOfUninit;
    }
    return false;
  } else {
    auto* cb = autoload ? Constant::load : Constant::lookup;
    return type(cb(name.get())) != KindOfUninit;
  }
}

int64_t HHVM_FUNCTION(ignore_user_abort, bool /*setting*/ /* = false */) {
  return 0;
}

Variant HHVM_FUNCTION(pack, const String& format, const Array& argv) {
  // pack() returns false if there was an error, String otherwise
  return ZendPack::getInstance()->pack(format, argv);
}

int64_t HHVM_FUNCTION(sleep, int64_t seconds) {
  IOStatusHelper io("sleep");
  Transport *transport = g_context->getTransport();
  if (transport) {
    transport->incSleepTime(seconds);
  }
  sleep(seconds);
  return 0;
}

void HHVM_FUNCTION(usleep, int64_t micro_seconds) {
  IOStatusHelper io("usleep");
  Transport *transport = g_context->getTransport();
  if (transport) {
    transport->incuSleepTime(micro_seconds);
  }
  usleep(micro_seconds);
}

static void recordNanosleepTime(
  const struct timespec &req,
  const struct timespec *rem
) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    int64_t req_s = req.tv_sec;
    int32_t req_n = req.tv_nsec;
    int64_t rem_s = 0;
    int32_t rem_n = 0;

    if (rem) {
      rem_s = rem->tv_sec;
      rem_n = rem->tv_nsec;
    }

    int32_t nanos = req_n - rem_n;
    int64_t seconds = req_s - rem_s;
    if (nanos < 0) {
      nanos += 1000000000;
      seconds--;
    }

    transport->incnSleepTime(seconds, nanos);
  }
}

const StaticString
  s_seconds("seconds"),
  s_nanoseconds("nanoseconds");

Variant HHVM_FUNCTION(time_nanosleep, int64_t seconds, int64_t nanoseconds) {
  if (seconds < 0) {
    raise_invalid_argument_warning("seconds: cannot be negative");
    return false;
  }
  if (nanoseconds < 0 || nanoseconds > 999999999) {
    raise_invalid_argument_warning("nanoseconds: has to be 0 to 999999999");
    return false;
  }

  struct timespec req, rem;
  req.tv_sec = (time_t)seconds;
  req.tv_nsec = nanoseconds;

  IOStatusHelper io("nanosleep");
  if (!nanosleep(&req, &rem)) {
    recordNanosleepTime(req, nullptr);
    return true;
  }

  recordNanosleepTime(req, &rem);
  if (errno == EINTR) {
    return make_dict_array(
      s_seconds, (int64_t)rem.tv_sec,
      s_nanoseconds, (int64_t)rem.tv_nsec
    );
  }
  return false;
}

bool HHVM_FUNCTION(time_sleep_until, double timestamp) {
  struct timeval tm;
  if (gettimeofday((struct timeval *)&tm, NULL) != 0) {
    return false;
  }

  double c_ts = (double)(timestamp - tm.tv_sec - tm.tv_usec / 1000000.0);
  if (c_ts < 0) {
    raise_invalid_argument_warning
      ("timestamp: Sleep until to time is less than current time");
    return false;
  }

  struct timespec req, rem;
  req.tv_sec = (time_t)c_ts;
  req.tv_nsec = (long)((c_ts - req.tv_sec) * 1000000000.0);

  IOStatusHelper io("nanosleep");
  while (nanosleep(&req, &rem)) {
    recordNanosleepTime(req, &rem);
    if (errno != EINTR) return false;
    req.tv_sec = rem.tv_sec;
    req.tv_nsec = rem.tv_nsec;
  }
  recordNanosleepTime(req, nullptr);
  return true;
}

String HHVM_FUNCTION(uniqid, const String& prefix /* = null_string */,
                     bool more_entropy /* = false */) {
  if (!more_entropy) {
    Transport *transport = g_context->getTransport();
    if (transport) {
      transport->incuSleepTime(1);
    }
    usleep(1);
  }

  struct timeval tv;
  gettimeofday((struct timeval *)&tv, NULL);
  int sec = (int)tv.tv_sec;
  int usec = (int)(tv.tv_usec % 0x100000);

  String uniqid(prefix.size() + 64, ReserveString);
  auto ptr = uniqid.mutableData();
  // StringData::capacity() returns the buffer size without the null terminator.
  // snprintf() expects a "size" parameter that is the buffer capacity including
  // room for the null terminator, writes the null terminator, and returns the
  // number of formatted chars not counting the null terminator, whether or not
  // all the chars were actually written.
  // The return value may be larger than "size" if the buffer was too small.
  auto capacity = uniqid.capacity();
  uint32_t len;
  if (more_entropy) {
    len = snprintf(ptr, capacity + 1, "%s%08x%05x%.8F",
                   prefix.c_str(), sec, usec, math_combined_lcg() * 10);
  } else {
    len = snprintf(ptr, capacity + 1, "%s%08x%05x",
                   prefix.c_str(), sec, usec);
  }
  uniqid.setSize(std::min(len, capacity));
  return uniqid;
}

Variant HHVM_FUNCTION(unpack, const String& format, const String& data) {
  return ZendPack::getInstance()->unpack(format, data);
}

Array HHVM_FUNCTION(sys_getloadavg) {
  double load[3];
  getloadavg(load, 3);
  return make_vec_array(load[0], load[1], load[2]);
}

Variant HHVM_FUNCTION(array_mark_legacy, const Variant& v, bool recursive) {
  Variant force_cow = v;
  auto const result =
    recursive ? arrprov::markTvRecursively(*v.asTypedValue(), /*legacy=*/true)
              : arrprov::markTvShallow(*v.asTypedValue(), /*legacy=*/true);
  return Variant::attach(result);
}

Variant HHVM_FUNCTION(array_unmark_legacy, const Variant& v, bool recursive) {
  Variant force_cow = v;
  auto const result =
    recursive ? arrprov::markTvRecursively(*v.asTypedValue(), /*legacy=*/false)
              : arrprov::markTvShallow(*v.asTypedValue(), /*legacy=*/false);
  return Variant::attach(result);
}

bool HHVM_FUNCTION(is_array_marked_legacy, const Variant& v) {
  return v.isArray() && v.asCArrRef()->isLegacyArray();
}

String HHVM_FUNCTION(hphp_to_string, const Variant& v) {
  return v.toString();
}

// Adds an optimized FCallBuiltin for max with 2 operands to SystemLib
Variant HHVM_FUNCTION(SystemLib_max2, const Variant& value1,
                      const Variant& value2) {
  return less(value1, value2) ? value2 : value1;
}

// Adds an optimized FCallBuiltin for min with 2 operands to SystemLib
Variant HHVM_FUNCTION(SystemLib_min2, const Variant& value1,
                      const Variant& value2) {
  return more(value1, value2) ? value2 : value1;
}

String HHVM_FUNCTION(hhvm_binary) {
  return String(current_executable_path());
}

void StandardExtension::initMisc() {
    HHVM_FALIAS(HH\\server_warmup_status, server_warmup_status);
    HHVM_FALIAS(HH\\server_warmup_status_monotonic,
                server_warmup_status_monotonic);
    HHVM_FALIAS(HH\\set_endpoint_name, set_endpoint_name);
    HHVM_FALIAS(HH\\execution_context, execution_context);
    HHVM_FE(connection_aborted);
    HHVM_FE(connection_status);
    HHVM_FE(connection_timeout);
    HHVM_FE(constant);
    HHVM_FE(defined);
    HHVM_FE(ignore_user_abort);
    HHVM_FE(pack);
    HHVM_FE(sleep);
    HHVM_FE(usleep);
    HHVM_FE(time_nanosleep);
    HHVM_FE(time_sleep_until);
    HHVM_FE(uniqid);
    HHVM_FE(unpack);
    HHVM_FE(sys_getloadavg);
    HHVM_FE(hphp_to_string);
    HHVM_FALIAS(HH\\array_mark_legacy, array_mark_legacy);
    HHVM_FALIAS(HH\\array_unmark_legacy, array_unmark_legacy);
    HHVM_FALIAS(HH\\is_array_marked_legacy, is_array_marked_legacy);
    HHVM_FALIAS(__SystemLib\\max2, SystemLib_max2);
    HHVM_FALIAS(__SystemLib\\min2, SystemLib_min2);
    HHVM_FALIAS(HH\\__internal\\hhvm_binary, hhvm_binary);

    HHVM_RC_BOOL(TRUE, true);
    HHVM_RC_BOOL(true, true);
    HHVM_RC_BOOL(FALSE, false);
    HHVM_RC_BOOL(false, false);
    Native::registerConstant<KindOfNull>(makeStaticString("NULL"));
    Native::registerConstant<KindOfNull>(makeStaticString("null"));

    HHVM_RC_INT(PHP_MAXPATHLEN, PATH_MAX);
#ifndef NDEBUG
    HHVM_RC_BOOL(PHP_DEBUG, true);
#else
    HHVM_RC_BOOL(PHP_DEBUG, false);
#endif

    HHVM_RC_INT(INI_CONSTANT, int64_t(IniSetting::Mode::Constant));
    HHVM_RC_INT(INI_CONFIG,   int64_t(IniSetting::Mode::Config));
    HHVM_RC_INT(INI_REQUEST,  int64_t(IniSetting::Mode::Request));

    HHVM_RC_STR(PHP_OS, HHVM_FN(php_uname)("s").toString());

    HHVM_RC_INT(PHP_INT_MIN, k_PHP_INT_MIN);
    HHVM_RC_INT(PHP_INT_MAX, k_PHP_INT_MAX);

    if (Cfg::PHP7::Builtins) {
      HHVM_RC_INT(PHP_MAJOR_VERSION, PHP_MAJOR_VERSION_7);
      HHVM_RC_INT(PHP_MINOR_VERSION, PHP_MINOR_VERSION_7);
      HHVM_RC_STR(PHP_VERSION, PHP_VERSION_7);
      HHVM_RC_INT(PHP_VERSION_ID, PHP_VERSION_ID_7);
    } else {
      HHVM_RC_INT(PHP_MAJOR_VERSION, PHP_MAJOR_VERSION_5);
      HHVM_RC_INT(PHP_MINOR_VERSION, PHP_MINOR_VERSION_5);
      HHVM_RC_STR(PHP_VERSION, PHP_VERSION_5);
      HHVM_RC_INT(PHP_VERSION_ID, PHP_VERSION_ID_5);
    }

    HHVM_RC_INT_SAME(PHP_RELEASE_VERSION);
    HHVM_RC_STR_SAME(PHP_EXTRA_VERSION);

    HHVM_RC_INT(CONNECTION_NORMAL,  k_CONNECTION_NORMAL);
    HHVM_RC_INT(CONNECTION_ABORTED, k_CONNECTION_ABORTED);
    HHVM_RC_INT(CONNECTION_TIMEOUT, k_CONNECTION_TIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////
}
