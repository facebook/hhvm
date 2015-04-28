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

#include <folly/Optional.h>

#include "hphp/runtime/ext/extension.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/timer.h"

namespace HPHP {

struct XDebugServer;

///////////////////////////////////////////////////////////////////////////////

#define XDEBUG_NAME "xdebug"
#define XDEBUG_VERSION NO_EXTENSION_VERSION_YET
#define XDEBUG_AUTHOR "HHVM"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2013 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2013"
#define XDEBUG_URL "http://hhvm.com/"

#define OUTPUT_NOT_CHECKED -1
#define OUTPUT_IS_TTY       1
#define OUTPUT_NOT_TTY      0

// TODO(#3704) Not all of these should be thread local and settable via ini_set
// Request Local ini config settings
// Different from php5 xdebug:
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
//
// Unimplemented:
//  extended_info, coverage_enable:
//    unused because enabling/disabling these would have no effect on hhvm
//    since we can toggle tracking the required information at runtime
//  collect_vars:
//    Unused because we can always get the variables at runtime
//  collect_assignments:
//    Currently unimplemented as hhvm does not have infrastructure for this.
//  manual_url:
//    This is not used in php5 xdebug, so it is not implemented
//  force_display_errors:
//    This is not relevant in hhvm as force_display_errors is always on, it is
//    either stdout or stderr. In php, it could be a boolean in old versions.
//  show_exception_trace:
//    TODO(#3704) Will be implemented with error handling
//  default_enable, dump_globals, file_link_format, show_local_vars:
//    TODO(#3704) Will be implemented with stack trace printing
//  cli_color, collect_includes, collect_params, collect_return,
//  overload_var_dump, var_display_max_children, var_display_max_data,
//  var_display_max_depth:
//    TODO(#3704) Will be implemented when var_dump is implemented
#define XDEBUG_CFG \
  XDEBUG_OPT(bool, "auto_trace", AutoTrace, false) \
  XDEBUG_OPT(int, "cli_color", CliColor, 0) \
  XDEBUG_OPT(bool, "collect_assignments", CollectAssignments, false) \
  XDEBUG_OPT(int, "collect_params", CollectParams, 0) \
  XDEBUG_OPT(bool, "collect_return", CollectReturn, false) \
  XDEBUG_OPT(bool, "collect_vars", CollectVars, false) \
  XDEBUG_OPT(bool, "dump_undefined", DumpUndefined, false) \
  XDEBUG_OPT(std::string, "file_link_format", FileLinkFormat, "") \
  XDEBUG_OPT(bool, "force_display_errors", ForceDisplayErrors, false) \
  XDEBUG_OPT(uint64_t, "framebuf_size", FramebufSize, 100000) \
  XDEBUG_OPT(double, "framebuf_expansion", FramebufExpansion, 1.5) \
  XDEBUG_OPT(std::string, "idekey", IdeKey, "") \
  XDEBUG_OPT(std::string, "manual_url", ManualUrl, "http://www.php.net") \
  XDEBUG_OPT(bool, "profiler_append", ProfilerAppend, false) \
  XDEBUG_OPT(bool, "profiler_enable", ProfilerEnable, false) \
  XDEBUG_OPT(bool, "profiler_enable_trigger", ProfilerEnableTrigger, false) \
  XDEBUG_OPT(std::string, "profiler_output_dir", ProfilerOutputDir, "/tmp") \
  XDEBUG_OPT(std::string, "profiler_output_name", ProfilerOutputName, \
             "cachegrind.out.%t") \
  XDEBUG_OPT(bool, "remote_connect_back", RemoteConnectBack, false) \
  XDEBUG_OPT(int, "remote_cookie_expire_time", RemoteCookieExpireTime, 3600) \
  XDEBUG_OPT(std::string, "remote_handler", RemoteHandler, "dbgp") \
  XDEBUG_OPT(std::string, "remote_host", RemoteHost, "localhost") \
  XDEBUG_OPT(std::string, "remote_log", RemoteLog, "") \
  XDEBUG_OPT(std::string, "remote_mode", RemoteMode, "req") \
  XDEBUG_OPT(int, "remote_port", RemotePort, 9000) \
  XDEBUG_OPT(double, "remote_timeout", RemoteTimeout, 0.2) \
  XDEBUG_OPT(bool, "show_exception_trace", ShowExcptionTrace, false) \
  XDEBUG_OPT(bool, "show_local_vars", ShowLocalVars, false) \
  XDEBUG_OPT(bool, "show_mem_delta", ShowMemDelta, false) \
  XDEBUG_OPT(bool, "trace_enable_trigger", TraceEnableTrigger, false) \
  XDEBUG_OPT(int, "trace_format", TraceFormat, 0) \
  XDEBUG_OPT(bool, "trace_options", TraceOptions, false) \
  XDEBUG_OPT(std::string, "trace_output_dir", TraceOutputDir, "/tmp") \
  XDEBUG_OPT(std::string, "trace_output_name", TraceOutputName, "trace.%c") \
  XDEBUG_OPT(int, "var_display_max_children", VarDisplayMaxChildren, 128) \
  XDEBUG_OPT(int, "var_display_max_data", VarDisplayMaxData, 512) \
  XDEBUG_OPT(int, "var_display_max_depth", VarDisplayMaxDepth, 3)

// Config options that have corresponding hhvm runtime options
#define XDEBUG_MAPPED_CFG \
  XDEBUG_OPT(bool, "scream", Scream, false) \
  XDEBUG_OPT(int, "force_error_reporting", ForceErrorReporting, 0) \
  XDEBUG_OPT(int, "halt_level", HaltLevel, 0) \

// Config options that can also be set from HDF configs.
#define XDEBUG_HDF_CFG \
  XDEBUG_OPT(bool, "collect_includes", CollectIncludes, true) \
  XDEBUG_OPT(bool, "default_enable", DefaultEnable, true) \
  XDEBUG_OPT(bool, "dump_globals", DumpGlobals, true) \
  XDEBUG_OPT(bool, "dump_once", DumpOnce, true) \
  XDEBUG_OPT(int, "overload_var_dump", OverloadVarDump, 1) \
  XDEBUG_OPT(bool, "remote_autostart", RemoteAutostart, false) \
  XDEBUG_OPT(bool, "remote_enable", RemoteEnable, false) \

// xdebug.dump.* settings
#define XDEBUG_DUMP_CFG \
  XDEBUG_OPT(std::string, "COOKIE", DumpCookie, "") \
  XDEBUG_OPT(std::string, "FILES", DumpFiles, "") \
  XDEBUG_OPT(std::string, "GET", DumpGet, "") \
  XDEBUG_OPT(std::string, "POST", DumpPost, "") \
  XDEBUG_OPT(std::string, "REQUEST", DumpRequest, "") \
  XDEBUG_OPT(std::string, "SERVER", DumpServer, "") \
  XDEBUG_OPT(std::string, "SESSION", DumpSession, "") \

// Options that notify the profiler on change
//  collect_memory, collect_time:
//    Added options specifying whether or not we should collect memory
//    information and function start times for stack traces. These require
//    profiling, which takes a lot of memory and slows things down, so these
//    are disabled by default. If off, 0 will be displayed.
//  max_nesting_level:
//    Same as in php5 xdebug, except that the default value has been changed to
//    0, from 100 in order to prevent the profiler from being automatically
//    enabled.
#define XDEBUG_PROF_CFG \
  XDEBUG_OPT(bool, "collect_memory", CollectMemory, false) \
  XDEBUG_OPT(bool, "collect_time", CollectTime, false) \
  XDEBUG_OPT(uint64_t, "max_nesting_level", MaxNestingLevel, 0)

// These aren't settable via ini, but are request local globals
#define XDEBUG_CUSTOM_GLOBALS \
  XDEBUG_OPT(bool, nullptr, ProfilerAttached, false) \
  XDEBUG_OPT(int64_t, nullptr, InitTime, Timer::GetCurrentTimeMicros()) \
  XDEBUG_OPT(XDebugServer*, nullptr, Server, nullptr) \
  XDEBUG_OPT(folly::Optional<bool>, nullptr, OutputIsTTY, folly::none)

// Retrieves the value of the given xdebug global
#define XDEBUG_GLOBAL(name) (*XDebugExtension::name)

// Returns the ini name for the given hhvm configuration option.
#define XDEBUG_INI(name) ((XDEBUG_NAME ".") + std::string(name))

///////////////////////////////////////////////////////////////////////////////
constexpr int64_t k_XDEBUG_CC_UNUSED = 1;
constexpr int64_t k_XDEBUG_CC_DEAD_CODE = 2;
constexpr int64_t k_XDEBUG_TRACE_APPEND = 1;
constexpr int64_t k_XDEBUG_TRACE_COMPUTERIZED = 2;
constexpr int64_t k_XDEBUG_TRACE_HTML = 4;
constexpr int64_t k_XDEBUG_TRACE_NAKED_FILENAME = 8;
constexpr int64_t k_XDEBUG_PROFILE_APPEND = 1;
///////////////////////////////////////////////////////////////////////////////
Variant HHVM_FUNCTION(xdebug_get_profiler_filename);
void HHVM_FUNCTION(xdebug_var_dump, const Variant&, const Array&);
///////////////////////////////////////////////////////////////////////////////

struct XDebugExtension final : Extension {
  XDebugExtension() : Extension(XDEBUG_NAME, XDEBUG_VERSION) { }

  void moduleLoad(const IniSetting::Map& ini, Hdf xdebug_hdf) override;
  void moduleInit() override;
  void requestInit() override;
  void requestShutdown() override;

  // Standard config options
  #define XDEBUG_OPT(T, name, sym, val) static DECLARE_THREAD_LOCAL(T, sym);
  XDEBUG_CFG
  XDEBUG_MAPPED_CFG
  XDEBUG_HDF_CFG
  XDEBUG_DUMP_CFG
  XDEBUG_PROF_CFG
  XDEBUG_CUSTOM_GLOBALS
  #undef XDEBUG_OPT

  // Indicates whether the xdebug extension has been enabled via xdebug.enable
  static bool Enable;
  bool moduleEnabled() const override { return Enable; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_XDEBUG_H_
