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
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/zend-pack.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/type-profile.h"

#include "hphp/runtime/ext/std/ext_std_math.h"
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

namespace HPHP {

RDS_LOCAL(std::string, s_misc_highlight_default_string);
RDS_LOCAL(std::string, s_misc_highlight_default_comment);
RDS_LOCAL(std::string, s_misc_highlight_default_keyword);
RDS_LOCAL(std::string, s_misc_highlight_default_default);
RDS_LOCAL(std::string, s_misc_highlight_default_html);
RDS_LOCAL(std::string, s_misc_display_errors);

const std::string s_1("1"), s_2("2"), s_stdout("stdout"), s_stderr("stderr");
const double k_INF = std::numeric_limits<double>::infinity();
const double k_NAN = std::numeric_limits<double>::quiet_NaN();

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

  return empty_string();
}

String HHVM_FUNCTION(server_warmup_status_monotonic) {
  return String(jit::tc::warmupStatusString());
}

const StaticString
  s_clisrv("clisrv"),
  s_cli("cli"),
  s_worker("worker");

String HHVM_FUNCTION(execution_context) {
  if (is_cli_mode()) return s_clisrv;

  if (auto t = g_context->getTransport()) {
    return t->describe();
  }

  return RuntimeOption::ServerExecutionMode() ? s_worker : s_cli;
}

TypedValue HHVM_FUNCTION(sequence, const Array& args) {
  if (args.empty()) return make_tv<KindOfNull>();
  int64_t idx = args.size() - 1;
  auto ret = args.rvalAt(idx).tv();
  tvIncRefGen(ret);
  return ret;
}

}

void StandardExtension::threadInitMisc() {
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.string", "#DD0000",
      s_misc_highlight_default_string.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.comment", "#FF8000",
      s_misc_highlight_default_comment.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.keyword", "#007700",
      s_misc_highlight_default_keyword.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.default", "#0000BB",
      s_misc_highlight_default_default.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "highlight.html", "#000000",
      s_misc_highlight_default_html.get()
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
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
        nullptr
      ),
      s_misc_display_errors.get()
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
  return RuntimeOption::PHP7_Builtins ? v7 : v5;
}

void StandardExtension::initMisc() {
    HHVM_FALIAS(HH\\server_warmup_status, server_warmup_status);
    HHVM_FALIAS(HH\\server_warmup_status_monotonic,
                server_warmup_status_monotonic);
    HHVM_FALIAS(HH\\execution_context, execution_context);
    HHVM_FALIAS(HH\\sequence, sequence);
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
    HHVM_FALIAS(HH\\enable_legacy_behavior, enable_legacy_behavior);
    HHVM_FALIAS(HH\\is_legacy_behavior_enabled, is_legacy_behavior_enabled);
    HHVM_FALIAS(__SystemLib\\max2, SystemLib_max2);
    HHVM_FALIAS(__SystemLib\\min2, SystemLib_min2);

    HHVM_RC_BOOL(TRUE, true);
    HHVM_RC_BOOL(true, true);
    HHVM_RC_BOOL(FALSE, false);
    HHVM_RC_BOOL(false, false);
    Native::registerConstant<KindOfNull>(makeStaticString("NULL"));
    Native::registerConstant<KindOfNull>(makeStaticString("null"));

    HHVM_RC_BOOL(ZEND_THREAD_SAFE, true);

    HHVM_RC_DBL(INF, k_INF);
    HHVM_RC_DBL(NAN, k_NAN);
    HHVM_RC_INT(PHP_MAXPATHLEN, PATH_MAX);
#ifndef NDEBUG
    HHVM_RC_BOOL(PHP_DEBUG, true);
#else
    HHVM_RC_BOOL(PHP_DEBUG, false);
#endif

    HHVM_RC_INT(UPLOAD_ERR_OK,         0);
    HHVM_RC_INT(UPLOAD_ERR_INI_SIZE,   1);
    HHVM_RC_INT(UPLOAD_ERR_FORM_SIZE,  2);
    HHVM_RC_INT(UPLOAD_ERR_PARTIAL,    3);
    HHVM_RC_INT(UPLOAD_ERR_NO_FILE,    4);
    HHVM_RC_INT(UPLOAD_ERR_NO_TMP_DIR, 6);
    HHVM_RC_INT(UPLOAD_ERR_CANT_WRITE, 7);
    HHVM_RC_INT(UPLOAD_ERR_EXTENSION,  8);

    HHVM_RC_INT(CREDITS_GROUP,    1 << 0);
    HHVM_RC_INT(CREDITS_GENERAL,  1 << 1);
    HHVM_RC_INT(CREDITS_SAPI,     1 << 2);
    HHVM_RC_INT(CREDITS_MODULES,  1 << 3);
    HHVM_RC_INT(CREDITS_DOCS,     1 << 4);
    HHVM_RC_INT(CREDITS_FULLPAGE, 1 << 5);
    HHVM_RC_INT(CREDITS_QA,       1 << 6);
    HHVM_RC_INT(CREDITS_ALL, 0xFFFFFFFF);

    HHVM_RC_INT(INI_SYSTEM, IniSetting::PHP_INI_SYSTEM);
    HHVM_RC_INT(INI_PERDIR, IniSetting::PHP_INI_PERDIR);
    HHVM_RC_INT(INI_USER,   IniSetting::PHP_INI_USER);
    HHVM_RC_INT(INI_ALL,    IniSetting::PHP_INI_SYSTEM |
                            IniSetting::PHP_INI_PERDIR |
                            IniSetting::PHP_INI_USER);

    HHVM_RC_DYNAMIC(PHP_BINARY,
                    make_tv<KindOfPersistentString>(
                      makeStaticString(current_executable_path())));
    HHVM_RC_DYNAMIC(PHP_BINDIR,
                    make_tv<KindOfPersistentString>(
                      makeStaticString(current_executable_directory())));
    HHVM_RC_STR(PHP_OS, HHVM_FN(php_uname)("s").toString());
    HHVM_RC_DYNAMIC(PHP_SAPI,
                    make_tv<KindOfPersistentString>(
                      makeStaticString(HHVM_FN(php_sapi_name()))));

    HHVM_RC_INT(PHP_INT_SIZE, sizeof(int64_t));
    HHVM_RC_INT(PHP_INT_MIN, k_PHP_INT_MIN);
    HHVM_RC_INT(PHP_INT_MAX, k_PHP_INT_MAX);

    if (RuntimeOption::PHP7_Builtins) {
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

    // FIXME: These values are hardcoded from their previous IDL values
    // Grab their correct values from the system as appropriate
    HHVM_RC_STR(PHP_EOL, "\n");
    HHVM_RC_STR(PHP_CONFIG_FILE_PATH, "");
    HHVM_RC_STR(PHP_CONFIG_FILE_SCAN_DIR, "");
    HHVM_RC_STR(PHP_DATADIR, "");
    HHVM_RC_STR(PHP_EXTENSION_DIR, "");
    HHVM_RC_STR(PHP_LIBDIR, "");
    HHVM_RC_STR(PHP_LOCALSTATEDIR, "");
    HHVM_RC_STR(PHP_PREFIX, "");
    HHVM_RC_STR(PHP_SHLIB_SUFFIX, "so");
    HHVM_RC_STR(PHP_SYSCONFDIR, "");
    HHVM_RC_STR(PEAR_EXTENSION_DIR, "");
    HHVM_RC_STR(PEAR_INSTALL_DIR, "");
    HHVM_RC_STR(DEFAULT_INCLUDE_PATH, "");

    // I'm honestly not sure where these constants came from
    // I've brought them for ward from their IDL definitions
    // with their previous hard-coded values.
    HHVM_RC_INT(CODESET,         14);
    HHVM_RC_INT(RADIXCHAR,    65536);
    HHVM_RC_INT(THOUSEP,      65537);
    HHVM_RC_INT(ALT_DIGITS,  131119);
    HHVM_RC_INT(AM_STR,      131110);
    HHVM_RC_INT(PM_STR,      131111);
    HHVM_RC_INT(D_T_FMT,     131112);
    HHVM_RC_INT(D_FMT,       131113);
    HHVM_RC_INT(ERA,         131116);
    HHVM_RC_INT(ERA_D_FMT,   131118);
    HHVM_RC_INT(ERA_D_T_FMT, 131120);
    HHVM_RC_INT(ERA_T_FMT,   131121);
    HHVM_RC_INT(CRNCYSTR,    262159);

    loadSystemlib("std_misc");
  }


///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(connection_aborted) {
  return HHVM_FN(connection_status)() == k_CONNECTION_ABORTED;
}

int64_t HHVM_FUNCTION(connection_status) {
  // FIXME: WAT?
  return k_CONNECTION_NORMAL;
}

int64_t HHVM_FUNCTION(connection_timeout) {
  return HHVM_FN(connection_status)() == k_CONNECTION_TIMEOUT;
}

static Class* getClassByName(const char* name, int len) {
  String className(name, len, CopyString);
  return Unit::loadClass(className.get());
}

Variant HHVM_FUNCTION(constant, const String& name) {
  auto const warning = "constant() is deprecated and subject"
  " to removal from the Hack language";
  switch (RuntimeOption::DisableConstant) {
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
      Cell cns = cls->clsCnsGet(cnsName.get());
      if (cns.m_type != KindOfUninit) {
        return cellAsCVarRef(cns);
      }
    }
  } else {
    auto const cns = Unit::loadCns(name.get());
    if (cns) return Variant::wrap(*cns);
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
    auto* cb = autoload ? Unit::loadCns : Unit::lookupCns;
    return cb(name.get()).is_set();
  }
}

int64_t HHVM_FUNCTION(ignore_user_abort, bool /*setting*/ /* = false */) {
  return 0;
}

Variant HHVM_FUNCTION(pack, const String& format, const Array& argv) {
  // pack() returns false if there was an error, String otherwise
  return ZendPack().pack(format, argv);
}

int64_t HHVM_FUNCTION(sleep, int seconds) {
  IOStatusHelper io("sleep");
  Transport *transport = g_context->getTransport();
  if (transport) {
    transport->incSleepTime(seconds);
  }
  sleep(seconds);
  return 0;
}

void HHVM_FUNCTION(usleep, int micro_seconds) {
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

Variant HHVM_FUNCTION(time_nanosleep, int seconds, int nanoseconds) {
  if (seconds < 0) {
    throw_invalid_argument("seconds: cannot be negative");
    return false;
  }
  if (nanoseconds < 0 || nanoseconds > 999999999) {
    throw_invalid_argument("nanoseconds: has to be 0 to 999999999");
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
    return make_map_array(s_seconds, (int64_t)rem.tv_sec,
                          s_nanoseconds, (int64_t)rem.tv_nsec);
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
    throw_invalid_argument
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
  // StringData::capacity() returns the buffer size without the null
  // terminator. snprintf expects a the buffer capacity including room
  // for the null terminator, writes the null termintor, and returns
  // the full length not counting the null terminator.
  auto capacity = uniqid.capacity() + 1;
  int64_t len;
  if (more_entropy) {
    len = snprintf(ptr, capacity, "%s%08x%05x%.8F",
                   prefix.c_str(), sec, usec, math_combined_lcg() * 10);
  } else {
    len = snprintf(ptr, capacity, "%s%08x%05x",
                   prefix.c_str(), sec, usec);
  }
  uniqid.setSize(len);
  return uniqid;
}

Variant HHVM_FUNCTION(unpack, const String& format, const String& data) {
  return ZendPack().unpack(format, data);
}

Array HHVM_FUNCTION(sys_getloadavg) {
  double load[3];
  getloadavg(load, 3);
  return make_varray(load[0], load[1], load[2]);
}

Variant HHVM_FUNCTION(enable_legacy_behavior, const Variant& v) {
  if (v.isVecArray() || v.isDict()) {
    auto arr = v.toCArrRef().copy();
    arr->setLegacyArray(true);
    return arr;
  } else {
    raise_warning("enable_legacy_behavior expects a dict or vec");
    return v;
  }
}

bool HHVM_FUNCTION(is_legacy_behavior_enabled, const Variant& v) {
  if (!v.isVecArray() && !v.isDict()) {
    raise_warning("is_legacy_behavior_enabled expects a dict or vec");
    return false;
  }
  return v.asCArrRef()->isLegacyArray();
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

///////////////////////////////////////////////////////////////////////////////
}
