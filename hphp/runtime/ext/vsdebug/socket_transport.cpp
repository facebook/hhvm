/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/vsdebug/socket_transport.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

SocketTransport::SocketTransport(Debugger* debugger, int listenPort) :
  DebugTransport(debugger),
  m_terminating(false),
  m_clientConnected(false),
  m_listenPort(listenPort),
  m_connectThread(this, &SocketTransport::listenForClientConnection) {

  Lock lock(m_lock);

  assert(m_abortPipeFd[0] == -1 && m_abortPipeFd[1] == -1);

  // Create a set of pipe file descriptors to use to inform the thread
  // polling for socket connections that it's time to exit.
  createAbortPipe();
  m_connectThread.start();
}

SocketTransport::~SocketTransport() {
  {
    Lock lock(m_lock);
    m_terminating = true;
  }

  char value = '\0';
  write(m_abortPipeFd[1], &value, 1);
  close(m_abortPipeFd[1]);
  m_abortPipeFd[1] = -1;

  m_connectThread.waitForEnd();
}

void SocketTransport::createAbortPipe() {

  if (m_abortPipeFd[0] != -1) {
    close(m_abortPipeFd[0]);
  }

  if (m_abortPipeFd[1] != -1) {
    close(m_abortPipeFd[1]);
  }

  if (pipe(m_abortPipeFd) < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to open pipe for transport termination event."
    );

    // This is unexpected and treated as fatal because we won't be able
    // to stop the polling thread in an orderly fashion at this point.
    assert(false);
  }
}

void SocketTransport::onClientDisconnected() {
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "SocketTransport client disconnected."
  );

  DebugTransport::onClientDisconnected();

  // If we're not shutting down, start listening for new connections.
  {
    Lock lock(m_lock);

    if (!m_terminating && m_clientConnected) {
      m_connectThread.waitForEnd();
      m_clientConnected = false;
      createAbortPipe();
      m_connectThread.start();
    }
  }
}

bool SocketTransport::clientConnected() const {
  Lock lock(m_lock);
  return m_clientConnected;
}

void SocketTransport::listenForClientConnection() {
  // If there was a previous connection in the base class, shut it down.
  // This will close the old transport and wait for both the send and receive
  // worker threads to terminate before proceeding.
  if (getTransportFd() >= 0) {
    shutdown();
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "SocketTransport worker thread listening for connections..."
  );

  int abortFd;
  {
    Lock lock(m_lock);
    abortFd = m_abortPipeFd[0];
    assert(abortFd >= 0);
  }

  struct addrinfo hint;
  struct addrinfo* ai = nullptr;
  std::vector<int> socketFds;

  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE;

  SCOPE_EXIT {
    if (ai != nullptr) {
      freeaddrinfo(ai);
    }

    for (auto it = socketFds.begin(); it != socketFds.end(); it++) {
      close(*it);
    }

    close(abortFd);
    m_abortPipeFd[0] = -1;

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "SocketTransport connection polling thread exiting."
    );
  };

  // Share existing DebuggerDisableIPv6 configuration with hphpd.
  // TODO: Do we need our own separate config item for this?
  if (RuntimeOption::DebuggerDisableIPv6) {
    hint.ai_family = AF_INET;
  }

  const auto name = RuntimeOption::DebuggerServerIP.empty()
    ? "localhost"
    : RuntimeOption::DebuggerServerIP.c_str();
  if (getaddrinfo(name, std::to_string(m_listenPort).c_str(), &hint, &ai)) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to call getaddrinfo: %d.",
      errno
    );

    return;
  }

  // Attempt to listen on the specified port on each of this host's available
  // addresses.
  struct addrinfo* address;
  for (address = ai; address != nullptr; address = address->ai_next) {
    if (!bindAndListen(address, socketFds)) {
      return;
    }
  }

  waitForConnection(socketFds, abortFd);
}

bool SocketTransport::bindAndListen(
  struct addrinfo* address,
  std::vector<int>& socketFds
) {
  int fd = socket(address->ai_family,
                  address->ai_socktype,
                  address->ai_protocol);

  if (fd < 0 && errno == EAFNOSUPPORT) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "SocketTransport: socket() call failed: %d",
      errno
    );

    // Don't bind to this address, but still try to process other
    // addresses if there are any.
    return true;
  }

  constexpr int yes = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if (address->ai_family == AF_INET6) {
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes));
  }

  socketFds.push_back(fd);

  if (bind(fd, address->ai_addr, address->ai_addrlen) < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to call bind: %d. (%s)",
      errno,
      strerror(errno)
    );

    return false;
  }

  if (listen(fd, 1) < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to call listen on port %u: %d. (%s)",
      m_listenPort,
      errno,
      strerror(errno)
    );

    return false;
  }

  return true;
}

void SocketTransport::waitForConnection(
  std::vector<int>& socketFds,
  int abortFd
) {
  // Wait for a connection on any of the fds we are listening to. We allow only
  // one debugger client to connect a a time, so once any bound fd accepts a
  // connection, we stop listening on all the others.
  int count = socketFds.size() + 1;
  size_t size = sizeof(struct pollfd) * count;
  struct pollfd* fds = (struct pollfd*)malloc(size);
  if (fds == nullptr) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "SocketTransport out of memory while trying to create pollfd."
    );
    return;
  }

  memset(fds, 0, size);

  SCOPE_EXIT {
    if (fds != nullptr) {
      free(fds);
    }
  };

  // fds[0] will contain the read end of our "abort" pipe. Another thread will
  // write data to tihs pipe to signal it's time for this worker to stop
  // blocking in poll() and terminate.
  int eventMask = POLLIN | POLLERR | POLLHUP;
  fds[0].fd = abortFd;
  fds[0].events = eventMask;

  for (unsigned int i = 0; i < count; i++) {
    fds[i + 1].fd = socketFds[i];
    fds[i + 1].events = eventMask;
  }

  // Poll socket fds until a connection is established or we're terminated.
  while (true) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "SocketTransport polling for connections..."
    );

    int ret = poll(fds, count, -1);
    if (ret < 0) {
      if (ret == -EINTR) {
        continue;
      }

      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Polling inputs failed: %d. (%s)",
        errno,
        strerror(errno)
      );
      return;
    }

    if (fds[0].revents != 0) {
      {
        Lock lock(m_lock);
        assert(m_terminating);
      }
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelInfo,
        "Socket polling thread terminating due to shutdown request."
      );
      return;
    }

    struct sockaddr sa;
    socklen_t len = sizeof(sa);
    for (unsigned int i = 1; i < count; i++) {
      if (fds[i].revents & POLLIN) {
        int newFd = ::accept(fds[i].fd, &sa, &len);
        if (newFd < 0) {
          VSDebugLogger::Log(
            VSDebugLogger::LogLevelWarning,
            "Accept returned an error: %d. (%s)",
            errno,
            strerror(errno)
          );
        } else {
          Lock lock(m_lock);

          VSDebugLogger::Log(
            VSDebugLogger::LogLevelInfo,
            "SocketTransport: new client connection accepted."
          );

          // We have established a connection with a client.
          m_clientConnected = true;
          setTransportFd(newFd);
          m_debugger->setClientConnected(true);

          // Return from this routine to stop listening for more connections.
          return;
        }
      }

      // Reset the event flags.
      fds[i].revents = 0;
    }
  }
}

}
}
