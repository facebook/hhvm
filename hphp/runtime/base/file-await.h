#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/socket-event.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct FileAwait;

struct FileTimeoutHandler : AsioTimeoutHandler {
 friend struct FileAwait;

  FileTimeoutHandler(AsioEventBase* base, FileAwait& fa):
    AsioTimeoutHandler(base), m_fileAwait(fa) {}

  void timeoutExpired() noexcept override;

 private:
  FileAwait& m_fileAwait;
};

struct FileEventHandler : AsioEventHandler {
 friend struct FileAwait;

  FileEventHandler(AsioEventBase* base, int fd, FileAwait& fa):
    AsioEventHandler(base, folly::NetworkSocket::fromFd(fd)), m_fileAwait(fa) {}

  void handlerReady(uint16_t events) noexcept override;

 private:
  FileAwait& m_fileAwait;
};

struct FileAwait : AsioExternalThreadEvent {
  enum Status {
    ERROR = -1,
    TIMEOUT = 0,
    READY,
    CLOSED,
  };

  FileAwait(int fd, uint16_t events, std::chrono::nanoseconds timeout);
  ~FileAwait();
  void unserialize(TypedValue& c) override;

  /**
   * This should be called from the web request thread when the file descriptor
   * is closed. It does any necessary cleanup, marks the associated wait handle
   * as finished and sets the return value to FileAwait::CLOSED.
   *
   * If this isn't called, closing the file descriptor may result in undefined
   * behavior for any pending FileAwaits, depending on the exact platform,
   * backend, type of the file descriptor and events. For example, pending
   * FileAwaits will hang forever on Linux with libevent when a server file
   * descriptor is closed (https://github.com/facebook/hhvm/issues/8716).
   */
  static void closeAllForFD(int fd);

 private:
  void setFinished(int64_t status, bool remove_from_map);
  static std::unordered_map<int, std::unordered_set<FileAwait*>>&
    fdToFileAwaits();

  int m_fd;
  FileEventHandler m_eventHandler;
  FileTimeoutHandler m_timeoutHandler;
  int m_result{-1};

  friend struct FileEventHandler;
  friend struct FileTimeoutHandler;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
