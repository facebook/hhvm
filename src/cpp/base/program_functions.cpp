/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/base/program_functions.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/execution_context.h>
#include <cpp/base/runtime_option.h>
#include <util/shared_memory_allocator.h>
#include <lib/system/gen/sys/system_globals.h>
#include <cpp/base/server/pagelet_server.h>
#include <cpp/base/server/xbox_server.h>
#include <cpp/base/server/http_server.h>
#include <cpp/base/server/replay_transport.h>
#include <cpp/base/server/http_request_handler.h>
#include <cpp/base/server/server_stats.h>
#include <cpp/base/server/server_note.h>
#include <cpp/base/memory/memory_manager.h>
#include <util/process.h>
#include <util/capability.h>
#include <cpp/base/source_info.h>
#include <cpp/base/rtti_info.h>
#include <cpp/base/util/light_process.h>
#include <cpp/base/frame_injection.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <libgen.h>

using namespace std;
using namespace boost::program_options;
extern char **environ;

#define MAX_INPUT_NESTING_LEVEL 64

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

class StartTime {
public:
  StartTime() : startTime(time(NULL)) {}
  time_t startTime;
};
static StartTime s_startTime;

time_t start_time() {
  return s_startTime.startTime;
}

static void process_cmd_arguments(int argc, char **argv) {
  SystemGlobals *g = (SystemGlobals *)get_global_variables();
  g->gv_argc = argc;
  for (int i = 0; i < argc; i++) {
    g->gv_argv.lvalAt() = argv[i];
  }
}

void process_env_variables(Variant &variables) {
  for (map<string, string>::const_iterator iter =
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
        gpc_elements.back() =
          ref(symtable->lvalAt((int)symtable->toArray().size()));
      } else {
        String key(index, index_len, CopyString);
        Variant v = symtable->rvalAt(key);
        if (v.isNull() || !v.is(KindOfArray)) {
          symtable->set(key, Array::Create());
        }
        gpc_elements.push_back(null);
        gpc_elements.back() = ref(symtable->lvalAt(key));
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
  WarmupDocException = 0,
  ReqInitException,
  InvokeException,
  HandlerException,
};

static bool handle_exception(ExecutionContext *context, std::string &errorMsg,
                             ContextOfException where, bool &error) {
  bool ret = false;
  try {
    throw;
  } catch (ExitException &e) {
    ret = true;
    // ExitException is fine
  } catch (PhpFileDoesNotExistException &e) {
    if (where == WarmupDocException) {
      Logger::Error("warmup error: %s", e.getMessage().c_str());
    }
  } catch (Exception &e) {
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
      context->onError(e);
    } else {
      Logger::Error("%s", errorMsg.c_str());
    }
    error = true;
  } catch (Object &e) {
    if (where == HandlerException) {
      errorMsg = "Exception handler threw an object exception: ";
    }
    try {
      errorMsg += e.toString().data();
    } catch (...) {
      errorMsg += "(unable to call toString())";
    }
    if (where == InvokeException) {
      context->onException(e);
    } else {
      Logger::Error("%s", errorMsg.c_str());
    }
    error = true;
  } catch (...) {
    if (where == InvokeException) throw;
    errorMsg = "(unknown exception was thrown)";
    Logger::Error("%s", errorMsg.c_str());
    error = true;
  }
  return ret;
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

void handle_destructor_exception() {
  string errorMsg;
  try {
    throw;
  } catch (ExitException &e) {
    // ExitException is fine
  } catch (Exception &e) {
    errorMsg = "Destructor threw an exception: ";
    if (RuntimeOption::ServerStackTrace) {
      errorMsg += e.what();
    } else {
      errorMsg += e.getStackTrace().hexEncode();
      errorMsg += " ";
      errorMsg += e.getMessage();
    }
    Logger::Error("%s", errorMsg.c_str());
  } catch (Object &e) {
    errorMsg = "Destructor threw an object exception: ";
    try {
      errorMsg += e.toString().data();
    } catch (...) {
      errorMsg += "(unable to call toString())";
    }
    Logger::Error("%s", errorMsg.c_str());
  } catch (...) {
    errorMsg = "(unknown exception was thrown from destructor)";
    Logger::Error("%s", errorMsg.c_str());
  }
}

static int execute_command_line(const char * file, int argc, char **argv) {
  hphp_process_init();
  ExecutionContext *context = g_context.get();
  hphp_session_init();

  SystemGlobals *g = (SystemGlobals *)get_global_variables();
  process_env_variables(g->gv__ENV);
  g->gv__ENV.set("HPHP", 1);

  process_cmd_arguments(argc, argv);

  Variant &server = g->gv__SERVER;
  server.set("DOCUMENT_ROOT", "");
  server.set("SCRIPT_FILENAME", argv[0]);
  server.set("SCRIPT_NAME", argv[0]);
  server.set("PHP_SELF", argv[0]);
  server.set("argv", g->gv_argv);
  server.set("argc", g->gv_argc);
  server.set("PWD", g_context->getCwd());
  char hostname[1024];
  if (!gethostname(hostname, 1024)) {
    server.set("HOSTNAME", String(hostname, CopyString));
  }

  if (RuntimeOption::EnableCliRTTI) RTTIInfo::TheRTTIInfo.init(true);

  int exitCode = -1;
  bool ret = false;
  string errorMsg = "";
  bool error;
  ret = hphp_invoke(context, file, false, Array(), null, "", "", error,
                    errorMsg);
  hphp_context_exit(context, true);
  hphp_session_exit();
  if (ret) {
    exitCode = ExitException::ExitCode;
  }
  return exitCode;
}

static int start_server(const std::string &popenLog,
                        const std::string &username) {
  // Load all code into memory to prevent slow initial queries while
  // binary is pulled from disk
  if (mlockall(MCL_CURRENT)) {
    Logger::Warning("Could not mlockall");
  }

  if (!popenLog.empty()) {
    Logger::Output = popen(popenLog.c_str(), "w");
  }
  RuntimeOption::ExecutionMode = "srv";

  if (!username.empty()) {
    Capability::ChangeUnixUser(username);
    LightProcess::change_user(username);
  }

  HttpServer::Server = HttpServerPtr(new HttpServer());
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

  ostringstream out;
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

struct ProgramOptions {
  string mode;
  string config;
  vector<string> confStrings;
  int    port;
  int    admin_port;
  string user;
  string file;
  int    count;
  bool   noSafeAccessCheck;
  vector<string> args;
  string buildId;
};

int execute_program(int argc, char **argv) {
  string usage = "Usage:\n\n\t";
  usage += argv[0];
  usage += " [-m <mode>] [<options>] [<arg1>] [<arg2>] ...\n\nOptions";

  ProgramOptions po;
  options_description desc(usage.c_str());
  desc.add_options()
    ("help", "display this message")
    ("mode,m", value<string>(&po.mode)->default_value("run"),
     "run | server | daemon | replay | translate")
    ("config,c", value<string>(&po.config),
     "load specified config file")
    ("config-value,v", value<vector<string> >(&po.confStrings)->composing(),
     "individual configuration string in a format of name=value, where "
     "name can be any valid configuration for a config file")
    ("port,p", value<int>(&po.port)->default_value(-1),
     "start an HTTP server at specified port")
    ("admin-port", value<int>(&po.admin_port)->default_value(-1),
     "start admin listerner at specified port")
    ("user,u", value<string>(&po.user),
     "run server under this user account")
    ("file,f", value<string>(&po.file),
     "executing specified file")
    ("count", value<int>(&po.count)->default_value(1),
     "how many times to repeat execution")
    ("no-safe-access-check",
      value<bool>(&po.noSafeAccessCheck)->default_value(false),
     "whether to ignore safe file access check")
    ("arg", value<vector<string> >(&po.args)->composing(),
     "arguments")
    ("extra-header", value<string>(&Logger::ExtraHeader),
     "extra-header to add to log lines")
    ("build-id", value<string>(&po.buildId),
     "unique identifier of compiled server code")
    ;

  positional_options_description p;
  p.add("arg", -1);
  variables_map vm;
  try {
    store(command_line_parser(argc, argv).options(desc).positional(p).run(),
          vm);
    notify(vm);
  } catch (error &e) {
    cerr << "Error in command line: " << e.what() << "\n\n";
    cout << desc << "\n";
    return -1;
  } catch (...) {
    cerr << "Error in command line:\n\n";
    cout << desc << "\n";
    return -1;
  }
  if (vm.count("help")) {
    cout << desc << "\n";
    return -1;
  }

  Hdf config;
  if (!po.config.empty()) {
    config.open(po.config);
  }
  for (unsigned int i = 0; i < po.confStrings.size(); i++) {
    config.fromString(po.confStrings[i].c_str());
  }
  RuntimeOption::Load(config);

  LightProcess::initialize();

  RuntimeOption::BuildId = po.buildId;
  if (po.port != -1) {
    RuntimeOption::ServerPort = po.port;
  }
  if (po.admin_port != -1) {
    RuntimeOption::AdminServerPort = po.admin_port;
  }
  if (po.noSafeAccessCheck) {
    RuntimeOption::SafeFileAccess = false;
  }
  if (argc <= 1 || po.mode == "run") {
    RuntimeOption::ExecutionMode = "cli";

    int new_argc = po.args.size() + 1;
    char **new_argv = (char **)malloc((new_argc + 1) * sizeof(char*));
    new_argv[0] = argv[0];
    for (int i = 1; i < new_argc; i++) {
      new_argv[i] = (char*)po.args[i-1].c_str();
    }
    new_argv[new_argc] = NULL;

    int ret = -1;
    for (int i = 0; i < po.count; i++) {
      ret = execute_command_line(po.file.c_str(), new_argc, new_argv);
    }

    free(new_argv);
    return ret;
  }

  string popenLog;
  if (RuntimeOption::LogFile[0] == '|') {
    popenLog = RuntimeOption::LogFile.substr(1);
    RuntimeOption::LogFile.clear();
  }

  if (po.mode == "daemon") {
    Process::Daemonize(RuntimeOption::LogFile.c_str(),
                       RuntimeOption::LogFile.c_str());
    return start_server(popenLog, po.user);
  }
  if (po.mode == "server") {
    return start_server(popenLog, po.user);
  }

  if (po.mode == "replay" && !po.args.empty()) {
    RuntimeOption::RecordInput = false;
    RuntimeOption::ExecutionMode = "srv";
    HttpServer server; // so we initialize runtime properly
    HttpRequestHandler handler;
    for (int i = 0; i < po.count; i++) {
      ReplayTransport rt;
      rt.replayInput(po.args[0].c_str());
      handler.handleRequest(&rt);
      printf("%s\n", rt.getResponse().c_str());
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
  String path = Util::canonicalize(string(p.c_str()));
  if (path.charAt(0) == '/') {
    string &sourceRoot = RuntimeOption::SourceRoot;
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

class WarmupState {
public:
  WarmupState() : done(false), enabled(false),
                  atCheckpoint(false), failed(false) {}
  bool done;
  bool enabled;
  bool atCheckpoint;
  bool failed;
};
static ThreadLocal<WarmupState> s_warmup_state;

void hphp_process_init() {
  init_static_variables();
  Process::InitProcessStatics();
  PageletServer::Restart();
  XboxServer::Restart();
}

IMPLEMENT_THREAD_LOCAL(int, RecursionInjection::s_stackdepth);

void hphp_session_init() {
  RequestInjection::s_reqInjectionData->started = time(0);
  RequestInjection::s_reqInjectionData->timedout = false;
  *RecursionInjection::s_stackdepth = 0;
  *FrameInjection::s_top = NULL;

  MemoryManager::TheMemoryManager()->resetStats();

  if (!s_warmup_state->done) {
    free_global_variables(); // just to be safe
    init_global_variables();
  }
}

bool hphp_is_warmup_enabled() {
  return s_warmup_state->enabled;
}

void hphp_set_warmup_enabled() {
  s_warmup_state->enabled = true;
}

ExecutionContext *hphp_context_init() {
  ExecutionContext *context = g_context.get();
  context->obStart();
  context->obProtect(true);
  return context;
}

bool hphp_warmup(ExecutionContext *context, const std::string &warmupDoc,
                 const std::string reqInitFunc, bool &error) {
  bool ret = true;
  error = false;
  std::string errorMsg;
  if (!s_warmup_state->done) {
    MemoryManager *mm = MemoryManager::TheMemoryManager().get();
    if (mm->beforeCheckpoint()) {
      if (!s_warmup_state->failed && !warmupDoc.empty()) {
        try {
          s_warmup_state->enabled = true;
          ServerStatsHelper ssh("warmup");
          invoke_file(warmupDoc, true, get_variable_table());
        } catch (...) {
          ret = handle_exception(context, errorMsg, WarmupDocException, error);
        }
      }
      if (!ret) {
        hphp_session_init();
        s_warmup_state->enabled = false;
        s_warmup_state->failed = true;
        return ret;
      }
      s_warmup_state->done = true;
      mm->checkpoint();
      s_warmup_state->atCheckpoint = true;
    }
  }

  if (!reqInitFunc.empty() && s_warmup_state->enabled &&
      s_warmup_state->atCheckpoint) {
    ServerStatsHelper ssh("reqinit");
    try {
      invoke(reqInitFunc.c_str(), Array());
    } catch (...) {
      ret = handle_exception(context, errorMsg, ReqInitException, error);
    }
  }
  s_warmup_state->atCheckpoint = false;
  return ret;
}
bool hphp_invoke(ExecutionContext *context, const std::string &cmd,
                 bool func, CArrRef funcParams, Variant funcRet,
                 const std::string &warmupDoc, const std::string reqInitFunc,
                 bool &error, std::string &errorMsg) {
  bool ret = false;
  bool isServer = (strcmp(RuntimeOption::ExecutionMode, "srv") == 0);
  error = false;
  String oldCwd;
  try {
    try {
      if (isServer) {
        oldCwd = context->getCwd();
        hphp_chdir_file(warmupDoc);
      }
      if (!hphp_warmup(context, warmupDoc, reqInitFunc, error)) {
        if (isServer) context->setCwd(oldCwd);
        return false;
      }
      ServerStatsHelper ssh("invoke");
      if (func) {
        funcRet = invoke(cmd.c_str(), funcParams);
      } else {
        if (isServer) hphp_chdir_file(cmd);
        invoke_file(cmd.c_str(), true, get_variable_table());
      }
      ret = true;
      context->onShutdownPreSend();
    } catch (...) {
      ret = handle_exception(context, errorMsg, InvokeException, error);
    }
  } catch (...) {
    ret = handle_exception(context, errorMsg, HandlerException, error);
  }
  if (isServer) context->setCwd(oldCwd);
  return ret;
}

void hphp_context_exit(ExecutionContext *context, bool psp) {
  if (psp) {
    ServerStats::SetThreadMode(ServerStats::PostProcessing);
    try {
      try {
        ServerStatsHelper ssh("psp");
        context->onShutdownPostSend();
      } catch (ExitException &e) {
        // do nothing
      } catch (Exception &e) {
        context->onError(e);
      } catch (Object &e) {
        context->onException(e);
      }
    } catch (...) {
      Logger::Error("unknown exception was thrown from psp");
    }
    ServerStats::SetThreadMode(ServerStats::Idling);
  }

  context->onRequestShutdown();
  context->obProtect(false);
  context->obEnd();
}

void hphp_session_exit() {
  // Server note has to live long enough for the access log to fire.
  // RequestLocal is too early.
  ServerNote::Reset();
  g_context.reset();

  MemoryManager *mm = MemoryManager::TheMemoryManager().get();
  if (RuntimeOption::CheckMemory) {
    mm->checkMemory(false);
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemoryStats) {
    mm->logStats();
  }
  mm->resetStats();

  if (mm->afterCheckpoint()) {
    ServerStatsHelper ssh("rollback");
    mm->rollback();
    s_warmup_state->atCheckpoint = true;
  } else {
    ServerStatsHelper ssh("free");
    free_global_variables();
  }
}

void hphp_process_exit() {
  // do nothing
}

///////////////////////////////////////////////////////////////////////////////
}
