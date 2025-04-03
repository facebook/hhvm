/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

#include <fcntl.h>
#include <signal.h>

#include <folly/String.h>
#include <folly/portability/Stdlib.h>
#include <folly/portability/SysTime.h>
#include <folly/portability/Unistd.h>

#include "hphp/util/light-process.h"
#include "hphp/util/process.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/rds-local.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/server/cli-server.h"

# define MAYBE_WIFEXITED(var) if (WIFEXITED(var)) { var = WEXITSTATUS(var); }

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// build environment pair list
static char **build_envp(const Array& envs, std::vector<std::string> &senvs) {
  char **envp = nullptr;
  int size = envs.size();
  if (size) {
    envp = (char **)malloc((size + 1) * sizeof(char *));
    for (ArrayIter iter(envs); iter; ++iter) {
      senvs.push_back(folly::sformat("{}={}",
                                     iter.first().toString(),
                                     iter.second().toString()));
    }
    int i = 0;
    for (auto& env : senvs) {
      *(envp + i++) = (char *)env.data();
    }
    *(envp + i) = nullptr;
  }
  return envp;
}

///////////////////////////////////////////////////////////////////////////////
// popen

#define EXEC_INPUT_BUF 4096

namespace {

struct ShellExecContext final {
  ShellExecContext() {
    // Use the default handler while exec'ing in a shell,
    // saving the previous signal action.
    struct sigaction sa = {};
    sa.sa_handler = SIG_DFL;
    if (sigaction(SIGCHLD, &sa, &m_old_sa) != 0) {
      Logger::Error("Couldn't install SIGCHLD handler");
      abort();
    }
  }

  ~ShellExecContext() {
    if (m_proc) {
      LightProcess::pclose(m_proc);
    }
    if (m_old_sa.sa_handler != SIG_DFL) {
      if (sigaction(SIGCHLD, &m_old_sa, nullptr) != 0) {
        Logger::Error("Couldn't restore SIGCHLD handler");
        abort();
      }
    }
  }

  FILE *exec(const String& cmd_string) {
    assertx(m_proc == nullptr);
    const auto cmd = cmd_string.c_str();
    if (strlen(cmd) != cmd_string.size()) {
      raise_warning("NULL byte detected. Possible attack");
      return nullptr;
    }
    m_proc = LightProcess::popen(cmd, "r", g_context->getCwd().data());

    if (m_proc == nullptr) {
      raise_warning("Unable to execute '%s'", cmd);
    }
    return m_proc;
  }

  int exit() {
    int status = LightProcess::pclose(m_proc);
    m_proc = nullptr;
    return status;
  }

private:
  struct sigaction m_old_sa = {};
  FILE *m_proc{nullptr};
};

}

Variant HHVM_FUNCTION(shell_exec,
                      const String& cmd) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(cmd);
  if (!fp) return init_null();
  StringBuffer sbuf;
  sbuf.read(fp);
  return sbuf.detach();
}

String HHVM_FUNCTION(exec,
                     const String& command,
                     Array& output,
                     int64_t& return_var) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return empty_string();
  StringBuffer sbuf;
  sbuf.read(fp);

  Array lines = StringUtil::Explode(sbuf.detach(), "\n").toArray();
  int ret = ctx.exit();
  MAYBE_WIFEXITED(ret);
  return_var = ret;
  int count = lines.size();
  if (count > 0 && lines[count - 1].toString().empty()) {
    count--; // remove explode()'s last empty line
  }

  VecInit pai(count);
  for (int i = 0; i < count; i++) {
    pai.append(HHVM_FN(rtrim)(lines[i].toString(), "\f\n\r\t\x0b\x00 "));
  }
  output = pai.toArray();

  if (!count || lines.empty()) {
    return String();
  }

  return HHVM_FN(rtrim)(lines[count - 1].toString());
}

void HHVM_FUNCTION(passthru,
                   const String& command,
                   int64_t& return_var) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return;

  char buffer[1024];
  while (true) {
    int len = read(fileno(fp), buffer, sizeof(buffer) - 1);
    if (len == -1 && errno == EINTR) continue;
    if (len <= 0) break; // break on error or EOF
    buffer[len] = '\0';
    g_context->write(String(buffer, len, CopyString));
  }
  int ret = ctx.exit();
  MAYBE_WIFEXITED(ret);
  return_var = ret;
}

String HHVM_FUNCTION(system,
                     const String& command,
                     int64_t& return_var) {
  ShellExecContext ctx;
  FILE *fp = ctx.exec(command);
  if (!fp) return empty_string();
  StringBuffer sbuf;
  if (fp) {
    sbuf.read(fp);
  }

  Array lines = StringUtil::Explode(sbuf.detach(), "\n").toArray();
  int ret = ctx.exit();
  MAYBE_WIFEXITED(ret);
  return_var = ret;
  int count = lines.size();
  if (count > 0 && lines[count - 1].toString().empty()) {
    count--; // remove explode()'s last empty line
  }

  auto& ectx = *g_context;
  for (int i = 0; i < count; i++) {
    ectx.write(lines[i].toString());
    ectx.write("\n");
  }
  if (!count || lines.empty()) {
    return String();
  }

  return HHVM_FN(rtrim)(lines[count - 1].toString());
}

///////////////////////////////////////////////////////////////////////////////
// proc

struct ChildProcess : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(ChildProcess)

  pid_t child;
  Array pipes;
  String command;
  Variant env;

  CLASSNAME_IS("process");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  int close() {
    // Although the PHP doc about proc_close() says that the pipes need to be
    // explicitly pclose()'ed, it seems that Zend is implicitly closing the
    // pipes when proc_close() is called.
    for (ArrayIter iter(pipes); iter; ++iter) {
      cast<PlainFile>(iter.second())->close();
    }
    pipes.clear();

    pid_t wait_pid;
    int wstatus;
    do {
      wait_pid = LightProcess::waitpid(child, &wstatus, 0,
                                       Cfg::Server::RequestTimeoutSeconds);
    } while (wait_pid == -1 && errno == EINTR);

    if (wait_pid == -1) {
      return -1;
    }

    MAYBE_WIFEXITED(wstatus);
    return wstatus;
  }
};

void ChildProcess::sweep() {
  // do nothing here, as everything will be collected by MemoryManager
}

#define DESC_PIPE       1
#define DESC_FILE       2
#define DESC_PARENT_MODE_WRITE  8

const StaticString s_w("w");

struct DescriptorItem {
private:
  typedef int FileDescriptor;
  static constexpr int defaultFd = -1;
  static int dupFd(int fd) {
    return dup(fd);
  }
  static void closeFd(int fd) {
    if (fd >= 0) {
      close(fd);
    }
  }

public:
  DescriptorItem() :
    index(-1), parentend(defaultFd), childend(defaultFd),
    mode(-1), mode_flags(-1) {
  }

  ~DescriptorItem() {
  }

  void cleanup() {
    closeFd(childend);
    closeFd(parentend);
  }

  int index;               // desired fd number in child process
  FileDescriptor parentend, childend; // fds for pipes in parent/child
  int mode;                // mode for proc_open code
  int mode_flags;          // mode flags for opening fds

  static Mutex s_mutex;    // Prevents another thread from forking at the
                           // same time, before FD_CLOEXEC is set on the fds.
                           // NOTE: no need to lock with light processes.

  bool readFile(const req::ptr<File>& file) {
    mode = DESC_FILE;
    childend = dupFd(file->fd());
    if (childend < 0) {
      raise_warning("unable to dup File-Handle for descriptor %d - %s",
                      index, folly::errnoStr(errno).c_str());
      return false;
    }
    return true;
  }

  bool readPipe(const String& zmode) {
    mode = DESC_PIPE;
    FileDescriptor newpipe[2];
    if (0 != pipe(newpipe)) {
      raise_warning("unable to create pipe %s",
                      folly::errnoStr(errno).c_str());
      return false;
    }

    if (zmode != s_w) {
      parentend = newpipe[1];
      childend = newpipe[0];
      mode |= DESC_PARENT_MODE_WRITE;
    } else {
      parentend = newpipe[0];
      childend = newpipe[1];
    }
    mode_flags = mode & DESC_PARENT_MODE_WRITE ? O_WRONLY : O_RDONLY;
    return true;
  }

  bool openFile(const String& zfile, const String& zmode) {
    mode = DESC_FILE;
    /* try a wrapper */
    Variant vfile = HHVM_FN(fopen)(zfile.c_str(), zmode.c_str());
    if (!vfile.isResource()) {
      raise_warning("Unable to open specified file: %s (mode %s)",
                      zfile.data(), zmode.data());
      return false;
    } else {
      auto file = cast<File>(vfile);
      file->flush();
      childend = dupFd(file->fd());
      if (childend < 0) {
        raise_warning("unable to dup File-Handle for descriptor %d - %s",
                        index, folly::errnoStr(errno).c_str());
        return false;
      }
      return true;
    }
  }

  void dupChild() {
    if ((mode & ~DESC_PARENT_MODE_WRITE) == DESC_PIPE) {
      closeFd(parentend);
      parentend = defaultFd;
    }
    if (dup2(childend, index) < 0) {
      perror("dup2");
    }
    if (childend != index) {
      closeFd(childend);
      childend = defaultFd;
    }
  }

  /* clean up all the child ends and then open streams on the parent
   * ends, where appropriate */
  OptResource dupParent() {
    closeFd(childend);
    childend = defaultFd;

    if ((mode & ~DESC_PARENT_MODE_WRITE) == DESC_PIPE) {
      /* mark the descriptor close-on-exec, so that it won't be inherited
         by potential other children */
      fcntl(parentend, F_SETFD, FD_CLOEXEC);
      return OptResource(req::make<PlainFile>(parentend, true));
    }

    return OptResource();
  }
};

/**
 * This mutex must be non-reentrant so when the child process tries to unlock
 * it after a fork(), the call to pthread_mutex_unlock() will succeed.
 */
Mutex DescriptorItem::s_mutex(false);

const StaticString s_pipe("pipe");
const StaticString s_file("file");

static bool pre_proc_open(const Array& descriptorspec,
                          std::vector<DescriptorItem> &items) {
  /* walk the descriptor spec and set up files/pipes */
  items.resize(descriptorspec.size());
  int i = 0;
  for (ArrayIter iter(descriptorspec); iter; ++iter, ++i) {
    DescriptorItem &item = items[i];

    auto const index = iter.first().toString();
    if (!index.isNumeric()) {
      raise_warning("descriptor spec must be an integer indexed array");
      break;
    }
    item.index = (int)index.toInt64();

    Variant descitem = iter.second();
    if (descitem.isResource()) {
      auto file = cast<File>(descitem);
      if (!item.readFile(file)) break;
    } else if (!descitem.isArray()) {
      raise_warning("Descriptor must be either an array or a File-Handle");
      break;
    } else {
      Array descarr = descitem.toArray();
      if (!descarr.exists(int64_t(0))) {
        raise_warning("Missing handle qualifier in array");
        break;
      }
      String ztype = descarr[int64_t(0)].toString();
      if (ztype == s_pipe) {
        if (!descarr.exists(int64_t(1))) {
          raise_warning("Missing mode parameter for 'pipe'");
          break;
        }
        if (!item.readPipe(descarr[int64_t(1)].toString())) break;
      } else if (ztype == s_file) {
        if (!descarr.exists(int64_t(1))) {
          raise_warning("Missing file name parameter for 'file'");
          break;
        }
        if (!descarr.exists(int64_t(2))) {
          raise_warning("Missing mode parameter for 'file'");
          break;
        }
        if (!item.openFile(descarr[int64_t(1)].toString(),
                           descarr[int64_t(2)].toString())) {
          break;
        }
      } else {
        raise_warning("%s is not a valid descriptor spec", ztype.data());
        break;
      }
    }
  }

  if (i >= descriptorspec.size()) return true;
  for (int j = 0; j < i; j++) {
    items[j].cleanup();
  }
  return false;
}

static Variant post_proc_open(const String& cmd, Array& pipes,
                              const Variant& env,
                              std::vector<DescriptorItem> &items,
                              pid_t child
                              ) {
  if (child < 0) {
    /* failed to fork() */
    for (auto& item : items) {
      item.cleanup();
    }
    raise_warning("fork failed - %s", folly::errnoStr(errno).c_str());
    return false;
  }

  /* we forked/spawned and this is the parent */
  auto proc = req::make<ChildProcess>();
  proc->command = cmd;
  proc->child = child;
  proc->env = env;

  // need to set pipes to a new empty array, ignoring whatever it was
  // previously set to
  pipes = Array::CreateDict();

  for (auto& item : items) {
    OptResource f = item.dupParent();
    if (!f.isNull()) {
      proc->pipes.append(f);
      pipes.set(item.index, f);
    }
  }
  return Variant(std::move(proc));
}

Variant
HHVM_FUNCTION(proc_open, const String& cmd, const Array& descriptorspec,
              Array& pipes, const Variant& cwd /* = uninit_variant */,
              const Variant& env /* = uninit_variant */,
              const Variant& /*other_options*/ /* = uninit_variant */) {
  if (cmd.size() != strlen(cmd.c_str())) {
    raise_warning("NULL byte detected. Possible attack");
    return false;
  }

  std::vector<DescriptorItem> items;

  std::string scwd;
  if (!cwd.isNull() && cwd.isString() && !cwd.asCStrRef().empty()) {
    scwd = cwd.asCStrRef().c_str();
  } else if (!g_context->getCwd().empty()) {
    scwd = g_context->getCwd().c_str();
  }

  Array enva;

  if (env.isNull()) {
    if (is_cli_server_mode()) {
      enva = cli_env();
    } else {
      // Build out an environment that conceptually matches what we'd
      // see if we were to iterate the environment and call getenv()
      // for each name.

      // Env vars defined in the hdf file go in first
      for (const auto& envvar : RuntimeOption::EnvVariables) {
        enva.set(String(envvar.first), String(envvar.second));
      }

      // global environment overrides the hdf
      for (char **env = environ; env && *env; env++) {
        char *p = strchr(*env, '=');
        if (p) {
          String name(*env, p - *env, CopyString);
          String val(p + 1, CopyString);
          enva.set(name, val);
        }
      }
    }

    // and then any putenv() changes take precedence
    for (ArrayIter iter(g_context->getEnvs()); iter; ++iter) {
      enva.set(iter.first(), iter.second());
    }
  } else {
    enva = env.toArray();
  }

  pid_t child;

  if (LightProcess::Available()) {
    // light process available
    // there is no need to do any locking, because the forking is delegated
    // to the light process
    if (!pre_proc_open(descriptorspec, items)) return false;
    const int item_size = items.size();
    std::vector<int> created;
    created.reserve(item_size);
    std::vector<int> intended;
    intended.reserve(item_size);
    for (int i = 0; i < item_size; i++) {
      const auto& item = items[i];
      created.push_back(item.childend);
      intended.push_back(item.index);
    }

    std::vector<std::string> envs;
    for (ArrayIter iter(enva); iter; ++iter) {
      StringBuffer nvpair;
      nvpair.append(iter.first().toString());
      nvpair.append('=');
      nvpair.append(iter.second().toString());
      std::string tmp = nvpair.detach().c_str();
      if (tmp.find('\n') == std::string::npos) {
        envs.push_back(tmp);
      }
    }

    child = LightProcess::proc_open(cmd.c_str(), created, intended,
                                    scwd.c_str(), envs);
    assertx(child);
    return post_proc_open(cmd, pipes, enva, items, child);
  }

  std::vector<std::string> senvs; // holding those char *
  char** envp = nullptr;

  {
    /* the unix way */
    Lock lock(DescriptorItem::s_mutex);
    if (!pre_proc_open(descriptorspec, items)) return false;
    envp = build_envp(enva, senvs);
    child = fork();
    if (child) {
      // the parent process
      free(envp);
      return post_proc_open(cmd, pipes, enva, items, child);
    }
  }

  assertx(child == 0);
  /* this is the child process */

  /* close those descriptors that we just opened for the parent stuff,
   * dup new descriptors into required descriptors and close the original
   * cruft */
  for (auto& item : items) {
    item.dupChild();
  }
  if (scwd.length() > 0 && chdir(scwd.c_str())) {
    // chdir failed, the working directory remains unchanged
  }
  execle("/bin/sh", "sh", "-c", cmd.data(), nullptr, envp);
  free(envp);
  _exit(HPHP_EXIT_FAILURE);
}

bool HHVM_FUNCTION(proc_terminate,
                   const OptResource& process,
                   int64_t signal /* = SIGTERM */) {
  return kill(cast<ChildProcess>(process)->child, signal) == 0;
}

int64_t HHVM_FUNCTION(proc_close,
                      const OptResource& process) {
  return cast<ChildProcess>(process)->close();
}

const StaticString
  s_command("command"),
  s_pid("pid"),
  s_running("running"),
  s_signaled("signaled"),
  s_stopped("stopped"),
  s_exitcode("exitcode"),
  s_termsig("termsig"),
  s_stopsig("stopsig");

Array HHVM_FUNCTION(proc_get_status,
                    const OptResource& process) {
  auto proc = cast<ChildProcess>(process);

  errno = 0;
  bool running = true, signaled = false, stopped = false;
  int exitcode = -1, termsig = 0, stopsig = 0;
  int wstatus;
  pid_t wait_pid =
    LightProcess::waitpid(proc->child, &wstatus, WNOHANG|WUNTRACED);

  if (wait_pid == proc->child) {
    if (WIFEXITED(wstatus)) {
      running = false;
      exitcode = WEXITSTATUS(wstatus);
    }
    if (WIFSIGNALED(wstatus)) {
      running = false;
      signaled = true;
      termsig = WTERMSIG(wstatus);
    }
    if (WIFSTOPPED(wstatus)) {
      stopped = true;
      stopsig = WSTOPSIG(wstatus);
    }
  } else if (wait_pid == -1) {
    running = false;
  }

  return make_dict_array(
    s_command,  proc->command,
    s_pid, (int)proc->child,
    s_running,  running,
    s_signaled, signaled,
    s_stopped,  stopped,
    s_exitcode, exitcode,
    s_termsig,  termsig,
    s_stopsig,  stopsig
  );
}

bool HHVM_FUNCTION(proc_nice,
                   int64_t increment) {
  errno = 0;
  if (nice(increment) == -1 && errno) {
    raise_warning("Only a super user may attempt to increase the "
                    "priority of a process");
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// string functions

const StaticString s_twosinglequotes("''");

String HHVM_FUNCTION(escapeshellarg,
                     const String& arg) {
  if (!arg.empty()) {
    return string_escape_shell_arg(arg.c_str());
  } else {
    return String(s_twosinglequotes);
  }
}

String HHVM_FUNCTION(escapeshellcmd,
                     const String& command) {
  if (!command.empty()) {
    return string_escape_shell_cmd(command.c_str());
  }
  return command;
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::registerNativeProcess() {
  HHVM_FE(shell_exec);
  HHVM_FE(exec);
  HHVM_FE(passthru);
  HHVM_FE(system);
  HHVM_FE(proc_open);
  HHVM_FE(proc_terminate);
  HHVM_FE(proc_close);
  HHVM_FE(proc_get_status);
  HHVM_FE(proc_nice);
  HHVM_FE(escapeshellarg);
  HHVM_FE(escapeshellcmd);
}

///////////////////////////////////////////////////////////////////////////////
}
