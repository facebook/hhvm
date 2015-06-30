#include "hphp/runtime/base/file-await.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void FileTimeoutHandler::timeoutExpired() noexcept {
  if (m_fileAwait) {
    m_fileAwait->setFinished(FileAwait::TIMEOUT);
  }
}

void FileEventHandler::handlerReady(uint16_t events) noexcept {
  if (m_fileAwait) {
    m_fileAwait->setFinished(events ? FileAwait::READY : FileAwait::CLOSED);
  }
}

/////////////////////////////////////////////////////////////////////////////

FileAwait::FileAwait(int fd, uint16_t events, double timeout) {
  assert(fd >= 0);
  assert(events & FileEventHandler::READ_WRITE);

  m_file = std::make_shared<FileEventHandler>
    (s_asio_event_base.get(), fd, this);
  m_file->registerHandler(events);

  int64_t timeout_ms = timeout * 1000.0;
  if (timeout_ms > 0) {
    m_timeout = std::make_shared<FileTimeoutHandler>
      (s_asio_event_base.get(), this);
    s_asio_event_base->runInEventBaseThread([this,timeout_ms]{
      if (m_timeout) {
        m_timeout->scheduleTimeout(timeout_ms);
      }
    });
  }
}

FileAwait::~FileAwait() {
  if (m_file) {
    // Avoid possible race condition
    m_file->m_fileAwait = nullptr;

    m_file->unregisterHandler();
    m_file.reset();
  }
  if (m_timeout) {
    // Avoid race condition, we may (likely) finish destructing
    // before the timeout cancels
    m_timeout->m_fileAwait = nullptr;

    std::shared_ptr<FileTimeoutHandler> to = m_timeout;
    s_asio_event_base->runInEventBaseThread([to]{
      to.get()->cancelTimeout();
    });
    m_timeout.reset();
  }
}

void FileAwait::unserialize(Cell& c) {
  c.m_type = KindOfInt64;
  c.m_data.num = m_result;
}

void FileAwait::setFinished(int64_t status) {
  if (status > m_result) {
    m_result = status;
  }
  if (!m_finished) {
    markAsFinished();
    m_finished = true;
  }
}

/////////////////////////////////////////////////////////////////////////////

Object File::await(uint16_t events, double timeout) {
  if (isClosed()) {
    Cell closedResult;
    closedResult.m_type = KindOfInt64;
    closedResult.m_data.num = FileAwait::CLOSED;
    return c_StaticWaitHandle::CreateSucceeded(closedResult);
  }
  if (fd() < 0) {
    SystemLib::throwExceptionObject(
      "Unable to await on stream, invalid file descriptor");
  }
  events = events & FileEventHandler::READ_WRITE;
  if (!events) {
    SystemLib::throwExceptionObject(
      "Must await for reading, writing, or both.");
  }

  auto ev = new FileAwait(fd(), events, timeout);
  try {
    return ev->getWaitHandle();
  } catch (...) {
    assert(false);
    ev->abandon();
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
