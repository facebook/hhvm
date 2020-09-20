#pragma once

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/curl/curl-multi-resource.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct CurlEventHandler;
struct CurlTimeoutHandler;

struct CurlMultiAwait : AsioExternalThreadEvent {
  CurlMultiAwait(req::ptr<CurlMultiResource> multi, double timeout);
  ~CurlMultiAwait();

  void unserialize(TypedValue& c) override;

 private:
  friend struct CurlEventHandler;
  friend struct CurlTimeoutHandler;
  void setFinished(int fd);

  void addHandle(int fd, int events);
  int addLowHandles(req::ptr<CurlMultiResource> multi);
  int addHighHandles(req::ptr<CurlMultiResource> multi);

  req::shared_ptr<CurlTimeoutHandler> m_timeout;
  req::vector<req::shared_ptr<CurlEventHandler>> m_handlers;
  int m_result{-1};
  bool m_finished{false};
};

/////////////////////////////////////////////////////////////////////////////
}
