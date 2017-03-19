#ifndef incl_HPHP_ASIO_SOCKET_EVENT_H
#define incl_HPHP_ASIO_SOCKET_EVENT_H

#include "hphp/runtime/ext/extension.h"
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventHandler.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/Singleton.h>
#include <thread>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

typedef folly::EventHandler AsioEventHandler;
typedef folly::AsyncTimeout AsioTimeoutHandler;

class AsioEventBase : public folly::EventBase {
 public:
  AsioEventBase();
  ~AsioEventBase();

 private:
  std::thread m_thread;
};

extern folly::Singleton<AsioEventBase> s_asio_event_base;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_ASIO_SOCKET_EVENT_H
