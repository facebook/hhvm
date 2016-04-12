#ifndef incl_HPHP_CURL_MULTI_AWAIT_H
#define incl_HPHP_CURL_MULTI_AWAIT_H

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/curl/curl-multi-resource.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

struct CurlEventHandler;
struct CurlTimeoutHandler;

struct CurlMultiAwait : AsioExternalThreadEvent {
  CurlMultiAwait(req::ptr<CurlMultiResource> multi, double timeout);
  ~CurlMultiAwait();

  void unserialize(Cell& c) override;

 private:
  friend struct CurlEventHandler;
  friend struct CurlTimeoutHandler;
  void setFinished(int fd);

  void addHandle(int fd, int events);
  int addLowHandles(req::ptr<CurlMultiResource> multi);
  int addHighHandles(req::ptr<CurlMultiResource> multi);

  std::shared_ptr<CurlTimeoutHandler> m_timeout;
  std::vector<std::shared_ptr<CurlEventHandler>> m_handlers;
  int m_result{-1};
  bool m_finished{false};
};

/////////////////////////////////////////////////////////////////////////////
}
#endif
