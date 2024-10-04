/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/async/DuplexChannel.h>

#include <folly/io/Cursor.h>

using folly::io::RWPrivateCursor;
using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;

namespace apache::thrift {

DuplexChannel::DuplexChannel(
    Who::WhoEnum who,
    const shared_ptr<folly::AsyncTransport>& transport,
    HeaderClientChannel::Options options)
    : cpp2Channel_(
          new DuplexCpp2Channel(
              who, transport, make_unique<DuplexFramingHandler>(*this)),
          folly::DelayedDestruction::Destructor()),
      clientChannel_(
          new DuplexClientChannel(*this, cpp2Channel_, std::move(options)),
          folly::DelayedDestruction::Destructor()),
      clientFramingHandler_(*clientChannel_.get()),
      serverChannel_(
          new DuplexServerChannel(*this, cpp2Channel_),
          folly::DelayedDestruction::Destructor()),
      serverFramingHandler_(*serverChannel_.get()),
      mainChannel_(who) {
  mainChannel_.get(); // check that it's not UNKNOWN
  // Tell the cpp2 channel which callback is the client and which is the server
  // so it can do its magic
  cpp2Channel_->primeCallbacks(clientChannel_.get(), serverChannel_.get());
}

FramingHandler& DuplexChannel::DuplexFramingHandler::getHandler(
    Who::WhoEnum who) {
  switch (who) {
    case Who::CLIENT:
      return duplex_.clientFramingHandler_;
    case Who::SERVER:
      return duplex_.serverFramingHandler_;
    default:
      throw std::runtime_error("bad who value");
  }
}

std::tuple<std::unique_ptr<folly::IOBuf>, size_t, std::unique_ptr<THeader>>
DuplexChannel::DuplexFramingHandler::removeFrame(folly::IOBufQueue* q) {
  if (!q || !q->front() || q->front()->empty()) {
    return make_tuple(std::unique_ptr<IOBuf>(), 0, nullptr);
  }

  uint32_t len = q->front()->computeChainDataLength();

  if (len < 4) {
    size_t remaining = 4 - len;
    return make_tuple(unique_ptr<IOBuf>(), remaining, nullptr);
  }

  Cursor c(q->front());
  uint32_t msgLen = c.readBE<uint32_t>();

  if (msgLen > THeader::MAX_FRAME_SIZE) {
    // Not a framed or header message. Either unframed of HTTP, so
    // pass it to the main channel
    return getHandler(duplex_.mainChannel_.get()).removeFrame(q);
  }

  if (len - 4 < msgLen) {
    // Framed message, but haven't received whole message yet
    size_t remaining = msgLen - (len - 4);
    return make_tuple(unique_ptr<IOBuf>(), remaining, nullptr);
  }

  // Got whole message, check if it's header
  // Header starts with LEN(4bytes) | MAGIC(2bytes) | FLAGS(2bytes)
  if (c.readBE<uint16_t>() != THeader::HEADER_MAGIC >> 16) {
    // Framed, not header
    // pass it to the main channel
    return getHandler(duplex_.mainChannel_.get()).removeFrame(q);
  }

  // Header, check if reverse
  bool reverse = c.readBE<uint16_t>() & HEADER_FLAG_DUPLEX_REVERSE;
  Who::WhoEnum msgWho =
      reverse ? duplex_.mainChannel_.getOther() : duplex_.mainChannel_.get();
  duplex_.cpp2Channel_->useCallback(msgWho);

  // Next message in queue_ might have a different reverse bit, so split
  // the current message, pass it to the correct framing handler
  // and retain the rest of q for the next invocation
  IOBufQueue thisMessageQueue;
  thisMessageQueue.append(q->split(4 + msgLen));

  return getHandler(msgWho).removeFrame(&thisMessageQueue);
}

std::unique_ptr<folly::IOBuf> DuplexChannel::DuplexFramingHandler::addFrame(
    std::unique_ptr<folly::IOBuf> buf, THeader* header) {
  buf = getHandler(duplex_.lastSender_.get()).addFrame(std::move(buf), header);

  if (duplex_.lastSender_.get() != duplex_.mainChannel_.get()) {
    // Add reverse bit to header
    // Header starts with LEN(4bytes) | MAGIC(2bytes) | FLAGS(2bytes)
    Cursor c(buf.get());
    if (c.length() >= 8 && // long enough to possible be header protocol
        c.readBE<uint32_t>() <= THeader::MAX_FRAME_SIZE && // is framed
        c.readBE<uint16_t>() == THeader::HEADER_MAGIC >> 16) {
      uint16_t flags = c.readBE<uint16_t>();
      flags |= HEADER_FLAG_DUPLEX_REVERSE;
      RWPrivateCursor wc(buf.get());
      wc.skip(6); // position at start of flags
      wc.writeBE(flags);
    }
  }

  return buf;
}

} // namespace apache::thrift
