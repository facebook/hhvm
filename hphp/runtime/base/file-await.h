#ifndef incl_HPHP_FILE_AWAIT_H
#define incl_HPHP_FILE_AWAIT_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/socket-event.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct FileAwait;

struct FileTimeoutHandler : AsioTimeoutHandler {
 friend struct FileAwait;

  FileTimeoutHandler(AsioEventBase* base, FileAwait* fa):
    AsioTimeoutHandler(base), m_fileAwait(fa) {}

  void timeoutExpired() noexcept override;

 private:
  FileAwait* m_fileAwait;
};

struct FileEventHandler : AsioEventHandler {
 friend struct FileAwait;

  FileEventHandler(AsioEventBase* base, int fd, FileAwait* fa):
    AsioEventHandler(base, fd), m_fileAwait(fa) {}

  void handlerReady(uint16_t events) noexcept override;

 private:
  FileAwait* m_fileAwait;
};

struct FileAwait : AsioExternalThreadEvent {
  enum Status {
    ERROR = -1,
    TIMEOUT = 0,
    READY,
    CLOSED,
  };

  FileAwait(int fd, uint16_t events, double timeout);
  ~FileAwait();
  void unserialize(Cell& c) override;
  void setFinished(int64_t status);
 private:
  std::shared_ptr<FileEventHandler> m_file;
  std::shared_ptr<FileTimeoutHandler> m_timeout;
  int m_result{-1};
  bool m_finished{false};
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
#endif // incl_HPHP_FILE_AWAIT_H
