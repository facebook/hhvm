/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/listener.h"
#include <folly/Exception.h>
#include <folly/MapUtil.h>
#include <folly/SocketAddress.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <folly/net/NetworkSocket.h>
#include <atomic>
#include <chrono>
#include <optional>
#include <thread>
#include "watchman/Client.h"
#include "watchman/Constants.h"
#include "watchman/GroupLookup.h"
#include "watchman/SanityCheck.h"
#include "watchman/Shutdown.h"
#include "watchman/SignalHandler.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/portability/WinError.h"
#include "watchman/sockname.h"
#include "watchman/state.h"
#include "watchman/watchman_cmd.h"

using namespace watchman;

static FileDescriptor listener_fd;

#if defined(HAVE_KQUEUE) || defined(HAVE_FSEVENTS)
#ifdef __OpenBSD__
#include <sys/siginfo.h> // @manual
#endif
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#endif

#ifndef _WIN32

// If we are running under inetd-style supervision, call this function
// to move the inetd provided socket descriptor(s) to a new descriptor
// number and remember that we can just use these when we're starting
// up the listener.
void w_listener_prep_inetd() {
  if (listener_fd) {
    throw std::runtime_error(
        "w_listener_prep_inetd: listener_fd is already assigned");
  }

  listener_fd = FileDescriptor(
      dup(STDIN_FILENO),
      "dup(stdin) for listener",
      // It's probably a socket but we don't know for sure
      FileDescriptor::FDType::Unknown);
}

#endif

static FileDescriptor get_listener_unix_domain_socket(const char* path) {
#ifndef _WIN32
  mode_t perms = cfg_get_perms(
      "sock_access", true /* write bits */, false /* execute bits */);
#endif
  FileDescriptor listener_fd;

  struct sockaddr_un un {};
  if (strlen(path) >= sizeof(un.sun_path) - 1) {
    logf(ERR, "{}: path is too long\n", path);
    return FileDescriptor();
  }

  listener_fd = FileDescriptor(
      ::socket(PF_LOCAL, SOCK_STREAM, 0),
      "socket",
      FileDescriptor::FDType::Socket);

  un.sun_family = PF_LOCAL;
  memcpy(un.sun_path, path, strlen(path) + 1);

  (void)unlink(path);
  if (::bind(listener_fd.system_handle(), (struct sockaddr*)&un, sizeof(un)) !=
      0) {
    logf(ERR, "bind({}): {}\n", path, folly::errnoStr(errno));
    return FileDescriptor();
  }

#ifndef _WIN32
  // The permissions in the containing directory should be correct, so this
  // should be correct as well. But set the permissions in any case.
  if (chmod(path, perms) == -1) {
    logf(ERR, "chmod({}, {:o}): {}", path, perms, folly::errnoStr(errno));
    return FileDescriptor();
  }

  // Double-check that the socket has the right permissions. This can happen
  // when the containing directory was created in a previous run, with a group
  // the user is no longer in.
  struct stat st;
  if (lstat(path, &st) == -1) {
    watchman::log(
        watchman::ERR, "lstat(", path, "): ", folly::errnoStr(errno), "\n");
    return FileDescriptor();
  }

  // This is for testing only
  // (test_sock_perms.py:test_user_previously_in_sock_group). Do not document.
  const char* sock_group_name = cfg_get_string("__sock_file_group", nullptr);
  if (!sock_group_name) {
    sock_group_name = cfg_get_string("sock_group", nullptr);
  }

  if (sock_group_name) {
    const struct group* sock_group = w_get_group(sock_group_name);
    if (!sock_group) {
      return FileDescriptor();
    }
    if (st.st_gid != sock_group->gr_gid) {
      watchman::log(
          watchman::ERR,
          "for socket '",
          path,
          "', gid ",
          st.st_gid,
          " doesn't match expected gid ",
          sock_group->gr_gid,
          " (group name ",
          sock_group_name,
          "). Ensure that you are still a member of group ",
          sock_group_name,
          ".\n");
      return FileDescriptor();
    }
  }
#endif

  if (::listen(listener_fd.system_handle(), 200) != 0) {
    logf(ERR, "listen({}): {}\n", path, folly::errnoStr(errno));
    return FileDescriptor();
  }

  return listener_fd;
}

#ifdef _WIN32

static FileDescriptor create_pipe_server(const char* path) {
  return FileDescriptor(
      intptr_t(CreateNamedPipe(
          path,
          PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_REJECT_REMOTE_CLIENTS,
          PIPE_UNLIMITED_INSTANCES,
          WATCHMAN_IO_BUF_SIZE,
          512,
          0,
          nullptr)),
      FileDescriptor::FDType::Pipe);
}

static void named_pipe_accept_loop_internal(
    std::shared_ptr<watchman_event> listener_event) {
  HANDLE handles[2];
  auto olap = OVERLAPPED();
  HANDLE connected_event = CreateEvent(nullptr, FALSE, TRUE, nullptr);
  auto path = get_named_pipe_sock_path();

  if (!connected_event) {
    logf(
        ERR,
        "named_pipe_accept_loop_internal: CreateEvent failed: {}\n",
        win32_strerror(GetLastError()));
    return;
  }

  handles[0] = connected_event;
  handles[1] = (HANDLE)listener_event->system_handle();
  olap.hEvent = connected_event;

  logf(ERR, "waiting for pipe clients on {}\n", path);
  while (!w_is_stopping()) {
    FileDescriptor client_fd;
    DWORD res;

    client_fd = create_pipe_server(path.c_str());
    if (!client_fd) {
      logf(
          ERR,
          "CreateNamedPipe(%s) failed: %s\n",
          path,
          win32_strerror(GetLastError()));
      continue;
    }

    ResetEvent(connected_event);
    if (!ConnectNamedPipe((HANDLE)client_fd.handle(), &olap)) {
      res = GetLastError();

      if (res == ERROR_PIPE_CONNECTED) {
        UserClient::create(w_stm_fdopen(std::move(client_fd)));
        continue;
      }

      if (res != ERROR_IO_PENDING) {
        logf(ERR, "ConnectNamedPipe: {}\n", win32_strerror(GetLastError()));
        continue;
      }

      res = WaitForMultipleObjectsEx(2, handles, false, INFINITE, true);
      if (res == WAIT_OBJECT_0 + 1) {
        // Signalled to stop
        CancelIoEx((HANDLE)client_fd.handle(), &olap);
        continue;
      }

      if (res != WAIT_OBJECT_0) {
        logf(
            ERR,
            "WaitForMultipleObjectsEx: ConnectNamedPipe: "
            "unexpected status {}\n",
            res);
        CancelIoEx((HANDLE)client_fd.handle(), &olap);
        continue;
      }
    }
    UserClient::create(w_stm_fdopen(std::move(client_fd)));
  }
  logf(ERR, "is_stopping is true, so acceptor is done\n");
}

static void named_pipe_accept_loop() {
  log(DBG, "Starting pipe listener on ", get_named_pipe_sock_path(), "\n");

  std::shared_ptr<watchman_event> listener_event = w_event_make_named_pipe();
  w_push_listener_thread_event(listener_event);

  std::vector<std::thread> acceptors;
  for (json_int_t i = 0; i < cfg_get_int("win32_concurrent_accepts", 32); ++i) {
    acceptors.push_back(std::thread([i, listener_event]() {
      w_set_thread_name("accept", i);
      named_pipe_accept_loop_internal(listener_event);
    }));
  }
  for (auto& thr : acceptors) {
    thr.join();
  }
}
#endif

/** A helper for owning and running a socket-style (rather than
 * named pipe style) accept loop that runs in another thread.
 */
class AcceptLoop {
 public:
  /** Start an accept loop thread using the provided socket
   * descriptor (`fd`).  The `name` parameter is used to name the
   * thread */
  AcceptLoop(std::string name, FileDescriptor&& fd) {
    fd.setCloExec();
    fd.setNonBlock();

    std::shared_ptr<watchman_event> listener_event = w_event_make_sockets();
    w_push_listener_thread_event(listener_event);

    thread_ = std::thread([listener_fd = std::move(fd),
                           name = std::move(name),
                           listener_event]() mutable {
      w_set_thread_name(name);
      accept_thread(std::move(listener_fd), listener_event);
    });
  }

  AcceptLoop(const AcceptLoop&) = delete;
  AcceptLoop& operator=(const AcceptLoop&) = delete;

  AcceptLoop(AcceptLoop&& other) {
    *this = std::move(other);
  }

  AcceptLoop& operator=(AcceptLoop&& other) {
    thread_ = std::move(other.thread_);
    joined_ = other.joined_;
    // Ensure that we don't try to join the source,
    // as std::thread::join will std::terminate in that case.
    // If it weren't for this we could use the compiler
    // default implementation of move.
    other.joined_ = true;
    return *this;
  }

  ~AcceptLoop() {
    join();
  }

  void join() {
    if (joined_) {
      return;
    }
    thread_.join();
    joined_ = true;
  }

 private:
  static void accept_thread(
      FileDescriptor&& listenerDescriptor,
      std::shared_ptr<watchman_event> listener_event) {
    auto listener = w_stm_fdopen(std::move(listenerDescriptor));
    while (!w_is_stopping()) {
      FileDescriptor client_fd;
      EventPoll pfd[2];

      pfd[0].evt = listener->getEvents();
      pfd[1].evt = listener_event.get();

      if (w_poll_events(pfd, 2, 60000) == 0) {
        if (w_is_stopping()) {
          break;
        }
        // Timed out, or error.
        continue;
      }

      if (w_is_stopping()) {
        break;
      }

#ifdef HAVE_ACCEPT4
      client_fd = FileDescriptor(
          accept4(
              listener->getFileDescriptor().system_handle(),
              nullptr,
              0,
              SOCK_CLOEXEC),
          FileDescriptor::FDType::Socket);
#else
      client_fd = FileDescriptor(
          ::accept(listener->getFileDescriptor().system_handle(), nullptr, 0),
          FileDescriptor::FDType::Socket);
#endif
      if (!client_fd) {
        continue;
      }
      client_fd.setCloExec();
      int bufsize = WATCHMAN_IO_BUF_SIZE;
      ::setsockopt(
          client_fd.system_handle(),
          SOL_SOCKET,
          SO_SNDBUF,
          (char*)&bufsize,
          sizeof(bufsize));

      UserClient::create(w_stm_fdopen(std::move(client_fd)));
    }
  }

  std::thread thread_;
  bool joined_{false};
};

bool w_start_listener() {
#ifndef _WIN32
  struct sigaction sa;
  sigset_t sigset;
#endif

#if defined(HAVE_KQUEUE) || defined(HAVE_FSEVENTS)
  {
    struct rlimit limit;
#ifndef __OpenBSD__
    int mib[2] = {
        CTL_KERN,
#ifdef KERN_MAXFILESPERPROC
        KERN_MAXFILESPERPROC
#else
        KERN_MAXFILES
#endif
    };
#endif
    int maxperproc;

    getrlimit(RLIMIT_NOFILE, &limit);

#ifndef __OpenBSD__
    {
      size_t len;

      len = sizeof(maxperproc);
      sysctl(mib, 2, &maxperproc, &len, nullptr, 0);
      logf(
          ERR,
          "file limit is {} kern.maxfilesperproc={}\n",
          limit.rlim_cur,
          maxperproc);
    }
#else
    maxperproc = limit.rlim_max;
    logf(
        ERR,
        "openfiles-cur is {} openfiles-max={}\n",
        limit.rlim_cur,
        maxperproc);
#endif

    if (limit.rlim_cur != RLIM_INFINITY && maxperproc > 0 &&
        limit.rlim_cur < (rlim_t)maxperproc) {
      limit.rlim_cur = maxperproc;

      if (setrlimit(RLIMIT_NOFILE, &limit)) {
        logf(
            ERR,
            "failed to raise limit to {} ({}).\n",
            limit.rlim_cur,
            folly::errnoStr(errno));
      } else {
        logf(ERR, "raised file limit to {}\n", limit.rlim_cur);
      }
    }

    getrlimit(RLIMIT_NOFILE, &limit);
#ifndef HAVE_FSEVENTS
    if (limit.rlim_cur < 10240) {
      logf(
          ERR,
          "Your file descriptor limit is very low ({})"
          "please consult the watchman docs on raising the limits\n",
          limit.rlim_cur);
    }
#endif
  }
#endif

#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);

  /* allow SIGUSR1 and SIGCHLD to wake up a blocked thread, without restarting
   * syscalls */
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = [](int) {};
  sa.sa_flags = 0;
  sigaction(SIGUSR1, &sa, nullptr);
  sigaction(SIGCHLD, &sa, nullptr);

  // Block SIGCHLD everywhere
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sigset, nullptr);
#endif
  // TODO: We are trying out folly signal handling on Linux. Eventually we
  // should remove this if and use folly signal handling on all platforms.
  if (!kUseFollySignalHandler) {
    setup_signal_handlers();
  }

  std::optional<AcceptLoop> tcp_loop;
  std::optional<AcceptLoop> unix_loop;

  // When we unwind, ensure that we stop the accept threads
  SCOPE_EXIT {
    if (!w_is_stopping()) {
      w_request_shutdown();
    }
    unix_loop.reset();
    tcp_loop.reset();
  };

  if (listener_fd) {
    // Assume that it was prepped by w_listener_prep_inetd()
    logf(ERR, "Using socket from inetd as listening socket\n");
  } else {
    listener_fd = get_listener_unix_domain_socket(get_unix_sock_name().c_str());
    if (!listener_fd) {
      logf(ERR, "Failed to initialize unix domain listener\n");
      return false;
    }
  }

  if (listener_fd && !disable_unix_socket) {
    unix_loop = AcceptLoop("unix-listener", std::move(listener_fd));
  }

  if (Configuration().getBool("enable-sanity-check", true)) {
    startSanityCheckThread();
  }

#ifdef _WIN32
  // Start the named pipes and join them; this will
  // block until the server is shutdown.
  if (!disable_named_pipe) {
    named_pipe_accept_loop();
  }
#endif

  // Clearing these will cause .join() to be called,
  // so the next two lines will block until the server
  // shutdown is initiated, rather than cause the server
  // to shutdown.
  unix_loop.reset();
  tcp_loop.reset();

  // Wait for clients, waking any sleeping clients up in the process
  {
    auto interval = std::chrono::microseconds(2000);
    const auto max_interval = std::chrono::seconds(1);
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(10);

    size_t last_count = 0;
    size_t n_clients = 0;

    while (true) {
      {
        auto clients = UserClient::getAllClients();
        n_clients = clients.size();
        for (auto& client : clients) {
          client->ping->notify();
        }
      }

      // The clients lock and shared_ptr refcounts are released here, so entries
      // may be removed from the active clients table.

      if (n_clients == 0) {
        break;
      }

      if (std::chrono::steady_clock::now() >= deadline) {
        log(ERR, "Abandoning wait for ", n_clients, " outstanding clients\n");
        break;
      }

      if (n_clients != last_count) {
        log(ERR, "waiting for ", n_clients, " clients to terminate\n");
      }

      /* sleep override */
      std::this_thread::sleep_for(interval);
      interval *= 2;
      if (interval > max_interval) {
        interval = max_interval;
      }
    }
  }

  return true;
}

/* get-pid */
static UntypedResponse cmd_get_pid(Client*, const json_ref&) {
  UntypedResponse resp;
  resp.set("pid", json_integer(::getpid()));
  return resp;
}
W_CMD_REG("get-pid", cmd_get_pid, CMD_DAEMON, nullptr);

/* vim:ts=2:sw=2:et:
 */
