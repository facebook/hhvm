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

  assertx(m_abortPipeFd[0] == -1 && m_abortPipeFd[1] == -1);

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

  stopConnectionThread();
}

void SocketTransport::stopConnectionThread() {
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
    assertx(false);
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
      stopConnectionThread();
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
    assertx(abortFd >= 0);
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
  bool anyInterfaceBound = false;
  for (address = ai; address != nullptr; address = address->ai_next) {
    if (bindAndListen(address, socketFds)) {
      anyInterfaceBound = true;
    }
  }

  if (!anyInterfaceBound) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "Debugger failed to bind to any interface!"
    );
    return;
  } else {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Debugger bound to at least one interface."
    );
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

  if (fd < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelWarning,
      "SocketTransport: socket() call failed: %d",
      errno
    );

    return false;
  }

  constexpr int yes = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if (address->ai_family == AF_INET6) {
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes));
  }

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

  socketFds.push_back(fd);
  return true;
}

void SocketTransport::rejectClientWithMsg(int newFd, int abortFd) {
  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "SocketTransport: new client connection rejected because another "
      "client is already connected."
  );

  folly::dynamic rejectMsg = folly::dynamic::object;
  rejectMsg["category"] = OutputLevelError;
  rejectMsg["output"] = "Failed to attach to HHVM: another debugger "
    "client is already attached!";

  folly::dynamic response = folly::dynamic::object;
  response["event"] = EventTypeOutput;
  response["body"] = rejectMsg;
  response["type"] = MessageTypeEvent;

  // Send the user an error message.
  std::string serialized =  folly::toJson(response);
  const char* output = serialized.c_str();
  write(newFd, output, strlen(output) + 1);

  // Send a custom refused event that clients can detect.
  folly::dynamic refusedEvent = folly::dynamic::object;
  refusedEvent["event"] = EventTypeConnectionRefused;
  refusedEvent["type"] = MessageTypeEvent;
  const char* refusedEventOutput = folly::toJson(refusedEvent).c_str();
  write(newFd, refusedEventOutput, strlen(refusedEventOutput) + 1);
  shutdownSocket(newFd, abortFd);
}

void SocketTransport::cleanupFd(int fd) {
  shutdownSocket(fd, m_abortPipeFd[0]);
}

void SocketTransport::shutdownSocket(int sockFd, int abortFd) {
  // Perform an orderly shutdown of the socket so that the message is actually
  // sent and received. This requires us to shutdown the write end of the socket
  // and then drain the receive buffer before closing the socket. If abortFd
  // is signalled before this is complete, we just close the socket and bail.
  ::shutdown(sockFd, SHUT_WR);

  int result;
  size_t size = sizeof(struct pollfd) * 2;
  struct pollfd* fds = (struct pollfd*)malloc(size);
  char* buffer = (char*)malloc(1024);
  SCOPE_EXIT {
    if (fds != nullptr) {
      free(fds);
    }

    close(sockFd);

    if (buffer != nullptr) {
      free(buffer);
    }
  };

  if (buffer == nullptr || fds == nullptr) {
    return;
  }

  int eventMask = POLLIN | POLLERR | POLLHUP;
  constexpr int abortIdx = 0;
  constexpr int readIdx = 1;

  memset(fds, 0, size);
  fds[abortIdx].fd = abortFd;
  fds[abortIdx].events = eventMask;
  fds[readIdx].fd = sockFd;
  fds[readIdx].events = eventMask;

  while (true) {
    result = poll(fds, 2, -1);
    if (result == -EINTR) {
      continue;
    } else if (result < 0 ||
               fds[abortIdx].revents != 0 ||
               fds[readIdx].revents & POLLERR ||
               fds[readIdx].revents & POLLHUP
               ) {

      break;
    } else if (fds[readIdx].revents & POLLIN) {
      result = read(sockFd, buffer, 1024);
      if (result <= 0) {
        break;
      }
    }
  }
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

  for (unsigned int i = 1; i < count; i++) {
    fds[i].fd = socketFds[i - 1];
    fds[i].events = eventMask;
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

          if (m_clientConnected) {
            // A client is already connected!
            m_lock.unlock();
            rejectClientWithMsg(newFd, abortFd);
            m_lock.lock();
          } else {
            VSDebugLogger::Log(
              VSDebugLogger::LogLevelInfo,
              "SocketTransport: new client connection accepted."
            );

            // We have established a connection with a client.
            m_clientConnected = true;
            setTransportFd(newFd);
            m_debugger->setClientConnected(true);
          }
        }
      }

      // Reset the event flags.
      fds[i].revents = 0;
    }
  }
}

}
}
