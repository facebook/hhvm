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

#ifndef incl_HPHP_EXT_XDEBUG_H_
#define incl_HPHP_EXT_XDEBUG_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/util/thread-local.h"

using std::string;
using std::map;

namespace HPHP {

struct XDebugServer;

///////////////////////////////////////////////////////////////////////////////

#define XDEBUG_NAME "xdebug-not-done"
#define XDEBUG_VERSION NO_EXTENSION_VERSION_YET
#define XDEBUG_AUTHOR "HHVM"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2013 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2013"
#define XDEBUG_URL "http://hhvm.com/"

// TODO(#4489053) Document these
// TODO(#4489053) Not all of these should be thread local
// Request Local ini config settings
// Differences b/w xdebug:
//  extended_info, coverage_enable:
//    unused because enabling/disabling these would have no effect on hhvm
//    since we can toggle tracking the required information at runtime
//  collect_vars:
//    Unused because we can always get the variables at runtime
//  collect_assignments:
//    Currently unimplemented as hhvm does not have infrastructure for this.
//  framebuf_size:
//    Added option specifying the initial number of frames the frame buffer will
//    hold when xdebug needs to turn frame tracing/profiling on. By default this
//    takes on 100,000, which is significantly smaller than the 2 million frames
//    provided by the internal trace profiler, which is good, since it is very
//    easy to turn on a feature requiring tracing/profiling support, and a huge
//    memory hit is not expected.
//  framebuf_expansion:
//    Added option specifying the amount to increase the framebuffer by each
//    time we have to resize. The previous size is multiplied by this number.
//    By default this takes on the value 1.5 which is slightly higher than
//    the internal trace profiler due to the decrease in initial buffer size.
//  remote_timeout:
//    Added option specifying the timeout to use for remote debugging, in
//    seconds. By default, .2 (200ms).
#define XDEBUG_CFG \
  XDEBUG_OPT(bool, "auto_trace", AutoTrace, false) \
  XDEBUG_OPT(int, "cli_color", CliColor, 0) \
  XDEBUG_OPT(bool, "collect_assignments", CollectAssignments, false) \
  XDEBUG_OPT(bool, "collect_includes", CollectIncludes, true) \
  XDEBUG_OPT(int, "collect_params", CollectParams, 0) \
  XDEBUG_OPT(bool, "collect_return", CollectReturn, false) \
  XDEBUG_OPT(bool, "collect_vars", CollectVars, false) \
  XDEBUG_OPT(bool, "default_enable", DefaultEnable, true) \
  XDEBUG_OPT(bool, "dump_globals", DumpGlobals, true) \
  XDEBUG_OPT(bool, "dump_once", DumpOnce, true) \
  XDEBUG_OPT(bool, "dump_undefined", DumpUndefined, false) \
  XDEBUG_OPT(string, "file_link_format", FileLinkFormat, "") \
  XDEBUG_OPT(bool, "force_display_errors", ForceDisplayErrors, false) \
  XDEBUG_OPT(int, "force_error_reporting", ForceErrorReporting, 0) \
  XDEBUG_OPT(uint64_t, "framebuf_size", FramebufSize, 100000) \
  XDEBUG_OPT(double, "framebuf_expansion", FramebufExpansion, 1.5) \
  XDEBUG_OPT(int, "halt_level", HaltLevel, 0) \
  XDEBUG_OPT(string, "ide_key", IdeKey, "") \
  XDEBUG_OPT(string, "manual_url", ManualUrl, "http://www.php.net") \
  XDEBUG_OPT(int, "max_nesting_level", MaxNestingLevel, 100) \
  XDEBUG_OPT(int, "overload_var_dump", OverloadVarDump, true) \
  XDEBUG_OPT(bool, "profiler_append", ProfilerAppend, false) \
  XDEBUG_OPT(bool, "profiler_enable", ProfilerEnable, false) \
  XDEBUG_OPT(bool, "profiler_enable_trigger", ProfilerEnableTrigger, false) \
  XDEBUG_OPT(string, "profiler_output_dir", ProfilerOutputDir, "/tmp") \
  XDEBUG_OPT(string, "profiler_output_name", ProfilerOutputName, \
             "cachegrind.out.%p") \
  XDEBUG_OPT(bool, "remote_autostart", RemoteAutostart, false) \
  XDEBUG_OPT(bool, "remote_connect_back", RemoteConnectBack, false) \
  XDEBUG_OPT(int, "remote_cookie_expire_time", RemoteCookieExpireTime, 3600) \
  XDEBUG_OPT(bool, "remote_enable", RemoteEnable, false) \
  XDEBUG_OPT(string, "remote_handler", RemoteHandler, "dbgp") \
  XDEBUG_OPT(string, "remote_host", RemoteHost, "localhost") \
  XDEBUG_OPT(string, "remote_log", RemoteLog, "") \
  XDEBUG_OPT(string, "remote_mode", RemoteMode, "req") \
  XDEBUG_OPT(int, "remote_port", RemotePort, 9000) \
  XDEBUG_OPT(double, "remote_timeout", RemoteTimeout, 0.2) \
  XDEBUG_OPT(bool, "scream", Scream, false) \
  XDEBUG_OPT(bool, "show_exception_trace", ShowExcptionTrace, false) \
  XDEBUG_OPT(bool, "show_local_vars", ShowLocalVars, false) \
  XDEBUG_OPT(bool, "show_mem_delta", ShowMemDelta, false) \
  XDEBUG_OPT(bool, "trace_enable_trigger", TraceEnableTrigger, false) \
  XDEBUG_OPT(int, "trace_format", TraceFormat, 0) \
  XDEBUG_OPT(bool, "trace_options", TraceOptions, false) \
  XDEBUG_OPT(string, "trace_output_dir", TraceOutputDir, "/tmp") \
  XDEBUG_OPT(string, "trace_output_name", TraceOutputName, "trace.%c") \
  XDEBUG_OPT(int, "var_display_max_children", VarDisplayMaxChildren, 128) \
  XDEBUG_OPT(int, "var_display_max_data", VarDisplayMaxData, 512) \
  XDEBUG_OPT(int, "var_display_max_depth", VarDisplayMaxDepth, 3)

// xdebug.dump.* settings
#define XDEBUG_DUMP_CFG \
  XDEBUG_OPT(string, "COOKIE", DumpCookie, "") \
  XDEBUG_OPT(string, "FILES", DumpFiles, "") \
  XDEBUG_OPT(string, "GET", DumpGet, "") \
  XDEBUG_OPT(string, "POST", DumpPost, "") \
  XDEBUG_OPT(string, "REQUEST", DumpRequest, "") \
  XDEBUG_OPT(string, "SERVER", DumpServer, "") \
  XDEBUG_OPT(string, "SESSION", DumpSession, "") \

// Options that notify the profiler on change
//  collect_memory, collect_time:
//    Added options specifying whether or not we should collect memory
//    information and function start times for stack traces. These require
//    profiling, which takes a lot of memory and slows things down, so these
//    are disabled by default. If off, 0 will be displayed.
#define XDEBUG_PROF_CFG \
  XDEBUG_OPT(bool, "collect_memory", CollectMemory, false) \
  XDEBUG_OPT(bool, "collect_time", CollectTime, false)

// These aren't settable via ini, but are request local globals
#define XDEBUG_CUSTOM_GLOBALS \
  XDEBUG_OPT(bool, nullptr, ProfilerAttached, false) \
  XDEBUG_OPT(int64_t, nullptr, InitTime, Timer::GetCurrentTimeMicros()) \
  XDEBUG_OPT(XDebugServer*, nullptr, Server, nullptr)

// Retrieves the value of the given xdebug global
#define XDEBUG_GLOBAL(name) (*XDebugExtension::name)

// Returns the ini name for the given hhvm configuration option.
// TODO(#4489053) This should not be hhvm. Need to change tests.
#define XDEBUG_INI(name) (("hhvm." XDEBUG_NAME ".") + string(name))

// TODO(#4489053) Remove when xdebug fully implemented
#define XDEBUG_NOTIMPLEMENTED  { throw_not_implemented(__FUNCTION__); }

///////////////////////////////////////////////////////////////////////////////
const int64_t k_XDEBUG_CC_UNUSED = 1;
const int64_t k_XDEBUG_CC_DEAD_CODE = 2;
const int64_t k_XDEBUG_TRACE_APPEND = 1;
const int64_t k_XDEBUG_TRACE_COMPUTERIZED = 2;
const int64_t k_XDEBUG_TRACE_HTML = 4;
const int64_t k_XDEBUG_TRACE_NAKED_FILENAME = 8;
const int64_t k_XDEBUG_PROFILE_APPEND = 1;
///////////////////////////////////////////////////////////////////////////////
Variant HHVM_FUNCTION(xdebug_get_profiler_filename);
///////////////////////////////////////////////////////////////////////////////

class XDebugExtension : public Extension {
public:
  XDebugExtension() : Extension(XDEBUG_NAME, XDEBUG_VERSION) { }

  virtual void moduleLoad(const IniSetting::Map& ini, Hdf xdebug_hdf);
  virtual void moduleInit();
  virtual void requestInit();
  virtual void requestShutdown();

  // Standard config options
  #define XDEBUG_OPT(T, name, sym, val) static DECLARE_THREAD_LOCAL(T, sym);
  XDEBUG_CFG
  XDEBUG_PROF_CFG
  XDEBUG_DUMP_CFG
  XDEBUG_CUSTOM_GLOBALS
  #undef XDEBUG_OPT

  // Config options that aren't bound or are other edge cases
  static bool Enable;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_XDEBUG_H_
