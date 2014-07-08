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

using std::string;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define XDEBUG_NAME "xdebug-not-done"
#define XDEBUG_VERSION NO_EXTENSION_VERSION_YET

// TODO(#4489053) Document these
// Differences b/w xdebug:
//  extended_info, coverage_enable:
//    unused because enabling/disabling these would have no effect on hhvm
//    since we can toggle tracking the required information at runtime
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
  XDEBUG_OPT(bool, "scream", Scream, false) \
  XDEBUG_OPT(bool, "show_exception_trace", ShowExcptionTrace, false) \
  XDEBUG_OPT(bool, "show_local_vars", ShowLocalVars, false) \
  XDEBUG_OPT(bool, "show_mem_delta", ShowMemDelta, false) \
  XDEBUG_OPT(bool, "trace_enable_trigger", TraceEnableTrigger, false) \
  XDEBUG_OPT(int, "trace_format", TraceFormat, 0) \
  XDEBUG_OPT(bool, "trace_options", TraceOptions, 0 ) \
  XDEBUG_OPT(string, "trace_output_dir", TraceOutputDir, "/tmp") \
  XDEBUG_OPT(string, "trace_output_name", TraceOutputName, "trace.%c") \
  XDEBUG_OPT(int, "var_display_max_children", VarDisplayMaxChildren, 128) \
  XDEBUG_OPT(int, "var_display_max_data", VarDisplayMaxData, 512) \
  XDEBUG_OPT(int, "var_display_max_depth", VarDisplayMaxDepth, 3)

// TODO(#4489053) Remove when xdebug fully implemented
#define XDEBUG_NOTIMPLEMENTED  { throw_not_implemented(__FUNCTION__); }

///////////////////////////////////////////////////////////////////////////////
const int64_t k_XDEBUG_CC_UNUSED = 1;
const int64_t k_XDEBUG_CC_DEAD_CODE = 2;
///////////////////////////////////////////////////////////////////////////////

class XDebugExtension : public Extension {
public:
  XDebugExtension() : Extension(XDEBUG_NAME, XDEBUG_VERSION) { }

  // Standard config options
  #define XDEBUG_OPT(T, name, sym, val) static T sym;
  XDEBUG_CFG
  #undef XDEBUG_OPT

  // Config options that aren't bound or are other edge cases
  static bool Enable;
  static string DumpCookie;
  static string DumpFiles;
  static string DumpGet;
  static string DumpPost;
  static string DumpRequest;
  static string DumpServer;
  static string DumpSession;

  virtual void moduleLoad(const IniSetting::Map& ini, Hdf xdebug_hdf);
  virtual void moduleInit();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_XDEBUG_H_
