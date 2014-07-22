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
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/timer.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static const StaticString s_CALL_FN_MAIN("{main}");

struct XdebugRequestData final : RequestEventHandler {
  void requestInit() override {
    m_init_time = Timer::GetCurrentTimeMicros();
  }

  void requestShutdown() override {
    m_init_time = 0;
  }

  int64_t m_init_time;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(XdebugRequestData, s_request);

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns the frame of the callee's callee. Useful for the xdebug_call_*
 * functions. Only returns nullptr if the callee is the top level pseudo-main
 *
 * If an offset pointer is passed, we store in it it the pc offset of the
 * call to the callee.
 */
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

///////////////////////////////////////////////////////////////////////////////

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

static void HHVM_FUNCTION(xdebug_start_trace,
                          const String& trace_file,
                          int64_t options /* = 0 */)
  XDEBUG_NOTIMPLEMENTED

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

static void HHVM_FUNCTION(xdebug_stop_trace)
  XDEBUG_NOTIMPLEMENTED

static double HHVM_FUNCTION(xdebug_time_index) {
  int64_t micro = Timer::GetCurrentTimeMicros() - s_request->m_init_time;
  return micro * 1.0e-6;
}

static TypedValue* HHVM_FN(xdebug_var_dump)(ActRec* ar)
  XDEBUG_NOTIMPLEMENTED

///////////////////////////////////////////////////////////////////////////////

static const StaticString s_XDEBUG_CC_UNUSED("XDEBUG_CC_UNUSED");
static const StaticString s_XDEBUG_CC_DEAD_CODE("XDEBUG_CC_DEAD_CODE");

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
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_CC_UNUSED.get(), k_XDEBUG_CC_UNUSED
  );
  Native::registerConstant<KindOfInt64>(
    s_XDEBUG_CC_DEAD_CODE.get(), k_XDEBUG_CC_DEAD_CODE
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

void XDebugExtension::requestInit() {
  if (Enable) {
    s_request->requestInit();
  }
}

void XDebugExtension::requestShutdown() {
  if (Enable) {
    s_request->requestShutdown();
  }
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
