#include "hphp/runtime/ext/curl/curl-multi-await.h"
#include "hphp/runtime/ext/curl/curl-resource.h"
#include "hphp/runtime/ext/asio/socket-event.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////
// CurlEventHandler

struct CurlEventHandler : AsioEventHandler {
  CurlEventHandler(AsioEventBase* base, int fd, CurlMultiAwait* cma):
    AsioEventHandler(base, fd), m_curlMultiAwait(cma), m_fd(fd) {}

  void handlerReady(uint16_t events) noexcept override {
    m_curlMultiAwait->setFinished(m_fd);
  }

 private:
  CurlMultiAwait* m_curlMultiAwait;
  int m_fd;
};

/////////////////////////////////////////////////////////////////////////////
// CurlTimeoutHandler

struct CurlTimeoutHandler : AsioTimeoutHandler {
  CurlTimeoutHandler(AsioEventBase* base, CurlMultiAwait* cma):
    AsioTimeoutHandler(base), m_curlMultiAwait(cma) {}

  void timeoutExpired() noexcept override {
    m_curlMultiAwait->setFinished(-1);
  }

 private:
  CurlMultiAwait* m_curlMultiAwait;
};

/////////////////////////////////////////////////////////////////////////////
// CurlMultiAwait

CurlMultiAwait::CurlMultiAwait(req::ptr<CurlMultiResource> multi,
                               double timeout) {
 if ((addLowHandles(multi) + addHighHandles(multi)) == 0) {
    // Nothing to do
    markAsFinished();
    return;
  }

  // Add optional timeout
  int64_t timeout_ms = timeout * 1000;
  if (timeout_ms > 0) {
    auto asio_event_base = getSingleton<AsioEventBase>();
    m_timeout = std::make_shared<CurlTimeoutHandler>(asio_event_base.get(),
                                                     this);

    asio_event_base->runInEventBaseThreadAndWait([this,timeout_ms] {
      m_timeout->scheduleTimeout(timeout_ms);
    });
  }
}

CurlMultiAwait::~CurlMultiAwait() {
  for (auto handler : m_handlers) {
    handler->unregisterHandler();
  }
  if (m_timeout) {
    auto asio_event_base = getSingleton<AsioEventBase>();
    auto to = std::move(m_timeout);
    asio_event_base->runInEventBaseThreadAndWait([to] {
      to.get()->cancelTimeout();
    });
  }
  m_handlers.clear();
}

void CurlMultiAwait::unserialize(Cell& c) {
  c.m_type = KindOfInt64;
  c.m_data.num = m_result;
}

void CurlMultiAwait::setFinished(int fd) {
  if (m_result < fd) {
    m_result = fd;
  }
  if (!m_finished) {
    markAsFinished();
    m_finished = true;
  }
}

void CurlMultiAwait::addHandle(int fd, int events) {
  auto asio_event_base = getSingleton<AsioEventBase>();
  auto handler =
    std::make_shared<CurlEventHandler>(asio_event_base.get(), fd, this);
  handler->registerHandler(events);
  m_handlers.push_back(handler);
}

// Ask curl_multi for its handles directly
// This is preferable as we get to know which
// are blocking on reads, and which on writes.
int CurlMultiAwait::addLowHandles(req::ptr<CurlMultiResource> multi) {
  fd_set read_fds, write_fds;
  int max_fd = -1, count = 0;
  FD_ZERO(&read_fds); FD_ZERO(&write_fds);
  if ((CURLM_OK != curl_multi_fdset(multi->get(), &read_fds, &write_fds,
                                    nullptr, &max_fd)) ||
      (max_fd < 0)) {
    return count;
  }
  for (int i = 0 ; i <= max_fd; ++i) {
    int events = 0;
    if (FD_ISSET(i, &read_fds))  events |= AsioEventHandler::READ;
    if (FD_ISSET(i, &write_fds)) events |= AsioEventHandler::WRITE;
    if (events) {
      addHandle(i, events);
      ++count;
    }
  }
  return count;
}

// Check for file descriptors >= FD_SETSIZE
// which can't be returned in an fdset
// This is a little hacky, but necessary given cURL's APIs
int CurlMultiAwait::addHighHandles(req::ptr<CurlMultiResource> multi) {
  int count = 0;
  auto easy_handles = multi->getEasyHandles();
  for (ArrayIter iter(easy_handles); iter; ++iter) {
    Variant easy_handle = iter.second();
    auto easy = dyn_cast_or_null<CurlResource>(easy_handle);
    if (!easy) continue;
    long sock;
    if ((curl_easy_getinfo(easy->get(),
                           CURLINFO_LASTSOCKET, &sock) != CURLE_OK) ||
        (sock < FD_SETSIZE)) {
      continue;
    }
    // No idea which type of event it needs, ask for everything
    addHandle(sock, AsioEventHandler::READ_WRITE);
    ++count;
  }
  return count;
}


/////////////////////////////////////////////////////////////////////////////
}
