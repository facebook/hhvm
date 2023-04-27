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
void HttpStreamServerTransport::write(folly::StringPiece data) {
  assertx(m_transport);
  if (m_eom_sent) {
    return;
  }
  m_transport->sendStreamResponse(data.data(), data.size());
}

void HttpStreamServerTransport::setOnData(OnDataType callback) {
  assertx(!callback || !onData);
  onData = callback;
  assertx(m_transport);
  if (callback) {
    m_transport->onStreamReady();
  }
}

void HttpStreamServerTransport::close() {
  if (m_eom_sent) {
    return;
  }
  assertx(m_transport);
  m_transport->sendStreamEOM();
  m_eom_sent = true;
}
} // namaespace stream_transport
} // namespace HPHP
