/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/program-functions.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/hhprof.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/member-reflection.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/perf-mem-event.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/thread-safe-setlocale.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/base/zend-strtod.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/debugger_hook_handler.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/xhprof/ext_xhprof.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/xdebug/status.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"
#include "hphp/runtime/server/admin-request-handler.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/log-writer.h"
#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/replay-transport.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/arch.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/build-info.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/capability.h"
#include "hphp/util/embedded-data.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/hphp-config.h"
#include "hphp/util/kernel-version.h"
#ifndef _MSC_VER
#include "hphp/util/light-process.h"
#endif
#include "hphp/util/perf-event.h"
#include "hphp/util/process-exec.h"
#include "hphp/util/process.h"
#include "hphp/util/service-data.h"
#include "hphp/util/shm-counter.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/timer.h"
#include "hphp/util/type-scan.h"

#include "hphp/zend/zend-string.h"

#include <folly/Random.h>
#include <folly/Range.h>
#include <folly/Portability.h>
#include <folly/Singleton.h>
#include <folly/portability/Fcntl.h>
#include <folly/portability/Libgen.h>
#include <folly/portability/Stdlib.h>
#include <folly/portability/Unistd.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>

#include <oniguruma.h>
#include <signal.h>
#include <libxml/parser.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <windows.h>
#include <winuser.h>
#endif

using namespace boost::program_options;
using std::cout;

constexpr auto MAX_INPUT_NESTING_LEVEL = 64;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Forward declarations.

void initialize_repo();

/*
 * XXX: VM process initialization is handled through a function
 * pointer so libhphp_runtime.a can be linked into programs that don't
 * actually initialize the VM.
 */
void (*g_vmProcessInit)();

void timezone_init();

void pcre_init();
void pcre_reinit();

///////////////////////////////////////////////////////////////////////////////
// helpers

struct ProgramOptions {
  std::string mode;
  std::vector<std::string> config;
  std::vector<std::string> confStrings;
  std::vector<std::string> iniStrings;
  int port;
  int portfd;
  int sslportfd;
  int admin_port;
  std::string user;
  std::string file;
  std::string lint;
  bool isTempFile;
  int count;
  bool noSafeAccessCheck;
  std::vector<std::string> args;
  std::string buildId;
  std::string instanceId;
  int xhprofFlags;
  std::string show;
  std::string parse;

  Eval::DebuggerClientOptions debugger_options;
};

struct StartTime {
  StartTime() : startTime(time(nullptr)) {}
  time_t startTime;
};

static StartTime s_startTime;
static std::string tempFile;
std::vector<std::string> s_config_files;
std::vector<std::string> s_ini_strings;

time_t start_time() {
  return s_startTime.startTime;
}

const StaticString
  s_HPHP("HPHP"),
  s_HHVM("HHVM"),
  s_HHVM_JIT("HHVM_JIT"),
  s_HHVM_ARCH("HHVM_ARCH"),
  s_REQUEST_START_TIME("REQUEST_START_TIME"),
  s_REQUEST_TIME("REQUEST_TIME"),
  s_REQUEST_TIME_FLOAT("REQUEST_TIME_FLOAT"),
  s_DOCUMENT_ROOT("DOCUMENT_ROOT"),
  s_SCRIPT_FILENAME("SCRIPT_FILENAME"),
  s_SCRIPT_NAME("SCRIPT_NAME"),
  s_PHP_SELF("PHP_SELF"),
  s_argc("argc"),
  s_argv("argv"),
  s_PWD("PWD"),
  s_HOSTNAME("HOSTNAME"),
  s__SERVER("_SERVER"),
  s__ENV("_ENV");

static __thread bool s_sessionInitialized{false};

static void process_cmd_arguments(int argc, char **argv) {
  php_global_set(s_argc, Variant(argc));
  Array argvArray(staticEmptyArray());
  for (int i = 0; i < argc; i++) {
    argvArray.append(String(argv[i]));
  }
  php_global_set(s_argv, argvArray);
}

static void process_env_variables(Array& variables, char** envp,
                           std::map<std::string, std::string>& envVariables) {
  for (auto& kv : envVariables) {
    variables.set(String(kv.first), String(kv.second));
  }
  for (char **env = envp; env && *env; env++) {
    char *p = strchr(*env, '=');
    if (p) {
      String name(*env, p - *env, CopyString);
      register_variable(variables, (char*)name.data(),
                        String(p + 1, CopyString));
    }
  }
}

void process_env_variables(Array& variables) {
  process_env_variables(variables, environ, RuntimeOption::EnvVariables);
}

// Handle adding a variable to an array, supporting keys that look
// like array expressions (like 'FOO[][key1][k2]').
void register_variable(Array& variables, char *name, const Variant& value,
                       bool overwrite /* = true */) {
  // ignore leading spaces in the variable name
  char *var = name;
  while (*var && *var == ' ') {
    var++;
  }

  // ensure that we don't have spaces or dots in the variable name
  // (not binary safe)
  bool is_array = false;
  char *ip = nullptr; // index pointer
  char *p = var;
  for (; *p; p++) {
    if (*p == ' ' || *p == '.') {
      *p = '_';
    } else if (*p == '[') {
      is_array = true;
      ip = p;
      *p = 0;
      break;
    }
  }
  int var_len = p - var;
  if (var_len == 0) {
    // empty variable name, or variable name with a space in it
    return;
  }

  // GPC elements holds Variants that are acting as smart pointers to
  // RefDatas that we've created in the process of a multi-dim key.
  std::vector<Variant> gpc_elements;
  if (is_array) gpc_elements.reserve(MAX_INPUT_NESTING_LEVEL);

  // The array pointer we're currently adding to.  If we're doing a
  // multi-dimensional set, this will point at the m_data.parr inside
  // of a RefData sometimes (via toArrRef on the variants in
  // gpc_elements).
  Array* symtable = &variables;

  char* index = var;
  int index_len = var_len;

  if (is_array) {
    int nest_level = 0;
    while (true) {
      if (++nest_level > MAX_INPUT_NESTING_LEVEL) {
        Logger::Warning("Input variable nesting level exceeded");
        return;
      }

      ip++;
      char *index_s = ip;
      int new_idx_len = 0;
      if (isspace(*ip)) {
        ip++;
      }
      if (*ip == ']') {
        index_s = nullptr;
      } else {
        ip = strchr(ip, ']');
        if (!ip) {
          // PHP variables cannot contain '[' in their names,
          // so we replace the character with a '_'
          *(index_s - 1) = '_';

          index_len = 0;
          if (index) {
            index_len = strlen(index);
          }
          goto plain_var;
        }
        *ip = 0;
        new_idx_len = strlen(index_s);
      }

      if (!index) {
        auto& val = symtable->lvalAt();
        val = Array::Create();
        gpc_elements.push_back(uninit_null());
        gpc_elements.back().assignRef(val);
      } else {
        String key(index, index_len, CopyString);
        auto const& v = symtable->rvalAt(key);
        if (v.isNull() || !v.isArray()) {
          symtable->set(key, Array::Create());
        }
        gpc_elements.push_back(uninit_null());
        gpc_elements.back().assignRef(symtable->lvalAt(key));
      }
      symtable = &gpc_elements.back().toArrRef();
      /* ip pointed to the '[' character, now obtain the key */
      index = index_s;
      index_len = new_idx_len;

      ip++;
      if (*ip == '[') {
        is_array = true;
        *ip = 0;
      } else {
        goto plain_var;
      }
    }
  } else {
  plain_var:
    if (!index) {
      symtable->append(value);
    } else {
      String key(index, index_len, CopyString);
      if (overwrite || !symtable->exists(key)) {
        symtable->set(key, value);
      }
    }
  }
}

enum class ContextOfException {
  ReqInit = 1,
  Invoke,
  Handler,
};

static void handle_exception_append_bt(std::string& errorMsg,
                                       const ExtendedException& e) {
  Array bt = e.getBacktrace();
  if (!bt.empty()) {
    errorMsg += ExtendedLogger::StringOfStackTrace(bt);
  }
}

void bump_counter_and_rethrow(bool isPsp) {
  try {
    throw;
  } catch (const RequestTimeoutException& e) {
    if (isPsp) {
      static auto requestTimeoutPSPCounter = ServiceData::createTimeSeries(
        "requests_timed_out_psp", {ServiceData::StatsType::COUNT});
      requestTimeoutPSPCounter->addValue(1);
      ServerStats::Log("request.timed_out.psp", 1);
    } else {
      static auto requestTimeoutCounter = ServiceData::createTimeSeries(
        "requests_timed_out_non_psp", {ServiceData::StatsType::COUNT});
      requestTimeoutCounter->addValue(1);
      ServerStats::Log("request.timed_out.non_psp", 1);
    }
    throw;
  } catch (const RequestCPUTimeoutException& e) {
    if (isPsp) {
      static auto requestCPUTimeoutPSPCounter = ServiceData::createTimeSeries(
        "requests_cpu_timed_out_psp", {ServiceData::StatsType::COUNT});
      requestCPUTimeoutPSPCounter->addValue(1);
      ServerStats::Log("request.cpu_timed_out.psp", 1);
    } else {
      static auto requestCPUTimeoutCounter = ServiceData::createTimeSeries(
        "requests_cpu_timed_out_non_psp", {ServiceData::StatsType::COUNT});
      requestCPUTimeoutCounter->addValue(1);
      ServerStats::Log("request.cpu_timed_out.non_psp", 1);
    }
    throw;
  } catch (const RequestMemoryExceededException& e) {
    if (isPsp) {
      static auto requestMemoryExceededPSPCounter =
        ServiceData::createTimeSeries(
          "requests_memory_exceeded_psp", {ServiceData::StatsType::COUNT});
      requestMemoryExceededPSPCounter->addValue(1);
      ServerStats::Log("request.memory_exceeded.psp", 1);
    } else {
      static auto requestMemoryExceededCounter = ServiceData::createTimeSeries(
        "requests_memory_exceeded_non_psp", {ServiceData::StatsType::COUNT});
      requestMemoryExceededCounter->addValue(1);
      ServerStats::Log("request.memory_exceeded.non_psp", 1);
    }

#ifdef USE_JEMALLOC
    // Capture a pprof (C++) dump when we OOM a request
    // TODO: (t3753133) Should dump a PHP-instrumented pprof dump here as well
    jemalloc_pprof_dump("", false);
#endif

    throw;
  }
}

static void handle_exception_helper(bool& ret,
                                    ExecutionContext* context,
                                    std::string& errorMsg,
                                    ContextOfException where,
                                    bool& error,
                                    bool richErrorMsg) {
  // Clear oom/timeout while handling exception and restore them afterwards.
  auto& flags = stackLimitAndSurprise();
  auto const origFlags = flags.fetch_and(~ResourceFlags) & ResourceFlags;

  SCOPE_EXIT {
    flags.fetch_or(origFlags);
  };

  try {
    bump_counter_and_rethrow(false /* isPsp */);
  } catch (const Eval::DebuggerException &e) {
    throw;
  } catch (const ExitException &e) {
    if (where == ContextOfException::ReqInit) {
      ret = false;
    } else if (where != ContextOfException::Handler &&
        !context->getExitCallback().isNull() &&
        is_callable(context->getExitCallback())) {
      Array stack = e.getBacktrace();
      Array argv = make_packed_array(ExitException::ExitCode.load(), stack);
      vm_call_user_func(context->getExitCallback(), argv);
    }
  } catch (const XDebugExitExn&) {
    // Do nothing, this is normal behavior.
  } catch (const PhpFileDoesNotExistException &e) {
    ret = false;
    if (where != ContextOfException::Handler) {
      raise_notice("%s", e.getMessage().c_str());
    } else {
      Logger::Error("%s", e.getMessage().c_str());
    }
    if (richErrorMsg) {
      handle_exception_append_bt(errorMsg, e);
    }
  } catch (const Exception &e) {
    bool oldRet = ret;
    bool origError = error;
    std::string origErrorMsg = errorMsg;
    ret = false;
    error = true;
    errorMsg = "";
    if (where == ContextOfException::Handler) {
      errorMsg = "Exception handler threw an exception: ";
    }
    errorMsg += e.what();
    if (where == ContextOfException::Invoke) {
      bool handlerRet = context->onFatalError(e);
      if (handlerRet) {
        ret = oldRet;
        error = origError;
        errorMsg = origErrorMsg;
      }
    } else {
      Logger::Error("%s", errorMsg.c_str());
    }
    if (richErrorMsg) {
      const ExtendedException *ee = dynamic_cast<const ExtendedException *>(&e);
      if (ee) {
        handle_exception_append_bt(errorMsg, *ee);
      }
    }
  } catch (const Object &e) {
    bool oldRet = ret;
    bool origError = error;
    auto const origErrorMsg = errorMsg;
    ret = false;
    error = true;
    errorMsg = "";
    if (where == ContextOfException::Handler) {
      errorMsg = "Exception handler threw an object exception: ";
    }
    try {
      errorMsg += e.toString().data();
    } catch (...) {
      errorMsg += "(unable to call toString())";
    }
    if (where == ContextOfException::Invoke) {
      bool handlerRet = context->onUnhandledException(e);
      if (handlerRet) {
        ret = oldRet;
        error = origError;
        errorMsg = origErrorMsg;
      }
    } else {
      Logger::Error("%s", errorMsg.c_str());
    }
  } catch (...) {
    ret = false;
    error = true;
    errorMsg = "(unknown exception was thrown)";
    Logger::Error("%s", errorMsg.c_str());
  }
}

static bool hphp_chdir_file(const std::string& filename) {
  bool ret = false;
  String s = File::TranslatePath(filename);
  char *buf = strndup(s.data(), s.size());
  char *dir = dirname(buf);
  assert(dir);
  if (dir) {
    if (File::IsVirtualDirectory(dir)) {
      g_context->setCwd(String(dir, CopyString));
      ret = true;
    } else {
      struct stat sb;
      stat(dir, &sb);
      if ((sb.st_mode & S_IFMT) == S_IFDIR) {
        ret = true;
        if (*dir != '.') {
          g_context->setCwd(String(dir, CopyString));
        }
      }
    }
  }
  free(buf);
  return ret;
}

static void handle_resource_exceeded_exception() {
  try {
    throw;
  } catch (RequestTimeoutException&) {
    setSurpriseFlag(TimedOutFlag);
  } catch (RequestCPUTimeoutException&) {
    setSurpriseFlag(CPUTimedOutFlag);
  } catch (RequestMemoryExceededException&) {
    setSurpriseFlag(MemExceededFlag);
  } catch (...) {}
}

void handle_destructor_exception(const char* situation) {
  std::string errorMsg;

  try {
    throw;
  } catch (ExitException &e) {
    // ExitException is fine, no need to show a warning.
    TI().setPendingException(e.clone());
    return;
  } catch (Object &e) {
    // For user exceptions, invoke the user exception handler
    errorMsg = situation;
    errorMsg += " threw an object exception: ";
    try {
      errorMsg += e.toString().data();
    } catch (...) {
      handle_resource_exceeded_exception();
      errorMsg += "(unable to call toString())";
    }
  } catch (Exception &e) {
    TI().setPendingException(e.clone());
    errorMsg = situation;
    errorMsg += " raised a fatal error: ";
    errorMsg += e.what();
  } catch (...) {
    errorMsg = situation;
    errorMsg += " threw an unknown exception";
  }
  // For fatal errors and unknown exceptions, we raise a warning.
  // If there is a user error handler it will be invoked, otherwise
  // the default error handler will be invoked.
  try {
    raise_warning_unsampled("%s", errorMsg.c_str());
  } catch (...) {
    handle_resource_exceeded_exception();

    // The user error handler fataled or threw an exception,
    // print out the error message directly to the log
    Logger::Warning("%s", errorMsg.c_str());
  }
}

void init_command_line_session(int argc, char** argv) {
  StackTraceNoHeap::AddExtraLogging("ThreadType", "CLI");
  std::string args;
  for (int i = 0; i < argc; i++) {
    if (i) args += " ";
    args += argv[i];
  }
  StackTraceNoHeap::AddExtraLogging("Arguments", args.c_str());

  hphp_session_init();
  auto const context = g_context.getNoCheck();
  context->obSetImplicitFlush(true);
}

void
init_command_line_globals(int argc, char** argv, char** envp,
                          int xhprof,
                          std::map<std::string, std::string>& serverVariables,
                          std::map<std::string, std::string>& envVariables) {
  auto& variablesOrder = RID().getVariablesOrder();

  if (variablesOrder.find('e') != std::string::npos ||
      variablesOrder.find('E') != std::string::npos) {
    Array envArr(Array::Create());
    process_env_variables(envArr, envp, envVariables);
    envArr.set(s_HPHP, 1);
    envArr.set(s_HHVM, 1);
    if (RuntimeOption::EvalJit) {
      envArr.set(s_HHVM_JIT, 1);
    }
    switch (arch()) {
    case Arch::X64:
      envArr.set(s_HHVM_ARCH, "x64");
      break;
    case Arch::ARM:
      envArr.set(s_HHVM_ARCH, "arm");
      break;
    case Arch::PPC64:
      envArr.set(s_HHVM_ARCH, "ppc64");
      break;
    }
    php_global_set(s__ENV, envArr);
  }

  process_cmd_arguments(argc, argv);

  if (variablesOrder.find('s') != std::string::npos ||
      variablesOrder.find('S') != std::string::npos) {
    Array serverArr(Array::Create());
    process_env_variables(serverArr, envp, envVariables);
    time_t now;
    struct timeval tp = {0};
    double now_double;
    if (!gettimeofday(&tp, nullptr)) {
      now_double = (double)(tp.tv_sec + tp.tv_usec / 1000000.00);
      now = tp.tv_sec;
    } else {
      now = time(nullptr);
      now_double = (double)now;
    }
    String file = empty_string();
    if (argc > 0) {
      file = String::attach(StringData::Make(argv[0], CopyString));
    }
    serverArr.set(s_REQUEST_START_TIME, now);
    serverArr.set(s_REQUEST_TIME, now);
    serverArr.set(s_REQUEST_TIME_FLOAT, now_double);
    serverArr.set(s_DOCUMENT_ROOT, empty_string_variant_ref);
    serverArr.set(s_SCRIPT_FILENAME, file);
    serverArr.set(s_SCRIPT_NAME, file);
    serverArr.set(s_PHP_SELF, file);
    serverArr.set(s_argv, php_global(s_argv));
    serverArr.set(s_argc, php_global(s_argc));
    serverArr.set(s_PWD, g_context->getCwd());
    char hostname[1024];
    if (RuntimeOption::ServerExecutionMode() &&
        !is_cli_mode() &&
        !gethostname(hostname, sizeof(hostname))) {
      // gethostname may not null-terminate
      hostname[sizeof(hostname) - 1] = '\0';
      serverArr.set(s_HOSTNAME, String(hostname, CopyString));
    }

    for (auto& kv : serverVariables) {
      serverArr.set(String(kv.first.c_str()), String(kv.second.c_str()));
    }

    php_global_set(s__SERVER, serverArr);
  }

  if (xhprof) {
    HHVM_FN(xhprof_enable)(xhprof, uninit_null().toArray());
  }

  if (RuntimeOption::RequestTimeoutSeconds) {
    RID().setTimeout(RuntimeOption::RequestTimeoutSeconds);
  }

  if (RuntimeOption::XenonForceAlwaysOn) {
    Xenon::getInstance().surpriseAll();
  }

  InitFiniNode::GlobalsInit();
  // Initialize the debugger
  DEBUGGER_ATTACHED_ONLY(phpDebuggerRequestInitHook());
}

void execute_command_line_begin(int argc, char **argv, int xhprof) {
  init_command_line_session(argc, argv);
  init_command_line_globals(argc, argv, environ, xhprof,
                            RuntimeOption::ServerVariables,
                            RuntimeOption::EnvVariables);
}

void execute_command_line_end(int xhprof, bool coverage, const char *program) {
  if (RuntimeOption::EvalDumpTC ||
      RuntimeOption::EvalDumpIR ||
      RuntimeOption::EvalDumpRegion) {
    jit::mcgen::joinWorkerThreads();
    jit::tc::dump();
  }
  if (xhprof) {
    Variant profileData = HHVM_FN(xhprof_disable)();
    if (!profileData.isNull()) {
      HHVM_FN(var_dump)(Variant::attach(
        HHVM_FN(json_encode)(HHVM_FN(xhprof_disable)())
      ));
    }
  }
  g_context->onShutdownPostSend(); // runs more php
  Eval::Debugger::InterruptPSPEnded(program);
  hphp_context_exit();
  hphp_session_exit();
  auto& ti = TI();
  if (coverage && ti.m_reqInjectionData.getCoverage() &&
      !RuntimeOption::CodeCoverageOutputFile.empty()) {
    ti.m_coverage->Report(RuntimeOption::CodeCoverageOutputFile);
  }
}

#if defined(__APPLE__) || defined(_MSC_VER)
const void* __hot_start = nullptr;
const void* __hot_end = nullptr;
#define AT_END_OF_TEXT
#else
#define AT_END_OF_TEXT    __attribute__((__section__(".stub")))
#endif

#define ALIGN_HUGE_PAGE   __attribute__((__aligned__(2 * 1024 * 1024)))

static void
NEVER_INLINE AT_END_OF_TEXT ALIGN_HUGE_PAGE __attribute__((__optimize__("2")))
hugifyText(char* from, char* to) {
#if !defined FOLLY_SANITIZE_ADDRESS && defined MADV_HUGEPAGE
  if (from > to || (to - from) < sizeof(uint64_t)) {
    // This shouldn't happen if HHVM is behaving correctly (I think),
    // but if it does then there is nothing to do and we should bail
    // out early because the call to wordcpy() below can't handle
    // zero size or negative sizes.
    return;
  }
  size_t sz = to - from;
  void* mem = malloc(sz);
  memcpy(mem, from, sz);

  // This maps out a portion of our executable
  // We need to be very careful about what we do
  // until we replace the original code
  mmap(from, sz,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
       -1, 0);
  // This is in glibc, which isn't a problem, except for
  // the trampoline code in .plt, which we dealt with
  // in the linker script
  madvise(from, sz, MADV_HUGEPAGE);
  // Don't use memcpy because its probably one of the
  // functions thats been mapped out.
  // Needs the attribute((optimize("2")) to prevent
  // g++ from turning this back into memcpy(!)
  wordcpy((uint64_t*)from, (uint64_t*)mem, sz / sizeof(uint64_t));
  mprotect(from, sz, PROT_READ | PROT_EXEC);
  free(mem);
  mlock(from, to - from);
  Debug::DebugInfo::setPidMapOverlay(from, to);
  std::stringstream ss;
  ss << "Mapped text section onto huge pages from " <<
      std::hex << (uint64_t*)from << " to " << (uint64_t*)to;
  Logger::Info(ss.str());
#endif
}

static void pagein_self(void) {
  unsigned long begin, end, inode, pgoff;
  char mapname[PATH_MAX];
  char perm[5];
  char dev[6];
  char *buf;
  int bufsz;
  int r;
  FILE *fp;

  // pad due to the spaces between the inode number and the mapname
  bufsz = sizeof(unsigned long) * 4 + sizeof(mapname) + sizeof(char) * 11 + 100;
  buf = (char *)malloc(bufsz);
  if (buf == nullptr)
    return;

  BootStats::Block timer("mapping self");
  fp = fopen("/proc/self/maps", "r");
  if (fp != nullptr) {
    while (!feof(fp)) {
      if (fgets(buf, bufsz, fp) == 0)
        break;
      r = sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s",
                 &begin, &end, perm, &pgoff, dev, &inode, mapname);

      // page in read-only segments that correspond to a file on disk
      if (r != 7 ||
          perm[0] != 'r' ||
          perm[1] != '-' ||
          access(mapname, F_OK) != 0) {
        continue;
      }

      auto beginPtr = (char*)begin;
      auto endPtr = (char*)end;
      auto hotStart = (char*)__hot_start;
      auto hotEnd = (char*)__hot_end;
      const size_t hugePageBytes = 2L * 1024 * 1024;

      if (mlock(beginPtr, end - begin) == 0) {
        if (RuntimeOption::EvalMaxHotTextHugePages > 0 &&
            __hot_start &&
            __hot_end &&
            hugePagesSupported() &&
            beginPtr <= hotStart &&
            hotEnd <= endPtr) {

          char* from = hotStart - ((intptr_t)hotStart & (hugePageBytes - 1));
          char* to = hotEnd + (hugePageBytes - 1);
          to -= (intptr_t)to & (hugePageBytes - 1);
          const size_t maxHugeHotTextBytes =
            RuntimeOption::EvalMaxHotTextHugePages * hugePageBytes;
          if (to - from >  maxHugeHotTextBytes) {
            to = from + maxHugeHotTextBytes;
          }
          if (to <= (void*)hugifyText) {
            hugifyText(from, to);
          }
        }
        if (!RuntimeOption::LockCodeMemory) {
          munlock(beginPtr, end - begin);
        }
      }
    }
    fclose(fp);
  }
  free(buf);
}

/* Sets RuntimeOption::ExecutionMode according to commandline options prior to
 * config load.  Returns false upon unrecognized mode.
 */
static bool set_execution_mode(folly::StringPiece mode) {
  if (mode == "daemon" || mode == "server" || mode == "replay") {
    RuntimeOption::ServerMode = true;
    Logger::Escape = true;
    return true;
  } else if (mode == "run" || mode == "debug" || mode == "translate" ||
             mode == "dumphhas" || mode == "verify") {
    // We don't run PHP in "translate" mode, so just treat it like cli mode.
    RuntimeOption::ServerMode = false;
    Logger::Escape = false;
    return true;
  }
  // Invalid mode.
  return false;
}

/* Reads a file into the OS page cache, with rate limiting. */
static bool readahead_rate(const char* path, int64_t mbPerSec) {
  int ret = open(path, O_RDONLY);
  if (ret < 0) return false;
  const int fd = ret;
  SCOPE_EXIT { close(fd); };

  constexpr size_t kReadaheadBytes = 1 << 20;
  std::unique_ptr<char[]> buf(new char[kReadaheadBytes]);
  int64_t total = 0;
  auto startTime = std::chrono::steady_clock::now();
  do {
    ret = read(fd, buf.get(), kReadaheadBytes);
    if (ret > 0) {
      total += ret;
      // Unit math: bytes / (MB / seconds) = microseconds
      auto endTime = startTime + std::chrono::microseconds(total / mbPerSec);
      auto sleepT = endTime - std::chrono::steady_clock::now();
      // Don't sleep too frequently.
      if (sleepT >= std::chrono::seconds(1)) {
        Logger::Info(folly::sformat(
          "readahead sleeping {}ms after total {}b",
          std::chrono::duration_cast<std::chrono::milliseconds>(sleepT).count(),
          total));
        /* sleep override */ std::this_thread::sleep_for(sleepT);
      }
    }
  } while (ret > 0);
  return ret == 0;
}

static int start_server(const std::string &username, int xhprof) {
  BootStats::start();
  HttpServer::CheckMemAndWait();
  InitFiniNode::ServerPreInit();

  if (!RuntimeOption::EvalUnixServerPath.empty()) {
    init_cli_server(RuntimeOption::EvalUnixServerPath.c_str());
  }

  // Before we start the webserver, make sure the entire
  // binary is paged into memory.
  pagein_self();
  BootStats::mark("pagein_self");

  set_execution_mode("server");

#if !defined(SKIP_USER_CHANGE)
  if (!username.empty()) {
    if (Logger::UseCronolog) {
      for (const auto& el : RuntimeOption::ErrorLogs) {
        Cronolog::changeOwner(username, el.second.symLink);
      }
    }
    if (!Capability::ChangeUnixUser(username, RuntimeOption::AllowRunAsRoot)) {
      _exit(1);
    }
    LightProcess::ChangeUser(username);
    compilers_set_user(username);
  } else if (getuid() == 0 && !RuntimeOption::AllowRunAsRoot) {
    Logger::Error("hhvm not allowed to run as root unless "
                  "-vServer.AllowRunAsRoot=1 is used.");
    _exit(1);
  }
  Capability::SetDumpable();
#endif

  hphp_process_init();
  SCOPE_EXIT {
    hphp_process_exit();
    try { Logger::Info("all servers stopped"); } catch(...) {}
  };

  HttpRequestHandler::GetAccessLog().init
    (RuntimeOption::AccessLogDefaultFormat, RuntimeOption::AccessLogs,
     username);
  AdminRequestHandler::GetAccessLog().init
    (RuntimeOption::AdminLogFormat, RuntimeOption::AdminLogSymLink,
     RuntimeOption::AdminLogFile,
     username);
  RPCRequestHandler::GetAccessLog().init
    (RuntimeOption::AccessLogDefaultFormat, RuntimeOption::RPCLogs,
     username);
  SCOPE_EXIT { HttpRequestHandler::GetAccessLog().flushAllWriters(); };
  SCOPE_EXIT { AdminRequestHandler::GetAccessLog().flushAllWriters(); };
  SCOPE_EXIT { RPCRequestHandler::GetAccessLog().flushAllWriters(); };
  SCOPE_EXIT { Logger::FlushAll(); };

  if (RuntimeOption::ServerInternalWarmupThreads > 0) {
    HttpServer::CheckMemAndWait();
    InitFiniNode::WarmupConcurrentStart(
      RuntimeOption::ServerInternalWarmupThreads);
  }

  HttpServer::CheckMemAndWait();
  // Create the HttpServer before any warmup requests to properly
  // initialize the process
  HttpServer::Server = std::make_shared<HttpServer>();

  if (xhprof) {
    HHVM_FN(xhprof_enable)(xhprof, uninit_null().toArray());
  }

  std::unique_ptr<std::thread> readaheadThread;

  if (RuntimeOption::RepoLocalReadaheadRate > 0 &&
      !RuntimeOption::RepoLocalPath.empty()) {
    HttpServer::CheckMemAndWait();
    readaheadThread = std::make_unique<std::thread>([&] {
        BootStats::Block timer("Readahead Repo");
        auto path = RuntimeOption::RepoLocalPath.c_str();
        Logger::Info("readahead %s", path);
#ifdef __linux__
        // glibc doesn't have a wrapper for ioprio_set(), so we need to use
        // syscall().  The constants here are consistent with the kernel source.
        // See http://lxr.free-electrons.com/source/include/linux/ioprio.h
        auto constexpr IOPRIO_CLASS_SHIFT = 13;
        enum {
          IOPRIO_CLASS_NONE,
          IOPRIO_CLASS_RT,
          IOPRIO_CLASS_BE,
          IOPRIO_CLASS_IDLE,
        };
        // Set to lowest IO priority.
        constexpr int ioprio = (IOPRIO_CLASS_IDLE << IOPRIO_CLASS_SHIFT);

        // ioprio_set() is available starting kernel 2.6.13
        KernelVersion version;
        if (version.m_major > 2 ||
            (version.m_major == 2 &&
             (version.m_minor > 6 ||
              (version.m_minor == 6 && version.m_release >= 13)))) {
          syscall(SYS_ioprio_set,
                  1 /* IOPRIO_WHO_PROCESS, in fact, it is this thread */,
                  0 /* current thread */,
                  ioprio);
        }
#endif
        const auto mbPerSec = RuntimeOption::RepoLocalReadaheadRate;
        if (!readahead_rate(path, mbPerSec)) {
          Logger::Error("readahead failed: %s", strerror(errno));
        }
      });
    if (!RuntimeOption::RepoLocalReadaheadConcurrent) {
      // TODO(10152762): Run this concurrently with non-disk warmup.
      readaheadThread->join();
      readaheadThread.reset();
    }
  }

  if (RuntimeOption::ServerInternalWarmupThreads > 0) {
    BootStats::Block timer("concurrentWaitForEnd");
    InitFiniNode::WarmupConcurrentWaitForEnd();
  }

  if (RuntimeOption::RepoPreload) {
    HttpServer::CheckMemAndWait();
    BootStats::Block timer("Preloading Repo");
    profileWarmupStart();
    preloadRepo();
    profileWarmupEnd();
  }

  // If we have any warmup requests, replay them before listening for
  // real connections
  {
    Logger::Info("Warming up");
    if (!RuntimeOption::EvalJitProfileWarmupRequests) profileWarmupStart();
    SCOPE_EXIT { profileWarmupEnd(); };
    std::map<std::string, int> seen;
    for (auto& file : RuntimeOption::ServerWarmupRequests) {
      HttpServer::CheckMemAndWait();
      // Take only the last part
      folly::StringPiece f(file);
      auto pos = f.rfind('/');
      std::string str(pos == f.npos ? file : f.subpiece(pos + 1).str());
      auto count = seen[str];
      BootStats::Block timer(folly::sformat("warmup:{}:{}", str, count++));
      seen[str] = count;

      HttpRequestHandler handler(0);
      ReplayTransport rt;
      timespec start;
      Timer::GetMonotonicTime(start);
      std::string error;
      Logger::Info("Replaying warmup request %s", file.c_str());

      try {
        rt.onRequestStart(start);
        rt.replayInput(Hdf(file));
        handler.run(&rt);

        timespec stop;
        Timer::GetMonotonicTime(stop);
        Logger::Info("Finished successfully in %ld seconds",
                     stop.tv_sec - start.tv_sec);
      } catch (std::exception& e) {
        error = e.what();
      }

      if (error.size()) {
        Logger::Info("Got exception during warmup: %s", error.c_str());
      }
    }
  }
  BootStats::mark("warmup");

  if (RuntimeOption::StopOldServer) HttpServer::StopOldServer();

  if (RuntimeOption::EvalEnableNuma && !getenv("HHVM_DISABLE_NUMA")) {
#ifdef USE_JEMALLOC
    unsigned narenas;
    size_t mib[3];
    size_t miblen = 3;
    if (mallctlWrite<uint64_t>("epoch", 1, true) == 0 &&
        mallctlRead("arenas.narenas", &narenas, true) == 0 &&
        mallctlnametomib("arena.0.purge", mib, &miblen) == 0) {
      mib[1] = size_t(narenas);
      mallctlbymib(mib, miblen, nullptr, nullptr, nullptr, 0);
    }
#endif
    enable_numa(RuntimeOption::EvalEnableNumaLocal);
    BootStats::mark("enable_numa");
  }

#ifdef FACEBOOK
  // we're serving real traffic now, start batching log output
  if (RuntimeOption::UseThriftLogger) {
    Logger::Info("Turning logger batching on, batch size %ld",
                 RuntimeOption::LoggerBatchSize);
    Logger::SetBatchSize(RuntimeOption::LoggerBatchSize);
    Logger::SetFlushTimeout(
      std::chrono::milliseconds(RuntimeOption::LoggerFlushTimeout));
  }
#endif

  HttpServer::CheckMemAndWait(true); // Final wait
  if (readaheadThread.get()) {
    readaheadThread->join();
    readaheadThread.reset();
  }

  if (!RuntimeOption::EvalUnixServerPath.empty()) {
    start_cli_server();
  }

  HttpServer::Server->runOrExitProcess();
  HttpServer::Server.reset();

  return 0;
}

static void logSettings() {
  if (RuntimeOption::ServerLogSettingsOnStartup) {
    Logger::Info("Settings: %s\n", IniSetting::GetAllAsJSON().c_str());
  }
}

static InitFiniNode s_logSettings(logSettings, InitFiniNode::When::ServerInit);

std::string translate_stack(const char *hexencoded, bool with_frame_numbers) {
  if (!hexencoded || !*hexencoded) {
    return "";
  }

  StackTrace st(hexencoded);
  std::vector<std::shared_ptr<StackFrameExtra>> frames;
  st.get(frames);

  std::ostringstream out;
  for (size_t i = 0; i < frames.size(); i++) {
    auto f = frames[i];
    if (with_frame_numbers) {
      out << "# " << (i < 10 ? " " : "") << i << ' ';
    }
    out << f->toString();
    out << '\n';
  }
  return out.str();
}

///////////////////////////////////////////////////////////////////////////////

static void prepare_args(int &argc,
                         char **&argv,
                         const std::vector<std::string> &args,
                         const char *file) {
  argv = (char **)malloc((args.size() + 2) * sizeof(char*));
  argc = 0;
  if (file && *file) {
    argv[argc++] = (char*)file;
  }
  for (int i = 0; i < (int)args.size(); i++) {
    argv[argc++] = (char*)args[i].c_str();
  }
  argv[argc] = nullptr;
}

static int execute_program_impl(int argc, char **argv);
int execute_program(int argc, char **argv) {
  int ret_code = -1;
  try {
    try {
      initialize_repo();
      ret_code = execute_program_impl(argc, argv);
    } catch (const Exception &e) {
      Logger::Error("Uncaught exception: %s", e.what());
      throw;
    } catch (const std::exception &e) {
      Logger::Error("Uncaught exception: %s", e.what());
      throw;
    } catch (...) {
      Logger::Error("Uncaught exception: (unknown)");
      throw;
    }
    if (tempFile.length() && boost::filesystem::exists(tempFile)) {
      boost::filesystem::remove(tempFile);
    }
  } catch (...) {
    if (HttpServer::Server ||
        folly::SingletonVault::singleton()->livingSingletonCount()) {
      // an exception was thrown that prevented proper shutdown. Its not
      // safe to destroy the globals, or run atexit handlers.
      // abort() so it shows up as a crash, and we can diagnose/fix the
      // exception
      abort();
    }
  }

  return ret_code;
}

static bool open_server_log_files() {
  bool openedLog = false;
  for (const auto& el : RuntimeOption::ErrorLogs) {
    bool ok = true;
    const auto& name    = el.first;
    const auto& errlog = el.second;
    if (!errlog.logFile.empty()) {
      if (errlog.isPipeOutput()) {
        auto output = popen(errlog.logFile.substr(1).c_str(), "w");
        ok = (output != nullptr);
        Logger::SetOutput(name, output, true);
      } else if (Logger::UseCronolog && errlog.hasTemplate()) {
        auto cronoLog = Logger::CronoOutput(name);
        always_assert(cronoLog);
        cronoLog->m_template = errlog.logFile;
        cronoLog->setPeriodicity();
        if (errlog.periodMultiplier) {
          cronoLog->m_periodMultiple = errlog.periodMultiplier;
        }
        cronoLog->m_linkName = errlog.symLink;
      } else {
        auto output = fopen(errlog.logFile.c_str(), "a");
        ok = (output != nullptr);
        Logger::SetOutput(name, output, false);
      }
      if (!ok) Logger::Error("Can't open log file: %s", errlog.logFile.c_str());
      openedLog |= ok;
    }
  }
  return openedLog;
}

static int compute_hhvm_argc(const options_description& desc,
                             int argc, char** argv) {
  enum ArgCode {
    NO_ARG = 0,
    ARG_REQUIRED = 1,
    ARG_OPTIONAL = 2
  };
  const auto& vec = desc.options();
  std::map<std::string,ArgCode> long_options;
  std::map<std::string,ArgCode> short_options;
  // Build lookup maps for the short options and the long options
  for (unsigned i = 0; i < vec.size(); ++i) {
    auto opt = vec[i];
    auto long_name = opt->long_name();
    ArgCode code = NO_ARG;
    if (opt->semantic()->max_tokens() == 1) {
      if (opt->semantic()->min_tokens() == 1) {
        code = ARG_REQUIRED;
      } else {
        code = ARG_OPTIONAL;
      }
    }
    long_options[long_name] = code;
    auto format_name = opt->format_name();
    if (format_name.size() >= 2 && format_name[0] == '-' &&
        format_name[1] != '-') {
      auto short_name = format_name.substr(1,1);
      short_options[short_name] = code;
    }
  }
  // Loop over the args
  int pos = 1;
  while (pos < argc) {
    const char* str = argv[pos];
    int len = strlen(str);
    if (len == 2 && memcmp(str, "--", 2) == 0) {
      // We found "--". All args after this are intended for the
      // PHP application
      ++pos;
      break;
    }
    if (len >= 3 && str[0] == '-' && str[1] == '-') {
      // Handle long options
      ++pos;
      std::string s(str+2);
      auto it = long_options.find(s);
      if (it != long_options.end() && it->second != NO_ARG && pos < argc &&
          (it->second == ARG_REQUIRED || argv[pos][0] != '-')) {
        ++pos;
      }
    } else if (len >= 2 && str[0] == '-') {
      // Handle short options
      ++pos;
      std::string s;
      s.append(1, str[1]);
      auto it = short_options.find(s);
      if (it != short_options.end() && it->second != 0 && len == 2 &&
          pos < argc && (it->second == ARG_REQUIRED || argv[pos][0] != '-')) {
        ++pos;
      }
    } else {
      // We've found a non-option argument. This arg and all args
      // that follow are intended for the PHP application
      break;
    }
  }
  return pos;
}

/*
 * alloc.h defines a minimum C++ stack size but that only applies to threads we
 * manually create.  When the main thread will be executing PHP rather than just
 * managing a server, make sure its stack is big enough.
 */
static void set_stack_size() {
  struct rlimit rlim;
  if (getrlimit(RLIMIT_STACK, &rlim) != 0) return;

  if (rlim.rlim_cur < kStackSizeMinimum || rlim.rlim_cur == RLIM_INFINITY) {
#ifdef _WIN32
    Logger::Error("stack limit too small, use peflags -x to increase %zd\n",
                  kStackSizeMinimum);
#else
    rlim.rlim_cur = kStackSizeMinimum;
    if (setrlimit(RLIMIT_STACK, &rlim)) {
      Logger::Error("failed to set stack limit to %zd\n", kStackSizeMinimum);
    }
#endif
  }
}

#if defined(BOOST_VERSION) && BOOST_VERSION <= 105400
std::string get_right_option_name(const basic_parsed_options<char>& opts,
                                  std::string& wrong_name) {
  // Remove any - from the wrong name for better comparing
  // since it will probably come prepended with --
  wrong_name.erase(
    std::remove(wrong_name.begin(), wrong_name.end(), '-'), wrong_name.end());
  for (basic_option<char> opt : opts.options) {
    std::string s_opt = opt.string_key;
    // We are only dealing with options that have a - in them.
    if (s_opt.find("-") != std::string::npos) {
      if (s_opt.find(wrong_name) != std::string::npos) {
        return s_opt;
      }
    }
  }
  return "";
}
#endif

//
// Note that confusingly there are two different implementations
// of zend_strtod.
//
// The one from
//   hphp/runtime/ext_zend_compat/php-src/Zend/zend_strtod.h
// does not wrap with HPHP namespace, and implements
// functionality required by the zend extension compatibility layer.
// Empirically, this zend_strtod.h file can't be included because
// it includes <zend.h> which isn't on any search path when compiling this.
//
// The zend_startup_strtod from
//   hphp/runtime/base/zend-strtod.h
// uses the HPHP namespace, is used for other purposes,
// and predates the EZC extensions.
//
// Before we can call zend_strtod from zend compatibility extensions,
// we need to initialize it.  Since it doesn't seem
// to work to include the .h file, just sleaze declare it here.
//
// See the related issue https://github.com/facebook/hhvm/issues/5244
//

static int execute_program_impl(int argc, char** argv) {
  std::string usage = "Usage:\n\n   ";
  usage += argv[0];
  usage += " [-m <mode>] [<options>] [<arg1>] [<arg2>] ...\n\nOptions";

  ProgramOptions po;
  options_description desc(usage.c_str());
  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("modules", "display modules")
    ("info", "PHP information")
    ("php", "emulate the standard php command line")
    ("compiler-id", "display the git hash for the compiler")
    ("repo-schema", "display the repository schema id")
    ("mode,m", value<std::string>(&po.mode)->default_value("run"),
     "run | debug (d) | server (s) | daemon | replay | translate (t) | verify")
    ("interactive,a", "Shortcut for --mode debug") // -a is from PHP5
    ("config,c", value<std::vector<std::string>>(&po.config)->composing(),
     "load specified config file")
    ("config-value,v",
     value<std::vector<std::string>>(&po.confStrings)->composing(),
     "individual configuration string in a format of name=value, where "
     "name can be any valid configuration for a config file")
    ("define,d", value<std::vector<std::string>>(&po.iniStrings)->composing(),
     "define an ini setting in the same format ( foo[=bar] ) as provided in a "
     ".ini file")
    ("no-config", "don't use the default php.ini")
    ("port,p", value<int>(&po.port)->default_value(-1),
     "start an HTTP server at specified port")
    ("port-fd", value<int>(&po.portfd)->default_value(-1),
     "use specified fd instead of creating a socket")
    ("ssl-port-fd", value<int>(&po.sslportfd)->default_value(-1),
     "use specified fd for SSL instead of creating a socket")
    ("admin-port", value<int>(&po.admin_port)->default_value(-1),
     "start admin listener at specified port")
    ("debug-config", value<std::string>(&po.debugger_options.configFName),
      "load specified debugger config file")
    ("debug-host,h",
     value<std::string>(&po.debugger_options.host)->implicit_value("localhost"),
     "connect to debugger server at specified address")
    ("debug-port", value<int>(&po.debugger_options.port)->default_value(-1),
     "connect to debugger server at specified port")
    ("debug-extension", value<std::string>(&po.debugger_options.extension),
     "PHP file that extends command 'arg'")
    ("debug-cmd", value<std::vector<std::string>>(
     &po.debugger_options.cmds)->composing(),
     "executes this debugger command and returns its output in stdout")
    ("debug-sandbox",
     value<std::string>(&po.debugger_options.sandbox)->default_value("default"),
     "initial sandbox to attach to when debugger is started")
    ("user,u", value<std::string>(&po.user),
     "run server under this user account")
    ("file,f", value<std::string>(&po.file),
     "execute specified file")
    ("lint,l", value<std::string>(&po.lint),
     "lint specified file")
    ("show,w", value<std::string>(&po.show),
     "output specified file and do nothing else")
    ("temp-file",
     "file specified is temporary and removed after execution")
    ("count", value<int>(&po.count)->default_value(1),
     "how many times to repeat execution")
    ("no-safe-access-check",
      value<bool>(&po.noSafeAccessCheck)->default_value(false),
     "whether to ignore safe file access check")
    ("arg", value<std::vector<std::string>>(&po.args)->composing(),
     "arguments")
    ("extra-header", value<std::string>(&Logger::ExtraHeader),
     "extra-header to add to log lines")
    ("build-id", value<std::string>(&po.buildId),
     "unique identifier of compiled server code")
    ("instance-id", value<std::string>(&po.instanceId),
     "unique identifier of server instance")
    ("xhprof-flags", value<int>(&po.xhprofFlags)->default_value(0),
     "Set XHProf flags")
    ;

  positional_options_description p;
  p.add("arg", -1);
  variables_map vm;

  // Before invoking the boost command line parser, we do a manual pass
  // to find the first occurrence of either "--" or a non-option argument
  // in order to determine which arguments should be consumed by HHVM and
  // which arguments should be passed along to the PHP application. This
  // is necessary so that the boost command line parser doesn't choke on
  // args intended for the PHP application.
  int hhvm_argc = compute_hhvm_argc(desc, argc, argv);
  // Need to have a parent try for opts so I can use opts in the catch of
  // one of the sub-tries below.
  try {
    // Invoke the boost command line parser to parse the args for HHVM.
    auto opts = command_line_parser(hhvm_argc, argv)
      .options(desc)
      .positional(p)
      // If these style options are changed, compute_hhvm_argc() will
      // need to be updated appropriately
      .style(command_line_style::default_style &
             ~command_line_style::allow_guessing &
             ~command_line_style::allow_sticky &
             ~command_line_style::long_allow_adjacent)
      .run();
    try {
      // Manually append the args for the PHP application.
      int pos = 0;
      for (unsigned m = 0; m < opts.options.size(); ++m) {
        const auto& bo = opts.options[m];
        if (bo.string_key == "arg") {
          ++pos;
        }
      }
      for (unsigned m = hhvm_argc; m < argc; ++m) {
        std::string str = argv[m];
        basic_option<char> bo;
        bo.string_key = "arg";
        bo.position_key = pos++;
        bo.value.push_back(str);
        bo.original_tokens.push_back(str);
        bo.unregistered = false;
        bo.case_insensitive = false;
        opts.options.push_back(bo);
      }
      // Process the options
      store(opts, vm);
      notify(vm);
      if (vm.count("interactive") /* or -a */) po.mode = "debug";
      else if (po.mode.empty()) po.mode = "run";
      else if (po.mode == "d") po.mode = "debug";
      else if (po.mode == "s") po.mode = "server";
      else if (po.mode == "t") po.mode = "translate";

      if (!set_execution_mode(po.mode)) {
        Logger::Error("Error in command line: invalid mode: %s",
                      po.mode.c_str());
        cout << desc << "\n";
        return -1;
      }
      if (po.config.empty() && !vm.count("no-config")
          && ::getenv("HHVM_NO_DEFAULT_CONFIGS") == nullptr) {
        auto file_callback = [&po] (const char *filename) {
          Logger::Verbose("Using default config file: %s", filename);
          po.config.push_back(filename);
        };
        add_default_config_files_globbed(DEFAULT_CONFIG_DIR "/php*.ini",
                                         file_callback);
        add_default_config_files_globbed(DEFAULT_CONFIG_DIR "/config*.hdf",
                                         file_callback);
      }
      const auto env_config = ::getenv("HHVM_CONFIG_FILE");
      if (env_config != nullptr) {
        add_default_config_files_globbed(
          env_config,
          [&po](const char* filename) {
            Logger::Verbose("Using config file from environment: %s", filename);
            po.config.push_back(filename);
          }
        );
      }
// When we upgrade boost, we can remove this and also get rid of the parent
// try statement and move opts back into the original try block
#if defined(BOOST_VERSION) && BOOST_VERSION >= 105000 && BOOST_VERSION <= 105400
    } catch (const error_with_option_name &e) {
      std::string wrong_name = e.get_option_name();
      std::string right_name = get_right_option_name(opts, wrong_name);
      std::string message = e.what();
      if (right_name != "") {
        boost::replace_all(message, wrong_name, right_name);
      }
      Logger::Error("Error in command line: %s", message.c_str());
      cout << desc << "\n";
      return -1;
#endif
    } catch (const error &e) {
      Logger::Error("Error in command line: %s", e.what());
      cout << desc << "\n";
      return -1;
    } catch (...) {
      Logger::Error("Error in command line.");
      cout << desc << "\n";
      return -1;
    }
  } catch (const error &e) {
    Logger::Error("Error in command line: %s", e.what());
    cout << desc << "\n";
    return -1;
  } catch (...) {
    Logger::Error("Error in command line parsing.");
    cout << desc << "\n";
    return -1;
  }
  // reuse -h for help command if possible
  if (vm.count("help") || (vm.count("debug-host") && po.mode != "debug")) {
    cout << desc << "\n";
    return 0;
  }
  if (vm.count("version")) {
    cout << "HipHop VM";
    cout << " " << HHVM_VERSION;
    cout << " (" << (debug ? "dbg" : "rel") << ")\n";
    cout << "Compiler: " << compilerId() << "\n";
    cout << "Repo schema: " << repoSchemaId() << "\n";
    return 0;
  }
  if (vm.count("modules")) {
    Array exts = ExtensionRegistry::getLoaded();
    cout << "[PHP Modules]" << "\n";
    for (ArrayIter iter(exts); iter; ++iter) {
      cout << iter.second().toString().toCppString() << "\n";
    }
    return 0;
  }
  if (vm.count("compiler-id")) {
    cout << compilerId() << "\n";
    return 0;
  }

  if (vm.count("repo-schema")) {
    cout << repoSchemaId() << "\n";
    return 0;
  }

  if (!po.show.empty()) {
    auto f = req::make<PlainFile>();
    f->open(po.show, "r");
    if (!f->valid()) {
      Logger::Error("Unable to open file %s", po.show.c_str());
      return 1;
    }
    f->print();
    f->close();
    return 0;
  }

  po.isTempFile = vm.count("temp-file");

  // forget the source for systemlib.php unless we are debugging
  if (po.mode != "debug") SystemLib::s_source = "";

  // we need to initialize pcre cache table very early
  pcre_init();

#ifdef ENABLE_ZEND_COMPAT
  //
  // Initialize in the zend extension compatibility layer, as needed
  // before any calls from legacy zend extensions to zend_strtod. See
  // the extern "C" declaration of this function, above.
  //
  zend_startup_strtod();
#endif

  MemoryManager::TlsWrapper::getCheck();
  if (RuntimeOption::ServerExecutionMode()) {
    // Create the hardware counter before reading options,
    // so that the main thread never has inherit set in server
    // mode
    HardwareCounter::s_counter.getCheck();
  }
  std::vector<std::string> messages;
  // We want the ini map to be freed after processing and loading the options
  // So put this in its own block
  {
    IniSettingMap ini = IniSettingMap();
    Hdf config;
    s_config_files = po.config;
    // Start with .hdf and .ini files
    for (auto& filename : s_config_files) {
      if (boost::filesystem::exists(filename)) {
        Config::ParseConfigFile(filename, ini, config);
      } else {
        Logger::Warning(
          "The configuration file %s does not exist",
          filename.c_str()
        );
      }
    }
    // Now, take care of CLI options and then officially load and bind things
    s_ini_strings = po.iniStrings;
    RuntimeOption::Load(ini, config, po.iniStrings, po.confStrings, &messages);
    std::vector<std::string> badnodes;
    config.lint(badnodes);
    for (const auto& badnode : badnodes) {
      const auto msg = "Possible bad config node: " + badnode;
      fprintf(stderr, "%s\n", msg.c_str());
      messages.push_back(msg);
    }
  }

  std::vector<int> inherited_fds;
  RuntimeOption::BuildId = po.buildId;
  RuntimeOption::InstanceId = po.instanceId;

  // Do this as early as possible to avoid creating temp files and spawing
  // light processes. Correct compilation still requires loading all of the
  // ini/hdf/cli options.
  if (po.mode == "dumphhas" || po.mode == "verify") {
    if (po.file.empty() && po.args.empty()) {
      std::cerr << "Nothing to do. Pass a php file to compile.\n";
      return 1;
    }

    auto const file = [] (std::string file) -> std::string {
      if (!FileUtil::isAbsolutePath(file)) {
        return SourceRootInfo::GetCurrentSourceRoot() + std::move(file);
      }
      return file;
    }(po.file.empty() ? po.args[0] : po.file);

    RuntimeOption::RepoCommit = false; // avoid initializing a repo

    std::fstream fs(file, std::ios::in);
    if (!fs) {
      std::cerr << "Unable to open \"" << file << "\"\n";
      return 1;
    }
    std::stringstream contents;
    contents << fs.rdbuf();

    auto const str = contents.str();
    auto const md5 = MD5{mangleUnitMd5(string_md5(str))};

    hphp_thread_init();
    g_context.getCheck();
    SCOPE_EXIT { hphp_thread_exit(); };

    // Initialize compiler state
    compile_file(0, 0, MD5(), 0);

    if (po.mode == "dumphhas")  RuntimeOption::EvalDumpHhas = true;
    else RuntimeOption::EvalVerifyOnly = true;
    SystemLib::s_inited = true;

    auto compiled = compile_file(str.c_str(), str.size(), md5, file.c_str(),
                                 nullptr);

    if (po.mode == "verify") {
      return 0;
    }

    // This will dump the hhas for file as EvalDumpHhas was set
    if (!compiled) {
      std::cerr << "Unable to compile \"" << file << "\"\n";
      return 1;
    }

    return 0;
  }

  if (po.port != -1) {
    RuntimeOption::ServerPort = po.port;
  }
  if (po.portfd != -1) {
    RuntimeOption::ServerPortFd = po.portfd;
    inherited_fds.push_back(po.portfd);
  }
  if (po.sslportfd != -1) {
    RuntimeOption::SSLPortFd = po.sslportfd;
    inherited_fds.push_back(po.sslportfd);
  }
  if (po.admin_port != -1) {
    RuntimeOption::AdminServerPort = po.admin_port;
  }
  if (po.noSafeAccessCheck) {
    RuntimeOption::SafeFileAccess = false;
  }
  IniSetting::s_system_settings_are_set = true;
  MM().resetRuntimeOptions();

  auto opened_logs = open_server_log_files();
  if (po.mode == "daemon") {
    if (!opened_logs) {
      Logger::Error("Log file not specified under daemon mode.\n\n");
    }
    proc::daemonize();
  }

  if (RuntimeOption::ServerExecutionMode()) {
    for (auto const& m : messages) {
      Logger::Info(m);
    }
  }

#ifndef _MSC_VER
  // Defer the initialization of light processes until the log file handle is
  // created, so that light processes can log to the right place. If we ever
  // lose a light process, stop the server instead of proceeding in an
  // uncertain state. Don't start them in DumpHhas mode because
  // it _Exit()s after loading the first non-systemlib unit.
  if (!RuntimeOption::EvalDumpHhas) {
    LightProcess::SetLostChildHandler([](pid_t /*child*/) {
      if (!HttpServer::Server) return;
      if (!HttpServer::Server->isStopped()) {
        HttpServer::Server->stopOnSignal(SIGCHLD);
      }
    });
    LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                             RuntimeOption::LightProcessCount,
                             RuntimeOption::EvalRecordSubprocessTimes,
                             inherited_fds);

    // HackC initialization should happen immediately prior to LightProcess
    // configuration as it will create a private delegate process to deal with
    // hackc instances.
    compilers_init();
  }
#endif

  // We want to do this as early as possible because any allocations before-hand
  // will get a generic unknown type type-index.
  try {
    type_scan::init();
  } catch (const type_scan::InitException& exn) {
    Logger::Error("Unable to initialize GC type-scanners: %s", exn.what());
    exit(HPHP_EXIT_FAILURE);
  }
  MemoryManager::TlsWrapper::fixTypeIndex();

  // It's okay if this fails.
  init_member_reflection();

  if (!ShmCounters::initialize(true, Logger::Error)) {
    exit(HPHP_EXIT_FAILURE);
  }
  // Initialize compiler state
  compile_file(0, 0, MD5(), 0);

  if (!po.lint.empty()) {
    Logger::LogHeader = false;
    Logger::LogLevel = Logger::LogInfo;
    Logger::UseCronolog = false;
    Logger::UseLogFile = true;
    // we're linting, reset whatever logger settings and write once to stdout
    Logger::ClearThreadLog();
    for (auto& el : RuntimeOption::ErrorLogs) {
      const auto& name = el.first;
      Logger::SetTheLogger(name, nullptr);
    }
    Logger::SetTheLogger(Logger::DEFAULT, new Logger());

    if (po.isTempFile) {
      tempFile = po.lint;
    }

    hphp_process_init();
    SCOPE_EXIT { hphp_process_exit(); };

    try {
      auto const unit = lookupUnit(
        makeStaticString(po.lint.c_str()), "", nullptr);
      if (unit == nullptr) {
        throw FileOpenException(po.lint);
      }
      const StringData* msg;
      int line;
      if (unit->compileTimeFatal(msg, line)) {
        VMParserFrame parserFrame;
        parserFrame.filename = po.lint.c_str();
        parserFrame.lineNumber = line;
        Array bt = createBacktrace(BacktraceArgs()
                                   .withSelf()
                                   .setParserFrame(&parserFrame));
        raise_fatal_error(msg->data(), bt);
      }
    } catch (FileOpenException &e) {
      Logger::Error("%s", e.getMessage().c_str());
      return 1;
    } catch (const FatalErrorException& e) {
      RuntimeOption::CallUserHandlerOnFatals = false;
      RuntimeOption::AlwaysLogUnhandledExceptions = false;
      g_context->onFatalError(e);
      return 1;
    }
    Logger::Info("No syntax errors detected in %s", po.lint.c_str());
    return 0;
  }

  if (argc <= 1 || po.mode == "run" || po.mode == "debug") {
    set_stack_size();

    if (po.isTempFile) {
      tempFile = po.file;
    }

    set_execution_mode("run");
    /* recreate the hardware counters for the main thread now that we know
     * whether to include subprocess times */
    HardwareCounter::s_counter.destroy();
    HardwareCounter::s_counter.getCheck();

    int new_argc;
    char **new_argv;
    prepare_args(new_argc, new_argv, po.args, po.file.c_str());

    std::string const cliFile = !po.file.empty() ? po.file :
                                new_argv[0] ? new_argv[0] : "";
    if (po.mode != "debug" && cliFile.empty()) {
      std::cerr << "Nothing to do. Either pass a .php file to run, or "
        "use -m server\n";
      return 1;
    }
    Repo::setCliFile(cliFile);

    int ret = 0;
    hphp_process_init();
    SCOPE_EXIT { hphp_process_exit(); };

    if (RuntimeOption::EvalUseRemoteUnixServer != "no" &&
        !RuntimeOption::EvalUnixServerPath.empty() &&
        (!po.file.empty() || !po.args.empty())) {
      std::vector<std::string> args;
      if (!po.file.empty()) {
        args.emplace_back(po.file);
      }
      args.insert(args.end(), po.args.begin(), po.args.end());
      run_command_on_cli_server(
        RuntimeOption::EvalUnixServerPath.c_str(), args
      );
      if (RuntimeOption::EvalUseRemoteUnixServer == "only") {
        Logger::Error("Failed to connect to unix server.");
        exit(255);
      }
    }

    std::string file;
    if (new_argc > 0) {
      file = new_argv[0];
    }

    if (po.mode == "debug") {
      StackTraceNoHeap::AddExtraLogging("IsDebugger", "True");
      RuntimeOption::EnableDebugger = true;
      po.debugger_options.fileName = file;
      po.debugger_options.user = po.user;
      Eval::DebuggerProxyPtr localProxy =
        Eval::Debugger::StartClient(po.debugger_options);
      if (!localProxy) {
        Logger::Error("Failed to start debugger client\n\n");
        return 1;
      }
      Eval::Debugger::RegisterSandbox(localProxy->getDummyInfo());
      std::shared_ptr<std::vector<std::string>> client_args;
      bool restart = false;
      ret = 0;
      while (true) {
        try {
          assert(po.debugger_options.fileName == file);
          execute_command_line_begin(new_argc, new_argv, po.xhprofFlags);
          // Set the proxy for this thread to be the localProxy we just
          // created. If we're script debugging, this will be the proxy that
          // does all of our work. If we're remote debugging, this proxy will
          // go unused until we finally stop it when the user quits the
          // debugger.
          g_context->setSandboxId(localProxy->getDummyInfo().id());
          if (restart) {
            // Systemlib.php is not loaded again, so we need this if we
            // are to hit any breakpoints in systemlib.
            proxySetBreakPoints(localProxy.get());
          }
          Eval::Debugger::DebuggerSession(po.debugger_options, restart);
          restart = false;
          execute_command_line_end(po.xhprofFlags, true, file.c_str());
        } catch (const Eval::DebuggerRestartException &e) {
          execute_command_line_end(0, false, nullptr);

          if (!e.m_args->empty()) {
            file = e.m_args->at(0);
            po.debugger_options.fileName = file;
            client_args = e.m_args;
            free(new_argv);
            prepare_args(new_argc, new_argv, *client_args, nullptr);
          }
          restart = true;
        } catch (const Eval::DebuggerClientExitException &e) {
          execute_command_line_end(0, false, nullptr);
          break; // end user quitting debugger
        }
      }

    } else {
      ret = 0;
      for (int i = 0; i < po.count; i++) {
        execute_command_line_begin(new_argc, new_argv, po.xhprofFlags);
        ret = 255;
        if (hphp_invoke_simple(file, false /* warmup only */)) {
          ret = ExitException::ExitCode;
        }
        execute_command_line_end(po.xhprofFlags, true, file.c_str());
      }
    }

    free(new_argv);

    return ret;
  }

  if (po.mode == "daemon" || po.mode == "server") {
    if (!po.user.empty()) RuntimeOption::ServerUser = po.user;
    return start_server(RuntimeOption::ServerUser, po.xhprofFlags);
  }

  if (po.mode == "replay" && !po.args.empty()) {
    RuntimeOption::RecordInput = false;
    set_execution_mode("server");
    HttpServer server; // so we initialize runtime properly
    HttpRequestHandler handler(0);
    for (int i = 0; i < po.count; i++) {
      for (unsigned int j = 0; j < po.args.size(); j++) {
        ReplayTransport rt;
        rt.replayInput(po.args[j].c_str());
        handler.run(&rt);
        printf("%s\n", rt.getResponse().c_str());
      }
    }
    return 0;
  }

  if (po.mode == "translate" && !po.args.empty()) {
    printf("%s", translate_stack(po.args[0].c_str()).c_str());
    return 0;
  }

  cout << desc << "\n";
  return -1;
}

String canonicalize_path(const String& p, const char* root, int rootLen) {
  String path = FileUtil::canonicalize(p);
  if (path.charAt(0) == '/') {
    auto const& sourceRoot = RuntimeOption::SourceRoot;
    int len = sourceRoot.size();
    if (len && strncmp(path.data(), sourceRoot.c_str(), len) == 0) {
      return path.substr(len);
    }
    if (root && rootLen && strncmp(path.data(), root, rootLen) == 0) {
      return path.substr(rootLen);
    }
  }
  return path;
}

static std::string systemlib_split(const std::string& slib, std::string* hhas) {
  auto pos = slib.find("\n<?hhas\n");
  if (pos != std::string::npos) {
    if (hhas) *hhas = slib.substr(pos + 8);
    return slib.substr(0, pos);
  }
  return slib;
}

// Retrieve a systemlib (or mini systemlib) from the
// current executable or another ELF object file.
//
// Additionally, when retrieving the main systemlib
// from the current executable, honor the
// HHVM_SYSTEMLIB environment variable as an override.
std::string get_systemlib(std::string* hhas,
                          const std::string &section /*= "systemlib" */,
                          const std::string &filename /*= "" */) {
  if (filename.empty() && section == "systemlib") {
    if (auto const file = getenv("HHVM_SYSTEMLIB")) {
      std::ifstream ifs(file);
      if (ifs.good()) {
        return systemlib_split(std::string(
                                 std::istreambuf_iterator<char>(ifs),
                                 std::istreambuf_iterator<char>()), hhas);
      }
    }
  }

  embedded_data desc;
  if (!get_embedded_data(section.c_str(), &desc, filename)) return "";

  auto const data = read_embedded_data(desc);
  return systemlib_split(data, hhas);
}

///////////////////////////////////////////////////////////////////////////////
// C++ ffi

#ifndef _MSC_VER
static void on_timeout(int sig, siginfo_t* info, void* /*context*/) {
  if (sig == SIGVTALRM && info && info->si_code == SI_TIMER) {
    auto data = (RequestTimer*)info->si_value.sival_ptr;
    if (data) {
      data->onTimeout();
    } else {
      Xenon::getInstance().onTimer();
    }
  }
}
#endif

/*
 * Update constants to their real values and sync some runtime options
 */
static void update_constants_and_options() {
  assert(ExtensionRegistry::modulesInitialised());
  // If extension constants were used in the ini files (e.g., E_ALL) they
  // would have come out as 0 in the previous pass until we load and
  // initialize our extensions, which we do in RuntimeOption::Load() via
  // ExtensionRegistry::ModuleLoad() and in ExtensionRegistry::ModuleInit()
  // in hphp_process_init(). We will re-import and set only the constants that
  // have been now bound to their proper value.
  IniSettingMap ini = IniSettingMap();
  for (auto& filename: s_config_files) {
    SuppressHackArrCompatNotices shacn;
    Config::ParseIniFile(filename, ini, true);
  }
  // Reset the INI settings from the CLI.
  for (auto& iniStr: s_ini_strings) {
    SuppressHackArrCompatNotices shacn;
    Config::ParseIniString(iniStr, ini, true);
  }

  // Reset, possibly, some request dependent runtime options based on certain
  // setting values. Do this here so we ensure the constants have been loaded
  // correctly (e.g., error_reporting E_ALL, etc.)
  Variant sys;
  if (IniSetting::GetSystem("error_reporting", sys)) {
    RuntimeOption::RuntimeErrorReportingLevel = sys.toInt64();
    RID().setErrorReportingLevel(RuntimeOption::RuntimeErrorReportingLevel);
  }
  if (IniSetting::GetSystem("memory_limit", sys)) {
    RID().setMemoryLimit(sys.toString().toCppString());
    RuntimeOption::RequestMemoryMaxBytes = RID().getMemoryLimitNumeric();
  }
}

void hphp_thread_init() {
#ifdef USE_JEMALLOC_CUSTOM_HOOKS
  thread_huge_tcache_create();
#endif
  ServerStats::GetLogger();
  zend_get_bigint_data();
  zend_rand_init();
  get_server_note();
  MemoryManager::TlsWrapper::getCheck();

  assert(ThreadInfo::s_threadInfo.isNull());
  ThreadInfo::s_threadInfo.getCheck()->init();

  HardwareCounter::s_counter.getCheck();
  ExtensionRegistry::threadInit();
  InitFiniNode::ThreadInit();

  // ensure that there's no request-allocated memory
  hphp_memory_cleanup();
}

void hphp_thread_exit() {
  InitFiniNode::ThreadFini();
  ExtensionRegistry::threadShutdown();
  if (!g_context.isNull()) g_context.destroy();
#ifdef USE_JEMALLOC_CUSTOM_HOOKS
  thread_huge_tcache_destroy();
#endif
}

void hphp_process_init() {
  pthread_attr_t attr;
// Linux+GNU extension
#if defined(_GNU_SOURCE) && defined(__linux__)
  if (pthread_getattr_np(pthread_self(), &attr) != 0 ) {
    Logger::Error("pthread_getattr_np failed before checking stack limits");
    _exit(1);
  }
#else
  if (pthread_attr_init(&attr) != 0 ) {
    Logger::Error("pthread_attr_init failed before checking stack limits");
    _exit(1);
  }
#endif
  init_stack_limits(&attr);
  if (pthread_attr_destroy(&attr) != 0 ) {
    Logger::Error("pthread_attr_destroy failed after checking stack limits");
    _exit(1);
  }
  BootStats::mark("pthread_init");

  Process::InitProcessStatics();
  BootStats::mark("Process::InitProcessStatics");

  HHProf::Init();

  {
    (void)type_scan::getIndexForMalloc<MArrayIter>();
    MIterTable::TlsWrapper tls;
  }

  // initialize the tzinfo cache.
  timezone_init();
  BootStats::mark("timezone_init");

  hphp_thread_init();

#ifndef _MSC_VER
  struct sigaction action = {};
  action.sa_sigaction = on_timeout;
  action.sa_flags = SA_SIGINFO | SA_NODEFER;
  sigaction(SIGVTALRM, &action, nullptr);
#endif
  // start takes milliseconds, Period is a double in seconds
  Xenon::getInstance().start(1000 * RuntimeOption::XenonPeriodSeconds);
  BootStats::mark("xenon");

  // reinitialize pcre table
  pcre_reinit();
  BootStats::mark("pcre_reinit");

  // the liboniguruma docs say this isnt needed,
  // but the implementation of init is not
  // thread safe due to bugs
  onig_init();
  BootStats::mark("onig_init");

  // simple xml also needs one time init
  xmlInitParser();
  BootStats::mark("xmlInitParser");

  g_context.getCheck();
  // Some event handlers are registered during the startup process.
  g_context->acceptRequestEventHandlers(true);
  InitFiniNode::ProcessPreInit();
  // TODO(9795696): Race in thread map may trigger spurious logging at
  // thread exit, so for now, only spawn threads if we're a server.
  const uint32_t maxWorkers = RuntimeOption::ServerExecutionMode() ? 3 : 0;
  folly::SingletonVault::singleton()->registrationComplete();
  InitFiniNode::ProcessInitConcurrentStart(maxWorkers);
  SCOPE_EXIT {
    InitFiniNode::ProcessInitConcurrentWaitForEnd();
    BootStats::mark("extra_process_init_concurrent_wait");
  };
  g_vmProcessInit();
  BootStats::mark("g_vmProcessInit");

  PageletServer::Restart();
  BootStats::mark("PageletServer::Restart");
  XboxServer::Restart();
  BootStats::mark("XboxServer::Restart");
  Stream::RegisterCoreWrappers();
  BootStats::mark("Stream::RegisterCoreWrappers");
  ExtensionRegistry::moduleInit();
  BootStats::mark("ExtensionRegistry::moduleInit");

  // Now that constants have been bound we can update options using constants
  // in ini files (e.g., E_ALL) and sync some other options
  update_constants_and_options();

  InitFiniNode::ProcessInit();
  BootStats::mark("extra_process_init");
  {
    UnlimitSerializationScope unlimit;
    // TODO(9755792): Add real execution mode for snapshot generation.
    if (apcExtension::PrimeLibraryUpgradeDest != "") {
      Timer timer(Timer::WallTime, "optimizeApcPrime");
      apc_load(apcExtension::LoadThread);
    } else {
      apc_load(apcExtension::LoadThread);
    }
    BootStats::mark("apc_load");
  }

  rds::requestExit();
  BootStats::mark("rds::requestExit");
  // Reset the preloaded g_context
  ExecutionContext *context = g_context.getNoCheck();
  context->onRequestShutdown(); // TODO T20898959 kill early REH usage.
  context->~ExecutionContext();
  new (context) ExecutionContext();
  BootStats::mark("ExecutionContext");

  // TODO(9755792): Add real execution mode for snapshot generation.
  if (apcExtension::PrimeLibraryUpgradeDest != "") {
    Logger::Info("APC PrimeLibrary upgrade mode completed; exiting.");
    hphp_process_exit();
    exit(0);
  }
}

static void handle_exception(bool& ret, ExecutionContext* context,
                             std::string& errorMsg, ContextOfException where,
                             bool& error, bool richErrorMsg) {
  assert(where == ContextOfException::Invoke ||
         where == ContextOfException::ReqInit);
  try {
    handle_exception_helper(ret, context, errorMsg, where, error, richErrorMsg);
  } catch (const ExitException &e) {
    // Got an ExitException during exception handling, handle
    // similarly to the case below but don't call obEndAll().
  } catch (...) {
    handle_exception_helper(ret, context, errorMsg, ContextOfException::Handler,
                            error, richErrorMsg);
    context->obEndAll();
  }
}

static void handle_reqinit_exception(bool &ret, ExecutionContext *context,
                                     std::string &errorMsg, bool &error) {
  handle_exception(ret, context, errorMsg, ContextOfException::ReqInit, error,
                   false);
}

static void handle_invoke_exception(bool &ret, ExecutionContext *context,
                                    std::string &errorMsg, bool &error,
                                    bool richErrorMsg) {
  handle_exception(ret, context, errorMsg, ContextOfException::Invoke, error,
                   richErrorMsg);
}

static bool hphp_warmup(ExecutionContext *context,
                        const std::string &reqInitFunc,
                        const std::string &reqInitDoc, bool &error) {
  bool ret = true;
  error = false;
  std::string errorMsg;

  ServerStatsHelper ssh("reqinit");
  try {
    if (!reqInitDoc.empty()) {
      include_impl_invoke(reqInitDoc, true);
    }
    if (!reqInitFunc.empty()) {
      invoke(reqInitFunc.c_str(), Array());
    }
    context->backupSession();
  } catch (...) {
    handle_reqinit_exception(ret, context, errorMsg, error);
  }

  return ret;
}

void hphp_session_init() {
  assert(!s_sessionInitialized);
  g_context.getCheck();
  AsioSession::Init();
  Socket::clearLastError();
  TI().onSessionInit();
  MM().resetExternalStats();

  g_thread_safe_locale_handler->reset();
  Treadmill::startRequest();

#ifdef ENABLE_SIMPLE_COUNTER
  SimpleCounter::Enabled = true;
  StackTrace::Enabled = true;
#endif

  // Ordering is sensitive; StatCache::requestInit produces work that
  // must be done in ExecutionContext::requestInit.
  StatCache::requestInit();

  // Allow request event handlers to be created now that a new request has
  // started.
  g_context->acceptRequestEventHandlers(true);

  g_context->requestInit();
  s_sessionInitialized = true;
  ExtensionRegistry::requestInit();

  // Sample function calls for this request
  if (RID().logFunctionCalls()) {
    EventHook::Enable();
  }

  auto const pme_freq = RuntimeOption::EvalPerfMemEventRequestFreq;
  if (pme_freq > 0 && folly::Random::rand32(pme_freq) == 0) {
    // Enable memory access sampling for this request.
    perf_event_enable(
      RuntimeOption::EvalPerfMemEventSampleFreq,
      [] (PerfEvent) { setSurpriseFlag(PendingPerfEventFlag); }
    );
  }
}

bool hphp_invoke_simple(const std::string& filename, bool warmupOnly) {
  bool error;
  std::string errorMsg;
  return hphp_invoke(g_context.getNoCheck(), filename, false, null_array,
                     uninit_null(), "", "", error, errorMsg,
                     true /* once */,
                     warmupOnly,
                     false /* richErrorMsg */);
}

bool hphp_invoke(ExecutionContext *context, const std::string &cmd,
                 bool func, const Array& funcParams, VRefParam funcRet,
                 const std::string &reqInitFunc, const std::string &reqInitDoc,
                 bool &error, std::string &errorMsg,
                 bool once, bool warmupOnly,
                 bool richErrorMsg) {
  bool isServer =
    RuntimeOption::ServerExecutionMode() && !is_cli_mode();
  error = false;

  // Make sure we have the right current working directory within the repo
  // based on what server.source_root was set to (current process directory
  // being the default)
  if (RuntimeOption::RepoAuthoritative) {
    context->setCwd(RuntimeOption::SourceRoot);
  }

  String oldCwd;
  if (isServer) {
    oldCwd = context->getCwd();
  }
  if (!hphp_warmup(context, reqInitFunc, reqInitDoc, error)) {
    if (isServer) context->setCwd(oldCwd);
    return false;
  }

  MM().resetCouldOOM(isStandardRequest());
  RID().resetTimer();

  bool ret = true;
  if (!warmupOnly) {
    try {
      ServerStatsHelper ssh("invoke");
      if (!RuntimeOption::AutoPrependFile.empty() &&
          RuntimeOption::AutoPrependFile != "none") {
        require(RuntimeOption::AutoPrependFile, false,
                context->getCwd().data(), true);
      }
      if (func) {
        funcRet.assignIfRef(invoke(cmd.c_str(), funcParams));
      } else {
        if (isServer) hphp_chdir_file(cmd);
        include_impl_invoke(cmd.c_str(), once);
      }
      if (!RuntimeOption::AutoAppendFile.empty() &&
          RuntimeOption::AutoAppendFile != "none") {
        require(RuntimeOption::AutoAppendFile, false,
                context->getCwd().data(), true);
      }
    } catch (...) {
      handle_invoke_exception(ret, context, errorMsg, error, richErrorMsg);
    }
  }

  try {
    context->onShutdownPreSend();
  } catch (...) {
    handle_invoke_exception(ret, context, errorMsg, error, richErrorMsg);
  }

  if (isServer) context->setCwd(oldCwd);
  return ret;
}

void hphp_context_shutdown() {
  // Run shutdown handlers. This may cause user code to run.
  g_thread_safe_locale_handler->reset();

  auto const context = g_context.getNoCheck();
  context->destructObjects();
  context->onRequestShutdown();

  try {
    // Shutdown the debugger.  This can throw, but we don't care about what the
    // error is.
    DEBUGGER_ATTACHED_ONLY(phpDebuggerRequestShutdownHook());
  } catch (...) {
    // Gotta catch 'em all!
  }

  // Extensions could have shutdown handlers
  ExtensionRegistry::requestShutdown();
  InitFiniNode::RequestFini();

  // Extension shutdown could have re-initialized some
  // request locals
  context->onRequestShutdown();

  // This causes request event handler registration to fail until the next
  // request starts.
  context->acceptRequestEventHandlers(false);
}

void hphp_context_exit(bool shutdown /* = true */) {
  if (shutdown) {
    hphp_context_shutdown();
  }

  // Clean up a bunch of request state. No user code after this point.
  MemoryManager::setExiting();
  auto const context = g_context.getNoCheck();
  context->requestExit();
  context->obProtect(false);
  context->obEndAll();
}

void hphp_memory_cleanup() {
  auto& mm = MM();
  // sweep functions are allowed to access g_context,
  // so we can't destroy it yet
  mm.sweep();

  // We should never have any registered RequestEventHandlers. If we do
  // something after onRequestShutdown registered a RequestEventHandler.
  // Its now too late to run the requestShutdown functions, but if we carry
  // on, requestInit and requestShutdown will never be called again.
  // I considered just clearing the inited flags; which works for some
  // RequestEventHandlers - but its a disaster for others. So just fail hard
  // here.
  always_assert(g_context.isNull() || !g_context->hasRequestEventHandlers());

  // g_context is request allocated, and has some members that need
  // cleanup, so destroy it before its too late
  g_context.destroy();

  weakref_cleanup();
  mm.resetAllocator();
  mm.resetCouldOOM();
}

void hphp_session_exit(const Transport* transport) {
  assert(s_sessionInitialized);
  // Server note and INI have to live long enough for the access log to fire.
  // RequestLocal is too early.
  ServerNote::Reset();
  IniSetting::ResetSavedDefaults();
  // In JitPGO mode, check if it's time to schedule the retranslation of all
  // profiled functions and, if so, schedule it.
  jit::mcgen::checkRetranslateAll();
  jit::tc::requestExit();
  // Similarly, apc strings could be in the ServerNote array, and
  // it's possible they are scheduled to be destroyed after this request
  // finishes.
  Treadmill::finishRequest();

  TI().onSessionExit();

  if (transport) {
    std::unique_ptr<StructuredLogEntry> entry;
    if (RuntimeOption::EvalProfileHWStructLog) {
      entry = std::make_unique<StructuredLogEntry>();
      entry->setInt("response_code", transport->getResponseCode());
    }
    HardwareCounter::UpdateServiceData(transport->getCpuTime(),
                                       transport->getWallTime(),
                                       entry.get(),
                                       true /*psp*/);
    if (entry) StructuredLog::log("hhvm_request_perf", *entry);
  }

  // We might have events from after the final surprise flag check of the
  // request, so consume them here.
  perf_event_consume(record_perf_mem_event);
  perf_event_disable();

  {
    ServerStatsHelper ssh("rollback");

    hphp_memory_cleanup();
  }

  assert(MM().empty());

  s_sessionInitialized = false;
  s_extra_request_microseconds = 0;
}

void hphp_process_exit() noexcept {
  // We want to do clean up on a best-effort basis: don't skip later steps if
  // an earlier step fails, and don't propagate exceptions ouf of this function
#define LOG_AND_IGNORE(voidexpr) try { voidexpr; } catch (...) { \
    Logger::Error("got exception in cleanup step: " #voidexpr); }
  LOG_AND_IGNORE(teardown_cli_server())
  LOG_AND_IGNORE(Xenon::getInstance().stop())
  LOG_AND_IGNORE(jit::mcgen::joinWorkerThreads())
  LOG_AND_IGNORE(jit::tc::processExit())
  LOG_AND_IGNORE(PageletServer::Stop())
  LOG_AND_IGNORE(XboxServer::Stop())
  // Debugger::Stop() needs an execution context
  LOG_AND_IGNORE(g_context.getCheck())
  LOG_AND_IGNORE(Eval::Debugger::Stop())
  LOG_AND_IGNORE(g_context.destroy())
  LOG_AND_IGNORE(ExtensionRegistry::moduleShutdown())
  LOG_AND_IGNORE(compilers_shutdown())
#ifndef _MSC_VER
  LOG_AND_IGNORE(LightProcess::Close())
#endif
  LOG_AND_IGNORE(InitFiniNode::ProcessFini())
  LOG_AND_IGNORE(folly::SingletonVault::singleton()->destroyInstances())
  LOG_AND_IGNORE(embedded_data_cleanup())
#undef LOG_AND_IGNORE
}

bool is_hphp_session_initialized() {
  return s_sessionInitialized;
}

static struct SetThreadInitFini {
  template<class ThreadT> static typename std::enable_if<
    std::is_integral<ThreadT>::value || std::is_pointer<ThreadT>::value>::type
  recordThreadAddr(ThreadT threadId, char* stackAddr, size_t stackSize) {
    // In the current glibc implementation, pthread_t is a 64-bit unsigned
    // integer, whose value equals the address of the thread control block
    // (TCB).  In x64_64, this is right above the TLS block.  In addition,
    // TLS and TCB sits at the high end of the stack, i.e.,
    //
    // stackAddr + stackSize ----> +---------------+
    //                             | TCB           |
    //              threadId ----> +---------------+
    //                             | TLS           |
    //                             +---------------+
    //                             | Stack         |
    //                             .               .
    //                             .               .
    //             stackAddr ----> +---------------+
    auto const tcbBase = reinterpret_cast<char*>(threadId);
    auto stackEnd = stackAddr + stackSize;
    if (tcbBase > stackAddr && tcbBase < stackEnd) { // the expected layout
      // TCB
      Debug::DebugInfo::recordDataMap(
        tcbBase, stackEnd,
        folly::sformat("Thread-{}", static_cast<void*>(tcbBase)));
      // TLS
      auto const tlsRange = getCppTdata();
      auto const tlsSize = (tlsRange.second + 15) / 16 * 16;
      stackEnd = tcbBase - tlsSize;
      if (tlsSize) {
        Debug::DebugInfo::recordDataMap(
          stackEnd, tcbBase,
          folly::sformat("TLS-{}", static_cast<void*>(tcbBase)));
      }
    }
    Debug::DebugInfo::recordDataMap(
      stackAddr, stackEnd,
      folly::sformat("Stack-{}", static_cast<void*>(tcbBase)));
  }
  template <class ThreadT>
  static typename std::enable_if<!std::is_integral<ThreadT>::value &&
                                 !std::is_pointer<ThreadT>::value>::type
  recordThreadAddr(ThreadT /*threadId*/, char* stackAddr, size_t stackSize) {
    // pthread_t is not an integer or pointer to TCB in this pthread
    // implementation.  But we can still figure out where TLS is.
    auto const tlsRange = getCppTdata();
    auto const tlsSize = (tlsRange.second + 15) / 16 * 16;
    auto const tlsBaseAddr = reinterpret_cast<char*>(tlsBase());
    Debug::DebugInfo::recordDataMap(
      tlsBaseAddr, tlsBaseAddr + tlsSize,
      folly::sformat("TLS-{}", static_cast<void*>(stackAddr)));
    Debug::DebugInfo::recordDataMap(
      stackAddr, stackAddr + stackSize,
      folly::sformat("Stack-{}", static_cast<void*>(stackAddr)));
  }

  SetThreadInitFini() {
    AsyncFuncImpl::SetThreadInitFunc(
      [] (void*) {
#if defined(_GNU_SOURCE) && defined(__linux__)
        if (RuntimeOption::EvalPerfDataMap) {
          pthread_t threadId = pthread_self();
          pthread_attr_t attr;
          pthread_getattr_np(threadId, &attr);
          void* stackAddr{nullptr};
          size_t stackSize{0};
          pthread_attr_getstack(&attr, &stackAddr, &stackSize);
          pthread_attr_destroy(&attr);
          recordThreadAddr(threadId, static_cast<char*>(stackAddr), stackSize);
        }
#endif
        hphp_thread_init();
      },
      nullptr);
    AsyncFuncImpl::SetThreadFiniFunc([](void*) { hphp_thread_exit(); },
                                     nullptr);
  }
} s_SetThreadInitFini;

///////////////////////////////////////////////////////////////////////////////
}
