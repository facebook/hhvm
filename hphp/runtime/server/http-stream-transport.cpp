/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/server/http-stream-transport.h"

#include "hphp/runtime/server/transport.h"

namespace HPHP {
namespace stream_transport {

namespace {

template<typename T>
inline T take(T& value, T&& repl = T{}) {
  auto tmp = std::move(repl);
  std::swap(tmp, value);
  return tmp;
}

} // anonymous namespace

void HttpStreamServerTransport::doOnData(std::unique_ptr<folly::IOBuf> buf) {
  auto onData = m_sharedData.lock()->onData;
  if (onData) {
    onData(std::move(buf));
  }
}

void HttpStreamServerTransport::doOnClose() {
  auto onClose = take(m_sharedData.lock()->onClose);
  if (onClose) {
    onClose();
  }
  m_sharedData.lock()->eom_received = true;
}

void HttpStreamServerTransport::setOnClose(OnCloseType callback) {
  auto eom_received = m_sharedData.withLock([callback = std::move(callback)](auto& sharedData) mutable {
    assertx(!callback || !sharedData.onClose);
    sharedData.onClose = std::move(callback);
    return sharedData.eom_received;
  });
  if (eom_received) {
    doOnClose();
  }
}

void HttpStreamServerTransport::write(folly::StringPiece data) {
  if (m_sharedData.lock()->eom_sent) {
    return;
  }
  m_transport->sendStreamResponse(data.data(), data.size());
}

void HttpStreamServerTransport::setOnData(OnDataType callback) {
  bool nonnull = (bool)callback;
  m_sharedData.withLock([callback = std::move(callback)](auto& sharedData) mutable {
    assertx(!callback || !sharedData.onData);
    sharedData.onData = std::move(callback);
  });
  if (nonnull) {
    m_transport->onStreamReady();
  }
}

void HttpStreamServerTransport::close() {
  m_sharedData.withLock([transport = m_transport](auto& sharedData) {
    if (sharedData.eom_sent) {
      return;
    }
    transport->sendStreamEOM();
    sharedData.eom_sent = true;
    sharedData.onData = nullptr;
    sharedData.onClose = nullptr;
  });
}

void HttpStreamServerTransport::closeNow() {
  close();
}

} // namaespace stream_transport
} // namespace HPHP
