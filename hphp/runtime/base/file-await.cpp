#include "hphp/runtime/base/file-await.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include "hphp/util/compatibility.h"

#include <folly/Chrono.h>

#include <fcntl.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void FileTimeoutHandler::timeoutExpired() noexcept {
  m_fileAwait.m_eventHandler.unregisterHandler();
  m_fileAwait.setFinished(FileAwait::TIMEOUT, true);
}

void FileEventHandler::handlerReady(uint16_t events) noexcept {
  m_fileAwait.m_timeoutHandler.cancelTimeout();
  m_fileAwait.setFinished(events ? FileAwait::READY : FileAwait::CLOSED, true);
}

/////////////////////////////////////////////////////////////////////////////

FileAwait::FileAwait(
  int fd,
  uint16_t events,
  std::chrono::nanoseconds timeout
) : m_fd(fd),
    m_eventHandler(getSingleton<AsioEventBase>().get(), fd, *this),
    m_timeoutHandler(getSingleton<AsioEventBase>().get(), *this) {

  assertx(timeout.count() >= 0);
  assertx(fd >= 0);
  assertx(events & FileEventHandler::READ_WRITE);

  getSingleton<AsioEventBase>()->runInEventBaseThread([this, events, timeout] {
    m_eventHandler.registerHandler(events);
    if (timeout != std::chrono::nanoseconds::zero()) {
      // Folly internally converts everything to milliseconds, so might as well
      // do the same
      m_timeoutHandler.scheduleTimeout(
        folly::chrono::ceil<std::chrono::milliseconds>(timeout));
    }
    FileAwait::fdToFileAwaits()[m_fd].insert(this);
  });
}

FileAwait::~FileAwait() {
  assertx(!m_eventHandler.isHandlerRegistered());
  assertx(!m_timeoutHandler.isScheduled());
  assertx(
    !FileAwait::fdToFileAwaits().count(m_fd) ||
    !FileAwait::fdToFileAwaits()[m_fd].count(this));
}

void FileAwait::unserialize(TypedValue& c) {
  c.m_type = KindOfInt64;
  c.m_data.num = m_result;
}

void FileAwait::closeAllForFD(int fd) {
  getSingleton<AsioEventBase>()->runInEventBaseThreadAndWait([fd] {
    for (FileAwait* fa : FileAwait::fdToFileAwaits()[fd]) {
      fa->m_eventHandler.unregisterHandler();
      fa->m_timeoutHandler.cancelTimeout();
      // don't remove_from_map because we're iterating over it, and we do it
      // more efficiently below
      fa->setFinished(FileAwait::CLOSED, false);
    }
    FileAwait::fdToFileAwaits().erase(fd);
  });
}

// must be called from the EventBase thread, as it's not thread-safe
void FileAwait::setFinished(int64_t status, bool remove_from_map) {
  assertx(!m_eventHandler.isHandlerRegistered());
  assertx(!m_timeoutHandler.isScheduled());

  if (remove_from_map) {
    FileAwait::fdToFileAwaits()[m_fd].erase(this);
    if (FileAwait::fdToFileAwaits()[m_fd].empty()) {
      FileAwait::fdToFileAwaits().erase(m_fd);
    }
  }

  if (status > m_result) {
    m_result = status;
  }
  markAsFinished();
}

std::unordered_map<int, std::unordered_set<FileAwait*>>&
    FileAwait::fdToFileAwaits() {
  static std::unordered_map<int, std::unordered_set<FileAwait*>> ret;
  return ret;
}

/////////////////////////////////////////////////////////////////////////////

Object File::await(uint16_t events, double timeout) {
  if (isClosed()) {
    TypedValue closedResult;
    closedResult.m_type = KindOfInt64;
    closedResult.m_data.num = FileAwait::CLOSED;
    return Object{c_StaticWaitHandle::CreateSucceeded(closedResult)};
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
  const auto originalFlags = ::fcntl(fd(), F_GETFL);
  // This always succeeds...
  ::fcntl(fd(), F_SETFL, originalFlags | O_ASYNC);
  // ... but sometimes doesn't actually do anything
  const bool isAsyncableFd = ::fcntl(fd(), F_GETFL) & O_ASYNC;
  ::fcntl(fd(), F_SETFL, originalFlags);

  if (!isAsyncableFd) {
    SystemLib::throwInvalidOperationExceptionObject(
      folly::sformat(
        "File descriptor {} is not awaitable - real file?",
        fd()
      )
    );
  }

  // we have a double timeout in seconds, need ms
  // keeping with ms rather than more accuracy to avoid introducing subtle behavior
  // changes to existing callsites that expect this behavior, e.g. `stream_await`;
  // they already need to consider <1ms to be 0 to avoid blocking forever due to
  // past bugs, so we don't get anything new here.
  auto ev = new FileAwait(
    fd(),
    events,
    std::chrono::milliseconds(static_cast<int64_t>(timeout * 1000.0))
  );
  return Object{ev->getWaitHandle()};
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
