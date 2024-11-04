#pragma once

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

struct AsioEventBase : folly::EventBase {
  AsioEventBase();
  ~AsioEventBase();

 private:
  std::thread m_thread;
};

extern folly::Singleton<AsioEventBase> s_asio_event_base;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
