/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/ext/xdebug/xdebug_profiler.h"
#include "hphp/runtime/ext/xdebug/xdebug_server.h"

#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/timer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Helpers

// Globals
static const StaticString
  s_SERVER("_SERVER"),
  s_COOKIE("_COOKIE");

// Returns the frame of the callee's callee. Useful for the xdebug_call_*
// functions. Only returns nullptr if the callee is the top level pseudo-main
//
// If an offset pointer is passed, we store in it it the pc offset of the
// call to the callee.
static ActRec *get_call_fp(Offset *off = nullptr) {
  // We want the frame of our callee's callee
  VMRegAnchor _; // Ensure consistent state for vmfp
  ActRec* fp0 = g_context->getPrevVMState(vmfp());
  assert(fp0);
  ActRec* fp1 = g_context->getPrevVMState(fp0, off);

  // fp1 should only be NULL if fp0 is the top-level pseudo-main
  if (!fp1) {
    assert(fp0->m_func->isPseudoMain());
    fp1 = nullptr;
  }
  return fp1;
}

// Keys in $_SERVER used by format_filename
static const StaticString
  s_HTTP_HOST("HTTP_HOST"),
  s_REQUEST_URI("REQUEST_URI"),
  s_SCRIPT_NAME("SCRIPT_NAME"),
  s_UNIQUE_ID("UNIQUE_ID"),
  s_SESSION_NAME("session.name");

// Helper for format_filename that removes characters defined by xdebug to be
// "special" characters. They are replaced with _. The string is modified in
// place, so there shouldn't be more than one reference to it.
static void replace_special_chars(StringData* str) {
  assert(!str->hasMultipleRefs());
  int len = str->size();
  char* data = str->mutableData();
  for (int i = 0; i < len; i++) {
    switch (data[i]) {
      case '/':
      case '\\':
      case '.':
      case '?':
      case '&':
      case '+':
      case ' ':
        data[i] = '_';
        break;
      default:
        break;
    }
  }
}

// Helper used to create an absolute filename using the passed
// directory and xdebug-specific format string
static String format_filename(String* dir,
                              const String& formatFile,
                              bool addSuffix) {
  // Create a string buffer and append the directory name
  int formatlen = formatFile.size();
  StringBuffer buf(formatlen * 2); // Slightly larger than formatlen
  if (dir != nullptr) {
    buf.append(*dir);
    buf.append('/');
  }

  // Append the filename
  ArrayData* globals = get_global_variables()->asArrayData();
  for (int pos = 0; pos < formatlen; pos++) {
    char c = formatFile.charAt(pos);
    if (c != '%' || pos + 1 == formatlen) {
      buf.append(c);
      continue;
    }

    c = formatFile.charAt(++pos);
    switch (c) {
      // crc32 of current working directory
      case 'c': {
        int64_t crc32 = f_crc32(g_context->getCwd());
        buf.append(crc32);
        break;
      }
      // process id
      case 'p':
        buf.append(getpid());
        break;
      // Random number
      case 'r':
        buf.printf("%lx", (uint64_t) f_rand());
        break;
      // Script name
      case 's': {
        Array server = globals->get(s_SERVER).toArray();
        if (server.exists(s_SCRIPT_NAME) && server[s_SCRIPT_NAME].isString()) {
          const String scriptname(server[s_SCRIPT_NAME].toString(), CopyString);
          replace_special_chars(scriptname.get());
          buf.append(scriptname);
        }
        break;
      }
      // Timestamp (seconds)
      case 't': {
        int64_t sec = (int64_t)time(nullptr);
        if (sec != -1) {
          buf.append(sec);
        }
        break;
      }
      // Timestamp (microseconds)
      case 'u': {
        struct timeval tv;
        if (gettimeofday(&tv, 0) != -1) {
          buf.printf("%ld_%ld", int64_t(tv.tv_sec), tv.tv_usec);
        }
        break;
      }
      // $_SERVER['HTTP_HOST']
      case 'H': {
        Array server = globals->get(s_SERVER).toArray();
        if (server.exists(s_HTTP_HOST) && server[s_HTTP_HOST].isString()) {
          const String hostname(server[s_HTTP_HOST].toString(), CopyString);
          replace_special_chars(hostname.get());
          buf.append(hostname);
        }
        break;
      }
      // $_SERVER['REQUEST_URI']
      case 'R': {
        Array server = globals->get(s_SERVER).toArray();
        if (globals->exists(s_REQUEST_URI)) {
          const String requri(server[s_REQUEST_URI].toString(), CopyString);
          replace_special_chars(requri.get());
          buf.append(requri);
        }
        break;
      }
      // $_SERVER['UNIQUE_ID']
      case 'U': {
        Array server = globals->get(s_SERVER).toArray();
        if (server.exists(s_UNIQUE_ID) && server[s_UNIQUE_ID].isString()) {
          const String uniqueid(server[s_UNIQUE_ID].toString(), CopyString);
          replace_special_chars(uniqueid.get());
          buf.append(uniqueid);
        }
        break;
      }
      // session id
      case 'S': {
        // First we grab the session name from the ini settings, then the id
        // from the cookies
        String session_name;
        if (IniSetting::Get(s_SESSION_NAME, session_name)) {
          Array cookies = globals->get(s_COOKIE).toArray();
          if (cookies.exists(session_name) &&
              cookies[session_name].isString()) {
            const String sessionstr(cookies[session_name].toString(),
                                    CopyString);
            replace_special_chars(sessionstr.get());
            buf.append(sessionstr);
          }
          break;
        }
      }
      // Literal
      case '%':
        buf.append('%');
        break;
      default:
        buf.append('%');
        buf.append(c);
        break;
    }
  }

  // Optionally add .xt file extension
  if (addSuffix) {
    buf.append(".xt");
  }
  return buf.copy();
}

// Profiler factory- for starting and stopping the profiler
DECLARE_EXTERN_REQUEST_LOCAL(ProfilerFactory, s_profiler_factory);

// Returns the attached xdebug profiler. Requires one is attached.
static inline XDebugProfiler* xdebug_profiler() {
  assert(XDEBUG_GLOBAL(ProfilerAttached));
  return (XDebugProfiler*) s_profiler_factory->getProfiler();
}

// Starts tracing using the given profiler
static void start_tracing(XDebugProfiler* profiler,
                          String* filename = nullptr,
                          int64_t options = 0) {
  // Add ini settings
  if (XDEBUG_GLOBAL(TraceOptions)) {
    options |= k_XDEBUG_TRACE_APPEND;
  }
  if (XDEBUG_GLOBAL(TraceFormat) == 1) {
    options |= k_XDEBUG_TRACE_COMPUTERIZED;
  }
  if (XDEBUG_GLOBAL(TraceFormat) == 2) {
    options |= k_XDEBUG_TRACE_HTML;
  }

  // If no filename is passed, php5 xdebug stores in the default output
  // directory with the default file name
  String* dirname = nullptr;
  String default_dirname(XDEBUG_GLOBAL(TraceOutputDir));
  String default_filename(XDEBUG_GLOBAL(TraceOutputName));
  if (filename == nullptr) {
    dirname = &default_dirname;
    filename = &default_filename;
  }

  bool suffix = !(options & k_XDEBUG_TRACE_NAKED_FILENAME);
  String abs_filename = format_filename(dirname, *filename, suffix);
  profiler->enableTracing(abs_filename, options);
}

// Starts profiling using the given profiler
static void start_profiling(XDebugProfiler* profiler) {
  // Add ini options
  int64_t opts = 0;
  if (XDEBUG_GLOBAL(ProfilerAppend)) {
    opts |= k_XDEBUG_PROFILE_APPEND;
  }

  // Create the filename then enable
  String dirname(XDEBUG_GLOBAL(ProfilerOutputDir));
  String filename(XDEBUG_GLOBAL(ProfilerOutputName));
  String abs_filename = format_filename(&dirname, filename, false);
  profiler->enableProfiling(abs_filename, opts);
}

// Attempts to attach the xdebug profiler to the current thread. Assumes it
// is not already attached. Raises an error on failure.
static void attach_xdebug_profiler() {
  assert(!XDEBUG_GLOBAL(ProfilerAttached));
  if (s_profiler_factory->start(ProfilerKind::XDebug, 0, false)) {
    XDEBUG_GLOBAL(ProfilerAttached) = true;
    // Enable profiling and tracing if we need to
    XDebugProfiler* profiler = xdebug_profiler();
    if (XDebugProfiler::isProfilingNeeded()) {
      start_profiling(profiler);
    }
    if (XDebugProfiler::isTracingNeeded()) {
      start_tracing(profiler);
    }
    profiler->setCollectMemory(XDEBUG_GLOBAL(CollectMemory));
    profiler->setCollectTime(XDEBUG_GLOBAL(CollectTime));
  } else {
    raise_error("Could not start xdebug profiler. Another profiler is "
                "likely already attached to this thread.");
  }
}

// Detaches the xdebug profiler from the current thread
static void detach_xdebug_profiler() {
  assert(XDEBUG_GLOBAL(ProfilerAttached));
  s_profiler_factory->stop();
  XDEBUG_GLOBAL(ProfilerAttached) = false;
}

// Detaches the xdebug profiler if it's no longer needed
static void detach_xdebug_profiler_if_needed() {
  assert(XDEBUG_GLOBAL(ProfilerAttached));
  XDebugProfiler* profiler = xdebug_profiler();
  if (!profiler->isCollecting()) {
    detach_xdebug_profiler();
  }
}

///////////////////////////////////////////////////////////////////////////////
// XDebug Implementation

static bool HHVM_FUNCTION(xdebug_break)
  XDEBUG_NOTIMPLEMENTED

static Variant HHVM_FUNCTION(xdebug_call_class) {
  // PHP5 xdebug returns false if the callee is top-level
  ActRec *fp = get_call_fp();
  if (fp == nullptr) {
    return false;
  }

  // PHP5 xdebug returns "" for no class
  Class* cls = fp->m_func->cls();
  if (!cls) {
    return staticEmptyString();
  }
  return String(cls->name()->data(), CopyString);
}

static String HHVM_FUNCTION(xdebug_call_file) {
  // PHP5 xdebug returns the top-level file if the callee is top-level
  ActRec *fp = get_call_fp();
  if (fp == nullptr) {
    VMRegAnchor _; // Ensure consistent state for vmfp
    fp = g_context->getPrevVMState(vmfp());
    assert(fp);
  }
  return String(fp->m_func->filename()->data(), CopyString);
}

static int64_t HHVM_FUNCTION(xdebug_call_line) {
  // PHP5 xdebug returns 0 when it can't determine the line number
  Offset pc;
  ActRec *fp = get_call_fp(&pc);
  if (fp == nullptr) {
    return 0;
  }

  Unit *unit = fp->m_func->unit();
  assert(unit);
  return unit->getLineNumber(pc);
}

// php5 xdebug main function string equivalent
static const StaticString s_CALL_FN_MAIN("{main}");

static Variant HHVM_FUNCTION(xdebug_call_function) {
  // PHP5 xdebug returns false if the callee is top-level
  ActRec *fp = get_call_fp();
  if (fp == nullptr) {
    return false;
  }

  // PHP5 xdebug returns "{main}" for pseudo-main
  if (fp->m_func->isPseudoMain()) {
    return s_CALL_FN_MAIN;
  }
  return String(fp->m_func->name()->data(), CopyString);
}

static bool HHVM_FUNCTION(xdebug_code_coverage_started) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  return ti->m_reqInjectionData.getCoverage();
}

static TypedValue* HHVM_FN(xdebug_debug_zval)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

static TypedValue* HHVM_FN(xdebug_debug_zval_stdout)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_disable)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_dump_superglobals)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_enable)
  XDEBUG_NOTIMPLEMENTED

static Array HHVM_FUNCTION(xdebug_get_code_coverage) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.getCoverage()) {
    return ti->m_coverage->Report(false);
  }
  return Array::Create();
}

static Array HHVM_FUNCTION(xdebug_get_collected_errors,
                           bool clean /* = false */)
  XDEBUG_NOTIMPLEMENTED

static const StaticString s_closure_varname("0Closure");

static Array HHVM_FUNCTION(xdebug_get_declared_vars) {
  // Grab the callee function
  VMRegAnchor _; // Ensure consistent state for vmfp
  ActRec* fp = g_context->getPrevVMState(vmfp());
  assert(fp);
  const Func* func = fp->func();

  // Add each named local to the returned array. Note that since this function
  // is supposed to return all _declared_ variables in scope, which includes
  // variables that have been unset.
  const Id numNames = func->numNamedLocals();
  PackedArrayInit vars(numNames);
  for (Id i = 0; i < numNames; ++i) {
    assert(func->lookupVarId(func->localVarName(i)) == i);
    String varname(func->localVarName(i)->data(), CopyString);
    // Skip the internal closure "0Closure" variable
    if (!s_closure_varname.equal(varname)) {
      vars.append(varname);
    }
  }
  return vars.toArray();
}

static Array HHVM_FUNCTION(xdebug_get_function_stack) {
  // Need to reverse the backtrace to match php5 xdebug
  Array bt = createBacktrace(BacktraceArgs()
                             .skipTop()
                             .withPseudoMain()
                             .withArgNames()
                             .withArgValues(*XDebugExtension::CollectParams));

  return ArrayUtil::Reverse(bt).toArray();
}

static Array HHVM_FUNCTION(xdebug_get_headers)
  XDEBUG_NOTIMPLEMENTED

Variant HHVM_FUNCTION(xdebug_get_profiler_filename) {
  if (!XDEBUG_GLOBAL(ProfilerAttached)) {
    return false;
  }

  XDebugProfiler* profiler = xdebug_profiler();
  if (profiler->isProfiling()) {
    return profiler->getProfilingFilename();
  } else {
    return false;
  }
}

static int64_t HHVM_FUNCTION(xdebug_get_stack_depth)
  XDEBUG_NOTIMPLEMENTED

static Variant HHVM_FUNCTION(xdebug_get_tracefile_name) {
  if (XDEBUG_GLOBAL(ProfilerAttached)) {
    XDebugProfiler* profiler = xdebug_profiler();
    if (profiler->isTracing()) {
      return profiler->getTracingFilename();
    }
  }
  return false;
}

static bool HHVM_FUNCTION(xdebug_is_enabled) {
  return false;
}

static int64_t HHVM_FUNCTION(xdebug_memory_usage) {
  // With jemalloc, the usage can go negative (see ext_std_options.cpp:831)
  int64_t usage = MM().getStats().usage;
  assert(use_jemalloc || usage >= 0);
  return std::max<int64_t>(usage, 0);
}

static int64_t HHVM_FUNCTION(xdebug_peak_memory_usage) {
  return MM().getStats().peakUsage;
}

static void HHVM_FUNCTION(xdebug_print_function_stack,
                          const String& message /* = "user triggered" */,
                          int64_t options /* = 0 */)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_start_code_coverage,
                          int64_t options /* = 0 */) {
  // XDEBUG_CC_UNUSED and XDEBUG_CC_DEAD_CODE not supported right now primarily
  // because the internal CodeCoverage class does support either unexecuted line
  // tracking or dead code analysis
  if (options != 0) {
    raise_error("XDEBUG_CC_UNUSED and XDEBUG_CC_DEAD_CODE constants are not "
                "currently supported.");
    return;
  }

  // If we get here, turn on coverage
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.setCoverage(true);
  if (g_context->isNested()) {
    raise_notice("Calling xdebug_start_code_coverage from a nested VM instance "
                 "may cause unpredicable results");
  }
  throw VMSwitchModeBuiltin();
}

static void HHVM_FUNCTION(xdebug_start_error_collection)
  XDEBUG_NOTIMPLEMENTED

static Variant HHVM_FUNCTION(xdebug_start_trace,
                             const Variant& traceFileVar,
                             int64_t options /* = 0 */) {
  // Allowed to pass null
  String traceFileStr;
  if (traceFileVar.isString()) {
    traceFileStr = traceFileVar.toString();
  }
  String* traceFile = traceFileVar.isString()? &traceFileStr : nullptr;

  // Initialize the profiler if it isn't already
  if (!XDEBUG_GLOBAL(ProfilerAttached)) {
    attach_xdebug_profiler();
  }

  // php5 xdebug returns false when tracing already started
  XDebugProfiler* profiler = xdebug_profiler();
  if (profiler->isTracing()) {
    return false;
  }

  // Start tracing, then grab the current begin frame
  start_tracing(profiler, traceFile, options);
  profiler->beginFrame(nullptr);
  return HHVM_FN(xdebug_get_tracefile_name)();
}

static void HHVM_FUNCTION(xdebug_stop_code_coverage,
                          bool cleanup /* = true */) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.setCoverage(false);
  if (cleanup) {
    ti->m_coverage->Reset();
  }
}

static void HHVM_FUNCTION(xdebug_stop_error_collection)
  XDEBUG_NOTIMPLEMENTED

static Variant HHVM_FUNCTION(xdebug_stop_trace) {
  if (!XDEBUG_GLOBAL(ProfilerAttached)) {
    return false;
  }

  XDebugProfiler* profiler = xdebug_profiler();
  if (!profiler->isTracing()) {
    return false;
  }

  // End with xdebug_stop_trace()
  profiler->endFrame(init_null().asTypedValue(), nullptr, false);
  String filename = profiler->getTracingFilename();
  profiler->disableTracing();
  detach_xdebug_profiler_if_needed();
  return filename;
}

static double HHVM_FUNCTION(xdebug_time_index) {
  int64_t micro = Timer::GetCurrentTimeMicros() - XDEBUG_GLOBAL(InitTime);
  return micro * 1.0e-6;
}

static TypedValue* HHVM_FN(xdebug_var_dump)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(_xdebug_check_trigger_vars) {
  if (XDebugExtension::Enable &&
      XDebugProfiler::isNeeded() &&
      !XDEBUG_GLOBAL(ProfilerAttached)) {
    attach_xdebug_profiler();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Module implementation

// XDebug constants
static const StaticString
  s_XDEBUG_CC_UNUSED("XDEBUG_CC_UNUSED"),
  s_XDEBUG_CC_DEAD_CODE("XDEBUG_CC_DEAD_CODE"),
  s_XDEBUG_TRACE_APPEND("XDEBUG_TRACE_APPEND"),
  s_XDEBUG_TRACE_COMPUTERIZED("XDEBUG_TRACE_COMPUTERIZED"),
  s_XDEBUG_TRACE_HTML("XDEBUG_TRACE_HTML"),
  s_XDEBUG_TRACE_NAKED_FILENAME("XDEBUG_TRACE_NAKED_FILENAME");

// Helper for requestInit that returns the initial value for the given config
// option.
template <typename T>
static inline T xdebug_init_opt(const char* name, T defVal,
                                map<string, string>& envCfg) {
  // First try to load the ini setting
  folly::dynamic ini_val = folly::dynamic::object();
  if (IniSetting::Get(XDEBUG_INI(name), ini_val)) {
    T val;
    ini_on_update(ini_val, val);
    return val;
  }

  // Then try to load from the environment
  map<string, string>::const_iterator env_iter = envCfg.find(name);
  if (env_iter != envCfg.end()) {
    T val;
    ini_on_update(env_iter->second, val);
    return val;
  }

  // Finally just use the default value
  return defVal;
}

// Attempts to load the default idekey from environment variables
static void loadIdeKey(map<string, string>& envCfg) {
  const char* dbgp_idekey = getenv("DBGP_IDEKEY");
  if (dbgp_idekey != nullptr) {
    envCfg["idekey"] = dbgp_idekey;
    return;
  }

  const char* user = getenv("USER");
  if (user != nullptr) {
    envCfg["idekey"] = user;
    return;
  }

  const char* username = getenv("USERNAME");
  if (username != nullptr) {
    envCfg["idekey"] = username;
  }
}

// Loads the "XDEBUG_CONFIG" environment variables.
static void loadEnvConfig(map<string, string>& envCfg) {
  // php5 xdebug grabs from getenv, not $_ENV
  const char* cfg_raw = getenv("XDEBUG_CONFIG");
  if (cfg_raw == nullptr) {
    return;
  }

  // Parse the config variable. Format is "key=val" list separated by spaces
  // This parsing isn't very efficient, but this isn't performance sensitive and
  // it's similar to what php5 xdebug does.
  Array cfg = StringUtil::Explode(String(cfg_raw, CopyString), " ").toArray();
  for (ArrayIter iter(cfg); iter; ++iter) {
    Array keyval = StringUtil::Explode(iter.second().toString(), "=").toArray();
    if (keyval.size() != 2) {
      continue;
    }

    string key = keyval[0].toString().toCppString();
    string val = keyval[1].toString().toCppString();
    if (key == "remote_enable" ||
        key == "remote_port" ||
        key == "remote_host" ||
        key == "remote_handler" ||
        key == "remote_mode" ||
        key == "idekey" ||
        key == "profiler_enable" ||
        key == "profiler_output_dir" ||
        key == "profiler_enable_trigger" ||
        key == "remote_log" ||
        key == "remote_cookie_expire_time" ||
        key == "cli_color") {
      envCfg[key] = val;
    }
  }
}

void XDebugExtension::moduleLoad(const IniSetting::Map& ini, Hdf xdebug_hdf) {
  Hdf hdf = xdebug_hdf[XDEBUG_NAME];
  Enable = Config::GetBool(ini, hdf["enable"], false);
}

void XDebugExtension::moduleInit() {
  if (!Enable) {
    return;
  }
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_CC_UNUSED.get(), k_XDEBUG_CC_UNUSED
  );
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_CC_DEAD_CODE.get(), k_XDEBUG_CC_DEAD_CODE
  );
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_TRACE_APPEND.get(), k_XDEBUG_TRACE_APPEND
  );
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_TRACE_COMPUTERIZED.get(), k_XDEBUG_TRACE_COMPUTERIZED
  );
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_TRACE_HTML.get(), k_XDEBUG_TRACE_HTML
  );
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_TRACE_NAKED_FILENAME.get(), k_XDEBUG_TRACE_NAKED_FILENAME
  );
  HHVM_FE(xdebug_break);
  HHVM_FE(xdebug_call_class);
  HHVM_FE(xdebug_call_file);
  HHVM_FE(xdebug_call_function);
  HHVM_FE(xdebug_call_line);
  HHVM_FE(xdebug_code_coverage_started);
  HHVM_FE(xdebug_debug_zval);
  HHVM_FE(xdebug_debug_zval_stdout);
  HHVM_FE(xdebug_disable);
  HHVM_FE(xdebug_dump_superglobals);
  HHVM_FE(xdebug_enable);
  HHVM_FE(xdebug_get_code_coverage);
  HHVM_FE(xdebug_get_collected_errors);
  HHVM_FE(xdebug_get_declared_vars);
  HHVM_NAMED_FE(__SystemLib\\xdebug_get_function_stack,
                HHVM_FN(xdebug_get_function_stack));
  HHVM_FE(xdebug_get_headers);
  HHVM_FE(xdebug_get_profiler_filename);
  HHVM_FE(xdebug_get_stack_depth);
  HHVM_FE(xdebug_get_tracefile_name);
  HHVM_FE(xdebug_is_enabled);
  HHVM_FE(xdebug_memory_usage);
  HHVM_FE(xdebug_peak_memory_usage);
  HHVM_FE(xdebug_print_function_stack);
  HHVM_FE(xdebug_start_code_coverage);
  HHVM_FE(xdebug_start_error_collection);
  HHVM_FE(xdebug_start_trace);
  HHVM_FE(xdebug_stop_code_coverage);
  HHVM_FE(xdebug_stop_error_collection);
  HHVM_FE(xdebug_stop_trace);
  HHVM_FE(xdebug_time_index);
  HHVM_FE(xdebug_var_dump);
  HHVM_FE(_xdebug_check_trigger_vars);
  loadSystemlib("xdebug");
}

void XDebugExtension::requestInit() {
  if (!Enable) {
    return;
  }

  // Load the settings passed in environment variables
  map<string, string> env_cfg;
  loadIdeKey(env_cfg);
  loadEnvConfig(env_cfg);

  // Thread local config options
  #define XDEBUG_OPT(T, name, sym, val) { \
    XDEBUG_GLOBAL(sym) = xdebug_init_opt<T>(name, val, env_cfg); \
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL, \
                     XDEBUG_INI(name), &XDEBUG_GLOBAL(sym)); \
  }
  XDEBUG_CFG
  #undef XDEBUG_OPT

  // hhvm.xdebug.dump.*
  #define XDEBUG_OPT(T, name, sym, val) { \
    XDEBUG_GLOBAL(sym) = xdebug_init_opt<T>(name, val, env_cfg); \
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL, \
                     XDEBUG_INI(name), &XDEBUG_GLOBAL(sym)); \
  }
  XDEBUG_DUMP_CFG
  #undef XDEBUG_OPT

  // Profiler config options
  #define XDEBUG_OPT(T, name, sym, def) \
    XDEBUG_GLOBAL(sym) = xdebug_init_opt<T>(name, def, env_cfg); \
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL, XDEBUG_INI(name), \
                     IniSetting::SetAndGet<T>([](const T& val) { \
                       if (XDEBUG_GLOBAL(ProfilerAttached)) { \
                         xdebug_profiler()->set##sym(val); \
                         detach_xdebug_profiler_if_needed(); \
                       } \
                       return true; \
                    }, nullptr), &XDEBUG_GLOBAL(sym));
  XDEBUG_PROF_CFG
  #undef XDEBUG_OPT

  // Custom request local globals
  #define XDEBUG_OPT(T, name, sym, val) XDEBUG_GLOBAL(sym) = val;
  XDEBUG_CUSTOM_GLOBALS
  #undef XDEBUG_OPT

  // Let the server do initialization
  XDebugServer::onRequestInit();

  // Potentially attach the xdebug profiler
  if (XDebugProfiler::isNeeded()) {
    attach_xdebug_profiler();
  }
}

void XDebugExtension::requestShutdown() {
  if (!Enable) {
    return;
  }

  // Potentially kill the profiler
  if (XDEBUG_GLOBAL(ProfilerAttached)) {
    detach_xdebug_profiler();
  }
}

// Non-bind config options and edge-cases
bool XDebugExtension::Enable = false;

// Standard config options
#define XDEBUG_OPT(T, name, sym, val) \
  IMPLEMENT_THREAD_LOCAL(T, XDebugExtension::sym);
XDEBUG_CFG
XDEBUG_DUMP_CFG
XDEBUG_PROF_CFG
XDEBUG_CUSTOM_GLOBALS
#undef XDEBUG_OPT

static XDebugExtension s_xdebug_extension;

///////////////////////////////////////////////////////////////////////////////
}
