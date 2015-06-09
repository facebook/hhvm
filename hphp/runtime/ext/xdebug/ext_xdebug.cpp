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

#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/std/ext_std_math.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_var.h"
#include "hphp/runtime/ext/xdebug/xdebug_profiler.h"
#include "hphp/runtime/ext/xdebug/xdebug_server.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

// TODO(#3704) Remove when xdebug fully implemented
#define XDEBUG_NOTIMPLEMENTED  { throw_not_implemented(__FUNCTION__); }

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Helpers

// Globals
const StaticString
  s_SERVER("_SERVER"),
  s_COOKIE("_COOKIE");

// Returns the frame of the callee's callee. Useful for the xdebug_call_*
// functions. Only returns nullptr if the callee is the top level pseudo-main
//
// If an offset pointer is passed, we store in it it the pc offset of the
// call to the callee.
static ActRec* get_call_fp(Offset* off = nullptr) {
  // We want the frame of our callee's callee
  VMRegAnchor _; // Ensure consistent state for vmfp
  auto fp0 = g_context->getPrevVMState(vmfp());
  assert(fp0);
  auto fp1 = g_context->getPrevVMState(fp0, off);

  // fp1 should only be NULL if fp0 is the top-level pseudo-main
  if (!fp1) {
    assert(fp0->m_func->isPseudoMain());
    fp1 = nullptr;
  }
  return fp1;
}

// Keys in $_SERVER used by format_filename
const StaticString
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
  auto const len = str->size();
  auto data = str->mutableData();
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
static String format_filename(StringSlice dir,
                              StringSlice formatFile,
                              bool addSuffix) {
  // Create a string buffer and append the directory name
  auto const formatlen = formatFile.size();
  StringBuffer buf(formatlen * 2); // Slightly larger than formatlen
  if (!dir.empty()) {
    buf.append(dir);
    buf.append('/');
  }

  // Append the filename
  auto globals = get_global_variables()->asArrayData();
  for (int pos = 0; pos < formatlen; pos++) {
    auto c = formatFile[pos];
    if (c != '%' || pos + 1 == formatlen) {
      buf.append(c);
      continue;
    }

    c = formatFile[++pos];
    switch (c) {
      // crc32 of current working directory
      case 'c': {
        auto const crc32 = HHVM_FN(crc32)(g_context->getCwd());
        buf.append(crc32);
        break;
      }
      // process id
      case 'p':
        buf.append(getpid());
        break;
      // Random number
      case 'r':
        buf.printf("%lx", (uint64_t)HHVM_FN(rand)());
        break;
      // Script name
      case 's': {
        auto server = globals->get(s_SERVER).toArray();
        if (server.exists(s_SCRIPT_NAME) && server[s_SCRIPT_NAME].isString()) {
          const String scriptname(server[s_SCRIPT_NAME].toString(), CopyString);
          replace_special_chars(scriptname.get());
          buf.append(scriptname);
        }
        break;
      }
      // Timestamp (seconds)
      case 't': {
        auto const sec = (int64_t)time(nullptr);
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
        auto server = globals->get(s_SERVER).toArray();
        if (globals->exists(s_REQUEST_URI)) {
          const String requri(server[s_REQUEST_URI].toString(), CopyString);
          replace_special_chars(requri.get());
          buf.append(requri);
        }
        break;
      }
      // $_SERVER['UNIQUE_ID']
      case 'U': {
        auto server = globals->get(s_SERVER).toArray();
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
          auto cookies = globals->get(s_COOKIE).toArray();
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
                          StringSlice filename = StringSlice(nullptr, 0),
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
  StringSlice dirname(nullptr, 0);

  if (filename.empty()) {
    auto& default_dirname = XDEBUG_GLOBAL(TraceOutputDir);
    auto& default_filename = XDEBUG_GLOBAL(TraceOutputName);

    dirname = StringSlice(default_dirname.data(), default_dirname.size());
    filename = StringSlice(default_filename.data(), default_filename.size());
  }

  auto const suffix = !(options & k_XDEBUG_TRACE_NAKED_FILENAME);
  auto abs_filename = format_filename(dirname, filename, suffix);
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
  auto& dirname = XDEBUG_GLOBAL(ProfilerOutputDir);
  auto& filename = XDEBUG_GLOBAL(ProfilerOutputName);
  auto dirname_slice = StringSlice(dirname.data(), dirname.size());
  auto filename_slice = StringSlice(filename.data(), filename.size());

  auto abs_filename = format_filename(dirname_slice, filename_slice, false);

  profiler->enableProfiling(abs_filename, opts);
}

// Attempts to attach the xdebug profiler to the current thread. Assumes it
// is not already attached. Raises an error on failure.
static void attach_xdebug_profiler() {
  assert(!XDEBUG_GLOBAL(ProfilerAttached));
  if (s_profiler_factory->start(ProfilerKind::XDebug, 0, false)) {
    XDEBUG_GLOBAL(ProfilerAttached) = true;
    // Enable profiling and tracing if we need to
    auto profiler = xdebug_profiler();
    if (XDebugProfiler::isProfilingNeeded()) {
      start_profiling(profiler);
    }
    if (XDebugProfiler::isTracingNeeded()) {
      start_tracing(profiler);
    }
    profiler->setCollectMemory(XDEBUG_GLOBAL(CollectMemory));
    profiler->setCollectTime(XDEBUG_GLOBAL(CollectTime));
    profiler->setMaxNestingLevel(XDEBUG_GLOBAL(MaxNestingLevel));
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
  auto profiler = xdebug_profiler();
  if (!profiler->isNeeded()) {
    detach_xdebug_profiler();
  }
}

// If the xdebug profiler is attached, ensures we still need it. If it is not
// attached, check if we need it.
static void refresh_xdebug_profiler() {
  if (XDEBUG_GLOBAL(ProfilerAttached)) {
    detach_xdebug_profiler_if_needed();
  } else if (XDebugProfiler::isCollectionNeeded()) {
    // We know that the profiler is not attached, so either we failed the
    // request init checks or it was turned off during runtime. So we
    // only want to turn on profiling if collection is now needed.
    attach_xdebug_profiler();
  }
}

static bool is_output_tty() {
  auto& tty = XDEBUG_GLOBAL(OutputIsTTY);
  if (tty.hasValue()) return *tty;
  return *(tty = isatty(STDOUT_FILENO));
}

///////////////////////////////////////////////////////////////////////////////
// XDebug Implementation

static bool HHVM_FUNCTION(xdebug_break) {
  auto server = XDEBUG_GLOBAL(Server);
  if (server == nullptr) {
    return false;
  }

  // Breakpoint displays the current file/line number
  auto file = g_context->getContainingFileName();
  auto filename = file == nullptr ? empty_string() : String(file);
  auto const line = g_context->getLine();

  // Attempt to perform the breakpoint, detach the server if something goes
  // wrong
  if (!server->breakpoint(filename, init_null(), init_null(), line)) {
    XDebugServer::detach();
  }
  return true;
}

static Variant HHVM_FUNCTION(xdebug_call_class) {
  // PHP5 xdebug returns false if the callee is top-level
  auto fp = get_call_fp();
  if (fp == nullptr) {
    return false;
  }

  // PHP5 xdebug returns "" for no class
  auto cls = fp->m_func->cls();
  if (!cls) {
    return staticEmptyString();
  }
  return String(const_cast<StringData*>(cls->name()));
}

static String HHVM_FUNCTION(xdebug_call_file) {
  // PHP5 xdebug returns the top-level file if the callee is top-level.
  auto fp = get_call_fp();
  const Func *func;
  if (fp == nullptr) {
    VMRegAnchor _;
    func = g_context->getPrevFunc(vmfp());
    assert(func);
  } else {
    func = fp->func();
  }
  return String(const_cast<StringData*>(func->filename()));
}

static int64_t HHVM_FUNCTION(xdebug_call_line) {
  // PHP5 xdebug returns 0 when it can't determine the line number.
  Offset pc;
  auto fp = get_call_fp(&pc);
  if (fp == nullptr) {
    return 0;
  }

  auto const unit = fp->m_func->unit();
  assert(unit);
  return unit->getLineNumber(pc);
}

// php5 xdebug main function string equivalent
const StaticString s_CALL_FN_MAIN("{main}");

static Variant HHVM_FUNCTION(xdebug_call_function) {
  // PHP5 xdebug returns false if the callee is top-level.
  auto fp = get_call_fp();
  if (fp == nullptr) {
    return false;
  }

  // PHP5 xdebug returns "{main}" for pseudo-main.
  if (fp->m_func->isPseudoMain()) {
    return s_CALL_FN_MAIN;
  }
  return String(const_cast<StringData*>(fp->m_func->name()));
}

static bool HHVM_FUNCTION(xdebug_code_coverage_started) {
  auto const ti = ThreadInfo::s_threadInfo.getNoCheck();
  return ti->m_reqInjectionData.getCoverage();
}

// TODO(#3704) This requires var_dump
/*
 * Requirements:
 *
 *   - xdebug_get_zval_value_fancy
 *   - xdebug_get_zval_value_ansi
 *   - xdebug_get_zval_value
 */
static TypedValue* HHVM_FN(xdebug_debug_zval)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

// TODO(#3704) Requires xdebug_debug_zval, just print to stdout
static TypedValue* HHVM_FN(xdebug_debug_zval_stdout)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_disable) {
  XDEBUG_GLOBAL(DefaultEnable) = false;
}

// TODO(#3704) This requires var_dump. Just dump the superglobals as specified
//             via ini
static void HHVM_FUNCTION(xdebug_dump_superglobals)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_enable) {
  XDEBUG_GLOBAL(DefaultEnable) = true;
}

static Array HHVM_FUNCTION(xdebug_get_code_coverage) {
  auto ti = ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_reqInjectionData.getCoverage()) {
    return ti->m_coverage->Report(false);
  }
  return Array::Create();
}

// TODO(#3704) see xdebug_start_error_collection()
static Array HHVM_FUNCTION(xdebug_get_collected_errors,
                           bool clean /* = false */)
  XDEBUG_NOTIMPLEMENTED

const StaticString s_closure_varname("0Closure");

static Array HHVM_FUNCTION(xdebug_get_declared_vars) {
  if (RuntimeOption::RepoAuthoritative) {
    raise_error("xdebug_get_declared_vars unsupported in RepoAuthoritative "
      "mode");
  }

  // Grab the callee function
  VMRegAnchor _; // Ensure consistent state for vmfp
  auto func = g_context->getPrevFunc(vmfp());
  if (!func) {
    return Array::Create();
  }

  // Add each named local to the returned array. Note that since this function
  // is supposed to return all _declared_ variables in scope, which includes
  // variables that have been unset.
  auto const numNames = func->numNamedLocals();
  PackedArrayInit vars(numNames);
  for (Id i = 0; i < numNames; ++i) {
    assert(func->lookupVarId(func->localVarName(i)) == i);
    String varname(const_cast<StringData*>(func->localVarName(i)));
    // Skip the internal closure "0Closure" variable
    if (!s_closure_varname.equal(varname)) {
      vars.append(varname);
    }
  }
  return vars.toArray();
}

static Array HHVM_FUNCTION(xdebug_get_function_stack) {
  // Need to reverse the backtrace to match php5 xdebug.
  auto bt = createBacktrace(BacktraceArgs()
                            .skipTop()
                            .withPseudoMain()
                            .withArgNames()
                            .withArgValues(*XDebugExtension::CollectParams));
  return ArrayUtil::Reverse(bt).toArray();
}

// TODO(#3704) In php5 xdebug this function works even in cli mode. If we choose
//             to support this, header() and setcookie() do not work without a
//             transport so we'd need to get around that. Beyond that, this is
//             identical to headers_list().
static Array HHVM_FUNCTION(xdebug_get_headers)
  XDEBUG_NOTIMPLEMENTED

Variant HHVM_FUNCTION(xdebug_get_profiler_filename) {
  if (!XDEBUG_GLOBAL(ProfilerAttached)) {
    return false;
  }

  auto profiler = xdebug_profiler();
  if (profiler->isProfiling()) {
    return profiler->getProfilingFilename();
  }
  return false;
}

const StaticString s_xdebug_get_stack_depth("xdebug_get_stack_depth");
static int64_t HHVM_FUNCTION(xdebug_get_stack_depth) {
  int64_t depth = XDebugUtils::stackDepth();
  if (auto ar = g_context->getStackFrame()) {
    // If the call to xdebug_get_stack_depth was NOT
    // done via CallBuiltin, then it will be included
    // in the depth count, and we need to manually substract it
    static auto get_stack_depth =
      Unit::lookupFunc(s_xdebug_get_stack_depth.get());
    if (ar->m_func == get_stack_depth) {
      --depth;
    }
  }
  return depth;
}

static Variant HHVM_FUNCTION(xdebug_get_tracefile_name) {
  if (XDEBUG_GLOBAL(ProfilerAttached)) {
    auto profiler = xdebug_profiler();
    if (profiler->isTracing()) {
      return profiler->getTracingFilename();
    }
  }
  return false;
}

static bool HHVM_FUNCTION(xdebug_is_enabled) {
  return XDEBUG_GLOBAL(DefaultEnable);
}

static int64_t HHVM_FUNCTION(xdebug_memory_usage) {
  // With jemalloc, the usage can go negative (see memory_get_usage)
  auto const usage = MM().getStats().usage;
  assert(use_jemalloc || usage >= 0);
  return std::max<int64_t>(usage, 0);
}

static int64_t HHVM_FUNCTION(xdebug_peak_memory_usage) {
  return MM().getStats().peakUsage;
}

// TODO(#3704) This requires var_dump, error handling, and stack trace printing
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
  auto ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.setCoverage(true);
  if (g_context->isNested()) {
    raise_notice("Calling xdebug_start_code_coverage from a nested VM instance "
                 "may cause unpredicable results");
  }
  throw VMSwitchModeBuiltin();
}

// TODO(#3704) This requires overriding the default behavior on
//             exceptions/errors. Unfortunately program_functions.cpp was not
//             at all written with this in mind. We need to be able to install
//             a handler (from any extension, generally) as we also need to be
//             able to print stack traces on errors/exceptions.
static void HHVM_FUNCTION(xdebug_start_error_collection)
  XDEBUG_NOTIMPLEMENTED

static Variant HHVM_FUNCTION(xdebug_start_trace,
                             const Variant& traceFileVar,
                             int64_t options /* = 0 */) {
  // Allowed to pass null.
  StringSlice trace_file(nullptr, 0);
  if (traceFileVar.isString()) {
    trace_file = traceFileVar.toString().slice();
  }

  // Initialize the profiler if it isn't already.
  if (!XDEBUG_GLOBAL(ProfilerAttached)) {
    attach_xdebug_profiler();
  }

  // php5 xdebug returns false when tracing already started.
  auto profiler = xdebug_profiler();
  if (profiler->isTracing()) {
    return false;
  }

  // Start tracing, then grab the current begin frame
  start_tracing(profiler, trace_file, options);
  profiler->beginFrame(nullptr);
  return HHVM_FN(xdebug_get_tracefile_name)();
}

static void HHVM_FUNCTION(xdebug_stop_code_coverage,
                          bool cleanup /* = true */) {
  auto ti = ThreadInfo::s_threadInfo.getNoCheck();
  ti->m_reqInjectionData.setCoverage(false);
  if (cleanup) {
    ti->m_coverage->Reset();
  }
}

// TODO(#3704) See xdebug_start_error_collection
static void HHVM_FUNCTION(xdebug_stop_error_collection)
  XDEBUG_NOTIMPLEMENTED

static Variant HHVM_FUNCTION(xdebug_stop_trace) {
  if (!XDEBUG_GLOBAL(ProfilerAttached)) {
    return false;
  }

  auto profiler = xdebug_profiler();
  if (!profiler->isTracing()) {
    return false;
  }

  // End with xdebug_stop_trace()
  profiler->endFrame(init_null().asTypedValue(), nullptr, false);
  auto filename = profiler->getTracingFilename();
  profiler->disableTracing();
  detach_xdebug_profiler_if_needed();
  return filename;
}

static double HHVM_FUNCTION(xdebug_time_index) {
  auto const micro = Timer::GetCurrentTimeMicros() - XDEBUG_GLOBAL(InitTime);
  return micro * 1.0e-6;
}

static void do_var_dump(const Variant& v) {
  auto const cli = XDEBUG_GLOBAL(CliColor);
  XDebugExporter exporter;
  exporter.max_depth = XDEBUG_GLOBAL(VarDisplayMaxDepth);
  exporter.max_children = XDEBUG_GLOBAL(VarDisplayMaxChildren);
  exporter.max_data = XDEBUG_GLOBAL(VarDisplayMaxData);
  exporter.page = 0;

  String str;

  auto const html_errors =
    ThreadInfo::s_threadInfo->m_reqInjectionData.hasHtmlErrors();
  if (html_errors) {
    str = xdebug_get_zval_value_fancy(v, exporter);
  } else if ((cli == 1 && is_output_tty()) || cli == 2) {
    str = xdebug_get_zval_value_ansi(v, exporter);
  } else {
    str = xdebug_get_zval_value_text(v, exporter);
  }

  g_context->write(str);
}

void HHVM_FUNCTION(
  xdebug_var_dump,
  const Variant& v,
  const Array& _argv /* = null_array */
) {
  if (!XDEBUG_GLOBAL(OverloadVarDump) || !XDEBUG_GLOBAL(DefaultEnable)) {
    HHVM_FN(var_dump)(v, _argv);
    return;
  }

  do_var_dump(v);
  auto const size = _argv.size();
  for (int64_t i = 0; i < size; ++i) {
    do_var_dump(_argv[i]);
  }
}

static void HHVM_FUNCTION(_xdebug_check_trigger_vars) {
  if (XDebugExtension::Enable &&
      XDebugProfiler::isAttachNeeded() &&
      !XDEBUG_GLOBAL(ProfilerAttached)) {
    attach_xdebug_profiler();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Module implementation

// XDebug constants
const StaticString
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
                                std::map<std::string, std::string>& envCfg) {
  // First try to load the ini setting
  folly::dynamic ini_val = folly::dynamic::object();
  if (IniSetting::Get(XDEBUG_INI(name), ini_val)) {
    T val;
    ini_on_update(ini_val, val);
    return val;
  }

  // Then try to load from the environment
  auto const env_iter = envCfg.find(name);
  if (env_iter != envCfg.end()) {
    T val;
    ini_on_update(env_iter->second, val);
    return val;
  }

  // Finally just use the default value
  return defVal;
}

// Environment variables the idekey is grabbed from
const StaticString
  s_DBGP_IDEKEY("DBGP_IDEKEY"),
  s_USER("USER"),
  s_USERNAME("USERNAME");

// Attempts to load the default idekey from environment variables
static void loadIdeKey(std::map<std::string, std::string>& envCfg) {
  auto const dbgp_idekey = g_context->getenv(s_DBGP_IDEKEY);
  if (!dbgp_idekey.empty()) {
    envCfg["idekey"] = dbgp_idekey.toCppString();
    return;
  }

  auto const user = g_context->getenv(s_USER);
  if (!user.empty()) {
    envCfg["idekey"] = user.toCppString();
    return;
  }

  auto const username = g_context->getenv(s_USERNAME);
  if (!username.empty()) {
    envCfg["idekey"] = username.toCppString();
  }
}

// Environment variable that can be used for certain settings
const StaticString s_XDEBUG_CONFIG("XDEBUG_CONFIG");

// Loads the "XDEBUG_CONFIG" environment variables.
static void loadEnvConfig(std::map<std::string, std::string>& envCfg) {
  auto const cfg_raw = g_context->getenv(s_XDEBUG_CONFIG);
  if (cfg_raw.empty()) {
    return;
  }

  // Parse the config variable. Format is "key=val" list separated by spaces
  // This parsing isn't very efficient, but this isn't performance sensitive and
  // it's similar to what php5 xdebug does.
  auto cfg = StringUtil::Explode(cfg_raw, " ").toArray();
  for (ArrayIter iter(cfg); iter; ++iter) {
    auto keyval = StringUtil::Explode(iter.second().toString(), "=").toArray();
    if (keyval.size() != 2) {
      continue;
    }

    auto key = keyval[0].toString().toCppString();
    auto val = keyval[1].toString().toCppString();
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

// Stores our HDF-specified values (only integral values for now) across
// requests.
static std::map<const char*, int> config_values;

void XDebugExtension::moduleLoad(const IniSetting::Map& ini, Hdf xdebug_hdf) {
  assert(config_values.empty());

  auto debugger = xdebug_hdf["Eval"]["Debugger"];

  // Get everything as bools.
  #define XDEBUG_OPT(T, name, sym, val) { \
    std::string key = "XDebug" #sym; \
    config_values[#sym] = Config::GetBool(ini, xdebug_hdf, \
                                       "Eval.Debugger." + key, val); \
  }
  XDEBUG_HDF_CFG
  #undef XDEBUG_OPT

  // But patch up overload_var_dump since it's actually an int.
  config_values["OverloadVarDump"] =
    Config::GetInt32(ini, xdebug_hdf, "Eval.Debugger.XDebugOverloadVarDump", 1);

  // XDebug is disabled by default.
  Config::Bind(Enable, ini, xdebug_hdf, "Eval.Debugger.XDebugEnable", false);
}

void XDebugExtension::moduleInit() {
  if (!Enable) {
    return;
  }

  // Stacktraces are always on when XDebug is enabled
  Logger::SetTheLogger(new ExtendedLogger());
  ExtendedLogger::EnabledByDefault = true;

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
  std::map<std::string, std::string> env_cfg;
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

  #define XDEBUG_OPT(T, name, sym, val) { \
    /* HDF values take priority over INI. */ \
    auto iter = config_values.find(#sym); \
    XDEBUG_GLOBAL(sym) = iter != config_values.end() \
      ? iter->second \
      : xdebug_init_opt<T>(name, val, env_cfg); \
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL, XDEBUG_INI(name), \
                     &XDEBUG_GLOBAL(sym)); \
  }
  XDEBUG_HDF_CFG
  #undef XDEBUG_OPT

  // xdebug.dump.*
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
                       XDEBUG_GLOBAL(sym) = val; \
                       if (XDEBUG_GLOBAL(ProfilerAttached)) { \
                         xdebug_profiler()->set##sym(val); \
                       } \
                       refresh_xdebug_profiler(); \
                       return true; \
                    }, []() { return XDEBUG_GLOBAL(sym); }));
  XDEBUG_PROF_CFG
  #undef XDEBUG_OPT

  // scream
  XDEBUG_GLOBAL(Scream) = RuntimeOption::NoSilencer;
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL, XDEBUG_INI("scream"),
                   IniSetting::SetAndGet<bool>([] (const bool& val) {
                      RuntimeOption::NoSilencer = val;
                      return true;
                    }, nullptr), &XDEBUG_GLOBAL(Scream));

  // force_error_reporting
  XDEBUG_GLOBAL(ForceErrorReporting) = RuntimeOption::ForceErrorReportingLevel;
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   XDEBUG_INI("force_error_reporting"),
                   IniSetting::SetAndGet<int>([] (const int& val) {
                      RuntimeOption::ForceErrorReportingLevel = val;
                      return true;
                    }, nullptr), &XDEBUG_GLOBAL(ForceErrorReporting));

  // halt_level
  XDEBUG_GLOBAL(HaltLevel) = RuntimeOption::ErrorUpgradeLevel;
  IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                   XDEBUG_INI("halt_level"),
                   IniSetting::SetAndGet<int>([] (const int& val) {
                      RuntimeOption::ErrorUpgradeLevel = val;
                      return true;
                    }, nullptr), &XDEBUG_GLOBAL(HaltLevel));

  // Custom request local globals
  #define XDEBUG_OPT(T, name, sym, val) XDEBUG_GLOBAL(sym) = val;
  XDEBUG_CUSTOM_GLOBALS
  #undef XDEBUG_OPT

  // Let the server do initialization
  XDebugServer::onRequestInit();

  // Potentially attach the xdebug profiler
  if (XDebugProfiler::isAttachNeeded()) {
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
XDEBUG_MAPPED_CFG
XDEBUG_HDF_CFG
XDEBUG_DUMP_CFG
XDEBUG_PROF_CFG
XDEBUG_CUSTOM_GLOBALS
#undef XDEBUG_OPT

static XDebugExtension s_xdebug_extension;

///////////////////////////////////////////////////////////////////////////////
}
