/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/SocketAddress.h>
#include <folly/net/NetworkSocket.h>
#include <memory>
#include "watchman/Constants.h"
#include "watchman/Logging.h"
#include "watchman/fs/FileDescriptor.h"
#include "watchman/fs/Pipe.h"
#include "watchman/portability/WinError.h"
#include "watchman/watchman_stream.h"

#ifdef HAVE_UCRED_H
#include <ucred.h> // @manual
#endif
#ifdef HAVE_SYS_UCRED_H
#include <sys/ucred.h> // @manual
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <folly/portability/Sockets.h> // @manual
#endif

#ifdef _WIN32
#include "eden/common/utils/WinUtil.h"
#endif

using namespace watchman;

static const int kWriteTimeout = 60000;

namespace {
// This trait allows w_poll_events to wait on either a PipeEvent or
// a descriptor contained in a UnixStream
class PollableEvent : public watchman_event {
 public:
  virtual FileDescriptor::system_handle_type getFd() const = 0;
};

// The event object, implemented as pipe
class PipeEvent : public PollableEvent {
 public:
  SocketPair pipe;

  void notify() override {
    ignore_result(pipe.write.write("a", 1).hasValue());
  }

  bool testAndClear() override {
    char buf[64];
    bool signalled = false;
    while (true) {
      auto res = pipe.read.read(buf, sizeof(buf));
      if (res.hasError() || res.value() == 0) {
        break;
      }
      signalled = true;
    }
    return signalled;
  }

  FileDescriptor::system_handle_type getFd() const override {
    return pipe.read.system_handle();
  }

  FileDescriptor::system_handle_type system_handle() override {
    return pipe.read.system_handle();
  }

  bool isSocket() override {
    return true;
  }
};

// Event object that UnixStream returns via getEvents.
// It cannot be poked by hand; it is just a helper to
// allow waiting on a socket using w_poll_events.
class FakeSocketEvent : public PollableEvent {
 private:
  FileDescriptor::system_handle_type socket;

 public:
  explicit FakeSocketEvent(FileDescriptor::system_handle_type fd)
      : socket(fd) {}

  void notify() override {}
  bool testAndClear() override {
    return false;
  }
  FileDescriptor::system_handle_type getFd() const override {
    return socket;
  }

  FileDescriptor::system_handle_type system_handle() override {
    return socket;
  }

  bool isSocket() override {
    return true;
  }
};

class UnixStream : public watchman_stream {
 public:
  FileDescriptor fd;
  FakeSocketEvent evt;
#ifdef SO_PEERCRED
  struct ucred cred;
#elif defined(LOCAL_PEERCRED)
  struct xucred cred;
  pid_t epid;
#elif defined(SO_RECVUCRED)
  struct ucred_deleter {
    void operator()(ucred_t* utp) {
      ucred_free(utp);
    }
  };
  std::unique_ptr<ucred_t, ucred_deleter> cred;
#endif
  bool credvalid{false};
  bool blocking_{false};

  explicit UnixStream(FileDescriptor&& descriptor)
      : fd(std::move(descriptor)), evt(fd.system_handle()) {
#ifdef SO_PEERCRED
    socklen_t len = sizeof(cred);
    credvalid = getsockopt(fd.fd(), SOL_SOCKET, SO_PEERCRED, &cred, &len) == 0;
#elif defined(LOCAL_PEERCRED)
    socklen_t len = sizeof(cred);
#if defined(__FreeBSD__) || defined(__DragonFly__)
    credvalid = getsockopt(fd.fd(), 0, LOCAL_PEERCRED, &cred, &len) == 0;
#else
    credvalid =
        getsockopt(fd.fd(), SOL_LOCAL, LOCAL_PEERCRED, &cred, &len) == 0;
#endif
    if (credvalid) {
#if defined(__APPLE__)
      len = sizeof(epid);
      credvalid =
          getsockopt(fd.fd(), SOL_LOCAL, LOCAL_PEEREPID, &epid, &len) == 0;
#elif defined(__FreeBSD__)
      epid = cred.cr_pid;
#else
      credvalid = false;
#endif
    }
#elif defined(SO_RECVUCRED)
    ucred_t* peer_cred{nullptr};
    credvalid = getpeerucred(fd.fd(), &peer_cred) == 0;
    cred.reset(peer_cred);
#endif
  }

  const FileDescriptor& getFileDescriptor() const override {
    return fd;
  }

  int read(void* buf, int size) override {
    auto res = fd.read(buf, size);
    if (res.hasError()) {
#ifdef _WIN32
      errno = map_win32_err(res.error().value());
#else
      errno = res.error().value();
#endif
      return -1;
    }
    errno = 0;
    return res.value();
  }

  int write(const void* buf, int size) override {
    if (blocking_) {
      int wrote = 0;

      while (size > 0) {
        struct pollfd pfd;
        pfd.fd = fd.system_handle();
        pfd.events = POLLOUT;
#ifdef _WIN32
        if (WSAPoll(&pfd, 1, kWriteTimeout) == 0) {
          errno = map_win32_err(WSAGetLastError());
          break;
        }
#else
        if (poll(&pfd, 1, kWriteTimeout) == 0) {
          break;
        }
#endif
        if (pfd.revents & (POLLERR | POLLHUP)) {
          break;
        }
        auto x = fd.write(buf, size);
        if (x.hasError()) {
#ifdef _WIN32
          errno = map_win32_err(x.error().value());
#else
          errno = x.error().value();
#endif
          break;
        }
        if (x.value() == 0) {
          errno = 0;
          break;
        }

        wrote += x.value();
        size -= x.value();
        buf = reinterpret_cast<const void*>(
            reinterpret_cast<const char*>(buf) + x.value());
      }
      return wrote == 0 ? -1 : wrote;
    }
    auto x = fd.write(buf, size);
    if (x.hasError()) {
#ifdef _WIN32
      errno = map_win32_err(x.error().value());
#else
      errno = x.error().value();
#endif
      return -1;
    }
    errno = 0;
    return x.value();
  }

  watchman_event* getEvents() override {
    return &evt;
  }

  void setNonBlock(bool nonb) override {
    if (nonb) {
      fd.setNonBlock();
    } else {
      fd.clearNonBlock();
    }
    blocking_ = !nonb;
  }

  bool rewind() override {
#ifndef _WIN32
    return lseek(fd.fd(), 0, SEEK_SET) == 0;
#else
    return false;
#endif
  }

  bool shutdown() override {
    return ::shutdown(
        fd.system_handle(),
#ifdef SHUT_RDWR
        SHUT_RDWR
#else
        SD_BOTH
#endif
    );
  }

  // For these PEERCRED things, the uid reported is the effective uid of
  // the process, which may have been altered due to setuid or similar
  // mechanisms.  We'll treat the other process as an owner if their
  // effective UID matches ours, or if they are root.
  bool peerIsOwner() override {
#ifdef _WIN32
    return true;
#else
    if (!credvalid) {
      return false;
    }
#ifdef SO_PEERCRED
    if (cred.uid == getuid() || cred.uid == 0) {
      return true;
    }
#elif defined(LOCAL_PEERCRED)
    if (cred.cr_uid == getuid() || cred.cr_uid == 0) {
      return true;
    }
#elif defined(SO_RECVUCRED)
    uid_t ucreduid = ucred_getruid(cred.get());
    if (ucreduid == getuid() || ucreduid == 0) {
      return true;
    }
#endif
    return false;
#endif
  }

  pid_t getPeerProcessID() const override {
#ifdef _WIN32
    return facebook::eden::getPeerProcessID(fd.system_handle());
#endif
    if (!credvalid) {
      return 0;
    }
#ifdef SO_PEERCRED
    return cred.pid;
#elif defined(LOCAL_PEERCRED)
    return epid;
#elif defined(SO_RECVUCRED)
    pid_t ucredpid = ucred_getpid(cred.get());
    if (ucredpid == (pid_t)-1) {
      return 0;
    }
    return ucredpid;
#else
    return 0;
#endif
  }
};
} // namespace

std::unique_ptr<watchman_event> w_event_make_sockets() {
  return std::make_unique<PipeEvent>();
}

#define MAX_POLL_EVENTS 63 // Must match MAXIMUM_WAIT_OBJECTS-1 on win
int w_poll_events_sockets(EventPoll* p, int n, int timeoutms) {
  struct pollfd pfds[MAX_POLL_EVENTS];
  int i;
  int res;

  if (n > MAX_POLL_EVENTS) {
    // Programmer error :-/
    logf(FATAL, "{} > MAX_POLL_EVENTS ({})\n", n, MAX_POLL_EVENTS);
  }

  for (i = 0; i < n; i++) {
    auto pe = dynamic_cast<PollableEvent*>(p[i].evt);
    w_check(pe != nullptr, "PollableEvent!?");
    pfds[i].fd = pe->getFd();
    pfds[i].events = POLLIN;
    pfds[i].revents = 0;
  }

#ifdef _WIN32
  res = WSAPoll(pfds, n, timeoutms);
  auto win_err = WSAGetLastError();
  errno = map_win32_err(win_err);
#else
  res = poll(pfds, n, timeoutms);
#endif

  for (i = 0; i < n; i++) {
    p[i].ready = pfds[i].revents != 0;
  }

  return res;
}

std::unique_ptr<watchman_stream> w_stm_fdopen(FileDescriptor&& fd) {
  if (!fd) {
    return nullptr;
  }
#ifdef _WIN32
  if (fd.fdType() != FileDescriptor::FDType::Socket) {
    return w_stm_fdopen_windows(std::move(fd));
  }
#endif
  return std::make_unique<UnixStream>(std::move(fd));
}

ResultErrno<std::unique_ptr<watchman_stream>> w_stm_connect_unix(
    const char* path,
    int timeoutms) {
  struct sockaddr_un un {};
  int max_attempts = timeoutms / 10;
  int attempts = 0;

  if (strlen(path) >= sizeof(un.sun_path) - 1) {
    logf(ERR, "w_stm_connect_unix({}) path is too long\n", path);
    return E2BIG;
  }

  FileDescriptor fd(
      ::socket(
          PF_LOCAL,
#ifdef SOCK_CLOEXEC
          SOCK_CLOEXEC |
#endif
              SOCK_STREAM,
          0),
      FileDescriptor::FDType::Socket);
  if (!fd) {
    return errno;
  }
  fd.setCloExec();

  un.sun_family = PF_LOCAL;
  memcpy(un.sun_path, path, strlen(path) + 1);

retry_connect:

  if (::connect(fd.system_handle(), (struct sockaddr*)&un, sizeof(un))) {
#ifdef _WIN32
    int win_err = WSAGetLastError();
    int err = map_win32_err(win_err);
#else
    int err = errno;
#endif

    if (err == ECONNREFUSED || err == ENOENT) {
      if (attempts++ < max_attempts) {
        /* sleep override */ std::this_thread::sleep_for(
            std::chrono::microseconds(10000));
        goto retry_connect;
      }
    }

    return err;
  }

  int bufsize = WATCHMAN_IO_BUF_SIZE;
  ::setsockopt(
      fd.system_handle(),
      SOL_SOCKET,
      SO_RCVBUF,
      reinterpret_cast<const char*>(&bufsize),
      sizeof(bufsize));

  return ResultErrno<std::unique_ptr<Stream>>{w_stm_fdopen(std::move(fd))};
}

#ifndef _WIN32
std::unique_ptr<watchman_stream>
w_stm_open(const char* filename, int flags, ...) {
  int mode = 0;

  // If we're creating, pull out the mode flag
  if (flags & O_CREAT) {
    va_list ap;
    va_start(ap, flags);
    mode = va_arg(ap, int);
    va_end(ap);
  }

  return w_stm_fdopen(FileDescriptor(
      open(filename, flags, mode), FileDescriptor::FDType::Unknown));
}
#endif
