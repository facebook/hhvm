/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>
#include <folly/io/async/EventBase.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <proxygen/lib/transport/test/MockAsyncTransportCertificate.h>
#include <quic/api/test/MockQuicSocket.h>
#include <unordered_map>

namespace quic {

using PeekIterator = std::deque<StreamBuffer>::const_iterator;

// The driver stores connection state in a Stream State structure
// so use an id outside the on the wire id space
constexpr uint64_t kConnectionStreamId = std::numeric_limits<uint64_t>::max();

// Checks the error condition, and if true either aborts the program or logs
// and executes code that is intended to treat the error as a soft-error. The
// behavior is controlled by the strictErrorCheck_ variable set via
// setStrictErrorCheck().
// NOTE: do not change this in a do { } while(0). softErrorHandler could be a
// `continue` statement
#define ERROR_IF(condition, message, softErrorHandler) \
  {                                                    \
    if (condition) {                                   \
      if (strictErrorCheck_) {                         \
        CHECK(!(condition)) << message;                \
      } else {                                         \
        LOG(ERROR) << message;                         \
        softErrorHandler;                              \
      }                                                \
    }                                                  \
  }

class MockQuicSocketDriver : public folly::EventBase::LoopCallback {
 public:
  enum StateEnum { NEW, OPEN, PAUSED, CLOSED, ERROR };
  enum TransportEnum { CLIENT, SERVER };

  // support giving a callback to the caller (i.e. the actual test code)
  // whenever the socket receives data, so that the test driver can parse
  // that data and do stuff with it; i.e. detect control streams and feed the
  // incoming data to a codec
  class LocalAppCallback {
   public:
    virtual ~LocalAppCallback() {
    }
    virtual void unidirectionalReadCallback(
        quic::StreamId id, std::unique_ptr<folly::IOBuf> buf) = 0;
    virtual void readCallback(quic::StreamId id,
                              std::unique_ptr<folly::IOBuf> buf) = 0;
  };

 private:
  struct StreamState {
    bool fireByteEventAt(uint64_t offset) {
      return (offset < nextWriteOffset ||
              (offset == nextWriteOffset && writeEOF));
    }

    uint64_t nextWriteOffset{0};
    // data to be read by application
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    uint64_t readBufOffset{0};
    uint64_t readOffset{0};
    bool readEOF{false};
    bool writeEOF{false};
    QuicSocket::WriteCallback* pendingWriteCb{nullptr};
    bool isPendingWriteCbStreamNotif{false};
    // data written by application
    folly::IOBufQueue unsentBuf{folly::IOBufQueue::cacheChainLength()};
    // BufMeta written by application
    WriteBufferMeta unsentBufMeta;
    bool pendingWriteEOF{false};
    // data waiting to be flushed
    folly::IOBufQueue pendingWriteBuf{folly::IOBufQueue::cacheChainLength()};
    // BufMeta waiting to be flushed
    uint64_t pendingBufMetaLength{0};
    // data 'delivered' to peer. There is currently no BufMeta version of this.
    folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
    StateEnum readState{NEW};
    StateEnum writeState{OPEN};
    folly::Optional<quic::ApplicationErrorCode> error;
    QuicSocket::ReadCallback* readCB{nullptr};
    QuicSocket::PeekCallback* peekCB{nullptr};
    using ByteEventList =
        std::list<std::pair<uint64_t, QuicSocket::ByteEventCallback*>>;
    ByteEventList txCallbacks;
    ByteEventList deliveryCallbacks;
    uint64_t flowControlWindow{65536};
    bool isControl{false};
    uint64_t lastSkipOffset{0};
    uint64_t fakePeekOffset{0};
    uint64_t numWriteChainInvocations{0};
  };

 public:
  using StreamStateMap = std::map<StreamId, StreamState>;
  using StreamStatePair = std::pair<const StreamId, StreamState>;

  explicit MockQuicSocketDriver(
      folly::EventBase* eventBase,
      QuicSocket::ConnectionSetupCallback* connSetupCb,
      QuicSocket::ConnectionCallback* connCb,
      TransportEnum transportType,
      std::string alpn = "h3")
      : eventBase_(eventBase),
        transportType_(transportType),
        sock_(std::make_shared<MockQuicSocket>(eventBase, connSetupCb, connCb)),
        alpn_(alpn) {

    if (transportType_ == TransportEnum::SERVER) {
      nextBidirectionalStreamId_ = 1;
      nextUnidirectionalStreamId_ = 3;
    } else {
      nextBidirectionalStreamId_ = 0;
      nextUnidirectionalStreamId_ = 2;
    }

    EXPECT_CALL(*sock_, setConnectionSetupCallback(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](QuicSocket::ConnectionSetupCallback* cb) {
              sock_->setupCb_ = cb;
            }));
    EXPECT_CALL(*sock_, setConnectionCallback(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](QuicSocket::ConnectionCallback* cb) {
              sock_->connCb_ = cb;
            }));
    EXPECT_CALL(*sock_, isClientStream(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](quic::StreamId stream) { return (stream & 0b01) == 0; }));

    EXPECT_CALL(*sock_, isServerStream(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](quic::StreamId stream) { return stream & 0b01; }));

    EXPECT_CALL(*sock_, isUnidirectionalStream(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](quic::StreamId stream) { return stream & 0b10; }));

    EXPECT_CALL(*sock_, isBidirectionalStream(testing::_))
        .WillRepeatedly(testing::Invoke(
            [](quic::StreamId stream) { return !(stream & 0b10); }));

    EXPECT_CALL(*sock_, getState()).WillRepeatedly(testing::Return(nullptr));

    EXPECT_CALL(*sock_, getTransportSettings())
        .WillRepeatedly(testing::ReturnRef(transportSettings_));

    EXPECT_CALL(*sock_, getConnectionBufferAvailable())
        .WillRepeatedly(testing::Return(bufferAvailable_));

    EXPECT_CALL(*sock_, getClientConnectionId())
        .WillRepeatedly(
            testing::Return(quic::ConnectionId({0x11, 0x11, 0x11, 0x11})));

    EXPECT_CALL(*sock_, getServerConnectionId())
        .WillRepeatedly(
            testing::Return(quic::ConnectionId({0x11, 0x11, 0x11, 0x11})));

    EXPECT_CALL(*sock_, getAppProtocol())
        .WillRepeatedly(testing::Return(alpn_));

    EXPECT_CALL(*sock_, good())
        .WillRepeatedly(testing::ReturnPointee(&sockGood_));

    EXPECT_CALL(*sock_, getEventBase())
        .WillRepeatedly(testing::ReturnPointee(&eventBase_));

    EXPECT_CALL(*sock_, setControlStream(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](quic::StreamId id) -> folly::Optional<quic::LocalErrorCode> {
              auto stream = streams_.find(id);
              if (id == kConnectionStreamId || stream == streams_.end()) {
                return quic::LocalErrorCode::STREAM_NOT_EXISTS;
              }
              stream->second.isControl = true;
              return folly::none;
            }));

    EXPECT_CALL(*sock_, getConnectionFlowControl())
        .WillRepeatedly(testing::Invoke([this]() {
          auto& connection = streams_[kConnectionStreamId];
          flowControlAccess_.emplace(kConnectionStreamId);
          return QuicSocket::FlowControlState(
              {connection.flowControlWindow,
               connection.nextWriteOffset + connection.flowControlWindow,
               0,
               0});
        }));

    EXPECT_CALL(*sock_, getStreamFlowControl(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id)
                -> folly::Expected<quic::QuicSocket::FlowControlState,
                                   quic::LocalErrorCode> {
              if (isReceivingStream(id)) {
                return folly::makeUnexpected(LocalErrorCode::INVALID_OPERATION);
              }
              checkNotReadOnlyStream(id);
              auto& stream = streams_[id];
              if (stream.writeState == CLOSED) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              }
              flowControlAccess_.emplace(id);
              return QuicSocket::FlowControlState(
                  {stream.flowControlWindow,
                   stream.nextWriteOffset + stream.flowControlWindow,
                   0,
                   0});
            }));
    EXPECT_CALL(*sock_, getMaxWritableOnStream(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id)
                -> folly::Expected<uint64_t, quic::LocalErrorCode> {
              auto streamFCW = sock_->getStreamFlowControl(id);
              if (!streamFCW) {
                return folly::makeUnexpected(streamFCW.error());
              }
              auto connFCW = sock_->getConnectionFlowControl();
              if (!connFCW) {
                return folly::makeUnexpected(connFCW.error());
              }

              auto fcAvailable = std::min(streamFCW->sendWindowAvailable,
                                          connFCW->sendWindowAvailable);
              return std::min(fcAvailable, bufferAvailable_);
            }));

    EXPECT_CALL(*sock_, setConnectionFlowControlWindow(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](uint64_t windowSize)
                -> folly::Expected<folly::Unit, quic::LocalErrorCode> {
              setConnectionFlowControlWindow(windowSize);
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, setStreamFlowControlWindow(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id, uint64_t windowSize)
                -> folly::Expected<folly::Unit, quic::LocalErrorCode> {
              checkNotReadOnlyStream(id);
              setStreamFlowControlWindow(id, windowSize);
              sock_->connCb_->onFlowControlUpdate(id);
              return folly::unit;
            }));

    using ReadCBResult = folly::Expected<folly::Unit, LocalErrorCode>;
    EXPECT_CALL(*sock_, setReadCallback(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](
                StreamId id,
                QuicSocket::ReadCallback* cb,
                folly::Optional<ApplicationErrorCode> error) -> ReadCBResult {
              checkNotWriteOnlyStream(id);
              auto& stream = streams_[id];
              if (cb == nullptr && stream.readCB == nullptr) {
                // matches real transport
                return folly::makeUnexpected(LocalErrorCode::INVALID_OPERATION);
              }
              stream.readCB = cb;
              if (cb == nullptr && error.hasValue()) {
                stream.error = error.value();
              }
              if (cb && stream.readState == NEW) {
                stream.readState = OPEN;
              } else if (cb && stream.readState == OPEN) {
                if (!stream.readBuf.empty()) {
                  eventBase_->runInLoop(this, true);
                }
              } else if (stream.readState == ERROR) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              }
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, pauseRead(testing::_))
        .WillRepeatedly(testing::Invoke([this](StreamId id) -> ReadCBResult {
          checkNotWriteOnlyStream(id);
          auto& stream = streams_[id];
          if (stream.readState == OPEN) {
            stream.readState = PAUSED;
            return folly::unit;
          } else {
            return folly::makeUnexpected(quic::LocalErrorCode::INTERNAL_ERROR);
          }
        }));

    EXPECT_CALL(*sock_, resumeRead(testing::_))
        .WillRepeatedly(testing::Invoke([this](StreamId id) -> ReadCBResult {
          checkNotWriteOnlyStream(id);
          auto& stream = streams_[id];
          if (stream.readState == PAUSED) {
            stream.readState = OPEN;
            if (!stream.readBuf.empty() || stream.readEOF) {
              // error is delivered immediately even if paused
              ERROR_IF(!stream.readCB,
                       "can not deliver readAvailable: read callback "
                       "is not set",
                       return folly::makeUnexpected(
                           quic::LocalErrorCode::INTERNAL_ERROR));
              stream.readCB->readAvailable(id);
            }
            return folly::unit;
          } else {
            return folly::makeUnexpected(quic::LocalErrorCode::INTERNAL_ERROR);
          }
        }));

    using PeekCBResult = folly::Expected<folly::Unit, LocalErrorCode>;
    EXPECT_CALL(*sock_, setPeekCallback(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id, QuicSocket::PeekCallback* cb) -> PeekCBResult {
              checkNotWriteOnlyStream(id);
              auto& stream = streams_[id];
              stream.peekCB = cb;
              if (cb && stream.readState == NEW) {
                stream.readState = OPEN;
              } else if (cb && stream.readState == OPEN) {
                eventBase_->runInLoop(this, true);
              } else if (stream.readState == ERROR) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              }
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, consume(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id, size_t amount)
                -> folly::Expected<folly::Unit, LocalErrorCode> {
              auto& stream = streams_[id];
              stream.readBuf.splitAtMost(amount);
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, readNaked(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id, size_t maxLen) -> MockQuicSocket::ReadResult {
              auto& stream = streams_[id];
              std::pair<folly::IOBuf*, bool> result;
              if (stream.readState == OPEN) {
                if (maxLen == 0) {
                  maxLen = std::numeric_limits<size_t>::max();
                }
                // Gather all buffers in the queue so that split won't
                // run dry
                stream.readBuf.gather(stream.readBuf.chainLength());

                result.first = stream.readBuf.splitAtMost(maxLen).release();
                result.second = stream.readBuf.empty() && stream.readEOF;
                if (result.second) {
                  stream.readState = CLOSED;
                }
              } else if (stream.readState == ERROR) {
                // If reads return a LocalErrorCode, writes are also in error
                stream.writeState = ERROR;
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              } else {
                result.second = true;
              }
              stream.readOffset += result.first->length();
              return result;
            }));

    EXPECT_CALL(*sock_, notifyPendingWriteOnStream(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](StreamId id, QuicSocket::WriteCallback* wcb)
                -> folly::Expected<folly::Unit, quic::LocalErrorCode> {
              checkNotReadOnlyStream(id);
              return notifyPendingWriteImpl(id, wcb, /* isStreamNotif */ true);
            }));

    EXPECT_CALL(*sock_, notifyPendingWriteOnConnection(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](QuicSocket::WriteCallback* wcb)
                -> folly::Expected<folly::Unit, quic::LocalErrorCode> {
              return notifyPendingWriteImpl(quic::kConnectionStreamId, wcb);
            }));

    EXPECT_CALL(*sock_, getDatagramSizeLimit())
        .WillRepeatedly(testing::Invoke(
            [this]() -> uint16_t { return getDatagramSizeLimitImpl(); }));

    EXPECT_CALL(*sock_, setDatagramCallback(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](QuicSocket::DatagramCallback* cb)
                -> folly::Expected<folly::Unit, quic::LocalErrorCode> {
              auto& connStream = streams_[kConnectionStreamId];
              if (connStream.writeState == CLOSED) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::CONNECTION_CLOSED);
              }
              datagramCB_ = cb;
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, writeDatagram(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](MockQuicSocket::SharedBuf data)
                                -> quic::MockQuicSocket::WriteResult {
              auto& connStream = streams_[kConnectionStreamId];
              if (connStream.writeState == CLOSED) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::CONNECTION_CLOSED);
              }
              if (data->computeChainDataLength() >
                  this->getDatagramSizeLimitImpl()) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INVALID_WRITE_DATA);
              }
              outDatagrams_.emplace_back(data->clone());
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, readDatagrams(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](size_t atMost) -> folly::Expected<std::vector<ReadDatagram>,
                                                     quic::LocalErrorCode> {
              auto& connStream = streams_[kConnectionStreamId];
              if (connStream.writeState == CLOSED) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::CONNECTION_CLOSED);
              }
              if (atMost == 0) {
                atMost = inDatagrams_.size();
              } else {
                atMost = std::min(atMost, inDatagrams_.size());
              }
              std::vector<ReadDatagram> retDatagrams;
              retDatagrams.reserve(atMost);
              std::transform(inDatagrams_.begin(),
                             inDatagrams_.begin() + atMost,
                             std::back_inserter(retDatagrams),
                             [](ReadDatagram& dg) { return std::move(dg); });
              inDatagrams_.erase(inDatagrams_.begin(),
                                 inDatagrams_.begin() + atMost);
              return retDatagrams;
            }));

    EXPECT_CALL(*sock_, readDatagramBufs(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](size_t atMost) -> folly::Expected<std::vector<quic::Buf>,
                                                     quic::LocalErrorCode> {
              auto& connStream = streams_[kConnectionStreamId];
              if (connStream.writeState == CLOSED) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::CONNECTION_CLOSED);
              }
              if (atMost == 0) {
                atMost = inDatagrams_.size();
              } else {
                atMost = std::min(atMost, inDatagrams_.size());
              }
              std::vector<Buf> retDatagrams;
              retDatagrams.reserve(atMost);
              std::transform(
                  inDatagrams_.begin(),
                  inDatagrams_.begin() + atMost,
                  std::back_inserter(retDatagrams),
                  [](ReadDatagram& dg) { return dg.bufQueue().move(); });
              inDatagrams_.erase(inDatagrams_.begin(),
                                 inDatagrams_.begin() + atMost);
              return retDatagrams;
            }));

    EXPECT_CALL(*sock_, setPingCallback(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](QuicSocket::PingCallback* cb)
                -> folly::Expected<folly::Unit, quic::LocalErrorCode> {
              auto& connStream = streams_[kConnectionStreamId];
              if (connStream.writeState == CLOSED) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::CONNECTION_CLOSED);
              }
              pingCB_ = cb;
              return folly::unit;
            }));

    EXPECT_CALL(*sock_,
                writeChain(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(
            testing::Invoke([this](StreamId id,
                                   MockQuicSocket::SharedBuf data,
                                   bool eof,
                                   QuicSocket::ByteEventCallback* cb)
                                -> quic::MockQuicSocket::WriteResult {
              ERROR_IF(id == kConnectionStreamId,
                       "writeChain(kConnectionStreamId) not handled",
                       return folly::makeUnexpected(
                           quic::LocalErrorCode::INTERNAL_ERROR));
              checkNotReadOnlyStream(id);
              auto& stream = streams_[id];
              auto& connState = streams_[kConnectionStreamId];
              ERROR_IF(connState.writeState == CLOSED,
                       "writeChain on CLOSED connection",
                       return folly::makeUnexpected(
                           quic::LocalErrorCode::INTERNAL_ERROR));
              // check stream.writeState == ERROR -> deliver error
              if (stream.writeState == ERROR) {
                // if writes return a LocalErrorCode, reads are also in error
                stream.readState = ERROR;
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              }
              stream.numWriteChainInvocations++;
              if (!data) {
                data = folly::IOBuf::create(0);
              } else {
                stream.unsentBuf.append(data->clone());
              }
              // clip to FCW
              uint64_t length = std::min(
                  static_cast<uint64_t>(stream.unsentBuf.chainLength()),
                  stream.flowControlWindow);
              length = std::min(length, connState.flowControlWindow);
              auto toSend = stream.unsentBuf.splitAtMost(length);
              if (localAppCb_) {
                if (sock_->isUnidirectionalStream(id)) {
                  localAppCb_->unidirectionalReadCallback(id, toSend->clone());
                } else {
                  localAppCb_->readCallback(id, toSend->clone());
                }
              }
              stream.pendingWriteBuf.append(std::move(toSend));
              setStreamFlowControlWindow(id, stream.flowControlWindow - length);
              setConnectionFlowControlWindow(connState.flowControlWindow -
                                             length);
              // handle non-zero -> 0 transition, call flowControlUpdate
              stream.nextWriteOffset += length;
              if (stream.unsentBuf.empty() && eof) {
                stream.writeEOF = true;
              } else if (eof) {
                stream.pendingWriteEOF = eof;
              }
              if (cb) {
                // The API doesn't allow for registering a last-byte tx cb here
                auto finOffset =
                    stream.nextWriteOffset + stream.unsentBuf.chainLength();
                auto type = quic::QuicSocket::ByteEvent::Type::ACK;
                VLOG(4) << "onByteEventRegistered id=" << id
                        << " offset=" << finOffset
                        << " type=" << uint64_t(type);
                cb->onByteEventRegistered({id, finOffset, type});
                stream.deliveryCallbacks.push_back({finOffset, cb});
              }
              eventBase_->runInLoop([this, deleted = deleted_] {
                if (!*deleted) {
                  flushWrites();
                }
              });
              CHECK(stream.unsentBuf.empty() || stream.flowControlWindow == 0 ||
                    connState.flowControlWindow == 0);
              return folly::unit;
            }));

    EXPECT_CALL(*sock_,
                writeBufMeta(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(
            testing::Invoke([this](StreamId id,
                                   const BufferMeta& data,
                                   bool eof,
                                   QuicSocket::ByteEventCallback* cb)
                                -> quic::MockQuicSocket::WriteResult {
              ERROR_IF(id == kConnectionStreamId,
                       "writeChain(kConnectionStreamId) not handled",
                       return folly::makeUnexpected(
                           quic::LocalErrorCode::INTERNAL_ERROR));
              checkNotReadOnlyStream(id);
              auto& stream = streams_[id];
              auto& connState = streams_[kConnectionStreamId];
              ERROR_IF(connState.writeState == CLOSED,
                       "writeChain on CLOSED connection",
                       return folly::makeUnexpected(
                           quic::LocalErrorCode::INTERNAL_ERROR));
              // check stream.writeState == ERROR -> deliver error
              if (stream.writeState == ERROR) {
                // if writes return a LocalErrorCode, reads are also in error
                stream.readState = ERROR;
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              }
              auto totalStreamLength =
                  stream.nextWriteOffset + stream.unsentBuf.chainLength();
              if (totalStreamLength == 0) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::INTERNAL_ERROR);
              }
              if (stream.unsentBufMeta.offset == 0) {
                // beginning offset = totalStreamLength
                stream.unsentBufMeta.offset = totalStreamLength;
              }
              stream.unsentBufMeta.length += data.length;
              if (eof) {
                stream.unsentBufMeta.eof = true;
              }
              // clip to FCW
              uint64_t length =
                  std::min(static_cast<uint64_t>(stream.unsentBufMeta.length),
                           stream.flowControlWindow);
              length = std::min(length, connState.flowControlWindow);
              auto toSend = stream.unsentBufMeta.split(length);
              if (localAppCb_) {
                if (sock_->isUnidirectionalStream(id)) {
                  // localAppCb_->unidirectionalReadCallback(id,
                  // toSend->clone());
                } else {
                  // localAppCb_->readCallback(id, toSend->clone());
                }
              }
              stream.pendingBufMetaLength += toSend.length;
              setStreamFlowControlWindow(id, stream.flowControlWindow - length);
              setConnectionFlowControlWindow(connState.flowControlWindow -
                                             length);
              if (stream.unsentBufMeta.length == 0 && eof) {
                stream.writeEOF = true;
              } else if (eof) {
                stream.pendingWriteEOF = eof;
              }
              if (cb) {
                // The API doesn't allow for registering a last-byte tx cb here
                auto finOffset = stream.nextWriteOffset +
                                 stream.pendingBufMetaLength +
                                 stream.unsentBufMeta.length;
                auto type = quic::QuicSocket::ByteEvent::Type::ACK;
                VLOG(4) << "onByteEventRegistered id=" << id
                        << " offset=" << finOffset
                        << " type=" << uint64_t(type);
                cb->onByteEventRegistered({id, finOffset, type});
                stream.deliveryCallbacks.push_back({finOffset, cb});
              }
              eventBase_->runInLoop([this, deleted = deleted_] {
                if (!*deleted) {
                  flushWrites();
                }
              });
              CHECK(stream.unsentBufMeta.length == 0 ||
                    stream.flowControlWindow == 0 ||
                    connState.flowControlWindow == 0);
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, closeGracefully())
        .WillRepeatedly(testing::Invoke([this]() {
          closeConnection();
          expectStreamsIdle();
        }));

    // For the purpose of testing close and closeNe=ow are identical
    EXPECT_CALL(*sock_, closeNow(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](folly::Optional<QuicError> errorCode) {
              closeImpl(errorCode);
            }));
    EXPECT_CALL(*sock_, close(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](folly::Optional<QuicError> errorCode) {
              // close does not invoke onConnectionEnd/onConnectionError
              sock_->connCb_ = nullptr;
              sock_->setupCb_ = nullptr;
              closeImpl(errorCode);
            }));
    EXPECT_CALL(*sock_, resetStream(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](quic::StreamId id, quic::ApplicationErrorCode error) {
              checkNotReadOnlyStream(id);
              auto& stream = streams_[id];
              stream.error = error;
              stream.writeState = ERROR;
              stream.unsentBuf.move();
              stream.pendingWriteBuf.move();
              stream.pendingWriteCb = nullptr;
              stream.pendingBufMetaLength = 0;
              stream.unsentBufMeta.length = 0;
              cancelDeliveryCallbacks(id, stream);
              return folly::unit;
            }));
    EXPECT_CALL(*sock_, unsetAllReadCallbacks())
        .WillRepeatedly(testing::Invoke([this]() {
          for (auto& stream : streams_) {
            if (!(sock_->isUnidirectionalStream(stream.first) &&
                  isSendingStream(stream.first))) {
              stream.second.readCB = nullptr;
            }
          }
        }));
    EXPECT_CALL(*sock_, stopSending(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](quic::StreamId id, quic::ApplicationErrorCode error) {
              checkNotWriteOnlyStream(id);
              auto& stream = streams_[id];
              stream.error = error;
              // This doesn't set readState to error, because we can
              // still receive after sending STOP_SENDING
              return folly::unit;
            }));
    EXPECT_CALL(*sock_, createBidirectionalStream(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](bool /*replaySafe*/)
                                -> folly::Expected<StreamId, LocalErrorCode> {
              if (nextBidirectionalStreamId_ >= maxBidiStreamID_) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::STREAM_LIMIT_EXCEEDED);
              }

              auto streamId = nextBidirectionalStreamId_;
              nextBidirectionalStreamId_ += 4;
              streams_[streamId].readState = OPEN;
              return streamId;
            }));
    EXPECT_CALL(*sock_, createUnidirectionalStream(testing::_))
        .WillRepeatedly(
            testing::Invoke([this](bool /*replaySafe*/)
                                -> folly::Expected<StreamId, LocalErrorCode> {
              if (nextUnidirectionalStreamId_ >= maxUniStreamID_) {
                return folly::makeUnexpected(
                    quic::LocalErrorCode::STREAM_LIMIT_EXCEEDED);
              }

              auto streamId = nextUnidirectionalStreamId_;
              nextUnidirectionalStreamId_ += 4;
              streams_[streamId];
              // caller of createUnidirectionalStream should not expect a
              // readState is Open.
              streams_[streamId].readState = CLOSED;
              return streamId;
            }));
    EXPECT_CALL(*sock_, getNumOpenableBidirectionalStreams())
        .WillRepeatedly(testing::Invoke([this] {
          return maxBidiStreamID_ >= nextBidirectionalStreamId_
                     ? (maxBidiStreamID_ - nextBidirectionalStreamId_) / 4
                     : 0;
        }));
    EXPECT_CALL(*sock_, getNumOpenableUnidirectionalStreams())
        .WillRepeatedly(testing::Invoke([this] {
          return maxUniStreamID_ >= nextUnidirectionalStreamId_
                     ? (maxUniStreamID_ - nextUnidirectionalStreamId_) / 4
                     : 0;
        }));
    EXPECT_CALL(*sock_, getStreamWriteOffset(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](
                quic::StreamId id) -> folly::Expected<size_t, LocalErrorCode> {
              checkNotReadOnlyStream(id);
              auto it = streams_.find(id);
              if (it == streams_.end()) {
                return folly::makeUnexpected(LocalErrorCode::STREAM_NOT_EXISTS);
              }
              ERROR_IF(
                  it->second.writeState == CLOSED,
                  fmt::format("getStreamWriteOffset on CLOSED streamId={}", id),
                  return folly::makeUnexpected(
                      LocalErrorCode::STREAM_NOT_EXISTS));
              // TODO: :/ sigh. BufMeta breaks this.
              return it->second.nextWriteOffset -
                     it->second.pendingWriteBuf.chainLength();
            }));
    EXPECT_CALL(*sock_, getStreamWriteBufferedBytes(testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](
                quic::StreamId id) -> folly::Expected<size_t, LocalErrorCode> {
              checkNotReadOnlyStream(id);
              auto it = streams_.find(id);
              if (it == streams_.end()) {
                return folly::makeUnexpected(LocalErrorCode::STREAM_NOT_EXISTS);
              }

              ERROR_IF(
                  it->second.writeState == CLOSED,
                  fmt::format(
                      "getStreamWriteBufferedBytes on CLOSED streamId={}", id),
                  return folly::makeUnexpected(
                      LocalErrorCode::STREAM_NOT_EXISTS));
              return it->second.pendingWriteBuf.chainLength() +
                     it->second.unsentBuf.chainLength() +
                     it->second.pendingBufMetaLength +
                     it->second.unsentBufMeta.length;
            }));
    EXPECT_CALL(*sock_,
                registerDeliveryCallback(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](quic::StreamId id,
                   uint64_t offset,
                   MockQuicSocket::ByteEventCallback* cb)
                -> folly::Expected<folly::Unit, LocalErrorCode> {
              return sock_->registerByteEventCallback(
                  quic::QuicSocket::ByteEvent::Type::ACK, id, offset, cb);
            }));
    EXPECT_CALL(*sock_, registerTxCallback(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](quic::StreamId id,
                   uint64_t offset,
                   MockQuicSocket::ByteEventCallback* cb)
                -> folly::Expected<folly::Unit, LocalErrorCode> {
              return sock_->registerByteEventCallback(
                  quic::QuicSocket::ByteEvent::Type::TX, id, offset, cb);
            }));
    EXPECT_CALL(*sock_,
                registerByteEventCallback(
                    testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [this](MockQuicSocket::ByteEvent::Type type,
                   quic::StreamId id,
                   uint64_t offset,
                   MockQuicSocket::ByteEventCallback* cb)
                -> folly::Expected<folly::Unit, LocalErrorCode> {
              auto& connState = streams_[kConnectionStreamId];
              ERROR_IF(connState.writeState == CLOSED,
                       "registerByteEventCallback on CLOSED connection",
                       return folly::makeUnexpected(
                           quic::LocalErrorCode::INTERNAL_ERROR));
              checkNotReadOnlyStream(id);
              auto it = streams_.find(id);
              if (it == streams_.end()) {
                return folly::makeUnexpected(LocalErrorCode::STREAM_NOT_EXISTS);
              }
              ERROR_IF(
                  it->second.writeState == CLOSED,
                  fmt::format("registerByteEventCallback on CLOSED streamId={}",
                              id),
                  return folly::makeUnexpected(
                      LocalErrorCode::STREAM_NOT_EXISTS));
              VLOG(4) << "onByteEventRegistered id=" << id
                      << " offset=" << offset << " type=" << uint64_t(type);
              cb->onByteEventRegistered({id, offset, type});
              if (it->second.fireByteEventAt(offset)) {
                // already available, fire the cb from the loop
                eventBase_->runInLoop(
                    [id, offset, type, cb] {
                      VLOG(4) << "onByteEvent id=" << id << " offset=" << offset
                              << " type=" << uint64_t(type);
                      cb->onByteEvent({id, offset, type});
                    },
                    /*thisIteration=*/true);
                return folly::unit;
              }

              if (type == quic::QuicSocket::ByteEvent::Type::ACK) {
                it->second.deliveryCallbacks.emplace_back(offset, cb);
              } else {
                it->second.txCallbacks.emplace_back(offset, cb);
              }
              return folly::unit;
            }));

    EXPECT_CALL(*sock_, cancelDeliveryCallbacksForStream(testing::_))
        .WillRepeatedly(testing::Invoke([this](quic::StreamId id) -> void {
          cancelDeliveryCallbacks(id, streams_[id]);
        }));
    localAddress_.setFromIpPort("0.0.0.0", folly::Random::rand32());
    peerAddress_.setFromIpPort("127.0.0.0", folly::Random::rand32());
    EXPECT_CALL(*sock_, getLocalAddress())
        .WillRepeatedly(testing::ReturnRef(localAddress_));
    EXPECT_CALL(*sock_, getPeerAddress())
        .WillRepeatedly(testing::ReturnRef(peerAddress_));
    EXPECT_CALL(*sock_, getPeerCertificate())
        .WillRepeatedly(testing::Return(mockCertificate));
  }

  uint16_t getDatagramSizeLimitImpl() {
    return quic::kDefaultUDPSendPacketLen - quic::kMaxDatagramPacketOverhead;
  }

  quic::StreamId getMaxStreamId() {
    return std::max_element(
               streams_.begin(),
               streams_.end(),
               [](const std::pair<const StreamId, StreamState>& a,
                  const std::pair<const StreamId, StreamState>& b) -> bool {
                 return a.first < b.first && b.first != kConnectionStreamId;
               })
        ->first;
  }

  void setMaxBidiStreams(uint64_t maxBidiStreams) {
    auto maxBidiStreamID = maxBidiStreams * 4 +
                           ((transportType_ == TransportEnum::SERVER) ? 2 : 0);
    if (maxBidiStreamID > maxBidiStreamID_) {
      maxBidiStreamID_ = maxBidiStreamID;
      if (sock_->connCb_) {
        auto openableStreams =
            (maxBidiStreamID_ - nextBidirectionalStreamId_) / 4;
        sock_->connCb_->onBidirectionalStreamsAvailable(openableStreams);
      }
    } else {
      maxBidiStreamID_ = maxBidiStreamID;
    }
  }

  void setMaxUniStreams(uint64_t maxUniStreams) {
    auto maxUniStreamID =
        maxUniStreams * 4 + ((transportType_ == TransportEnum::SERVER) ? 3 : 1);
    if (maxUniStreamID > maxUniStreamID_) {
      maxUniStreamID_ = maxUniStreamID;
      if (sock_->connCb_) {
        auto openableStreams =
            (maxUniStreamID_ - nextUnidirectionalStreamId_) / 4;
        sock_->connCb_->onUnidirectionalStreamsAvailable(openableStreams);
      }
    } else {
      maxUniStreamID_ = maxUniStreamID;
    }
  }

  bool isClosed() {
    return streams_[kConnectionStreamId].readState != OPEN &&
           streams_[kConnectionStreamId].writeState != OPEN;
  }

  void setLocalAppCallback(LocalAppCallback* localAppCb) {
    localAppCb_ = localAppCb;
  }

  void checkNotReadOnlyStream(quic::StreamId id) {
    CHECK(!(sock_->isUnidirectionalStream(id) && isReceivingStream(id)))
        << "API not supported on read-only unidirectional stream. streamID="
        << id;
  }

  void checkNotWriteOnlyStream(quic::StreamId id) {
    CHECK(!(sock_->isUnidirectionalStream(id) && isSendingStream(id)))
        << "API not supported on write-only unidirectional stream. streamID="
        << id;
  }

  bool isSendingStream(StreamId stream) {
    return sock_->isUnidirectionalStream(stream) &&
           ((transportType_ == TransportEnum::CLIENT &&
             sock_->isClientStream(stream)) ||
            (transportType_ == TransportEnum::SERVER &&
             sock_->isServerStream(stream)));
  }

  bool isReceivingStream(StreamId stream) {
    return sock_->isUnidirectionalStream(stream) &&
           ((transportType_ == TransportEnum::CLIENT &&
             sock_->isServerStream(stream)) ||
            (transportType_ == TransportEnum::SERVER &&
             sock_->isClientStream(stream)));
  }

  static bool isIdle(StateEnum state) {
    return state == CLOSED || state == ERROR;
  }

  bool isStreamIdle(StreamId id) {
    return isIdle(streams_[id].readState);
  }

  bool isStreamPaused(StreamId id) {
    return streams_[id].readState == PAUSED;
  }

  void deliverErrorOnAllStreams(QuicError error) {
    for (auto& it : streams_) {
      auto& stream = it.second;
      if (it.first == kConnectionStreamId) {
        deliverWriteError(it.first, stream, error.code);
        continue;
      }
      if (!isIdle(stream.readState)) {
        if (stream.readCB || stream.peekCB) {
          auto rcb = stream.readCB;
          auto pcb = stream.peekCB;
          stream.readCB = nullptr;
          stream.peekCB = nullptr;
          if (rcb) {
            rcb->readError(it.first, QuicError(error.code, error.message));
          } else {
            pcb->peekError(it.first, QuicError(error.code, error.message));
          }
        }
        stream.readState = ERROR;
      }
      if (!isIdle(stream.writeState)) {
        deliverWriteError(it.first, stream, error.code);
      }
      cancelDeliveryCallbacks(it.first, stream);
    }
  }

  void deliverConnectionError(QuicError error) {
    deliverErrorOnAllStreams(error);
    auto cb = sock_->connCb_;
    sock_->connCb_ = nullptr;
    if (cb) {
      bool noError = false;
      switch (error.code.type()) {
        case QuicErrorCode::Type::LocalErrorCode: {
          LocalErrorCode& err = *error.code.asLocalErrorCode();
          noError = err == LocalErrorCode::NO_ERROR ||
                    err == LocalErrorCode::IDLE_TIMEOUT;
          break;
        }
        case QuicErrorCode::Type::TransportErrorCode: {
          TransportErrorCode& err = *error.code.asTransportErrorCode();
          noError = err == TransportErrorCode::NO_ERROR;
          break;
        }
        case QuicErrorCode::Type::ApplicationErrorCode: {
          noError = false;
          break;
        }
      }
      if (noError) {
        cb->onConnectionEnd();
      } else {
        cb->onConnectionError(std::move(error));
      }
    }
  }

  void deliverWriteError(quic::StreamId id,
                         StreamState& stream,
                         QuicErrorCode errorCode) {
    if (stream.pendingWriteCb) {
      auto cb = stream.pendingWriteCb;
      stream.pendingWriteCb = nullptr;
      if (stream.isPendingWriteCbStreamNotif) {
        cb->onStreamWriteError(id, QuicError(errorCode));
      } else {
        cb->onConnectionWriteError(QuicError(errorCode));
      }
    }
    stream.writeState = ERROR;
  }

  void cancelDeliveryCallbacks(quic::StreamId id, StreamState& stream) {
    while (!stream.txCallbacks.empty()) {
      stream.txCallbacks.front().second->onByteEventCanceled(
          {id,
           stream.txCallbacks.front().first,
           QuicSocket::ByteEvent::Type::TX});
      stream.txCallbacks.pop_front();
    }
    while (!stream.deliveryCallbacks.empty()) {
      stream.deliveryCallbacks.front().second->onByteEventCanceled(
          {id,
           stream.deliveryCallbacks.front().first,
           QuicSocket::ByteEvent::Type::ACK});
      stream.deliveryCallbacks.pop_front();
    }
  }

  uint64_t maxConnWritable() {
    return streams_[kConnectionStreamId].flowControlWindow;
  }

  uint64_t maxStreamWritable(StreamId id) {
    return std::min(streams_[id].flowControlWindow, maxConnWritable());
  }

  folly::Expected<folly::Unit, quic::LocalErrorCode> notifyPendingWriteImpl(
      StreamId id, QuicSocket::WriteCallback* wcb, bool streamNotif = false) {
    auto& stream = streams_[id];
    if (stream.writeState == PAUSED) {
      stream.pendingWriteCb = wcb;
      stream.isPendingWriteCbStreamNotif = streamNotif;
      return folly::unit;
    } else if (stream.writeState == OPEN) {
      if (wcb == nullptr) {
        return folly::makeUnexpected(LocalErrorCode::INVALID_WRITE_CALLBACK);
      }

      stream.pendingWriteCb = wcb;
      stream.isPendingWriteCbStreamNotif = streamNotif;
      eventBase_->runInLoop(
          [this, id, &stream, deleted = deleted_] {
            if (*deleted) {
              return;
            }
            // This callback was scheduled to be delivered when the stream
            // writeState was OPEN, do not deliver the callback if the state
            // changed in the meantime
            if (stream.writeState != OPEN) {
              return;
            }
            ERROR_IF(!stream.pendingWriteCb,
                     fmt::format("write callback not set when calling "
                                 "onConnectionWriteReady for streamId={}",
                                 id),
                     return );
            auto maxStreamToWrite = maxStreamWritable(id);
            auto maxConnToWrite = maxConnWritable();
            if (!maxConnToWrite && !maxStreamToWrite) {
              return;
            }

            auto writeCb = stream.pendingWriteCb;
            stream.pendingWriteCb = nullptr;
            if (stream.isPendingWriteCbStreamNotif) {
              writeCb->onStreamWriteReady(id, maxStreamToWrite);
            } else {
              writeCb->onConnectionWriteReady(maxConnToWrite);
            }
          },
          true);
    } else {
      // closed, error
      return folly::makeUnexpected(LocalErrorCode::CONNECTION_CLOSED);
    }
    return folly::unit;
  }

  void expectStreamsIdle(bool connection = false) {
    for (auto& it : streams_) {
      if ((!it.second.isControl && it.first != kConnectionStreamId) ||
          connection) {
        EXPECT_TRUE(isIdle(it.second.readState))
            << "stream=" << it.first << " readState=" << it.second.readState;
        EXPECT_TRUE(isIdle(it.second.writeState))
            << "stream=" << it.first << " writeState=" << it.second.writeState;
      }
    }
  }

  void expectStreamWritesPaused(StreamId id) {
    EXPECT_EQ(streams_[id].writeState, PAUSED);
  }

  void expectConnWritesPaused() {
    EXPECT_EQ(streams_[quic::kConnectionStreamId].writeState, PAUSED);
  }

  ~MockQuicSocketDriver() {
    expectStreamsIdle(true);
    *deleted_ = true;
  }

  void writePendingDataAndAck(StreamState& stream, StreamId id) {
    stream.writeBuf.append(stream.pendingWriteBuf.move());
    if (stream.pendingBufMetaLength > 0) {
      stream.nextWriteOffset += stream.pendingBufMetaLength;
      stream.pendingBufMetaLength = 0;
    }
    if (stream.writeEOF) {
      stream.writeState = CLOSED;
    }

    // Deliver tx callbacks
    fireCallbacks(
        id, stream, stream.txCallbacks, QuicSocket::ByteEvent::Type::TX);

    // delay delivery callbacks 50ms
    eventBase_->runAfterDelay(
        [this, &stream, id, deleted = deleted_] {
          if (*deleted) {
            return;
          }
          fireCallbacks(id,
                        stream,
                        stream.deliveryCallbacks,
                        QuicSocket::ByteEvent::Type::ACK);
        },
        50);
  }

  void fireCallbacks(uint64_t id,
                     StreamState& stream,
                     StreamState::ByteEventList& callbacks,
                     QuicSocket::ByteEvent::Type type) {
    while (!callbacks.empty()) {
      auto cb = callbacks.front();
      if (!stream.fireByteEventAt(cb.first)) {
        break;
      }
      VLOG(4) << "onByteEvent id=" << id << " offset=" << cb.first
              << " type=" << uint64_t(type);
      cb.second->onByteEvent({id, cb.first, type});
      callbacks.pop_front();
    }
  }

  void closeConnection() {
    flushWrites();
    auto& connState = streams_[kConnectionStreamId];
    connState.readState = CLOSED;
    connState.writeState = CLOSED;
  }

  void closeImpl(folly::Optional<QuicError> errorCode) {

    closeConnection();
    if (errorCode) {
      quic::ApplicationErrorCode* err =
          errorCode->code.asApplicationErrorCode();
      if (err) {
        auto& connState = streams_[kConnectionStreamId];
        connState.error = *err;
      }
    }
    deliverConnectionError(errorCode.value_or(
        QuicError(LocalErrorCode::NO_ERROR, "Closing socket with no error")));
    sock_->connCb_ = nullptr;
    sock_->setupCb_ = nullptr;
  }
  folly::Optional<quic::ApplicationErrorCode> getConnErrorCode() {
    return streams_[kConnectionStreamId].error;
  }

  void flushWrites(StreamId id = kConnectionStreamId) {
    auto& connState = streams_[kConnectionStreamId];
    for (auto& it : streams_) {
      if (it.first == kConnectionStreamId ||
          (id != kConnectionStreamId && it.first != id)) {
        continue;
      }
      auto& stream = it.second;
      bool hasDataToWrite =
          (!stream.pendingWriteBuf.empty() || stream.pendingBufMetaLength > 0 ||
           stream.writeEOF);
      if (connState.writeState == OPEN && stream.writeState == OPEN &&
          hasDataToWrite) {
        // handle 0->non-zero transition, call flowControlUpdate
        setStreamFlowControlWindow(it.first,
                                   stream.flowControlWindow +
                                       stream.pendingWriteBuf.chainLength() +
                                       stream.pendingBufMetaLength);
        setConnectionFlowControlWindow(connState.flowControlWindow +
                                       stream.pendingWriteBuf.chainLength() +
                                       stream.pendingBufMetaLength);
        writePendingDataAndAck(stream, it.first);
      } else if (hasDataToWrite) {
        // If we are paused only write the data that we have pending and don't
        // trigger flow control updates to simulate reads from the other side
        writePendingDataAndAck(stream, it.first);
      }
    }
  }

  void addDatagramsAvailableReadEvent(
      std::chrono::milliseconds delayFromPrevious =
          std::chrono::milliseconds(0)) {
    addReadEventInternal(kConnectionStreamId,
                         nullptr,
                         false,
                         folly::none,
                         delayFromPrevious,
                         false,
                         true,
                         false);
  }

  void addPingReceivedReadEvent(std::chrono::milliseconds delayFromPrevious =
                                    std::chrono::milliseconds(0)) {
    addReadEventInternal(kConnectionStreamId,
                         nullptr,
                         false,
                         folly::none,
                         delayFromPrevious,
                         false,
                         false,
                         true);
  }

  void addPingAcknowledgedReadEvent(
      std::chrono::milliseconds delayFromPrevious =
          std::chrono::milliseconds(0)) {
    addReadEventInternal(kConnectionStreamId,
                         nullptr,
                         false,
                         folly::none,
                         delayFromPrevious,
                         false,
                         false,
                         false,
                         true);
  }

  void addReadEvent(StreamId streamId,
                    std::unique_ptr<folly::IOBuf> buf,
                    std::chrono::milliseconds delayFromPrevious =
                        std::chrono::milliseconds(0),
                    uint64_t fakePeekOffset = 0) {
    addReadEventInternal(streamId,
                         std::move(buf),
                         false,
                         folly::none,
                         delayFromPrevious,
                         false /* stopSending */,
                         false /* datagramsAvailable */,
                         false /* pingReceived */,
                         false /* pingAcknowledged */,
                         fakePeekOffset);
  }

  void addReadEvent(StreamId streamId,
                    std::unique_ptr<folly::IOBuf> buf,
                    bool eof,
                    std::chrono::milliseconds delayFromPrevious =
                        std::chrono::milliseconds(0)) {
    addReadEventInternal(
        streamId, std::move(buf), eof, folly::none, delayFromPrevious);
  }

  void addReadEOF(StreamId streamId,
                  std::chrono::milliseconds delayFromPrevious =
                      std::chrono::milliseconds(0)) {
    addReadEventInternal(
        streamId, nullptr, true, folly::none, delayFromPrevious);
  }

  void addReadError(StreamId streamId,
                    QuicErrorCode error,
                    std::chrono::milliseconds delayFromPrevious =
                        std::chrono::milliseconds(0)) {
    addReadEventInternal(streamId, nullptr, false, error, delayFromPrevious);
  }

  void addStopSending(StreamId streamId,
                      ApplicationErrorCode error,
                      std::chrono::milliseconds delayFromPrevious =
                          std::chrono::milliseconds(0)) {
    QuicErrorCode qec = error;
    addReadEventInternal(
        streamId, nullptr, false, qec, delayFromPrevious, true);
  }

  void addDatagram(std::unique_ptr<folly::IOBuf> datagram,
                   TimePoint recvTime = TimePoint(0ns)) {
    inDatagrams_.emplace_back(recvTime, std::move(datagram));
  }

  void addDatagram(ReadDatagram datagram) {
    inDatagrams_.push_back(std::move(datagram));
  }

  void setReadError(StreamId streamId) {
    streams_[streamId].readState = ERROR;
  }

  void setWriteError(StreamId streamId) {
    streams_[streamId].writeState = ERROR;
    cancelDeliveryCallbacks(streamId, streams_[streamId]);
  }

  void addOnConnectionEndEvent(uint32_t millisecondsDelay) {
    eventBase_->runAfterDelay(
        [this, deleted = deleted_] {
          if (!*deleted && sock_->connCb_) {
            deliverErrorOnAllStreams(
                {quic::LocalErrorCode::NO_ERROR, "onConnectionEnd"});
            auto& connState = streams_[kConnectionStreamId];
            connState.readState = CLOSED;
            connState.writeState = CLOSED;
            auto cb = sock_->connCb_;
            // clear or cancel all the callbacks
            sock_->connCb_ = nullptr;
            sock_->setupCb_ = nullptr;
            for (auto& it : streams_) {
              auto& stream = it.second;
              stream.readCB = nullptr;
              stream.peekCB = nullptr;
              stream.pendingWriteCb = nullptr;
            }
            if (cb) {
              cb->onConnectionEnd();
            }
          }
        },
        millisecondsDelay);
  }

  // Schedules a callback in this loop if the delay is zero,
  // otherwise sets a timeout
  void runInThisLoopOrAfterDelay(folly::Func cob, uint32_t millisecondsDelay) {
    if (millisecondsDelay == 0) {
      eventBase_->runInLoop(std::move(cob), true);
    } else {
      // runAfterDelay doesn't guarantee order if two events run after the
      // same delay.  So queue the function and only use runAfterDelay to
      // signal the event.
      events_.emplace_back(std::move(cob));
      eventBase_->runAfterDelay(
          [this] {
            ERROR_IF(events_.empty(), "no events to schedule", return );
            auto event = std::move(events_.front());
            events_.pop_front();
            event();
          },
          millisecondsDelay);
    }
  }

  struct ReadEvent {
    ReadEvent(StreamId s,
              std::unique_ptr<folly::IOBuf> b,
              bool e,
              folly::Optional<QuicErrorCode> er,
              bool ss,
              bool da = false,
              bool pr = false,
              bool pa = false,
              uint64_t fpo = 0)
        : streamId(s),
          buf(std::move(b)),
          eof(e),
          error(er),
          stopSending(ss),
          datagramsAvailable(da),
          pingReceived(pr),
          pingAcknowledged(pa),
          fakePeekOffset(fpo) {
    }

    StreamId streamId;
    std::unique_ptr<folly::IOBuf> buf;
    bool eof;
    folly::Optional<QuicErrorCode> error;
    bool stopSending;
    bool datagramsAvailable;
    bool pingReceived;
    bool pingAcknowledged;
    uint64_t fakePeekOffset;
  };

  void addReadEventInternal(StreamId streamId,
                            std::unique_ptr<folly::IOBuf> buf,
                            bool eof,
                            folly::Optional<QuicErrorCode> error,
                            std::chrono::milliseconds delayFromPrevious =
                                std::chrono::milliseconds(0),
                            bool stopSending = false,
                            bool datagramsAvailable = false,
                            bool pingReceived = false,
                            bool pingAcknowledged = false,
                            uint64_t fakePeekOffset = 0) {
    std::vector<ReadEvent> events;
    events.emplace_back(streamId,
                        std::move(buf),
                        eof,
                        error,
                        stopSending,
                        datagramsAvailable,
                        pingReceived,
                        pingAcknowledged,
                        fakePeekOffset);
    addReadEvents(std::move(events), delayFromPrevious);
  }

  void addReadEvents(std::vector<ReadEvent> events,
                     std::chrono::milliseconds delayFromPrevious =
                         std::chrono::milliseconds(0)) {
    ERROR_IF(streams_[kConnectionStreamId].readState == CLOSED,
             "adding read event on CLOSED connection",
             return );
    cumulativeDelay_ += delayFromPrevious;
    runInThisLoopOrAfterDelay(
        [events = std::move(events), this, deleted = deleted_]() mutable {
          // zero out cumulative delay
          cumulativeDelay_ = std::chrono::milliseconds(0);
          if (*deleted) {
            return;
          }
          // This read event was scheduled to run in the evb, when it was
          // scheduled the connection state was not CLOSED for reads.
          // let's make sure this still holds
          if (streams_[kConnectionStreamId].readState == CLOSED) {
            return;
          }
          for (auto& event : events) {
            auto& stream = streams_[event.streamId];
            if (event.fakePeekOffset > 0) {
              stream.fakePeekOffset = event.fakePeekOffset;
            }
            if (!event.error) {
              ERROR_IF(stream.readState == CLOSED,
                       fmt::format("scheduling event on CLOSED streamId={}",
                                   event.streamId),
                       return );
            } else {
              ERROR_IF((event.buf && !event.buf->empty()) || event.eof,
                       fmt::format("scheduling an error event with either a "
                                   "buffer or eof on streamId={}",
                                   event.streamId),
                       return );
            }
            if (event.streamId == kConnectionStreamId &&
                event.datagramsAvailable && !event.error && datagramCB_) {
              datagramCB_->onDatagramsAvailable();
              continue;
            }
            if (event.streamId == kConnectionStreamId && event.pingReceived &&
                !event.error && pingCB_) {
              pingCB_->onPing();
              continue;
            }
            if (event.streamId == kConnectionStreamId &&
                event.pingAcknowledged && !event.error && pingCB_) {
              pingCB_->pingAcknowledged();
              continue;
            }
            auto bufLen = event.buf ? event.buf->computeChainDataLength() : 0;
            stream.readBufOffset += bufLen;
            stream.readBuf.append(std::move(event.buf));
            stream.readEOF |= ((event.eof) ? 1 : 0);
            if (stream.readState == NEW) {
              stream.readState = OPEN;
              if (sock_->connCb_) {
                if (sock_->isUnidirectionalStream(event.streamId)) {
                  if (isPeerStream(event.streamId)) {
                    stream.writeState = CLOSED;
                    sock_->connCb_->onNewUnidirectionalStream(event.streamId);
                  } else {
                    CHECK(event.error) << "Non-error on self-uni stream";
                  }
                } else {
                  sock_->connCb_->onNewBidirectionalStream(event.streamId);
                }
              }
            }
            if (event.error && event.stopSending) {
              if (sock_->connCb_) {
                quic::ApplicationErrorCode* err =
                    event.error->asApplicationErrorCode();
                if (err) {
                  sock_->connCb_->onStopSending(event.streamId, *err);
                }
              }
              return;
            }
            if (stream.peekCB) {
              if (event.error) {
                if (!stream.readCB) {
                  stream.peekCB->peekError(event.streamId,
                                           QuicError(*event.error));
                  stream.readState = ERROR;
                }
              } else if (stream.readState != PAUSED && stream.readBuf.front()) {
                std::deque<StreamBuffer> fakeReadBuffer;
                stream.readBuf.gather(stream.readBuf.chainLength());
                auto copyBuf = stream.readBuf.front()->clone();
                fakeReadBuffer.emplace_back(std::move(copyBuf),
                                            stream.fakePeekOffset
                                                ? stream.fakePeekOffset
                                                : stream.readOffset,
                                            stream.readEOF);
                stream.peekCB->onDataAvailable(
                    event.streamId,
                    folly::Range<PeekIterator>(fakeReadBuffer.cbegin(),
                                               fakeReadBuffer.size()));
              }
            }
            if (stream.readCB) {
              if (event.error) {
                stream.readCB->readError(event.streamId,
                                         QuicError(*event.error));
                stream.readState = ERROR;
              } else if (stream.readState != PAUSED) {
                stream.readCB->readAvailable(event.streamId);
                eventBase_->runInLoop(this);
              } // else if PAUSED, no-op
            }
          }
        },
        cumulativeDelay_.count());
  }

  bool isPeerStream(quic::StreamId id) {
    return (
        (transportType_ == TransportEnum::SERVER &&
         sock_->isClientStream(id)) ||
        (transportType_ == TransportEnum::CLIENT && sock_->isServerStream(id)));
  }

  enum class PauseResumeResult { NONE = 0, PAUSED = 1, RESUMED = 2 };
  PauseResumeResult pauseOrResumeWrites(StreamState& stream,
                                        quic::StreamId streamId) {
    if (stream.writeState == OPEN && stream.flowControlWindow == 0) {
      pauseWrites(streamId);
      return PauseResumeResult::PAUSED;
    } else if (stream.writeState == PAUSED && stream.flowControlWindow > 0) {
      resumeWrites(streamId);
      return PauseResumeResult::RESUMED;
    }
    return PauseResumeResult::NONE;
  }

  void setConnectionFlowControlWindow(uint64_t windowSize) {
    auto& connStream = streams_[kConnectionStreamId];
    ERROR_IF(connStream.writeState == CLOSED,
             "setConnectionFlowControlWindow on CLOSED connection",
             return );
    connStream.flowControlWindow = windowSize;
    if (pauseOrResumeWrites(connStream, kConnectionStreamId) ==
        PauseResumeResult::RESUMED) {
      // Connection Flow control became unblocked, resume any streams that were
      // blocked on connection flow control
      for (auto& stream : streams_) {
        if (stream.first == kConnectionStreamId) {
          continue;
        }
        if ((!stream.second.unsentBuf.empty() ||
             stream.second.unsentBufMeta.length > 0) &&
            stream.second.flowControlWindow > 0) {
          resumeWrites(stream.first, /*connFCEvent=*/true);
        }
      }
    }
  }

  void setStreamFlowControlWindow(StreamId streamId, uint64_t windowSize) {
    auto& stream = streams_[streamId];

    ERROR_IF(stream.writeState == CLOSED,
             "setStreamFlowControlWindow on CLOSED connection",
             return );
    stream.flowControlWindow = windowSize;
    pauseOrResumeWrites(stream, streamId);
  }

  void pauseWrites(StreamId streamId) {
    auto& stream = streams_[streamId];
    ERROR_IF(stream.writeState != OPEN,
             fmt::format("pauseWrites on not OPEN streamId={}", streamId),
             return );
    stream.writeState = PAUSED;
  }

  // This is to model the fact that the transport may close a stream without
  // giving a readError callback
  void forceStreamClose(StreamId streamId) {
    auto& stream = streams_[streamId];
    stream.readState = CLOSED;
    stream.writeState = CLOSED;
    cancelDeliveryCallbacks(streamId, stream);
  }

  void resumeWrites(StreamId streamId, bool connFCEvent = false) {
    auto& stream = streams_[streamId];
    ERROR_IF(stream.writeState != PAUSED && !connFCEvent,
             fmt::format("resumeWrites on not PAUSED streamId={}", streamId),
             return );
    stream.writeState = OPEN;
    // first flush any buffered writes
    flushWrites(streamId);
    // now check onConnectionWriteReady/onStreamWriteReady call is warranted.
    uint64_t maxWritableOnStream = maxStreamWritable(streamId);
    uint64_t maxWritableOnConn = maxConnWritable();
    bool shouldResume = stream.writeState == OPEN && stream.pendingWriteCb &&
                        (maxWritableOnConn > 0 || maxWritableOnStream > 0);

    if (shouldResume) {
      uint64_t window = stream.isPendingWriteCbStreamNotif ? maxWritableOnStream
                                                           : maxWritableOnConn;

      eventBase_->runInLoop(
          [wcb = stream.pendingWriteCb,
           deleted = deleted_,
           streamId,
           window,
           streamNotif = stream.isPendingWriteCbStreamNotif] {
            if (!*deleted) {
              if (streamNotif) {
                wcb->onStreamWriteReady(streamId, window);
              } else {
                wcb->onConnectionWriteReady(window);
              }
            }
          },
          true);
    }
    if (streamId != quic::kConnectionStreamId) {
      sock_->connCb_->onFlowControlUpdate(streamId);
    }
    stream.pendingWriteCb = nullptr;
    if (!stream.unsentBuf.empty()) {
      // re-invoke write chain with the pending data
      sock_->writeChain(
          streamId, stream.unsentBuf.move(), stream.pendingWriteEOF, nullptr);
    }
    if (stream.unsentBufMeta.length > 0) {
      sock_->writeBufMeta(streamId,
                          quic::BufferMeta(stream.unsentBufMeta.length),
                          stream.unsentBufMeta.eof,
                          nullptr);
    }
  }

  std::shared_ptr<MockQuicSocket> getSocket() {
    return sock_;
  }

  void runLoopCallback() noexcept override {
    bool reschedule = false;
    for (auto& it : streams_) {
      auto streamID = it.first;
      auto& streamState = it.second;
      auto hasCB = streamState.readCB || streamState.peekCB;
      auto hasDataOrEOF = !streamState.readBuf.empty() || streamState.readEOF;
      if (streamID != kConnectionStreamId && hasCB &&
          streamState.readState == OPEN && hasDataOrEOF) {
        if (streamState.peekCB) {
          std::deque<StreamBuffer> fakeReadBuffer;
          std::unique_ptr<folly::IOBuf> copyBuf;
          std::size_t copyBufLen = 0;
          if (streamState.readBuf.chainLength() > 0) {
            copyBuf = streamState.readBuf.front()->clone();
            copyBufLen = copyBuf->computeChainDataLength();
          }
          VLOG(6) << "peek onDataAvailable id=" << it.first
                  << " len=" << copyBufLen
                  << " offset=" << streamState.readOffset;
          ERROR_IF(streamState.readBufOffset < copyBufLen,
                   fmt::format("readOffset({}) is lower than current read "
                               "buffer offset({}) for streamId={}",
                               streamState.readBufOffset,
                               copyBufLen,
                               it.first),
                   continue);
          fakeReadBuffer.emplace_back(std::move(copyBuf),
                                      streamState.fakePeekOffset
                                          ? streamState.fakePeekOffset
                                          : streamState.readOffset,
                                      streamState.readEOF);
          streamState.peekCB->onDataAvailable(
              it.first,
              folly::Range<PeekIterator>(fakeReadBuffer.cbegin(),
                                         fakeReadBuffer.size()));
        }
        if (streamState.readCB) {
          streamState.readCB->readAvailable(it.first);
          reschedule = true;
        }
      }
    }
    if (reschedule) {
      eventBase_->runInLoop(this);
    }
  }

  void setStrictErrorCheck(bool strict) {
    strictErrorCheck_ = strict;
  }

  uint64_t getNumWriteChainInvocations(StreamId id) {
    return streams_[id].numWriteChainInvocations;
  }

  bool strictErrorCheck_{true};
  folly::EventBase* eventBase_;
  TransportSettings transportSettings_;
  uint64_t bufferAvailable_{std::numeric_limits<uint64_t>::max()};
  // keeping this ordered for better debugging
  StreamStateMap streams_;
  std::list<folly::Func> events_;
  TransportEnum transportType_;
  std::shared_ptr<MockQuicSocket> sock_;
  std::chrono::milliseconds cumulativeDelay_{std::chrono::milliseconds(0)};
  bool sockGood_{true};
  std::set<StreamId> flowControlAccess_;
  uint64_t nextBidirectionalStreamId_;
  uint64_t nextUnidirectionalStreamId_;
  uint64_t maxBidiStreamID_{400};
  uint64_t maxUniStreamID_{0};
  std::shared_ptr<bool> deleted_{new bool(false)};
  LocalAppCallback* localAppCb_{nullptr};
  std::string alpn_;
  QuicSocket::DatagramCallback* datagramCB_{nullptr};
  std::vector<quic::BufQueue> outDatagrams_{};
  std::vector<ReadDatagram> inDatagrams_{};
  QuicSocket::PingCallback* pingCB_{nullptr};
  folly::SocketAddress localAddress_;
  folly::SocketAddress peerAddress_;
  std::shared_ptr<proxygen::MockAsyncTransportCertificate> mockCertificate{
      std::make_shared<proxygen::MockAsyncTransportCertificate>()};
}; // namespace quic

} // namespace quic
