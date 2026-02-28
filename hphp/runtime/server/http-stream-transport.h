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

#pragma once

#include "hphp/runtime/server/stream-transport.h"

#include "hphp/util/assertions.h"

#include <folly/io/IOBuf.h>
#include <folly/Synchronized.h>

namespace HPHP {

struct Transport;

namespace stream_transport {
// HttpStreamServerTransport uses non-blocking POST messages and
// chuncked responses to implement a StreamTransport for the server.
// A non-blocking POST message contains a custom NonBlockingPost header.
struct HttpStreamServerTransport final : StreamTransport {
  explicit HttpStreamServerTransport(Transport* t) : m_transport(t) {
    assertx(t);
  }
  ~HttpStreamServerTransport() override {}

  void write(folly::StringPiece slice) override;

  void close() override;
  void closeNow() override;
  bool isClosed() const override { return m_sharedData.lock()->eom_sent; }
  bool isClosing() const override { return m_sharedData.lock()->eom_sent; }

  bool isReady() const {
    return m_sharedData.lock()->onData != nullptr;
  }

  void setOnData(OnDataType callback) override;
  void setOnClose(OnCloseType callback) override;

  void doOnData(std::unique_ptr<folly::IOBuf> buf);
  void doOnClose();

private:
  Transport* const m_transport;
  struct SharedData {
    bool eom_sent{false};
    bool eom_received{false};
    OnDataType onData;
    OnCloseType onClose;
  };
  folly::Synchronized<SharedData, std::mutex> m_sharedData;
};
} // namespace stream_transport
} // namespace HPHP
