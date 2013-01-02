/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/types.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/thread_init_fini.h>
#include <runtime/base/code_coverage.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/compiler_id.h>
#include <util/shared_memory_allocator.h>
#include <system/gen/sys/system_globals.h>
#include <system/gen/php/globals/symbols.h>
#include <runtime/base/server/pagelet_server.h>
#include <runtime/base/server/xbox_server.h>
#include <runtime/base/server/http_server.h>
#include <runtime/base/server/replay_transport.h>
#include <runtime/base/server/http_request_handler.h>
#include <runtime/base/server/admin_request_handler.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/server/server_note.h>
#include <runtime/base/memory/memory_manager.h>
#include <util/process.h>
#include <util/capability.h>
#include <util/timer.h>
#include <util/stack_trace.h>
#include <util/light_process.h>
#include <runtime/base/stat_cache.h>
#include <runtime/base/source_info.h>
#include <runtime/base/rtti_info.h>
#include <runtime/base/frame_injection.h>
#include <runtime/ext/extension.h>
#include <runtime/ext/ext_fb.h>
#include <runtime/ext/ext_json.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_apc.h>
#include <runtime/ext/ext_function.h>
#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/debugger/debugger_client.h>
#include <runtime/base/util/simple_counter.h>
#include <runtime/base/util/extended_logger.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <libgen.h>

#include <runtime/eval/runtime/file_repository.h>

#include <runtime/vm/runtime.h>
#include <runtime/vm/repo.h>
#include <runtime/vm/translator/translator.h>
#include <compiler/builtin_symbols.h>

using namespace boost::program_options;
using std::cout;
extern char **environ;

#define MAX_INPUT_NESTING_LEVEL 64

namespace HPHP {

namespace VM { void initialize_repo(); }

/*
 * XXX: VM process initialization is handled through a function
 * pointer so libhphp_runtime.a can be linked into programs that don't
 * actually initialize the VM.
 */
void (*g_vmProcessInit)();

///////////////////////////////////////////////////////////////////////////////
// helpers

struct ProgramOptions {
  string     mode;
  string     config;
  StringVec  confStrings;
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
  StringVec  args;
  string     buildId;
  int        xhprofFlags;
  string     show;
  string     parse;

  Eval::DebuggerClientOptions debugger_options;
};

class StartTime {
public:
  StartTime() : startTime(time(NULL)) {}
  time_t startTime;
};
static StartTime s_startTime;
static string tempFile;

time_t start_time() {
  return s_startTime.startTime;
}

static void process_cmd_arguments(int argc, char **argv) {
  SystemGlobals *g = (SystemGlobals *)get_global_variables();
  g->GV(argc) = argc;
  for (int i = 0; i < argc; i++) {
    g->GV(argv).lvalAt() = argv[i];
  }
}

void process_env_variables(Variant &variables) {
  for (std::map<string, string>::const_iterator iter =
         RuntimeOption::EnvVariables.begin();
       iter != RuntimeOption::EnvVariables.end(); ++iter) {
    variables.set(String(iter->first), String(iter->second));
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

void register_variable(Variant &variables, char *name, CVarRef value,
                       bool overwrite /* = true */) {
  // ignore leading spaces in the variable name
  char *var = name;
  while (*var && *var == ' ') {
    var++;
  }

  // ensure that we don't have spaces or dots in the variable name
  // (not binary safe)
  bool is_array = false;
  char *ip = NULL; // index pointer
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

  vector<Variant> gpc_elements;
  gpc_elements.reserve(MAX_INPUT_NESTING_LEVEL); // important, so no resize
  Variant *symtable = &variables;
  char *index = var;
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
        index_s = NULL;
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
        symtable->append(Array::Create());
        gpc_elements.push_back(null);
        gpc_elements.back().assignRef(
          symtable->lvalAt((int)symtable->toArray().size() - 1));
      } else {
        String key(index, index_len, CopyString);
        Variant v = symtable->rvalAt(key);
        if (v.isNull() || !v.is(KindOfArray)) {
          symtable->set(key, Array::Create());
        }
        gpc_elements.push_back(null);
        gpc_elements.back().assignRef(symtable->lvalAt(key));
      }
      symtable = &gpc_elements.back();
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
      if (overwrite || !symtable->toArray().exists(key)) {
        symtable->set(key, value);
      }
    }
  }
}

enum ContextOfException {
  ReqInitException = 1,
  InvokeException,
  HandlerException,
};

static void handle_exception_append_bt(std::string& errorMsg,
                                       const ExtendedException& e) {
  ArrayPtr bt = e.getBackTrace();
  if (!bt->empty()) {
    errorMsg += ExtendedLogger::StringOfStackTrace(*bt);
  }
}

static void handle_exception_helper(bool& ret,
                                    ExecutionContext* context,
                                    std::string& errorMsg,
                                    ContextOfException where,
                                    bool& error,
                                    bool richErrorMsg) {
  try {
    throw;
  } catch (const Eval::DebuggerException &e) {
    throw;
  } catch (const ExitException &e) {
    if (where == ReqInitException) {
      ret = false;
    } else if (where != HandlerException &&
        !context->getExitCallback().isNull() &&
        f_is_callable(context->getExitCallback())) {
      Array stack = e.getBackTrace()->get();
      Array argv = CREATE_VECTOR2(e.ExitCode, stack);
      f_call_user_func_array(context->getExitCallback(), argv);
    }
  } catch (const PhpFileDoesNotExistException &e) {
    ret = false;
    if (where != HandlerException) {
      raise_notice("%s", e.getMessage().c_str());
    } else {
      Logger::Error("%s", e.getMessage().c_str());
    }
    if (richErrorMsg) {
      handle_exception_append_bt(errorMsg, e);
    }
  } catch (const UncatchableException &e) {
    ret = false;
    error = true;
    errorMsg = "";
    if (RuntimeOption::ServerStackTrace) {
      errorMsg = e.what();
    } else if (RuntimeOption::InjectedStackTrace) {
      errorMsg = e.getMessage();
      errorMsg += "\n";
      errorMsg += ExtendedLogger::StringOfStackTrace(*e.getBackTrace());
    } else {
      errorMsg = e.getStackTrace().hexEncode();
      errorMsg += " ";
      errorMsg += e.getMessage();
    }
    Logger::Error("%s", errorMsg.c_str());
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
    if (where == HandlerException) {
      errorMsg = "Exception handler threw an exception: ";
    }
    if (RuntimeOption::ServerStackTrace) {
      errorMsg += e.what();
    } else {
      errorMsg += e.getStackTrace().hexEncode();
      errorMsg += " ";
      errorMsg += e.getMessage();
    }
    if (where == InvokeException) {
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
    if (where == HandlerException) {
      errorMsg = "Exception handler threw an object exception: ";
    }
    try {
      errorMsg += e.toString().data();
    } catch (...) {
      errorMsg += "(unable to call toString())";
    }
    if (where == InvokeException) {
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
  ASSERT(dir);
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

void handle_destructor_exception(const char* situation) {
  string errorMsg;
  try {
    throw;
  } catch (ExitException &e) {
    // ExitException is fine
    return;
  } catch (Object &e) {
    // For user exceptions, invoke the user exception handler
    errorMsg = situation;
    errorMsg += " threw an object exception: ";
    try {
      errorMsg += e.toString().data();
    } catch (...) {
      errorMsg += "(unable to call toString())";
    }
  } catch (Exception &e) {
    errorMsg = situation;
    errorMsg += " raised a fatal error: ";
    if (RuntimeOption::ServerStackTrace) {
      errorMsg += e.what();
    } else {
      errorMsg += e.getStackTrace().hexEncode();
      errorMsg += " ";
      errorMsg += e.getMessage();
    }
  } catch (...) {
    errorMsg = situation;
    errorMsg += " threw an unknown exception";
  }
  // For fatal errors and unknown exceptions, we raise a warning.
  // If there is a user error handler it will be invoked, otherwise
  // the default error handler will be invoked.
  try {
    raise_debugging("%s", errorMsg.c_str());
  } catch (...) {
    // The user error handler fataled or threw an exception,
    // print out the error message directly to the log
    Logger::Warning("%s", errorMsg.c_str());
  }
}

void execute_command_line_begin(int argc, char **argv, int xhprof) {
  StackTraceNoHeap::AddExtraLogging("ThreadType", "CLI");
  string args;
  for (int i = 0; i < argc; i++) {
    if (i) args += " ";
    args += argv[i];
  }
  StackTraceNoHeap::AddExtraLogging("Arguments", args.c_str());

  hphp_session_init();
  ExecutionContext *context = g_context.getNoCheck();
  context->obSetImplicitFlush(true);

  SystemGlobals *g = (SystemGlobals *)get_global_variables();

  // reset global symbols to nulls or empty arrays
  pm_php$globals$symbols_php(false, g, g);

  process_env_variables(g->GV(_ENV));
  g->GV(_ENV).set("HPHP", 1);
  switch (getHphpBinaryType()) {
  case HphpBinary::hhvm:
    g->GV(_ENV).set("HHVM", 1);
    if (RuntimeOption::EvalJit) {
      g->GV(_ENV).set("HHVM_JIT", 1);
    }
    break;
  case HphpBinary::hphpi: g->GV(_ENV).set("HPHPI", 1); break;
  default: break;
  }

  process_cmd_arguments(argc, argv);

  Variant &server = g->GV(_SERVER);
  process_env_variables(server);
  time_t now = time(NULL);
  server.set("REQUEST_START_TIME", now);
  server.set("REQUEST_TIME", now);
  server.set("DOCUMENT_ROOT", "");
  server.set("SCRIPT_FILENAME", argv[0]);
  server.set("SCRIPT_NAME", argv[0]);
  server.set("PHP_SELF", argv[0]);
  server.set("argv", g->GV(argv));
  server.set("argc", g->GV(argc));
  server.set("PWD", g_context->getCwd());
  char hostname[1024];
  if (!gethostname(hostname, 1024)) {
    server.set("HOSTNAME", String(hostname, CopyString));
  }

  for(std::map<string,string>::iterator it =
        RuntimeOption::ServerVariables.begin(),
        end = RuntimeOption::ServerVariables.end(); it != end; ++it) {
    server.set(String(it->first.c_str()), String(it->second.c_str()));
  }

  if (RuntimeOption::EnableCliRTTI) RTTIInfo::TheRTTIInfo.init(true);

  if (xhprof) {
    f_xhprof_enable(xhprof, null);
  }
}

namespace Eval { void shutdown_hphpd(); }

void execute_command_line_end(int xhprof, bool coverage, const char *program) {
  ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();

  if (RuntimeOption::EvalJit && RuntimeOption::EvalDumpTC) {
    HPHP::VM::Transl::tc_dump();
  }

  if (xhprof) {
    f_var_dump(f_json_encode(f_xhprof_disable()));
  }
  hphp_context_exit(g_context.getNoCheck(), true, true, program);
  hphp_session_exit();
  if (coverage && ti->m_reqInjectionData.coverage &&
      !RuntimeOption::CodeCoverageOutputFile.empty()) {
    ti->m_coverage->Report(RuntimeOption::CodeCoverageOutputFile);
  }

  Eval::shutdown_hphpd();
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
  if (buf == NULL)
    return;

  Timer timer(Timer::WallTime, "mapping self");
  fp = fopen("/proc/self/maps", "r");
  if (fp != NULL) {
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

      if (mlock((void *)begin, end - begin) == 0) {
        if (!RuntimeOption::LockCodeMemory) {
          munlock((void *)begin, end - begin);
        }
      }
    }
    fclose(fp);
  }
  free(buf);
}

static int start_server(const std::string &username) {
  // Before we start the webserver, make sure the entire
  // binary is paged into memory.
  pagein_self();

  RuntimeOption::ExecutionMode = "srv";
  HttpRequestHandler::GetAccessLog().init
    (RuntimeOption::AccessLogDefaultFormat, RuntimeOption::AccessLogs,
     username);
  AdminRequestHandler::GetAccessLog().init
    (RuntimeOption::AdminLogFormat, RuntimeOption::AdminLogSymLink,
     RuntimeOption::AdminLogFile,
     username);

  void *sslCTX = NULL;
  if (RuntimeOption::EnableSSL) {
#ifdef _EVENT_USE_OPENSSL
    struct ssl_config config;
    if (RuntimeOption::SSLCertificateFile != "" &&
        RuntimeOption::SSLCertificateKeyFile != "") {
      config.cert_file = (char*)RuntimeOption::SSLCertificateFile.c_str();
      config.pk_file = (char*)RuntimeOption::SSLCertificateKeyFile.c_str();
      sslCTX = evhttp_init_openssl(&config);
      if (!RuntimeOption::SSLCertificateDir.empty()) {
        ServerNameIndication::load(sslCTX, config,
                                   RuntimeOption::SSLCertificateDir);
      }
    } else {
      Logger::Error("Invalid certificate file or key file");
    }
#else
    Logger::Error("A SSL enabled libevent is required");
#endif
  }

#if !defined(SKIP_USER_CHANGE)
  if (!username.empty()) {
    if (Logger::UseCronolog) {
      Cronolog::changeOwner(username, RuntimeOption::LogFileSymLink);
    }
    Capability::ChangeUnixUser(username);
    LightProcess::ChangeUser(username);
  }
#endif

  // Create the HttpServer before any warmup requests to properly
  // initialize the process
  HttpServer::Server = HttpServerPtr(new HttpServer(sslCTX));

  // If we have any warmup requests, replay them before listening for
  // real connections
  for (auto& file : RuntimeOption::ServerWarmupRequests) {
    HttpRequestHandler handler;
    ReplayTransport rt;
    timespec start;
    gettime(CLOCK_MONOTONIC, &start);
    std::string error;
    Logger::Info("Replaying warmup request %s", file.c_str());
    try {
      rt.onRequestStart(start);
      rt.replayInput(file);
      handler.handleRequest(&rt);
      Logger::Info("Finished successfully");
    } catch (std::exception& e) {
      error = e.what();
    }
    if (error.size()) {
      Logger::Info("Got exception during warmup: %s", error.c_str());
    }
  }

  HttpServer::Server->run();
  return 0;
}

string translate_stack(const char *hexencoded, bool with_frame_numbers) {
  if (!hexencoded || !*hexencoded) {
    return "";
  }

  StackTrace st(hexencoded);
  StackTrace::FramePtrVec frames;
  st.get(frames);

  std::ostringstream out;
  for (unsigned int i = 0; i < frames.size(); i++) {
    StackTrace::FramePtr f = frames[i];
    if (with_frame_numbers) {
      out << "# " << (i < 10 ? " " : "") << i << ' ';
    }
    out << f->toString();
    if (SourceInfo::TheSourceInfo.translate(f)) {
      out << " [" << f->filename << ':' << f->lineno << ']';
    }
    out << '\n';
  }
  return out.str();
}

void translate_rtti(const char *rttiDirectory) {
  RTTIInfo::TheRTTIInfo.translate_rtti(rttiDirectory);
}

///////////////////////////////////////////////////////////////////////////////

static void prepare_args(int &argc, char **&argv, const StringVec &args,
                         const char *file) {
  argv = (char **)malloc((args.size() + 2) * sizeof(char*));
  argc = 0;
  if (file) {
    argv[argc++] = (char*)file;
  }
  for (int i = 0; i < (int)args.size(); i++) {
    argv[argc++] = (char*)args[i].c_str();
  }
  argv[argc] = NULL;
}

static int execute_program_impl(int argc, char **argv);
int execute_program(int argc, char **argv) {
  int ret_code = -1;
  try {
    if (hhvm) VM::initialize_repo();
    init_thread_locals();
    ret_code = execute_program_impl(argc, argv);
  } catch (const Exception &e) {
    Logger::Error("Uncaught exception: %s", e.what());
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

/* Sets RuntimeOption::ExecutionMode according
 * to commandline options prior to config load
 */
static void set_execution_mode(string mode) {
  if (mode == "daemon" || mode == "server" || mode == "replay") {
    RuntimeOption::ExecutionMode = "srv";
  } else if (mode == "run" || mode == "debug") {
    RuntimeOption::ExecutionMode = "cli";
  } else if (mode == "translate") {
    RuntimeOption::ExecutionMode = "";
  } else {
    // Undefined mode
    always_assert(false);
  }
}

static int execute_program_impl(int argc, char **argv) {
  string usage = "Usage:\n\n\t";
  usage += argv[0];
  usage += " [-m <mode>] [<options>] [<arg1>] [<arg2>] ...\n\nOptions";

  ProgramOptions po;
  options_description desc(usage.c_str());
  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
#ifdef COMPILER_ID
    ("compiler-id", "display the git hash for the compiler id")
#endif
    ("repo-schema", "display the repo schema id used by this app")
    ("taint-status", "check if the compiler was built with taint enabled")
    ("mode,m", value<string>(&po.mode)->default_value("run"),
     "run | debug (d) | server (s) | daemon | replay | translate (t)")
    ("config,c", value<string>(&po.config),
     "load specified config file")
    ("config-value,v", value<StringVec >(&po.confStrings)->composing(),
     "individual configuration string in a format of name=value, where "
     "name can be any valid configuration for a config file")
    ("port,p", value<int>(&po.port)->default_value(-1),
     "start an HTTP server at specified port")
    ("port-fd", value<int>(&po.portfd)->default_value(-1),
     "use specified fd instead of creating a socket")
    ("ssl-port-fd", value<int>(&po.sslportfd)->default_value(-1),
     "use specified fd for SSL instead of creating a socket")
    ("admin-port", value<int>(&po.admin_port)->default_value(-1),
     "start admin listener at specified port")
    ("debug-host,h", value<string>(&po.debugger_options.host),
     "connect to debugger server at specified address")
    ("debug-port", value<int>(&po.debugger_options.port)->default_value(-1),
     "connect to debugger server at specified port")
    ("debug-extension", value<string>(&po.debugger_options.extension),
     "PHP file that extends y command")
    ("debug-cmd", value<StringVec>(&po.debugger_options.cmds)->composing(),
     "executes this debugger command and returns its output in stdout")
    ("debug-sandbox",
     value<string>(&po.debugger_options.sandbox)->default_value("default"),
     "initial sandbox to attach to when debugger is started")
    ("user,u", value<string>(&po.user),
     "run server under this user account")
    ("file,f", value<string>(&po.file),
     "executing specified file")
    ("lint,l", value<string>(&po.lint),
     "lint specified file")
    ("show,w", value<string>(&po.show),
     "output specified file and do nothing else")
    ("parse", value<string>(&po.parse),
     "parse specified file and dump the AST")
    ("temp-file",
     "file specified is temporary and removed after execution")
    ("count", value<int>(&po.count)->default_value(1),
     "how many times to repeat execution")
    ("no-safe-access-check",
      value<bool>(&po.noSafeAccessCheck)->default_value(false),
     "whether to ignore safe file access check")
    ("arg", value<StringVec >(&po.args)->composing(),
     "arguments")
    ("extra-header", value<string>(&Logger::ExtraHeader),
     "extra-header to add to log lines")
    ("build-id", value<string>(&po.buildId),
     "unique identifier of compiled server code")
    ("xhprof-flags", value<int>(&po.xhprofFlags)->default_value(0),
     "Set XHProf flags")
    ;

  positional_options_description p;
  p.add("arg", -1);
  variables_map vm;
  try {
    store(command_line_parser(argc, argv).options(desc).positional(p).run(),
          vm);
    notify(vm);
    if (po.mode == "d") po.mode = "debug";
    if (po.mode == "s") po.mode = "server";
    if (po.mode == "t") po.mode = "translate";
    if (po.mode == "")  po.mode = "run";
    set_execution_mode(po.mode);
  } catch (error &e) {
    Logger::Error("Error in command line: %s\n\n", e.what());
    cout << desc << "\n";
    return -1;
  } catch (...) {
    Logger::Error("Error in command line:\n\n");
    cout << desc << "\n";
    return -1;
  }
  if (vm.count("help")) {
    cout << desc << "\n";
    return 0;
  }
  if (vm.count("version")) {
#ifdef HPHP_VERSION
#undefine HPHP_VERSION
#endif
#define HPHP_VERSION(v) const char *version = #v;
#include "../../version"

    switch (getHphpBinaryType()) {
    case HphpBinary::hhvm: { cout << "HipHop VM"; break; }
    case HphpBinary::hphpi: { cout << "HipHop Interpreter"; break; }
    default: { cout << "Compiled by HipHop Compiler"; break; }
    }
    cout << " v" << version << " (" << (debug ? "dbg" : "rel") << ")\n";
#ifdef COMPILER_ID
    cout << "Compiler: " << COMPILER_ID << "\n";
#endif
    cout << "Repo schema: " << VM::Repo::kSchemaId << "\n";
    return 0;
  }
#ifdef COMPILER_ID
  if (vm.count("compiler-id")) {
    cout << COMPILER_ID << "\n";
    return 0;
  }
#endif

  if (vm.count("repo-schema")) {
    cout << VM::Repo::kSchemaId << "\n";
    return 0;
  }

  if (vm.count("taint-status")) {
#ifdef TAINTED
    cout << TAINTED << "\n";
#endif
    return 0;
  }

  if (!po.show.empty()) {
    PlainFile f;
    f.open(po.show, "r");
    if (!f.valid()) {
      Logger::Error("Unable to open file %s", po.show.c_str());
      return 1;
    }
    f.print();
    f.close();
    return 0;
  }

  po.isTempFile = vm.count("temp-file");

  Hdf config;
  if (!po.config.empty()) {
    config.open(po.config);
  }
  RuntimeOption::Load(config, &po.confStrings);
  vector<string> badnodes;
  config.lint(badnodes);
  for (unsigned int i = 0; i < badnodes.size(); i++) {
    Logger::Error("Possible bad config node: %s", badnodes[i].c_str());
  }

  vector<int> inherited_fds;
  RuntimeOption::BuildId = po.buildId;
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

  if (po.mode == "daemon") {
    if (RuntimeOption::LogFile.empty()) {
      Logger::Error("Log file not specified under daemon mode.\n\n");
    }
    int ret = open_server_log_file();
    Process::Daemonize();
    close_server_log_file(ret);
  }

  open_server_log_file();

  // Defer the initialization of light processes until the log file handle
  // is created, so that light processes can log to the right place.
  LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                           RuntimeOption::LightProcessCount,
                           inherited_fds);

  {
    const size_t stackSizeMinimum = 8 * 1024 * 1024;
    struct rlimit rlim;
    if (getrlimit(RLIMIT_STACK, &rlim) == 0 &&
        (rlim.rlim_cur == RLIM_INFINITY ||
         rlim.rlim_cur < stackSizeMinimum)) {
      rlim.rlim_cur = stackSizeMinimum;
      if (stackSizeMinimum > rlim.rlim_max) {
        rlim.rlim_max = stackSizeMinimum;
      }
      if (setrlimit(RLIMIT_STACK, &rlim)) {
        Logger::Error("failed to set stack limit to %lld\n", stackSizeMinimum);
      }
    }
  }

  ShmCounters::initialize(true, Logger::Error);

  if (!po.lint.empty()) {
    if (po.isTempFile) {
      tempFile = po.lint;
    }

    hphp_process_init();
    try {
      HPHP::Eval::PhpFile* phpFile = g_vmContext->lookupPhpFile(
        StringData::GetStaticString(po.lint.c_str()), "", NULL);
      if (phpFile == NULL) {
        throw FileOpenException(po.lint.c_str());
      }
      VM::Unit* unit = phpFile->unit();
      const StringData* msg;
      int line;
      if (unit->compileTimeFatal(msg, line)) {
        VMParserFrame parserFrame;
        parserFrame.filename = po.lint.c_str();
        parserFrame.lineNumber = line;
        ArrayPtr bt =
          ArrayPtr(new Array(
                     g_vmContext->debugBacktrace(false, true,
                                                 false, &parserFrame)));
        throw FatalErrorException(msg->data(), bt);
      }
    } catch (FileOpenException &e) {
      Logger::Error("%s", e.getMessage().c_str());
      return 1;
    } catch (const FatalErrorException& e) {
      RuntimeOption::CallUserHandlerOnFatals = false;
      RuntimeOption::AlwaysLogUnhandledExceptions = true;
      g_context->onFatalError(e);
      return 1;
    }
    Logger::Info("No syntax errors detected in %s", po.lint.c_str());
    return 0;
  }

  if (!po.parse.empty()) {
    hphp_process_init();
    const StringData *fileName = StringData::GetStaticString(po.parse);
    struct stat s;
    if (!Eval::FileRepository::findFile(fileName, &s)) return 0;
    Eval::FileRepository::FileInfo fileInfo;
    if (!Eval::FileRepository::readFile(fileName, s, fileInfo)) {
      return 0;
    }
    Eval::PhpFile *f = Eval::FileRepository::parseFile(po.parse, fileInfo);
    if (!f) return 0;
    // TODO: Task #1154018: We should support the "parse" command line option
    // under the VM or get rid of it. The logic above calls into FileRepository
    // to load the file. What is missing is the logic that gets fishes the AST
    // out of the PhpFile and outputs the AST as PHP source to STDOUT.
    return 0;
  }

  if (argc <= 1 || po.mode == "run" || po.mode == "debug") {
    if (po.isTempFile) {
      tempFile = po.file;
    }

    RuntimeOption::ExecutionMode = "cli";

    int new_argc;
    char **new_argv;
    prepare_args(new_argc, new_argv, po.args, po.file.c_str());

    if (hhvm) {
      if (!po.file.empty()) {
        VM::Repo::setCliFile(po.file);
      } else if (new_argc >= 2) {
        VM::Repo::setCliFile(new_argv[1]);
      }
    }

    int ret = 0;
    hphp_process_init();

    if (po.mode == "debug") {
      RuntimeOption::EnableDebugger = true;
      Eval::DebuggerProxyPtr proxy =
        Eval::Debugger::StartClient(po.debugger_options);
      if (!proxy) {
        Logger::Error("Failed to start debugger client\n\n");
        return 1;
      }
      Eval::Debugger::RegisterSandbox(proxy->getDummyInfo());
      Eval::Debugger::RegisterThread();
      string file = po.file;
      StringVecPtr client_args;
      bool restart = false;
      ret = 0;
      while (true) {
        try {
          execute_command_line_begin(new_argc, new_argv, po.xhprofFlags);
          g_context->setSandboxId(proxy->getDummyInfo().id());
          Eval::Debugger::DebuggerSession(po.debugger_options, file, restart);
          restart = false;
          execute_command_line_end(po.xhprofFlags, true, file.c_str());
        } catch (const Eval::DebuggerRestartException &e) {
          execute_command_line_end(0, false, NULL);

          if (!e.m_args->empty()) {
            file = e.m_args->at(0);
            client_args = e.m_args;
            free(new_argv);
            prepare_args(new_argc, new_argv, *client_args, NULL);
          }
          restart = true;
        } catch (const Eval::DebuggerClientExitException &e) {
          execute_command_line_end(0, false, NULL);
          break; // end user quitting debugger
        }
      }

    } else {
      ret = 0;
      for (int i = 0; i < po.count; i++) {
        execute_command_line_begin(new_argc, new_argv, po.xhprofFlags);
        ret = -1;
        if (hphp_invoke_simple(po.file)) {
          ret = ExitException::ExitCode;
        }
        execute_command_line_end(po.xhprofFlags, true, new_argv[0]);
      }
    }

    free(new_argv);
    hphp_process_exit();

    return ret;
  }

  if (po.mode == "daemon" || po.mode == "server") {
    if (!po.user.empty()) RuntimeOption::ServerUser = po.user;
    return start_server(RuntimeOption::ServerUser);
  }

  if (po.mode == "replay" && !po.args.empty()) {
    RuntimeOption::RecordInput = false;
    RuntimeOption::ExecutionMode = "srv";
    HttpServer server; // so we initialize runtime properly
    HttpRequestHandler handler;
    for (int i = 0; i < po.count; i++) {
      for (unsigned int j = 0; j < po.args.size(); j++) {
        ReplayTransport rt;
        rt.replayInput(po.args[j].c_str());
        handler.handleRequest(&rt);
        printf("%s\n", rt.getResponse().c_str());
      }
    }
    return 0;
  }

  if (po.mode == "translate" && !po.args.empty()) {
    if (!access(po.args[0].c_str(), F_OK)) {
      translate_rtti(po.args[0].c_str());
    } else {
      printf("%s", translate_stack(po.args[0].c_str()).c_str());
    }
    return 0;
  }

  cout << desc << "\n";
  return -1;
}

String canonicalize_path(CStrRef p, const char* root, int rootLen) {
  String path(Util::canonicalize(p.c_str(), p.size()), AttachString);
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

///////////////////////////////////////////////////////////////////////////////
// C++ ffi

extern "C" void hphp_fatal_error(const char *s) {
  throw_fatal(s);
}

void hphp_process_init() {
  init_thread_locals();
  ClassInfo::Load();
  Process::InitProcessStatics();
  init_static_variables();

  if (hhvm) {
    extern void sys_init_literal_varstrings();
    sys_init_literal_varstrings();
    g_vmProcessInit();
  } else {
    init_literal_varstrings();
  }

  PageletServer::Restart();
  XboxServer::Restart();
  Extension::InitModules();
  int64 save = RuntimeOption::SerializationSizeLimit;
  RuntimeOption::SerializationSizeLimit = StringData::MaxSize;
  apc_load(RuntimeOption::ApcLoadThread);
  RuntimeOption::SerializationSizeLimit = save;
  StaticString::FinishInit();

  if (hhvm) {
    VM::Transl::TargetCache::requestExit();
  }
  // Reset the preloaded g_context
  ExecutionContext *context = g_context.getNoCheck();
  context->~ExecutionContext();
  new (context) ExecutionContext();
}

static void handle_exception(bool& ret, ExecutionContext* context,
                             std::string& errorMsg, ContextOfException where,
                             bool& error, bool richErrorMsg) {
  ASSERT(where == InvokeException || where == ReqInitException);
  try {
    handle_exception_helper(ret, context, errorMsg, where, error, richErrorMsg);
  } catch (const ExitException &e) {
    // Got an ExitException during exception handling, handle
    // similarly to the case below but don't call obEndAll().
  } catch (...) {
    handle_exception_helper(ret, context, errorMsg, HandlerException, error,
                            richErrorMsg);
    context->obEndAll();
  }
}

static void handle_reqinit_exception(bool &ret, ExecutionContext *context,
                                     std::string &errorMsg, bool &error) {
  handle_exception(ret, context, errorMsg, ReqInitException, error, false);
}

static void handle_invoke_exception(bool &ret, ExecutionContext *context,
                                    std::string &errorMsg, bool &error,
                                    bool richErrorMsg) {
  handle_exception(ret, context, errorMsg, InvokeException, error,
                   richErrorMsg);
}

static bool hphp_warmup(ExecutionContext *context,
                        const string &reqInitFunc,
                        const string &reqInitDoc, bool &error) {
  bool ret = true;
  error = false;
  std::string errorMsg;

  MemoryManager *mm = MemoryManager::TheMemoryManager();
  if (mm->isEnabled()) {
    ServerStatsHelper ssh("reqinit");
    try {
      if (!reqInitDoc.empty()) {
        include_impl_invoke(reqInitDoc, true, get_variable_table());
      }
      if (!reqInitFunc.empty()) {
        invoke(reqInitFunc.c_str(), Array());
      }
      context->backupSession();
    } catch (...) {
      handle_reqinit_exception(ret, context, errorMsg, error);
    }
  }

  return ret;
}

void hphp_session_init() {
  init_thread_locals();
  ThreadInfo::s_threadInfo->onSessionInit();
  MemoryManager::TheMemoryManager()->resetStats();
  init_global_variables();

#ifdef ENABLE_SIMPLE_COUNTER
  SimpleCounter::Enabled = true;
  StackTrace::Enabled = true;
#endif

  if (has_eval_support) {
    // Ordering is sensitive; StatCache::requestInit produces work that
    // must be done in VMExecutionContext::requestInit.
    StatCache::requestInit();
  }

  if (hhvm) {
    g_vmContext->requestInit();
  }
}

void hphp_thread_init() {
  init_thread_locals();
}

bool hphp_is_warmup_enabled() {
  MemoryManager *mm = MemoryManager::TheMemoryManager();
  return mm->isEnabled();
}

ExecutionContext *hphp_context_init() {
  ExecutionContext *context = g_context.getNoCheck();
  context->obStart();
  context->obProtect(true);
  return context;
}

bool hphp_invoke_simple(const std::string &filename,
                        bool warmupOnly /* = false */) {
  bool error; string errorMsg;
  return hphp_invoke(g_context.getNoCheck(), filename, false, null_array, null,
                     "", "", error, errorMsg, true, warmupOnly);
}

bool hphp_invoke(ExecutionContext *context, const std::string &cmd,
                 bool func, CArrRef funcParams, VRefParam funcRet,
                 const string &reqInitFunc, const string &reqInitDoc,
                 bool &error, string &errorMsg,
                 bool once /* = true */, bool warmupOnly /* = false */,
                 bool richErrorMsg /* = false */) {
  bool isServer = RuntimeOption::serverExecutionMode();
  error = false;

  String oldCwd;
  if (isServer) {
    oldCwd = context->getCwd();
  }
  if (!hphp_warmup(context, reqInitFunc, reqInitDoc, error)) {
    if (isServer) context->setCwd(oldCwd);
    return false;
  }

  bool ret = true;
  if (!warmupOnly) {
    try {
      ServerStatsHelper ssh("invoke");
      if (func) {
        funcRet->assignVal(invoke(cmd.c_str(), funcParams));
      } else {
        if (isServer) hphp_chdir_file(cmd);
        include_impl_invoke(cmd.c_str(), once, get_variable_table());
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

void hphp_context_exit(ExecutionContext *context, bool psp,
                       bool shutdown /* = true */,
                       const char *program /* = NULL */) {
  if (psp) {
    context->onShutdownPostSend();
  }
  if (RuntimeOption::EnableDebugger) {
    try {
      Eval::Debugger::InterruptPSPEnded(program);
    } catch (const Eval::DebuggerException &e) {}
  }
  if (hhvm) {
    static_cast<VMExecutionContext*>(
      static_cast<BaseExecutionContext*>(context))->requestExit();
  }
  if (shutdown) {
    context->onRequestShutdown();
  }
  context->obProtect(false);
  context->obEndAll();
}

void hphp_thread_exit() {
  finish_thread_locals();
}

void hphp_session_exit() {
  // Server note has to live long enough for the access log to fire.
  // RequestLocal is too early.
  ServerNote::Reset();
  g_context.destroy();

  ThreadInfo::s_threadInfo->clearPendingException();

  MemoryManager *mm = MemoryManager::TheMemoryManager();
  if (RuntimeOption::CheckMemory) {
    mm->checkMemory();
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemoryStats) {
    mm->logStats();
  }
  mm->resetStats();

  if (mm->isEnabled()) {
    ServerStatsHelper ssh("rollback");
    // sweep may call g_context->, which is a noCheck, so we need to
    // reinitialize g_context here
    g_context.getCheck();
    // MemoryManager::sweepAll() will handle sweeping for PHP objects and
    // PHP resources (ex. File, Collator, XmlReader, etc.)
    mm->sweepAll();
    // Destroy g_context again because ExecutionContext has SmartAllocated
    // data members. These members cannot survive over rollback(), so we need
    // to destroy g_context before calling rollback().
    g_context.destroy();
    // MemoryManager::rollback() will handle sweeping for all types that have
    // dedicated allocators (ex. StringData, ZendArray, HphpArray, etc.) and
    // it reset all of the allocators in preparation for the next request.
    mm->rollback();
    // Do any post-sweep cleanup necessary for global variables
    free_global_variables_after_sweep();
    g_context.getCheck();
  } else {
    g_context.getCheck();
    ServerStatsHelper ssh("free");
    free_global_variables();
  }

  ThreadInfo::s_threadInfo->onSessionExit();
}

void hphp_process_exit() {
  XboxServer::Stop();
  Eval::Debugger::Stop();
  Extension::ShutdownModules();
  LightProcess::Close();
}

///////////////////////////////////////////////////////////////////////////////
}
