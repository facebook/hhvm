/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBufQueue.h>

#include "mcrouter/lib/carbon/Result.h"
#include "mcrouter/lib/debug/ConnectionFifo.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/CaretHeader.h"

namespace facebook {
namespace memcache {

/*
 * Determine the protocol by looking at the first byte
 */
inline mc_protocol_t determineProtocol(uint8_t firstByte) {
  switch (firstByte) {
    case kCaretMagicByte:
      return mc_caret_protocol;
    default:
      return mc_ascii_protocol;
  }
}

class McParser {
 public:
  class ParserCallback {
   public:
    virtual ~ParserCallback() = 0;

    /**
     * caretMessageReady should be called after we have successfully parsed the
     * Caret header and after the full Caret message body is in the read buffer.
     *
     * @param headerInfo  Parsed header data (header size, body size, etc.)
     * @param buffer      Coalesced IOBuf that holds the entire message
     *                    (header and body)
     * @return            False on any parse errors.
     */
    virtual bool caretMessageReady(
        const CaretMessageInfo& headerInfo,
        const folly::IOBuf& buffer) = 0;

    /**
     * Handle ascii data read.
     * The user is responsible for clearing or advancing the readBuffer.
     *
     * @param readBuffer  buffer with newly read data that needs to be parsed.
     */
    virtual void handleAscii(folly::IOBuf& readBuffer) = 0;

    /**
     * Called on fatal parse error (the stream should normally be closed)
     */
    virtual void parseError(
        carbon::Result result,
        folly::StringPiece reason) = 0;
  };

  McParser(
      ParserCallback& cb,
      size_t minBufferSize,
      size_t maxBufferSize,
      const bool useJemallocNodumpAllocator = false,
      ConnectionFifo* debugFifo = nullptr);

  ~McParser() = default;

  mc_protocol_t protocol() const {
    return protocol_;
  }

  void setProtocol(mc_protocol_t protocol__) {
    protocol_ = protocol__;
  }

  bool outOfOrder() const {
    return outOfOrder_;
  }

  /**
   * AsyncTransport-style getReadBuffer().
   *
   * Returns a buffer pointer and its size that should be safe
   * to read into.
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

  void reset();

  void setDebugFifo(ConnectionFifo* fifo) {
    debugFifo_ = fifo;
  }

 private:
  bool seenFirstByte_{false};
  bool outOfOrder_{false};

  mc_protocol_t protocol_{mc_unknown_protocol};

  ParserCallback& callback_;
  size_t bufferSize_{256};
  size_t minBufferSize_{256};
  size_t maxBufferSize_{4096};

  ConnectionFifo* debugFifo_{nullptr};

  uint64_t lastShrinkCycles_{0};

  folly::IOBuf readBuffer_;

  /**
   * If we've read a caret header, this will contain header/body sizes.
   */
  CaretMessageInfo msgInfo_;

  /**
   * Custom allocator states and method
   */
  bool useJemallocNodumpAllocator_{false};

  bool readCaretData();
  void readBuffReserve(size_t bufSize);
};

inline McParser::ParserCallback::~ParserCallback() {}

} // namespace memcache
} // namespace facebook
