#include "hphp/runtime/ext/asio/socket-event.h"

using folly::EventBase;
using folly::Singleton;

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

AsioEventBase::AsioEventBase() : EventBase(), m_thread([&] {
  loopForever();
}) {
  waitUntilRunning();
}

AsioEventBase::~AsioEventBase() {
  terminateLoopSoon();
  m_thread.join();
}

Singleton<AsioEventBase> s_asio_event_base;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
