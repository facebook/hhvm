/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/Pipe.h"
#include <folly/Exception.h>
#include <folly/String.h>
#include <folly/portability/Event.h>
#include <system_error>

namespace watchman {

Pipe::Pipe() {
#ifdef _WIN32
  HANDLE readPipe;
  HANDLE writePipe;
  auto sec = SECURITY_ATTRIBUTES();

  sec.nLength = sizeof(sec);
  sec.bInheritHandle = FALSE; // O_CLOEXEC equivalent
  constexpr DWORD kPipeSize = 64 * 1024;

  if (!CreatePipe(&readPipe, &writePipe, &sec, kPipeSize)) {
    throw std::system_error(
        GetLastError(), std::system_category(), "CreatePipe failed");
  }
  read = FileDescriptor(intptr_t(readPipe), FileDescriptor::FDType::Pipe);
  write = FileDescriptor(intptr_t(writePipe), FileDescriptor::FDType::Pipe);

#else
  int fds[2];
  int res;
#if HAVE_PIPE2
  res = pipe2(fds, O_NONBLOCK | O_CLOEXEC);
#else
  res = pipe(fds);
#endif

  if (res) {
    throw std::system_error(
        errno,
        std::system_category(),
        std::string("pipe error: ") + folly::errnoStr(errno));
  }
  read = FileDescriptor(fds[0], FileDescriptor::FDType::Pipe);
  write = FileDescriptor(fds[1], FileDescriptor::FDType::Pipe);

#if !HAVE_PIPE2
  read.setCloExec();
  read.setNonBlock();
  write.setCloExec();
  write.setNonBlock();
#endif
#endif
}

SocketPair::SocketPair() {
  FileDescriptor::system_handle_type pair[2];

#ifdef _WIN32
  // The win32 libevent implementation will attempt to use unix domain sockets
  // if available, but will fall back to using loopback TCP sockets.
  auto r = evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
#else
  auto r = ::socketpair(
      AF_UNIX,
#ifdef SOCK_NONBLOCK
      SOCK_NONBLOCK |
#endif
#ifdef SOCK_CLOEXEC
          SOCK_CLOEXEC |
#endif
          SOCK_STREAM,
      0,
      pair);
#endif
  folly::checkUnixError(r, "socketpair failed");

  read = FileDescriptor(pair[0], FileDescriptor::FDType::Socket);
  write = FileDescriptor(pair[1], FileDescriptor::FDType::Socket);

  read.setNonBlock();
  write.setNonBlock();
  read.setCloExec();
  write.setCloExec();
}
} // namespace watchman
