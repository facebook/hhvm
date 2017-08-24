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

/*

Welcome to the CLI Server!

This file implements a unix socket based server for spawning "requests" which
behave (by-in-large) as though they were scripts executing in the context of
the client's UID. In order to achieve this the file implements the following
classes and abstractions:

================================ [ The Server ] ================================

== CLIServer

This class is a singleton representing an active CLIServer bound to a unix
socket. It carries with it a state: UNDEFINED, READY, RUNNING, or STOPPED.

 - The server can be constructed via init_cli_server() which will call
   CLIServer::CLIServer. This will bind() the unix socket and construct
   the AsyncServerSocket.

 - In order to begin accepting connections, start_cli_server() must be called,
   it will spawn an event loop thread and begin accept()-ing connections. This
   function will also construct a JobQueueDispatcher to handle connections.

 - Once teardown_cli_server() is called the server will stop accepting new
   connections and shutdown the dispatcher. This function will join the event
   loop thread and then destruct the singleton instance of the CLIServer.

After constructing a CLIServer it is "READY" and once start() is called it will
either be "RUNNING" or if listen() or accept() fails "STOPPED".

A running CLIServer has an AsyncServerSocket bound to the unix socket on which
requests are served, as well as a std::thread executing the event loop for its
socket.

CLIServer offers separate init and start entry points in order to allow callers
to bind() the unix socket prior to dropping privileges, but defer new
connections until after privileges have been reset. This enables callers to
bind their sockets in privileged locations.

The teardown function must be called prior to process exit as JobQueue threads
may access global server state in their thread exit handlers.

== CLIWorker

These workers execute requests sent to the CLIServer and are managed by the
JobQueueDispatcher. The CLIWorker::doJob() function contains all of the server
side logic to extract the startup parameters from the client, manipulate the
ExecutionContext, and ultimately start the request via hphp_invoke.

Before beginning execution the doJob routine reads information from the client
over its unix socket. Most importantly it receives the client's stdin, stdout,
and stderr file-descriptors, a file-descriptor to communicate with the client's
light process pool, the command line arguments, and a JSON blob of INI settings.

The setup performed in doJob resembles normal command line execution. Notably,
however, the PHP_INI_USER options are read from the client as a JSON string and
used to override the server defaults, the STD[IN,OUT,ERR] constants are defined
the ExecutionContext is instructed to write to the client's stdout, and the
LightProcess abstraction is given an override socket to allow the client to
spawn child processes via its own light process pool (so that commands executed
by proc_open and friends have the correct uid/gid). Additionally the cwd is set
to the client's cwd, the is_cli_mode() function returns true, and safe
directory access is disabled.

Lastly, a special file wrapper, CLIWrapper, is set to handle all local file
system operations.

== Monitor thread

The CLIWorker will spawn a special monitor thread just prior to starting
the request. This thread will poll() the unix socket and wait for flags that
indicate the client has hung-up, the socket is in an error state, or the
file-descriptor is no longer valid. At this point it will set the TimedOutFlag
on the request and terminate. This ensure that if the client hangs up (by ^C
for instance), the server does not continue spinning, and promptly releases the
clients I/O file descriptors (which may remain open if the underlying terminal
is still active).

== CLIWrapper

This special Stream::Wrapper implements all file:// IO for requests handled by
CLIServer. It proxies all syscalls to the client process over the unix socket
connection. The client responds with the results of these calls, and in the
case of open() and opendir(), with the actual file-descriptors it opened (via
SCM_RIGHTS).

================================ [ The Client ] ================================

== cli_process_command_loop

After establishing a connection and transferring INI-settings and file-
descriptors to the server, the client sits in the cli_process_command_loop. In
this loop the client handles requests from the CLIWrapper, commands are sent
as strings followed by arguments. The special "exit" command is sent when
the request completes along with an argument indicating the code with which
the client should terminate.

== run_command_on_cli_server

This is the main entry point for the client. It attempts to establish a
connection with the CLIServer and transfers the initialization information and
file descriptors required by the server. The file descriptor for a specially
constructed light process pool is also sent to the server. If the client is
unable to establish a connection the function will return allowing the caller
to execute locally or fail. If an error occurs after the connection is
established or the server completes the request the function will not return.
Should an error occur executing locally is unsafe as the client will have no
way to determine how much progress the server made.

*/

#include "hphp/runtime/server/cli-server.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/util/afdt-util.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/light-process.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/Sockets.h>

#include <afdt.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <utime.h>

TRACE_SET_MOD(clisrv);

namespace HPHP {

namespace {

template<class... Args>
void cli_write(int afdt_fd, Args&&... args) {
  FTRACE(4, "cli_write({}, nargs={})\n", afdt_fd, sizeof...(args) + 1);
  try {
    afdt::sendx(afdt_fd, std::forward<Args>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::sendRaw: %s [%s]",
                    ex.what(), folly::errnoStr(errno).c_str());
  }
}

template<class... Args>
void cli_read(int afdt_fd, Args&&... args) {
  FTRACE(4, "cli_read({}, nargs={})\n", afdt_fd, sizeof...(args) + 1);
  try {
    afdt::recvx(afdt_fd, std::forward<Args>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::recvRaw: %s [%s]",
                    ex.what(), folly::errnoStr(errno).c_str());
  }
}

int cli_read_fd(int afdt_fd) {
  int fd = afdt::recv_fd(afdt_fd);
  FTRACE(4, "{} = cli_read_fd({})\n", fd, afdt_fd);
  if (fd < 0) {
    throw Exception("Failed in afdt::recv_fd: %s",
                    folly::errnoStr(errno).c_str());
  }
  return fd;
}

void cli_write_fd(int afdt_fd, int fd) {
  FTRACE(4, "cli_write_fd({}, {})\n", afdt_fd, fd);
  if (!afdt::send_fd(afdt_fd, fd)) {
    throw Exception("Failed in afdt::send_fd: %s",
                    folly::errnoStr(errno).c_str());
  }
}

#ifdef SCM_CREDENTIALS

template<class F, class R = typename std::result_of<F(msghdr*)>::type>
R with_ucred_message(int fd, F func) {
  iovec iov;
  int data = 1;
  msghdr msgh;
  union {
    struct cmsghdr cmh;
    char space[CMSG_SPACE(sizeof(ucred))];
  } control_un;

  bzero(&control_un, sizeof(control_un));
  bzero(&msgh, sizeof(msgh));

  control_un.cmh.cmsg_len = CMSG_LEN(sizeof(ucred));
  control_un.cmh.cmsg_level = SOL_SOCKET;
  control_un.cmh.cmsg_type = SCM_CREDENTIALS;

  iov.iov_base = &data;
  iov.iov_len = sizeof(data);

  msgh.msg_iov = &iov;
  msgh.msg_iovlen = 1;
  msgh.msg_control = &control_un.cmh;
  msgh.msg_controllen = sizeof(control_un);

  auto set_passcred = [&] (int val) {
    if (setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &val, sizeof(val)) == -1) {
      throw Exception("Could not set socket to SO_PASSCRED: %s",
                      folly::errnoStr(errno).c_str());
    }
  };

  set_passcred(1);
  SCOPE_EXIT { set_passcred(0); };
  return func(&msgh);
}

ucred cli_read_ucred(int fd) {
  return with_ucred_message(fd, [&] (msghdr* msgh) -> ucred {
    ssize_t ret = recvmsg(fd, msgh, MSG_WAITALL);

    if (ret < 0) {
      FTRACE(4, "cli_read_ucred({}): %s\n", fd, folly::errnoStr(errno));
      throw Exception("cli_read_ucred: recvmsg failed: %s",
                      folly::errnoStr(errno).c_str());
    }

    auto cmhp = CMSG_FIRSTHDR(msgh);
    if (!cmhp ||
        cmhp->cmsg_len != CMSG_LEN(sizeof(ucred)) ||
        cmhp->cmsg_level != SOL_SOCKET ||
        cmhp->cmsg_type != SCM_CREDENTIALS) {
      FTRACE(4, "cli_read_ucred({}): bad payload\n");
      throw Exception("cli_read_ucred: malformed response");
    }


    auto uc = (ucred*)CMSG_DATA(cmhp);

    FTRACE(4, "cli_read_ucred({}): ret = {} UID = {}, GID = {}, PID = {}\n",
           fd, ret, uc->uid, uc->gid, uc->pid);

    return ucred {uc->pid, uc->uid, uc->gid};
  });
}

void cli_write_ucred(int fd) {
  return with_ucred_message(fd, [&] (msghdr* msgh) {
    auto cred = (ucred*)CMSG_DATA(CMSG_FIRSTHDR(msgh));

    cred->uid = getuid();
    cred->gid = getgid();
    cred->pid = getpid();

    ssize_t ret = sendmsg(fd, msgh, MSG_WAITALL);

    FTRACE(4, "cli_write_ucred({}): ret = {} UID = {}, GID = {}, PID = {}\n",
           fd, ret, cred->uid, cred->gid, cred->pid);

    if (ret < 0) {
      FTRACE(4, "cli_write_ucred({}): %s\n", fd, folly::errnoStr(errno));
      throw Exception("cli_write_ucred: sendmsg failed: %s",
                      folly::errnoStr(errno).c_str());
    }
  });
}

#else /* defined(SCM_CREDENTIALS) */

ucred cli_read_ucred(int fd) {
  ucred u;
  if (getpeereid(fd, &u.uid, &u.gid) == -1) {
    throw Exception("getpeerid: %s", folly::errnoStr(errno).c_str());
  }
  u.pid = getpid();

  return u;
}
void cli_write_ucred(int fd) {}

#endif /* defined(SCM_CREDENTIALS) */

////////////////////////////////////////////////////////////////////////////////

struct CLIWorker
  : JobQueueWorker<int,void*,true,false,JobQueueDropVMStack>
{
  void doJob(int fd) override;
  void abortJob(int fd) override {
    Logger::Warning("CLI request (%i) dropped because of timeout.", fd);
    close(fd);
  }
};

struct CLIWrapper final : Stream::ExtendedWrapper {
  explicit CLIWrapper(int fd) : m_cli_fd(fd) {}

  req::ptr<File> open(const String& filename,
                      const String& mode,
                      int options,
                      const req::ptr<StreamContext>& context) override;
  int access(const String& path, int mode) override;
  int lstat(const String& path, struct stat* buf) override;
  int stat(const String& path, struct stat* buf) override;
  int unlink(const String& path) override;
  int rename(const String& oldname, const String& newname) override;
  int mkdir(const String& path, int mode, int options) override;
  int rmdir(const String& path, int options) override;
  req::ptr<Directory> opendir(const String& path) override;
  String realpath(const String& path) override;

  // extended api
  bool touch(const String& path, int64_t mtime, int64_t atime) override;
  bool chmod(const String& path, int64_t mode) override;
  bool chown(const String& path, int64_t uid) override;
  bool chown(const String& path, const String& uid) override;
  bool chgrp(const String& path, int64_t gid) override;
  bool chgrp(const String& path, const String& gid) override;

  bool isNormalFileStream() const override { return true; }

private:
  int m_cli_fd;
};

struct CLIServer final : folly::AsyncServerSocket::AcceptCallback {
  explicit CLIServer(const char* path);
  ~CLIServer() = default;

  void start();
  void stop();

  void connectionAccepted(int fd,
                          const folly::SocketAddress&) noexcept override {
    if (RuntimeOption::EvalUnixServerFailWhenBusy) {
      if (m_dispatcher->getActiveWorker() >=
          m_dispatcher->getTargetNumWorkers()) {
        Logger::Info("Queue is full, refusing CLI connection");
        close(fd);
        return;
      }
    }

    Logger::Info("Accepting CLI connection");
    FTRACE(1, "CLIServer::connectionAccepted({}): enqueue job\n", fd);
    int opts = fcntl(fd, F_GETFL);
    int res = fcntl(fd, F_SETFL, opts & ~O_NONBLOCK);
    if (res) {
      Logger::Warning("Could not set socket to blocking: %s",
                      folly::errnoStr(errno).c_str());
      close(fd);
      return;
    }

    m_dispatcher->enqueue(fd);
  }

  void acceptError(const std::exception& ex) noexcept override {
    Logger::Warning("Error accepting connection: %s", ex.what());
  }

private:
  enum class State { UNINIT, READY, RUNNING, STOPPED };
  using JobQueue = JobQueueDispatcher<CLIWorker>;

  std::unique_lock<std::mutex> lockState() {
    return std::unique_lock<std::mutex>(m_stateLock);
  }
  void waitFor(State s) {
    auto lock = lockState();
    m_stateCV.wait(lock, [=] {
      return m_state.load(std::memory_order_relaxed) == s;
    });
  }
  void waitWhile(State s) {
    auto lock = lockState();
    m_stateCV.wait(lock, [=] {
      return m_state.load(std::memory_order_relaxed) != s;
    });
  }
  void setState(State s) {
    auto lock = lockState();
    m_state.store(s, std::memory_order_relaxed);
    lock.unlock();
    m_stateCV.notify_all();
  }
  State getState() const { return m_state.load(std::memory_order_relaxed); }

  folly::EventBase* m_ev{nullptr};
  std::atomic<State> m_state{State::UNINIT};
  std::mutex m_stateLock;
  std::condition_variable m_stateCV;
  std::thread m_mainThread;

  folly::AsyncServerSocket::UniquePtr m_server;
  std::unique_ptr<JobQueue> m_dispatcher;
};

CLIServer* s_cliServer{nullptr};
__thread ucred* tl_ucred{nullptr};
__thread int tl_cliSock{-1};
__thread Array* tl_env;

////////////////////////////////////////////////////////////////////////////////

CLIServer::CLIServer(const char* path) {
  int err = unlink(path);
  if (err != 0 && errno != ENOENT) {
    Logger::Warning("Could not delete defunct unix socket: %s (%s)",
                    path, folly::errnoStr(errno).c_str());

    setState(State::STOPPED);
    return;
  }

  folly::SocketAddress addr;
  addr.setFromPath(path);
  m_server.reset(new folly::AsyncServerSocket());

  try {
    m_server->bind(addr);
  } catch (const std::exception& ex) {
    Logger::Warning("Unable to bind unix socket: %s", ex.what());
    m_server.reset();
    setState(State::STOPPED);
    return;
  }

  if (chmod(path, 0666) == -1) {
    Logger::Warning("Unable to chmod unix socket: %s",
                    folly::errnoStr(errno).c_str());
  }

  setState(State::READY);
}

void CLIServer::start() {
  if (getState() != State::READY) return;

  m_dispatcher = std::make_unique<JobQueue>(
    RuntimeOption::EvalUnixServerWorkers,
    RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
    RuntimeOption::ServerThreadDropStack,
    nullptr
  );

  m_dispatcher->start();

  m_mainThread = std::thread([&] {
    m_ev = folly::EventBaseManager::get()->getEventBase();

    Logger::Info("CLI server thread starting");
    FTRACE(1, "CLIServer::start(): starting...\n");

    try {
      m_server->listen(RuntimeOption::ServerBacklog);
      m_server->startAccepting();
    } catch (const std::exception& ex) {
      Logger::Warning("Unable to begin accepting CLI connections: %s",
                      ex.what());
      m_dispatcher->stop();
      m_server.reset();
      m_dispatcher.reset();
      setState(State::STOPPED);
      return;
    }

    m_server->attachEventBase(m_ev);
    m_server->addAcceptCallback(this, nullptr);

    setState(State::RUNNING);
    m_ev->loop();
    Logger::Info("CLI server thread terminating");
  });

  waitWhile(State::READY);
}

void CLIServer::stop() {
  if (getState() != State::RUNNING) return;

  m_ev->runInEventBaseThread([&] {
    FTRACE(1, "CLIServer::stop(): stopping...\n");
    m_server.reset();

    setState(State::STOPPED);
  });

  m_dispatcher->stop();
  m_dispatcher.reset();
  waitFor(State::STOPPED);

  m_mainThread.join();
}

////////////////////////////////////////////////////////////////////////////////

namespace {

struct CliStdoutHook final : ExecutionContext::StdoutHook {
  int fd;
  explicit CliStdoutHook(int fd) : fd(fd) {}
  void operator()(const char* s, int len) override {
    write(fd, s, len);
  }
};

struct CliLoggerHook final : LoggerHook {
  int fd;
  explicit CliLoggerHook(int fd) : fd(fd) {}
  void operator()(const char* /*hdr*/, const char* msg, const char* ending)
       override {
    write(fd, msg, strlen(msg));
    if (ending) write(fd, ending, strlen(ending));
  }
};

}

const StaticString
  s_STDIN("STDIN"),
  s_STDOUT("STDOUT"),
  s_STDERR("STDERR");

void define_stdio_constants() {
  auto defcns = [] (const StringData* name, const Variant& (*func)()) {
    auto handle = makeCnsHandle(name, false);
    always_assert(handle != rds::kInvalidHandle);

    rds::initHandle(handle);
    auto cns = &rds::handleToRef<TypedValue>(handle);

    cns->m_type = KindOfUninit;
    cns->m_data.pref = reinterpret_cast<RefData*>(func);
  };

  defcns(s_STDIN.get(),  BuiltinFiles::GetSTDIN);
  defcns(s_STDOUT.get(), BuiltinFiles::GetSTDOUT);
  defcns(s_STDERR.get(), BuiltinFiles::GetSTDERR);
}

const StaticString
  s_local_value("local_value"),
  s_access("access");

Array init_ini_settings(const std::string& settings) {
  String s(settings.c_str(), CopyString);
  auto var = Variant::attach(HHVM_FN(json_decode)(s, true));

  FTRACE(1, "init_ini_settings: CLI server decoding settings\n");

  if (!var.isArray()) {
    Logger::Warning("CLI server received malformed INI settings: %.64s%s",
                    settings.c_str(), settings.size() > 64 ? "..." : "");
    return Array{};
  }

  FTRACE(1, "init_ini_settings: {} settings to process\n",
         var.toArray().size());

  UNUSED int count = 0;
  for (ArrayIter it(var.toArray()); it; ++it) {
    if (!it.first().isString()) {
      Logger::Warning("CLI server received a malformed INI setting name");
      continue;
    }
    if (!it.second().isString() && !it.second().isArray()) {
      Logger::Warning("CLI server received a malformed INI setting value");
      continue;
    }

    auto name = it.first().toString();
    auto detail = it.second().toArray();
    auto value = detail[s_local_value];

    if ((detail[s_access].toInt64() & IniSetting::PHP_INI_USER) == 0) {
      FTRACE(5, "init_ini_settings: skipping INI setting {} = {}\n",
             name.data(),
             value.isArray() || value.isObject()
               ? "[array]"
               : value.toString().c_str());
      continue;
    }
    FTRACE(5, "init_ini_settings: loading INI setting {} = {}\n",
           name.data(),
           value.isArray() ? "[array]" : value.toString().c_str());

    bool res = IniSetting::SetUser(name, value);
    if (!res) {
      FTRACE(5, "init_ini_settings: unable to set PHP_INI_USER setting: {} "
             "(access = {})\n", name, detail[s_access].toInt64());
      Logger::Warning("CLI server received an invalid INI setting: %s "
                      "(access = %li)",
                      name.data(), detail[s_access].toInt64());
    } else {
      count++;
    }
  }

  FTRACE(1, "init_ini_settings: loaded {} settings\n", count);
  return var.toArray();
}

std::unordered_set<uid_t> s_allowedUsers;
std::unordered_set<gid_t> s_allowedGroups;

struct UserInfo final {
  explicit UserInfo(const char* name) {
    passwd* retpwptr = nullptr;
    int pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (pwbuflen < 1)   {
      throw Exception("Could not get _SC_GETPW_R_SIZE_MAX");
    }
    pwbuf.reset(new char[pwbuflen]);

    if (getpwnam_r(name, &pwd, pwbuf.get(), pwbuflen, &retpwptr)) {
      throw Exception("getpwnam_r: %s", folly::errnoStr(errno).c_str());
    }

    if (!retpwptr) {
      throw Exception("getpwnam_r: no such user: %s", name);
    }
  }

  explicit UserInfo(uid_t uid) {
    passwd* retpwptr = nullptr;
    int pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (pwbuflen < 1)   {
      throw Exception("Could not get _SC_GETPW_R_SIZE_MAX");
    }
    pwbuf.reset(new char[pwbuflen]);

    if (getpwuid_r(uid, &pwd, pwbuf.get(), pwbuflen, &retpwptr)) {
      throw Exception("getpwuid_r: %s", folly::errnoStr(errno).c_str());
    }

    if (!retpwptr) {
      throw Exception("getpwuid_r: no such uid: %u", uid);
    }
  }

  passwd pwd;
  std::unique_ptr<char[]> pwbuf;
};

struct GroupInfo final {
  explicit GroupInfo(const char* name) {
    group* retgrptr = nullptr;
    int grbuflen = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (grbuflen < 1)   {
      throw Exception("Could not get _SC_GETGR_R_SIZE_MAX");
    }
    grbuf.reset(new char[grbuflen]);

    if (getgrnam_r(name, &gr, grbuf.get(), grbuflen, &retgrptr)) {
      throw Exception("getgrnam_r: %s", folly::errnoStr(errno).c_str());
    }

    if (!retgrptr) {
      throw Exception("getgrnam_r: no such group: %s", name);
    }
  }

  group gr;
  std::unique_ptr<char[]> grbuf;
};

void check_cli_server_access(ucred& cred) {
  // Fail closed
  if (s_allowedUsers.empty() && s_allowedGroups.empty()) {
    if (cred.uid == getuid()) return;
    throw Exception("access denied");
  }

  FTRACE(2, "Checking access for uid = {}\n", cred.uid);

  // Fast checks
  if (s_allowedUsers.find(cred.uid) != s_allowedUsers.end()) return;
  if (s_allowedGroups.find(cred.gid) != s_allowedGroups.end()) return;

  try {
    if (!s_allowedGroups.empty()) {
      FTRACE(2, "check_cli_server_access: starting slow check...\n");

      // The signature for getgrouplist differs for Apple
#ifdef __APPLE__
      std::vector<int> groups;
#else
      std::vector<gid_t> groups;
#endif
      int ngroups = 0;
      UserInfo user(cred.uid);
      if (getgrouplist(user.pwd.pw_name, cred.gid, nullptr, &ngroups) != -1) {
        FTRACE(2, "check_cli_server_access: oops 1...\n");
        throw Exception("getgrouplist: could not get ngroups");
      }
      FTRACE(2, "Will check {} groups for user {}\n",
             ngroups, user.pwd.pw_name);
      groups.resize(ngroups);
      if (getgrouplist(user.pwd.pw_name, cred.gid, &groups[0], &ngroups) < 0) {
        FTRACE(2, "check_cli_server_access: oops 2...\n");
        throw Exception("getgrouplist: invalid return value");
      }
      for (int i = 0; i < ngroups; ++i) {
        FTRACE(2, "Checking access for gid = {}\n", groups[i]);
        if (s_allowedGroups.find((gid_t)groups[i]) != s_allowedGroups.end()) {
          return;
        }
      }
    }
  } catch (const Exception& ex) {
    Logger::Warning("Error checking access to CLI server: %s", ex.what());
  }

  throw Exception("access denied");
}

const StaticString
  s_serverVariablesIni("hhvm.server_variables"),
  s_envVariablesIni("hhvm.env_variables");

Array init_cli_globals(int argc, char** argv, int xhprof, Array& ini,
                       char** envp) {
  auto make_map = [] (const Variant& v) {
    std::map<std::string,std::string> ret;
    if (!v.isArray()) return ret;
    auto ar = v.toArray();
    for (ArrayIter iter(ar); iter; ++iter) {
      if (iter.first().isString() && iter.second().isString()) {
        ret.emplace(
          std::string(iter.first().toString().data()),
          std::string(iter.second().toString().data())
        );
      }
    }
    return ret;
  };

  Array retEnv;
  std::map<std::string,std::string> envVariables;
  std::map<std::string,std::string> serverVariables;

  if (ini.exists(s_serverVariablesIni)) {
    serverVariables = make_map(ini[s_serverVariablesIni]);
  }
  if (ini.exists(s_envVariablesIni)) {
    envVariables = make_map(ini[s_envVariablesIni]);
    for (const auto& envvar : envVariables) {
      retEnv.set(String(envvar.first), String(envvar.second));
    }
  } else {
    retEnv = empty_array();
  }

  for (auto env = envp; *env; ++env) {
    char *p = strchr(*env, '=');
    if (p) {
      String name(*env, p - *env, CopyString);
      String val(p + 1, CopyString);
      retEnv.set(name, val);
    }
  }

  init_command_line_globals(argc, argv, envp, xhprof, envVariables,
                            serverVariables);

  return retEnv;
}

struct RemoteFile final {
  explicit RemoteFile(int client, const char* name, const char* mode) {
    fd =  cli_read_fd(client);
    file = fdopen(fd, mode);
    FTRACE(2, "CLIWorker::doJob({}): {} = {}\n", client, name, fd);
  }
  ~RemoteFile() { if (file) fclose(file); }

  FILE* file{nullptr};
  int fd{-1};
};

struct MonitorThread final {
  explicit MonitorThread(int client);
  ~MonitorThread() {
    if (m_monitor.joinable()) {
      write(m_wpipe, "stop", 5);
      m_monitor.join();
    }
  }

private:
  int m_wpipe{-1};
  std::thread m_monitor;
};

MonitorThread::MonitorThread(int client) {
  int monitor_pipe[2];
  if (pipe(monitor_pipe) == -1) {
    throw Exception("Unable to construct pipe for monitor: %s",
                    folly::errnoStr(errno).c_str());
  }

  int rpipe = monitor_pipe[0];
  int wpipe = monitor_pipe[1];
  auto flags = &stackLimitAndSurprise();
  try {
    m_monitor = std::thread([wpipe,rpipe,client,flags] {
      SCOPE_EXIT {
        close(rpipe);
        close(wpipe);
      };
      int ret = 0;
      pollfd pfd[] = {{client, 0, 0}, {rpipe, POLLIN, 0}};
      while ((ret = poll(pfd, 2, -1)) != -1) {
        if (ret == 0) continue;
        if (pfd[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
          FTRACE(2, "CLIWorker::doJob({}): observed socket reset in monitor\n",
                 client);
          Logger::Info("CLIWorker::doJob(%i): monitor thread aborting request",
                       client);
          break;
        }
        if (pfd[1].revents) {
          FTRACE(2, "CLIWorker::doJob({}): monitor got request completed\n",
                 client);
          return;
        }
      }
      if (ret == -1) {
        Logger::Warning("CLIWorker::doJob(%i): monitor thread terminated: %s",
                        client, folly::errnoStr(errno).c_str());
        FTRACE(2, "CLIWorker::doJob({}): got error in monitor thread: {}",
               client, folly::errnoStr(errno));
        return;
      }

      flags->fetch_or(TimedOutFlag);
      FTRACE(2, "CLIWorker::doJob({}): monitor exiting...\n", client);
    });
  } catch (const std::system_error& err) {
    close(rpipe);
    close(wpipe);
    throw Exception("Could not start monitor thread: %s", err.what());
  }

  m_wpipe = wpipe;
}

void CLIWorker::doJob(int client) {
  FTRACE(1, "CLIWorker::doJob({}): starting job...\n", client);
  try {
    auto uc = cli_read_ucred(client);

    // Throw if the client is not authorized to access the CLI server
    check_cli_server_access(uc);

    std::string magic;
    cli_read(client, magic);
    FTRACE(2, "CLIWorker::doJob({}): magic = {}\n", client, magic);
    if (magic != "hello_server") {
      throw Exception("Got bad magic from client: %s", magic.c_str());
    }

    std::string cwd;
    cli_read(client, cwd);
    FTRACE(1, "CLIWorker::doJob({}): cwd = {}\n", client, cwd);

    std::string iniSettings;
    cli_read(client, iniSettings);

    RemoteFile cli_in(client, "stdin", "r");
    RemoteFile cli_out(client, "stdout", "w");
    RemoteFile cli_err(client, "stderr", "w");
    RemoteFile cli_afdt(client, "afdt", "rw");

    int xhprofFlags;
    std::vector<std::string> args;
    std::vector<std::string> env;
    cli_read(client, xhprofFlags, args, env);

    FTRACE(2, "CLIWorker::doJob({}): xhprofFlags = {}\n", client, xhprofFlags);
    FTRACE(2, "CLIWorker::doJob({}): args = \n", client);

    auto buf = std::make_unique<char*[]>(args.size());
    for (int i = 0; i < args.size(); ++i) {
      FTRACE(2, "CLIWorker::doJob({}):          {}\n", client, args[i]);
      buf[i] = const_cast<char*>(args[i].c_str());
    }

    FTRACE(3, "CLIWorker::doJob({}): env = \n", client);

    auto envp = std::make_unique<char*[]>(env.size() + 1);

    for (int i = 0; i < env.size(); ++i) {
      FTRACE(3, "CLIWorker::doJob({}):          {}\n", client, env[i]);
      envp[i] = const_cast<char*>(env[i].c_str());
    }
    envp[env.size()] = nullptr;

    tl_ucred = &uc;
    SCOPE_EXIT { tl_ucred = nullptr; };

    tl_cliSock = client;
    SCOPE_EXIT { tl_cliSock = -1; };

    Array envArr;

    int ret = 255;
    init_command_line_session(args.size(), buf.get());
    {
      SCOPE_EXIT {
        tl_env = nullptr;
        envArr.reset();
        g_context->setStdout(nullptr);
        clearThreadLocalIO();
        LightProcess::setThreadLocalAfdtOverride(nullptr);
        Logger::SetThreadHook(nullptr);
        Stream::setThreadLocalFileHandler(nullptr);
        execute_command_line_end(xhprofFlags, true, args[0].c_str());
        try {
          cli_write(client, "exit", ret);
        } catch (const Exception& ex) {
          Logger::Warning("Could not send exit code %i to CLI socket: %s\n",
                          ret, ex.what());
        }
      };

      auto ini = init_ini_settings(iniSettings);

      envArr = init_cli_globals(args.size(), buf.get(), xhprofFlags, ini,
                                envp.get());
      tl_env = &envArr;

      CliStdoutHook stdout_hook(cli_out.fd);
      CliLoggerHook logging_hook(cli_err.fd);
      g_context->setStdout(&stdout_hook);
      g_context->setCwd(String(cwd.c_str(), CopyString));
      setThreadLocalIO(cli_in.file, cli_out.file, cli_err.file);
      LightProcess::setThreadLocalAfdtOverride(cli_afdt.fd);
      Logger::SetThreadHook(&logging_hook);

      CLIWrapper wrapper(client);
      Stream::setThreadLocalFileHandler(&wrapper);
      RID().setSafeFileAccess(false);
      define_stdio_constants();

      MonitorThread monitor(client);
      FTRACE(1, "CLIWorker::doJob({}): invoking {}...\n", client, args[0]);
      if (hphp_invoke_simple(args[0], false /* warmup only */)) {
        ret = ExitException::ExitCode;
      }
      FTRACE(2, "CLIWorker::doJob({}): waiting for monitor...\n", client);
    }
  } catch (const Exception& ex) {
    Logger::Warning("CLI Job failed: %s", ex.what());
  }

  if (close(client) == -1) {
    if (errno == EBADF || close(client) == -1) {
      Logger::Warning("CLIWorker::doJob(%i): failed to close socket: %s\n",
                      client, folly::errnoStr(errno).c_str());
    }
  }
  FTRACE(1, "CLIWorker::doJob({}): done.\n", client);
}

////////////////////////////////////////////////////////////////////////////////

req::ptr<File>
CLIWrapper::open(const String& filename, const String& mode, int options,
                 const req::ptr<StreamContext>& /*context*/) {
  String fname;
  if (StringUtil::IsFileUrl(filename)) {
    fname = StringUtil::DecodeFileUrl(filename);
    if (fname.empty()) {
      raise_warning("invalid file:// URL");
      return nullptr;
    }
  } else {
    fname = filename;
  }

  if (options & File::USE_INCLUDE_PATH) {
    struct stat s;
    String resolved_fname = resolveVmInclude(fname.get(), "", &s);
    if (!resolved_fname.isNull()) {
      fname = resolved_fname;
    }
  }

  bool res;
  std::string error;
  FTRACE(3, "CLIWrapper({})::open({}, {}, {}): calling remote...\n",
         m_cli_fd, fname.data(), mode.data(), options);
  cli_write(m_cli_fd, "open", fname.data(), mode.data());
  cli_read(m_cli_fd, res, error);
  FTRACE(3, "{} = CLIWrapper({})::open(...) [err = {}]\n",
         res, m_cli_fd, error);

  if (!res) {
    raise_warning("%s", error.c_str());
    return nullptr;
  }

  return req::make<PlainFile>(cli_read_fd(m_cli_fd));
}

req::ptr<Directory> CLIWrapper::opendir(const String& path) {
  auto tpath = File::TranslatePath(path);

  bool res;
  std::string error;

  FTRACE(3, "CLIWrapper({})::opendir({}) calling remote...\n",
         m_cli_fd, tpath.data());

  cli_write(m_cli_fd, "opendir", tpath.data());
  cli_read(m_cli_fd, res, error);

  FTRACE(3, "{} = CLIWrapper({})::opendir(...) [err = {}]\n",
         res, m_cli_fd, error);

  if (!res) {
    raise_warning("%s", error.c_str());
    return nullptr;
  }

  return req::make<PlainDirectory>(cli_read_fd(m_cli_fd));
}

int CLIWrapper::lstat(const String& path, struct stat* buf) {
  FTRACE(3, "CLIWrapper({})::lstat({}) calling remote...\n",
         m_cli_fd, path.data());
  cli_write(m_cli_fd, "lstat", File::TranslatePath(path).data());
  int res;
  cli_read(m_cli_fd, res, *buf);
  return res;
}

int CLIWrapper::stat(const String& path, struct stat* buf) {
  FTRACE(3, "CLIWrapper({})::stat({}) calling remote...\n",
         m_cli_fd, path.data());
  cli_write(m_cli_fd, "stat", File::TranslatePath(path).data());
  int res;
  cli_read(m_cli_fd, res, *buf);
  return res;
}

String CLIWrapper::realpath(const String& path) {
  bool status;
  std::string ret;
  cli_write(m_cli_fd, "realpath", File::TranslatePath(path).data());
  cli_read(m_cli_fd, status, ret);
  if (!status) return null_string;
  return String(ret.c_str(), CopyString);
}

template<class T>
typename std::enable_if<
  !std::is_same<String,typename std::remove_cv<T>::type>::value,
  T
>::type cli_wire_type(T&& t) { return t; }

template<class T>
typename std::enable_if<
  std::is_same<String,typename std::remove_cv<T>::type>::value,
  std::string
>::type cli_wire_type(T&& t) {
  return File::TranslatePath(t).data();
}

template<class... Args>
int cli_send_wire(int fd, const char* name, Args... args) {
  FTRACE(3, "CLIWrapper({})::{}(...) calling remote...\n", fd, name);
  cli_write(fd, name, cli_wire_type(std::forward<Args>(args))...);
  int res;
  cli_read(fd, res);
  return res;
}

int CLIWrapper::access(const String& path, int mode) {
  return cli_send_wire(m_cli_fd, "access", path, mode);
}
int CLIWrapper::unlink(const String& path) {
  return cli_send_wire(m_cli_fd, "unlink", path);
}
int CLIWrapper::rename(const String& oldname, const String& newname) {
  return cli_send_wire(m_cli_fd, "rename", oldname, newname);
}
int CLIWrapper::mkdir(const String& path, int mode, int options) {
  return cli_send_wire(m_cli_fd, "mkdir", path, mode, options);
}
int CLIWrapper::rmdir(const String& path, int options) {
  return cli_send_wire(m_cli_fd, "rmdir", path, options);
}
bool CLIWrapper::touch(const String& path, int64_t mtime, int64_t atime) {
  return cli_send_wire(m_cli_fd, "touch", path, mtime, atime) != -1;
}
bool CLIWrapper::chmod(const String& path, int64_t mode) {
  return cli_send_wire(m_cli_fd, "chmod", path, mode) != -1;
}
bool CLIWrapper::chown(const String& path, int64_t uid) {
  return cli_send_wire(m_cli_fd, "chown", path, uid, (int64_t)-1) != -1;
}
bool CLIWrapper::chown(const String& path, const String& user) {
  uid_t id;
  try {
    UserInfo info(user.data());
    id = info.pwd.pw_uid;
  } catch (const Exception& ex) {
    return false;
  }
  return cli_send_wire(m_cli_fd, "chown", path, id, (int64_t)-1) != -1;
}
bool CLIWrapper::chgrp(const String& path, int64_t gid) {
  return cli_send_wire(m_cli_fd, "chown", path, (int64_t)-1, gid) != -1;
}
bool CLIWrapper::chgrp(const String& path, const String& group) {
  gid_t id;
  try {
    GroupInfo info(group.data());
    id = info.gr.gr_gid;
  } catch (const Exception& ex) {
    return false;
  }
  return cli_send_wire(m_cli_fd, "chown", path, (int64_t)-1, id) != -1;
}

////////////////////////////////////////////////////////////////////////////////

int mkdir_recursive(const char* path, int mode) {
  auto path_len = strlen(path);
  if (path_len > PATH_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }

  // Check first if the whole path exists
  if (access(path, F_OK) >= 0) {
    errno = EEXIST;
    return -1;
  }

  char dir[PATH_MAX+1];
  char *p;
  memcpy(dir, path, path_len + 1); // copy null terminator

  for (p = dir + 1; *p; p++) {
    if (FileUtil::isDirSeparator(*p)) {
      *p = '\0';
      if (access(dir, F_OK) < 0 && mkdir(dir, mode) < 0) {
        return -1;
      }
      *p = FileUtil::getDirSeparator();
    }
  }

  if (access(dir, F_OK) < 0) {
    if (mkdir(dir, mode) < 0) {
      return -1;
    }
  }

  return 0;
}

void cli_process_command_loop(int fd) {
  FTRACE(1, "cli_process_command_loop({}): starting...\n", fd);
  for (;;) {
    std::string cmd;
    cli_read(fd, cmd);

    FTRACE(2, "cli_process_command_loop({}): got command: {}\n", fd, cmd);

    if (cmd == "exit") {
      int ret;
      cli_read(fd, ret);
      FTRACE(1, "cli_process_command_loop({}): exiting with code {}\n",
             fd, ret);
      hphp_context_exit();
      hphp_session_exit();
      hphp_process_exit();
      exit(ret);
    }

    if (cmd == "open") {
      std::string name;
      std::string mode;
      cli_read(fd, name, mode);
      int md = -1;
      int fl = 0;

      switch (mode[0]) {
        case 'x':
          md = 0666;
          fl = O_CREAT|O_EXCL;
          fl |= (mode.find('+') == -1) ? O_WRONLY : O_RDWR;
          break;
        case 'c':
          md = 0666;
          fl = O_CREAT;
          fl |= (mode.find('+') == -1) ? O_WRONLY : O_RDWR;
          break;
        case 'r':
          fl = (mode.find('+') == -1) ? O_RDONLY : O_RDWR;
          break;
        case 'w':
          md = 0666;
          fl = O_CREAT | O_TRUNC;
          fl |= (mode.find('+') == -1) ? O_WRONLY : O_RDWR;
          break;
        case 'a':
          md = 0666;
          fl = O_CREAT | O_APPEND;
          fl |= (mode.find('+') == -1) ? O_WRONLY : O_RDWR;
          break;
        default:
          cli_write(fd, false, "Invalid mode string");
          continue;
      }

      int new_fd = md != -1
        ? open(name.c_str(), fl, md)
        : open(name.c_str(), fl);

      FTRACE(2, "cli_process_command_loop({}): {} = open({}, {})\n",
             fd, new_fd, name, mode);

      if (new_fd < 0) {
        cli_write(fd, false, folly::errnoStr(errno).c_str());
        continue;
      }
      cli_write(fd, true, "OK");
      cli_write_fd(fd, new_fd);
      close(new_fd);
      continue;
    }

    if (cmd == "opendir") {
      std::string name;
      cli_read(fd, name);
      int dir_fd = open(name.c_str(), O_RDONLY|O_DIRECTORY);

      FTRACE(2, "cli_process_command_loop({}): {} = diropen({})\n",
             fd, dir_fd, name);

      if (dir_fd < 0) {
        cli_write(fd, false, folly::errnoStr(errno).c_str());
        continue;
      }
      cli_write(fd, true, "OK");
      cli_write_fd(fd, dir_fd);
      close(dir_fd);
    }

    if (cmd == "lstat") {
      std::string path;
      struct stat st;
      cli_read(fd, path);
      FTRACE(2, "cli_process_command_loop({}): lstat({})\n",
             fd, path);
      int res = lstat(path.c_str(), &st);
      cli_write(fd, res, st);
      continue;
    }

    if (cmd == "stat") {
      std::string path;
      struct stat st;
      cli_read(fd, path);
      FTRACE(2, "cli_process_command_loop({}): stat({})\n",
             fd, path);
      int res = stat(path.c_str(), &st);
      cli_write(fd, res, st);
      continue;
    }

    if (cmd == "access") {
      std::string path;
      int mode;
      cli_read(fd, path, mode);
      FTRACE(2, "cli_process_command_loop({}): access({}, {})\n",
             fd, path, mode);
      cli_write(fd, access(path.c_str(), mode));
      continue;
    }

    if (cmd == "unlink") {
      std::string path;
      cli_read(fd, path);
      FTRACE(2, "cli_process_command_loop({}): unlink({})\n",
             fd, path);
      cli_write(fd, unlink(path.c_str()));
      continue;
    }

    if (cmd == "rename") {
      std::string oldp;
      std::string newp;
      cli_read(fd, oldp, newp);

      FTRACE(2, "cli_process_command_loop({}): rename({}, {})\n",
             fd, oldp, newp);

      int ret = RuntimeOption::UseDirectCopy
        ? FileUtil::directRename(oldp.c_str(), newp.c_str())
        : FileUtil::rename(oldp.c_str(), newp.c_str());

      cli_write(fd, ret);
      continue;
    }

    if (cmd == "mkdir") {
      int options;
      int mode;
      std::string path;
      cli_read(fd, path, mode, options);

      FTRACE(2, "cli_process_command_loop({}): mkdir({}, {}, {})\n",
             fd, path, mode, options);

      if (options & k_STREAM_MKDIR_RECURSIVE) {
        cli_write(fd, mkdir_recursive(path.c_str(), mode));
        continue;
      }

      cli_write(fd, mkdir(path.c_str(), mode));
      continue;
    }

    if (cmd == "rmdir") {
      std::string path;
      int options;
      cli_read(fd, path, options);

      FTRACE(2, "cli_process_command_loop({}): rmdir({})\n", fd, path);

      cli_write(fd, rmdir(path.c_str()));
      continue;
    }

    if (cmd == "touch") {
      std::string path;
      int64_t mtime;
      int64_t atime;
      cli_read(fd, path, mtime, atime);

      if (access(path.c_str(), F_OK)) {
        FILE *f = fopen(path.c_str(), "w");
        if (!f) {
          cli_write(fd, -1);
          continue;
        }
        fclose(f);
      }

      if (mtime == 0 && atime == 0) {
        // It is important to pass nullptr so that the OS sets mtime and atime
        // to the current time with maximum precision (more precise then
        // seconds)
        cli_write(fd, utime(path.c_str(), nullptr));
      } else {
        struct utimbuf newtime;
        newtime.actime = atime ? atime : mtime;
        newtime.modtime = mtime;
        cli_write(fd, utime(path.c_str(), &newtime));
      }
      FTRACE(2, "cli_process_command_loop({}): touch({}, {}, {})\n",
             fd, path, mtime, atime);
      continue;
    }

    if (cmd == "chmod") {
      std::string path;
      int64_t mode;
      cli_read(fd, path, mode);

      FTRACE(2, "cli_process_command_loop({}): chmod({}, {})\n",
             fd, path, mode);

      cli_write(fd, chmod(path.c_str(), mode));
      continue;
    }

    if (cmd == "chown") {
      std::string path;
      int64_t uid;
      int64_t gid;
      cli_read(fd, path, uid, gid);

      FTRACE(2, "cli_process_command_loop({}): chown({}, {}, {})\n",
             fd, path, uid, gid);

      cli_write(fd, chown(path.c_str(), (uid_t)uid, (gid_t)gid));
      continue;
    }

    if (cmd == "mkstemp") {
      std::string path;
      cli_read(fd, path);
      FTRACE(2, "cli_process_command_loop({}): mkstemp({})\n", fd, path);
      char* str = strdup(path.data());
      SCOPE_EXIT { free(str); };
      auto tempfd = mkstemp(str);
      if (tempfd < 0) {
        cli_write(fd, false, "ERROR");
        continue;
      }
      close(tempfd);
      std::string out = str;
      cli_write(fd, true, out);
      continue;
    }

    if (cmd == "realpath") {
      std::string path;
      cli_read(fd, path);
      FTRACE(2, "cli_process_command_loop({}): realpath({})\n", fd, path);
      char resolved_path[PATH_MAX];
      if (!realpath(path.c_str(), resolved_path)) {
        cli_write(fd, false, "ERROR");
        continue;
      }
      std::string out(resolved_path);
      cli_write(fd, true, out);
      continue;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
}

void init_cli_server(const char* socket_path) {
  if (RuntimeOption::RepoAuthoritative) return;

  for (auto user : RuntimeOption::EvalUnixServerAllowedUsers) {
    try {
      UserInfo info(user.c_str());
      s_allowedUsers.emplace(info.pwd.pw_uid);
    } catch (const Exception& ex) {
      Logger::Warning(
        "Could not get uid for user %s in Eval.UnixServerAllowedUsers: %s",
        user.c_str(), ex.what()
      );
    }
  }

  for (auto group : RuntimeOption::EvalUnixServerAllowedGroups) {
    try {
      GroupInfo info(group.c_str());
      s_allowedGroups.emplace(info.gr.gr_gid);
    } catch (const Exception& ex) {
      Logger::Warning(
        "Could not get gid for group %s in Eval.UnixServerAllowedGroups: %s",
        group.c_str(), ex.what()
      );
    }
  }

  assert(!s_cliServer);
  FTRACE(1, "init_cli_server({}): init...\n", socket_path);
  s_cliServer = new CLIServer(socket_path);
}

void start_cli_server() {
  if (!s_cliServer) return;

  FTRACE(1, "start_cli_server(): starting...\n");
  s_cliServer->start();
}

void teardown_cli_server() {
  if (!s_cliServer) return;

  FTRACE(1, "teardown_cli_server(): stopping...\n");

  s_cliServer->stop();
  delete s_cliServer;
  s_cliServer = nullptr;
}

ucred* get_cli_ucred() { return tl_ucred; }

bool cli_mkstemp(char* buf) {
  assert(tl_cliSock >= 0);
  FTRACE(2, "cli_mkstemp({}): fd = {}\n", buf, tl_cliSock);
  std::string out = buf;
  cli_write(tl_cliSock, "mkstemp", out);
  bool status;
  std::string path;
  cli_read(tl_cliSock, status, path);
  if (!status) return false;
  memcpy(buf, path.c_str(), std::min(strlen(buf), path.size()));
  return true;
}

Array cli_env() {
  return tl_env ? *tl_env : empty_array();
}

bool is_cli_mode() { return tl_cliSock != -1; }

////////////////////////////////////////////////////////////////////////////////

void run_command_on_cli_server(const char* sock_path,
                               const std::vector<std::string>& args) {
  if (RuntimeOption::RepoAuthoritative) {
    Logger::Warning("Unable to use CLI server to run script in "
                    "repo-auth mode.");
    return;
  }
  FTRACE(1, "run_command_on_cli_server({}, ...): sending command...\n",
         sock_path);

  std::vector<std::string> env_vec;

  for (char** env = environ; env && *env; env++) {
    env_vec.emplace_back(*env);
  }

  int delegate = LightProcess::createDelegate();
  if (delegate < 0) {
    Logger::Warning("Could not create delegate for CLI server: %s",
                    folly::errnoStr(errno).c_str());
    return;
  }
  FTRACE(2, "run_command_on_cli_server(): delegate = {}\n", delegate);

  afdt_error_t err = AFDT_ERROR_T_INIT;
  int fd = afdt_connect(sock_path, &err);
  if (fd < 0) {
    Logger::Info("Could not attach to CLI server: %s",
                 folly::errnoStr(errno).c_str());
    return;
  }

  FTRACE(2, "run_command_on_cli_server(): fd = {}\n", fd);

  try {
    cli_write_ucred(fd);
    cli_write(fd, "hello_server");

    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    cli_write(fd, cwd);

    hphp_session_init();
    SCOPE_EXIT {
      hphp_context_exit();
      hphp_session_exit();
    };
    auto settings = IniSetting::GetAllAsJSON();
    cli_write(fd, settings);

    FTRACE(2, "run_command_on_cli_server(): sending fds...\n", fd);

    cli_write_fd(fd, fileno(stdin));
    cli_write_fd(fd, fileno(stdout));
    cli_write_fd(fd, fileno(stderr));
    cli_write_fd(fd, delegate);

    FTRACE(2, "run_command_on_cli_server(): file/args...\n", fd);
    cli_write(fd, 0, args, env_vec);
    cli_process_command_loop(fd);
  } catch (const Exception& ex) {
    Logger::Error("Problem communicating with CLI server: %s", ex.what());
    exit(255);
  }
}

}
