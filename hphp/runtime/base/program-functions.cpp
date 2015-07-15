/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/pprof-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/simple-counter.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/thread-safe-setlocale.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/debugger/debugger_hook_handler.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/xhprof/ext_xhprof.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"
#include "hphp/runtime/server/admin-request-handler.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/runtime/server/rpc-request-handler.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/replay-transport.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/system/constants.h"
#include "hphp/util/code-cache.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/capability.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/embedded-data.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/light-process.h"
#include "hphp/util/process.h"
#include "hphp/util/repo-schema.h"
#include "hphp/util/service-data.h"
#include "hphp/util/shm-counter.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/timer.h"

#include <folly/Portability.h>
#include <folly/Singleton.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <libgen.h>
#include <oniguruma.h>
#include <signal.h>
#include <libxml/parser.h>
#include <exception>
#include <iterator>
#include <map>
#include <memory>
#include <vector>


#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
#include <windows.h>
#include <winuser.h>
#endif

using namespace boost::program_options;
using std::cout;
extern char **environ;

#define MAX_INPUT_NESTING_LEVEL 64

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Forward declarations.

extern InitFiniNode *extra_process_init, *extra_process_exit;

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
  string     mode;
  std::vector<std::string>
             config;
  std::vector<std::string>
             confStrings;
  std::vector<std::string>
             iniStrings;
  int        port;
  int        portfd;
  int        sslportfd;
  int        admin_port;
  string     user;
  string     file;
  string     lint;
  bool       isTempFile;
  int        count;
  bool       noSafeAccessCheck;
  std::vector<std::string>
             args;
  string     buildId;
  string     instanceId;
  int        xhprofFlags;
  string     show;
  string     parse;

  Eval::DebuggerClientOptions debugger_options;
};

class StartTime {
public:
  StartTime() : startTime(time(nullptr)) {}
  time_t startTime;
};
static StartTime s_startTime;
static string tempFile;

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

String k_PHP_BINARY;
String k_PHP_BINDIR;
String k_PHP_OS;
String k_PHP_SAPI;

static __thread bool s_sessionInitialized{false};

static void process_cmd_arguments(int argc, char **argv) {
  php_global_set(s_argc, Variant(argc));
  Array argvArray(staticEmptyArray());
  for (int i = 0; i < argc; i++) {
    argvArray.append(String(argv[i]));
  }
  php_global_set(s_argv, argvArray);
}

void process_env_variables(Array& variables) {
  for (auto& kv : RuntimeOption::EnvVariables) {
    variables.set(String(kv.first), String(kv.second));
  }
  for (char **env = environ; env && *env; env++) {
    char *p = strchr(*env, '=');
    if (p) {
      String name(*env, p - *env, CopyString);
      register_variable(variables, (char*)name.data(),
                        String(p + 1, CopyString));
    }
  }
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
        Variant v = symtable->rvalAt(key);
        if (v.isNull() || !v.is(KindOfArray)) {
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
      static auto requestTimeoutPSPCounter = ServiceData::createTimeseries(
        "requests_timed_out_psp", {ServiceData::StatsType::COUNT});
      requestTimeoutPSPCounter->addValue(1);
      ServerStats::Log("request.timed_out.psp", 1);
    } else {
      static auto requestTimeoutCounter = ServiceData::createTimeseries(
        "requests_timed_out_non_psp", {ServiceData::StatsType::COUNT});
      requestTimeoutCounter->addValue(1);
      ServerStats::Log("request.timed_out.non_psp", 1);
    }
    throw;
  } catch (const RequestCPUTimeoutException& e) {
    if (isPsp) {
      static auto requestCPUTimeoutPSPCounter = ServiceData::createTimeseries(
        "requests_cpu_timed_out_psp", {ServiceData::StatsType::COUNT});
      requestCPUTimeoutPSPCounter->addValue(1);
      ServerStats::Log("request.cpu_timed_out.psp", 1);
    } else {
      static auto requestCPUTimeoutCounter = ServiceData::createTimeseries(
        "requests_cpu_timed_out_non_psp", {ServiceData::StatsType::COUNT});
      requestCPUTimeoutCounter->addValue(1);
      ServerStats::Log("request.cpu_timed_out.non_psp", 1);
    }
    throw;
  } catch (const RequestMemoryExceededException& e) {
    if (isPsp) {
      static auto requestMemoryExceededPSPCounter =
        ServiceData::createTimeseries(
          "requests_memory_exceeded_psp", {ServiceData::StatsType::COUNT});
      requestMemoryExceededPSPCounter->addValue(1);
      ServerStats::Log("request.memory_exceeded.psp", 1);
    } else {
      static auto requestMemoryExceededCounter = ServiceData::createTimeseries(
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
  auto const origFlags = flags.load() & ResourceFlags;
  flags.fetch_and(~ResourceFlags);

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
    std::string origErrorMsg = errorMsg;
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

static bool hphp_chdir_file(const string filename) {
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
  string errorMsg;

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

void execute_command_line_begin(int argc, char **argv, int xhprof,
                                const std::vector<std::string>& config) {
  StackTraceNoHeap::AddExtraLogging("ThreadType", "CLI");
  string args;
  for (int i = 0; i < argc; i++) {
    if (i) args += " ";
    args += argv[i];
  }
  StackTraceNoHeap::AddExtraLogging("Arguments", args.c_str());

  hphp_session_init();
  auto const context = g_context.getNoCheck();
  context->obSetImplicitFlush(true);

  auto& variablesOrder = RID().getVariablesOrder();

  if (variablesOrder.find('e') != std::string::npos ||
      variablesOrder.find('E') != std::string::npos) {
    Array envArr(Array::Create());
    process_env_variables(envArr);
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
    }
    php_global_set(s__ENV, envArr);
  }

  process_cmd_arguments(argc, argv);

  if (variablesOrder.find('s') != std::string::npos ||
      variablesOrder.find('S') != std::string::npos) {
    Array serverArr(Array::Create());
    process_env_variables(serverArr);
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
        !gethostname(hostname, sizeof(hostname))) {
      // gethostname may not null-terminate
      hostname[sizeof(hostname) - 1] = '\0';
      serverArr.set(s_HOSTNAME, String(hostname, CopyString));
    }

    for (auto& kv : RuntimeOption::ServerVariables) {
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

  ExtensionRegistry::requestInit();
  // If extension constants were used in the ini files, they would have come
  // out as 0 in the previous pass. We will re-import only the constants that
  // have been later bound. All other non-constant configs should remain as they
  // are, even if the ini file actually tries to change them.
  IniSetting::Map ini = IniSetting::Map::object;
  for (auto& filename: config) {
    Config::ParseIniFile(filename, ini, true);
  }
  // Initialize the debugger
  DEBUGGER_ATTACHED_ONLY(phpDebuggerRequestInitHook());
}

void execute_command_line_end(int xhprof, bool coverage, const char *program) {
  MM().collect();
  if (RuntimeOption::EvalDumpTC || RuntimeOption::EvalDumpIR) {
    HPHP::jit::tc_dump();
  }
  if (xhprof) {
    Variant profileData = HHVM_FN(xhprof_disable)();
    if (!profileData.isNull()) {
      HHVM_FN(var_dump)(HHVM_FN(json_encode)(HHVM_FN(xhprof_disable)()));
    }
  }
  g_context->onShutdownPostSend();
  Eval::Debugger::InterruptPSPEnded(program);
  hphp_context_exit();
  hphp_session_exit();
  auto& ti = TI();
  if (coverage && ti.m_reqInjectionData.getCoverage() &&
      !RuntimeOption::CodeCoverageOutputFile.empty()) {
    ti.m_coverage->Report(RuntimeOption::CodeCoverageOutputFile);
  }
}

#if FACEBOOK && defined USE_SSECRC
// Overwrite the functiosn
NEVER_INLINE void copyFunc(void* dst, void* src, uint32_t sz = 64) {
  if (dst >= reinterpret_cast<void*>(__hot_start) &&
      dst < reinterpret_cast<void*>(__hot_end)) {
    memcpy(dst, src, sz);
    Logger::Info("Successfully overwrite function at %p.", dst);
  } else {
    Logger::Info("Failed to patch code at %p.", dst);
  }
}

NEVER_INLINE void copyHashFuncs() {
#ifdef __OPTIMIZE__
  if (IsSSEHashSupported()) {
    copyFunc(getMethodPtr(&HPHP::StringData::hashHelper),
             reinterpret_cast<void*>(g_hashHelper_crc), 64);
    typedef strhash_t (*HashFunc) (const char*, uint32_t);
    auto hash_func = [](HashFunc x) {
      return reinterpret_cast<void*>(x);
    };
    copyFunc(hash_func(hash_string_cs_unsafe),
             hash_func(hash_string_cs_crc), 48);
    copyFunc(hash_func(hash_string_cs),
             hash_func(hash_string_cs_unaligned_crc), 64);
    copyFunc(hash_func(hash_string_i_unsafe),
             hash_func(hash_string_i_crc), 64);
    copyFunc(hash_func(hash_string_i),
             hash_func(hash_string_i_unaligned_crc), 80);
  }
#endif
}
#endif

#if FACEBOOK
# define AT_END_OF_TEXT       __attribute__((__section__(".stub")))
#else
# define AT_END_OF_TEXT
#endif

static void NEVER_INLINE AT_END_OF_TEXT __attribute__((__optimize__("2")))
hugifyText(char* from, char* to) {
#if FACEBOOK && !defined FOLLY_SANITIZE_ADDRESS && defined MADV_HUGEPAGE
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
  // When supported, string hash functions using SSE 4.2 CRC32 instruction will
  // be used, so we don't have to check every time.
#ifdef USE_SSECRC
  copyHashFuncs();
#endif
  mprotect(from, sz, PROT_READ | PROT_EXEC);
  free(mem);
  mlock(from, to - from);
  Debug::DebugInfo::setPidMapOverlay(from, to);
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

  Timer timer(Timer::WallTime, "mapping self");
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
          if (to < (void*)hugifyText) {
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

/* Sets RuntimeOption::ExecutionMode according
 * to commandline options prior to config load
 */
static void set_execution_mode(string mode) {
  if (mode == "daemon" || mode == "server" || mode == "replay") {
    RuntimeOption::ExecutionMode = "srv";
    Logger::Escape = true;
  } else if (mode == "run" || mode == "debug") {
    RuntimeOption::ExecutionMode = "cli";
    Logger::Escape = false;
  } else if (mode == "translate") {
    RuntimeOption::ExecutionMode = "";
    Logger::Escape = false;
  } else {
    // Undefined mode
    always_assert(false);
  }
}

static int start_server(const std::string &username, int xhprof) {
  // Before we start the webserver, make sure the entire
  // binary is paged into memory.
  pagein_self();

  set_execution_mode("server");
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

#if !defined(SKIP_USER_CHANGE)
  if (!username.empty()) {
    if (Logger::UseCronolog) {
      Cronolog::changeOwner(username, RuntimeOption::LogFileSymLink);
    }
    Capability::ChangeUnixUser(username);
    LightProcess::ChangeUser(username);
  }
  Capability::SetDumpable();
#endif

  // Create the HttpServer before any warmup requests to properly
  // initialize the process
  HttpServer::Server = std::make_shared<HttpServer>();

  if (xhprof) {
    HHVM_FN(xhprof_enable)(xhprof, uninit_null().toArray());
  }

  if (RuntimeOption::RepoPreload) {
    Timer timer(Timer::WallTime, "Preloading Repo");
    profileWarmupStart();
    preloadRepo();
    profileWarmupEnd();
  }

  // If we have any warmup requests, replay them before listening for
  // real connections
  {
    if (!RuntimeOption::EvalJitProfileWarmupRequests) profileWarmupStart();
    SCOPE_EXIT { profileWarmupEnd(); };
    for (auto& file : RuntimeOption::ServerWarmupRequests) {
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

  if (RuntimeOption::EvalEnableNuma) {
#ifdef USE_JEMALLOC
    mallctl("arenas.purge", nullptr, nullptr, nullptr, 0);
#endif
    enable_numa(RuntimeOption::EvalEnableNumaLocal);
  }

  HttpServer::Server->runOrExitProcess();
  HttpServer::Server.reset();
  return 0;
}

string translate_stack(const char *hexencoded, bool with_frame_numbers) {
  if (!hexencoded || !*hexencoded) {
    return "";
  }

  StackTrace st(hexencoded);
  std::vector<std::shared_ptr<StackTrace::Frame>> frames;
  st.get(frames);

  std::ostringstream out;
  for (unsigned int i = 0; i < frames.size(); i++) {
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
    initialize_repo();
    ret_code = execute_program_impl(argc, argv);
  } catch (const Exception &e) {
    Logger::Error("Uncaught exception: %s", e.what());
  } catch (const FailedAssertion& fa) {
    fa.print();
    StackTraceNoHeap::AddExtraLogging("Assertion failure", fa.summary);
    abort();
  } catch (const std::exception &e) {
    Logger::Error("Uncaught exception: %s", e.what());
  } catch (...) {
    Logger::Error("Uncaught exception: (unknown)");
  }
  if (tempFile.length() && boost::filesystem::exists(tempFile)) {
    boost::filesystem::remove(tempFile);
  }
  return ret_code;
}

/* -1 - cannot open file
 * 0  - no need to open file
 * 1 - fopen
 * 2 - popen
 */
static int open_server_log_file() {
  if (!RuntimeOption::LogFile.empty()) {
    if (Logger::UseCronolog) {
      if (strchr(RuntimeOption::LogFile.c_str(), '%')) {
        Logger::cronOutput.m_template = RuntimeOption::LogFile;
        Logger::cronOutput.setPeriodicity();
        Logger::cronOutput.m_linkName = RuntimeOption::LogFileSymLink;
        return 0;
      } else {
        Logger::Output = fopen(RuntimeOption::LogFile.c_str(), "a");
        if (Logger::Output) return 1;
      }
    } else {
      if (Logger::IsPipeOutput) {
        Logger::Output = popen(RuntimeOption::LogFile.substr(1).c_str(), "w");
        if (Logger::Output) return 2;
      } else {
        Logger::Output = fopen(RuntimeOption::LogFile.c_str(), "a");
        if (Logger::Output) return 1;
      }
    }
    Logger::Error("Cannot open log file: %s", RuntimeOption::LogFile.c_str());
    return -1;
  }
  return 0;
}

static void close_server_log_file(int kind) {
  if (kind == 1) {
    fclose(Logger::Output);
  } else if (kind == 2) {
    pclose(Logger::Output);
  } else {
    always_assert(!Logger::Output);
  }
}

static int compute_hhvm_argc(const options_description& desc,
                             int argc, char** argv) {
  enum ArgCode {
    NO_ARG = 0,
    ARG_REQUIRED = 1,
    ARG_OPTIONAL = 2
  };
  const auto& vec = desc.options();
  std::map<string,ArgCode> long_options;
  std::map<string,ArgCode> short_options;
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
      string s(str+2);
      auto it = long_options.find(s);
      if (it != long_options.end() && it->second != NO_ARG && pos < argc &&
          (it->second == ARG_REQUIRED || argv[pos][0] != '-')) {
        ++pos;
      }
    } else if (len >= 2 && str[0] == '-') {
      // Handle short options
      ++pos;
      string s;
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
 * AsyncFuncImpl defines a minimum C++ stack size but that only applies to
 * threads we manually create. When the main thread will be executing PHP
 * rather than just managing a server, make sure its stack is big enough.
 */
static void set_stack_size() {
  struct rlimit rlim;
  if (getrlimit(RLIMIT_STACK, &rlim) != 0) return;

  if (rlim.rlim_cur < AsyncFuncImpl::kStackSizeMinimum
#ifndef __CYGWIN__
      || rlim.rlim_cur == RLIM_INFINITY
#endif
      ) {
#ifdef __CYGWIN__
    Logger::Error("stack limit too small, use peflags -x to increase  %zd\n",
                  AsyncFuncImpl::kStackSizeMinimum);
#else
    rlim.rlim_cur = AsyncFuncImpl::kStackSizeMinimum;
    if (setrlimit(RLIMIT_STACK, &rlim)) {
      Logger::Error("failed to set stack limit to %zd\n",
                    AsyncFuncImpl::kStackSizeMinimum);
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
extern "C" {
  void zend_startup_strtod(void);
}

static int execute_program_impl(int argc, char** argv) {
  string usage = "Usage:\n\n   ";
  usage += argv[0];
  usage += " [-m <mode>] [<options>] [<arg1>] [<arg2>] ...\n\nOptions";

  ProgramOptions po;
  options_description desc(usage.c_str());
  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("php", "emulate the standard php command line")
    ("compiler-id", "display the git hash for the compiler")
    ("repo-schema", "display the repository schema id")
    ("mode,m", value<string>(&po.mode)->default_value("run"),
     "run | debug (d) | server (s) | daemon | replay | translate (t)")
    ("interactive,a", "Shortcut for --mode debug") // -a is from PHP5
    ("config,c", value<vector<string> >(&po.config)->composing(),
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
    ("debug-config", value<string>(&po.debugger_options.configFName),
      "load specified debugger config file")
    ("debug-host,h",
     value<string>(&po.debugger_options.host)->implicit_value("localhost"),
     "connect to debugger server at specified address")
    ("debug-port", value<int>(&po.debugger_options.port)->default_value(-1),
     "connect to debugger server at specified port")
    ("debug-extension", value<string>(&po.debugger_options.extension),
     "PHP file that extends command 'arg'")
    ("debug-cmd", value<std::vector<std::string>>(
     &po.debugger_options.cmds)->composing(),
     "executes this debugger command and returns its output in stdout")
    ("debug-sandbox",
     value<string>(&po.debugger_options.sandbox)->default_value("default"),
     "initial sandbox to attach to when debugger is started")
    ("user,u", value<string>(&po.user),
     "run server under this user account")
    ("file,f", value<string>(&po.file),
     "execute specified file")
    ("lint,l", value<string>(&po.lint),
     "lint specified file")
    ("show,w", value<string>(&po.show),
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
    ("extra-header", value<string>(&Logger::ExtraHeader),
     "extra-header to add to log lines")
    ("build-id", value<string>(&po.buildId),
     "unique identifier of compiled server code")
    ("instance-id", value<string>(&po.instanceId),
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
        string str = argv[m];
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
      if (vm.count("interactive") /* or -a */) {
        po.mode = "debug";
      }
      if (po.mode == "d") po.mode = "debug";
      if (po.mode == "s") po.mode = "server";
      if (po.mode == "t") po.mode = "translate";
      if (po.mode == "")  po.mode = "run";
      if (po.mode == "daemon" || po.mode == "server" || po.mode == "replay" ||
          po.mode == "run" || po.mode == "debug"|| po.mode == "translate") {
        set_execution_mode(po.mode);
      } else {
        Logger::Error("Error in command line: invalid mode: %s",
                      po.mode.c_str());
        cout << desc << "\n";
        return -1;
      }
      if (po.config.empty() && !vm.count("no-config")) {
        auto file_callback = [&po] (const char *filename) {
          Logger::Verbose("Using default config file: %s", filename);
          po.config.push_back(filename);
        };
        add_default_config_files_globbed("/etc/hhvm/php*.ini",
                                         file_callback);
        add_default_config_files_globbed("/etc/hhvm/config*.hdf",
                                         file_callback);
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
    cout << "Compiler: " << kCompilerId << "\n";
    cout << "Repo schema: " << kRepoSchemaId << "\n";
    return 0;
  }
  if (vm.count("compiler-id")) {
    cout << kCompilerId << "\n";
    return 0;
  }

  if (vm.count("repo-schema")) {
    cout << kRepoSchemaId << "\n";
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
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  // Start with .hdf and .ini files
  for (auto& filename : po.config) {
    Config::ParseConfigFile(filename, ini, config);
  }
  std::vector<std::string> messages;
  // Now, take care of CLI options and then officially load and bind things
  RuntimeOption::Load(ini, config, po.iniStrings, po.confStrings, &messages);

  vector<string> badnodes;
  config.lint(badnodes);
  for (unsigned int i = 0; i < badnodes.size(); i++) {
    Logger::Error("Possible bad config node: %s", badnodes[i].c_str());
  }
  vector<int> inherited_fds;
  RuntimeOption::BuildId = po.buildId;
  RuntimeOption::InstanceId = po.instanceId;
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

  MM().resetRuntimeOptions();

  if (po.mode == "daemon") {
    if (RuntimeOption::LogFile.empty()) {
      Logger::Error("Log file not specified under daemon mode.\n\n");
    }
    int ret = open_server_log_file();
    Process::Daemonize();
    close_server_log_file(ret);
  }

  open_server_log_file();
  if (RuntimeOption::ServerExecutionMode()) {
    for (auto const& m : messages) {
      Logger::Info(m);
    }
  }

  // Defer the initialization of light processes until the log file handle is
  // created, so that light processes can log to the right place. If we ever
  // lose a light process, stop the server instead of proceeding in an
  // uncertain state.
  LightProcess::SetLostChildHandler([](pid_t child) {
    if (!HttpServer::Server) return;
    if (!HttpServer::Server->isStopped()) {
      HttpServer::Server->stop("lost light process child");
    }
  });
  LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                           RuntimeOption::LightProcessCount,
                           RuntimeOption::EvalRecordSubprocessTimes,
                           inherited_fds);

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
    Logger::SetNewOutput(nullptr);

    if (po.isTempFile) {
      tempFile = po.lint;
    }

    hphp_process_init();
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
        throw FatalErrorException(msg->data(), bt);
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

    string file;
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
          execute_command_line_begin(new_argc, new_argv,
                                     po.xhprofFlags, po.config);
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
        execute_command_line_begin(new_argc, new_argv,
                                   po.xhprofFlags, po.config);
        ret = 255;
        if (hphp_invoke_simple(file, false /* warmup only */)) {
          ret = ExitException::ExitCode;
        }
        execute_command_line_end(po.xhprofFlags, true, file.c_str());
      }
    }

    free(new_argv);
    hphp_process_exit();

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
    const string &sourceRoot = RuntimeOption::SourceRoot;
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

static string systemlib_split(string slib, string* hhas) {
  auto pos = slib.find("\n<?hhas\n");
  if (pos != string::npos) {
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
string get_systemlib(string* hhas, const string &section /*= "systemlib" */,
                                   const string &filename /*= "" */) {
  if (filename.empty() && section == "systemlib") {
    if (char *file = getenv("HHVM_SYSTEMLIB")) {
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

#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
  string ret = systemlib_split(std::string(
                                (const char*)LockResource(desc.m_handle),
                                desc.m_len), hhas);
#else
  std::ifstream ifs(desc.m_filename);
  if (!ifs.good()) return "";
  ifs.seekg(desc.m_start, std::ios::beg);
  std::unique_ptr<char[]> data(new char[desc.m_len]);
  ifs.read(data.get(), desc.m_len);
  string ret = systemlib_split(string(data.get(), desc.m_len), hhas);
#endif
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// C++ ffi

static void on_timeout(int sig, siginfo_t* info, void* context) {
  if (sig == SIGVTALRM && info && info->si_code == SI_TIMER) {
    auto data = (RequestTimer*)info->si_value.sival_ptr;
    if (data) {
      data->onTimeout();
    } else {
      Xenon::getInstance().onTimer();
    }
  }
}

void hphp_process_init() {
  pthread_attr_t attr;
// Linux+GNU extension
#if defined(_GNU_SOURCE) && (defined(__linux__) || defined(__CYGWIN__))
  pthread_getattr_np(pthread_self(), &attr);
#else
  pthread_attr_init(&attr);
#endif
  init_stack_limits(&attr);
  pthread_attr_destroy(&attr);

  Process::InitProcessStatics();

  // initialize the tzinfo cache.
  timezone_init();

  init_thread_locals();

  struct sigaction action = {};
  action.sa_sigaction = on_timeout;
  action.sa_flags = SA_SIGINFO | SA_NODEFER;
  sigaction(SIGVTALRM, &action, nullptr);
  // start takes milliseconds, Period is a double in seconds
  Xenon::getInstance().start(1000 * RuntimeOption::XenonPeriodSeconds);

  // Initialize per-process dynamic PHP-visible consts before ClassInfo::Load()
  k_PHP_BINARY = makeStaticString(current_executable_path());
  k_PHP_BINDIR = makeStaticString(current_executable_directory());
  k_PHP_OS = makeStaticString(HHVM_FN(php_uname)("s").toString());
  k_PHP_SAPI = makeStaticString(RuntimeOption::ExecutionMode);

  ClassInfo::Load();

  // reinitialize pcre table
  pcre_reinit();

  // the liboniguruma docs say this isnt needed,
  // but the implementation of init is not
  // thread safe due to bugs
  onig_init();

  // simple xml also needs one time init
  xmlInitParser();

  g_vmProcessInit();

  PageletServer::Restart();
  XboxServer::Restart();
  Stream::RegisterCoreWrappers();
  ExtensionRegistry::moduleInit();
  for (InitFiniNode *in = extra_process_init; in; in = in->next) {
    in->func();
  }
  int64_t save = RuntimeOption::SerializationSizeLimit;
  RuntimeOption::SerializationSizeLimit = StringData::MaxSize;
  apc_load(apcExtension::LoadThread);
  RuntimeOption::SerializationSizeLimit = save;

  rds::requestExit();
  // Reset the preloaded g_context
  ExecutionContext *context = g_context.getNoCheck();
  context->~ExecutionContext();
  new (context) ExecutionContext();
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
                        const string &reqInitFunc,
                        const string &reqInitDoc, bool &error) {
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
  init_thread_locals();
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

  g_context->requestInit();
  s_sessionInitialized = true;
}

ExecutionContext *hphp_context_init() {
  ExecutionContext *context = g_context.getNoCheck();
  context->obStart();
  context->obProtect(true);
  return context;
}

bool hphp_invoke_simple(const std::string& filename, bool warmupOnly) {
  bool error;
  string errorMsg;
  return hphp_invoke(g_context.getNoCheck(), filename, false, null_array,
                     uninit_null(), "", "", error, errorMsg,
                     true /* once */,
                     warmupOnly,
                     false /* richErrorMsg */);
}

bool hphp_invoke(ExecutionContext *context, const std::string &cmd,
                 bool func, const Array& funcParams, VRefParam funcRet,
                 const string &reqInitFunc, const string &reqInitDoc,
                 bool &error, string &errorMsg,
                 bool once, bool warmupOnly,
                 bool richErrorMsg) {
  bool isServer = RuntimeOption::ServerExecutionMode();
  error = false;

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

  LitstrTable::get().setReading();

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
        funcRet->assign(invoke(cmd.c_str(), funcParams));
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

  // Shutdown the debugger
  DEBUGGER_ATTACHED_ONLY(phpDebuggerRequestShutdownHook());

  // Extensions could have shutdown handlers
  ExtensionRegistry::requestShutdown();

  // Extension shutdown could have re-initialized some
  // request locals
  context->onRequestShutdown();
}

void hphp_context_exit(bool shutdown /* = true */) {
  if (shutdown) {
    hphp_context_shutdown();
  }

  // Clean up a bunch of request state. No user code after this point.
  auto const context = g_context.getNoCheck();
  context->requestExit();
  context->obProtect(false);
  context->obEndAll();
}

void hphp_thread_exit() {
  finish_thread_locals();
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

  mm.resetAllocator();
  mm.resetCouldOOM();
}

void hphp_session_exit() {
  assert(s_sessionInitialized);
  // Server note has to live long enough for the access log to fire.
  // RequestLocal is too early.
  ServerNote::Reset();
  // Similarly, apc strings could be in the ServerNote array, and
  // its possible they are scheduled to be destroyed after this request
  // finishes.
  Treadmill::finishRequest();

  TI().clearPendingException();

  {
    ServerStatsHelper ssh("rollback");

    hphp_memory_cleanup();
    // Do any post-sweep cleanup necessary for global variables
    free_global_variables_after_sweep();
  }

  TI().onSessionExit();
  assert(MM().empty());

  s_sessionInitialized = false;
  s_extra_request_microseconds = 0;
}

void hphp_process_exit() {
  Xenon::getInstance().stop();
  PageletServer::Stop();
  XboxServer::Stop();
  // Debugger::Stop() needs an execution context
  g_context.getCheck();
  Eval::Debugger::Stop();
  g_context.destroy();
  ExtensionRegistry::moduleShutdown();
  LightProcess::Close();
  for (InitFiniNode *in = extra_process_exit; in; in = in->next) {
    in->func();
  }
  delete jit::mcg;
  jit::mcg = nullptr;
  folly::SingletonVault::singleton()->destroyInstances();
}

bool is_hphp_session_initialized() {
  return s_sessionInitialized;
}

///////////////////////////////////////////////////////////////////////////////
}
