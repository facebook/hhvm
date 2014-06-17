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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static bool HHVM_FUNCTION(xdebug_break)
  XDEBUG_NOTIMPLEMENTED

static String HHVM_FUNCTION(xdebug_call_class)
  XDEBUG_NOTIMPLEMENTED

static String HHVM_FUNCTION(xdebug_call_file)
  XDEBUG_NOTIMPLEMENTED

static int64_t HHVM_FUNCTION(xdebug_call_line)
  XDEBUG_NOTIMPLEMENTED

static bool HHVM_FUNCTION(xdebug_code_coverage_started)
  XDEBUG_NOTIMPLEMENTED

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

static Array HHVM_FUNCTION(xdebug_get_code_coverage)
  XDEBUG_NOTIMPLEMENTED

static Array HHVM_FUNCTION(xdebug_get_collected_errors,
                           bool clean /* = false */)
  XDEBUG_NOTIMPLEMENTED

static Array HHVM_FUNCTION(xdebug_get_declared_vars)
  XDEBUG_NOTIMPLEMENTED

static Array HHVM_FUNCTION(xdebug_get_function_stack)
  XDEBUG_NOTIMPLEMENTED

static Array HHVM_FUNCTION(xdebug_get_headers)
  XDEBUG_NOTIMPLEMENTED

static String HHVM_FUNCTION(xdebug_get_profiler_filename)
  XDEBUG_NOTIMPLEMENTED

static int64_t HHVM_FUNCTION(xdebug_get_stack_depth)
  XDEBUG_NOTIMPLEMENTED

static String HHVM_FUNCTION(xdebug_get_tracefile_name)
  XDEBUG_NOTIMPLEMENTED

static bool HHVM_FUNCTION(xdebug_is_enabled)
  XDEBUG_NOTIMPLEMENTED

static int64_t HHVM_FUNCTION(xdebug_memory_usage)
  XDEBUG_NOTIMPLEMENTED

static int64_t HHVM_FUNCTION(xdebug_peak_memory_usage)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_print_function_stack,
                          const String& message /* = "user triggered" */,
                          int64_t options /* = 0 */)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_start_code_coverage, int64_t options /* = 0 */)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_start_error_collection)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_start_trace,
                          const String& trace_file,
                          int64_t options /* = 0 */)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_stop_code_coverage, bool cleanup /* = true */)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_stop_error_collection)
  XDEBUG_NOTIMPLEMENTED

static void HHVM_FUNCTION(xdebug_stop_trace)
  XDEBUG_NOTIMPLEMENTED

static double HHVM_FUNCTION(xdebug_time_index)
  XDEBUG_NOTIMPLEMENTED

static TypedValue* HHVM_FN(xdebug_var_dump)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

///////////////////////////////////////////////////////////////////////////////

void XDebugExtension::moduleLoad(const IniSetting::Map& ini, Hdf xdebug_hdf) {
  Hdf hdf = xdebug_hdf[XDEBUG_NAME];
  Enable = Config::GetBool(ini, hdf["enable"], false);
  if (!Enable) {
    return;
  }

  // Standard config options
  #define XDEBUG_OPT(T, name, sym, val) Config::Bind(sym, ini, hdf[name], val);
  XDEBUG_CFG
  #undef XDEBUG_OPT

  // hhvm.xdebug.dump.*
  Hdf dump = hdf["dump"];
  Config::Bind(DumpCookie, ini, dump["COOKIE"], "");
  Config::Bind(DumpFiles, ini, dump["FILES"], "");
  Config::Bind(DumpGet, ini, dump["GET"], "");
  Config::Bind(DumpPost, ini, dump["POST"], "");
  Config::Bind(DumpRequest, ini, dump["REQUEST"], "");
  Config::Bind(DumpServer, ini, dump["SERVER"], "");
  Config::Bind(DumpSession, ini, dump["SESSION"], "");
}

void XDebugExtension::moduleInit() {
  if (!Enable) {
    return;
  }

  HHVM_FE(xdebug_break);
  HHVM_FE(xdebug_call_class);
  HHVM_FE(xdebug_call_file);
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
  HHVM_FE(xdebug_get_function_stack);
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
  loadSystemlib("xdebug");
}

// Non-bind config options and edge-cases
bool XDebugExtension::Enable = false;
string XDebugExtension::DumpCookie = "";
string XDebugExtension::DumpFiles = "";
string XDebugExtension::DumpGet = "";
string XDebugExtension::DumpPost = "";
string XDebugExtension::DumpRequest = "";
string XDebugExtension::DumpServer = "";
string XDebugExtension::DumpSession = "";

// Standard config options
#define XDEBUG_OPT(T, name, sym, val) T XDebugExtension::sym = val;
XDEBUG_CFG
#undef XDEBUG_OPT

static XDebugExtension s_xdebug_extension;

///////////////////////////////////////////////////////////////////////////////
}
