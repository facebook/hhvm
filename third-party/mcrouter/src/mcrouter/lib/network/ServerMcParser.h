/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/network/AsciiSerialized.h"
#include "mcrouter/lib/network/McAsciiParser.h"
#include "mcrouter/lib/network/McParser.h"

namespace facebook {
namespace memcache {

class ConnectionFifo;

template <class Callback>
class ServerMcParser : private McParser::ParserCallback {
 public:
  ServerMcParser(Callback& cb, size_t minBufferSize, size_t maxBufferSize);

  ~ServerMcParser() override;

  /**
   * AsyncTransport-style getReadBuffer().
   *
   * @return  a buffer pointer and its size that should be safe to read into.
   *
   * The caller might use less than the whole buffer, and will call
   * readDataAvailable(n) giving the actual number of bytes used from
   * the beginning of this buffer.
   */
  std::pair<void*, size_t> getReadBuffer();

  /**
   * Feeds the new data into the parser.
   * @return false  On any parse error.
   */
  bool readDataAvailable(size_t len);

  mc_protocol_t protocol() const {
    return parser_.protocol();
  }

  bool outOfOrder() const {
    return parser_.outOfOrder();
  }

  /**
   * @return error message from ascii parser about parsing error.
   */
  folly::StringPiece getUnderlyingAsciiParserError() const {
    return asciiParser_.getErrorDescription();
  }

  void setDebugFifo(ConnectionFifo* fifo) {
    debugFifo_ = fifo;
    parser_.setDebugFifo(fifo);
  }

 private:
  McParser parser_;
  McServerAsciiParser asciiParser_;

  Callback& callback_;

  // Debug fifo fields and methods
  ConnectionFifo* debugFifo_{nullptr};
  template <class Request>
  FOLLY_NOINLINE void writeToPipe(const Request& req);

  /* McParser callbacks */
  bool caretMessageReady(
      const CaretMessageInfo& headerInfo,
      const folly::IOBuf& buffer) final;
  void handleAscii(folly::IOBuf& readBuffer) final;
  void parseError(carbon::Result result, folly::StringPiece reason) final;
  bool shouldReadToAsciiBuffer() const;

  // McServerAsciiParser callbacks
  template <class Request>
  void onRequest(Request&&, bool noreply);
  void multiOpEnd();

  // McServerAsciiParser callback wrapper.
  template <class C, class ReqsList>
  friend class detail::CallbackWrapper;
};
} // namespace memcache
} // namespace facebook

#include "ServerMcParser-inl.h"
