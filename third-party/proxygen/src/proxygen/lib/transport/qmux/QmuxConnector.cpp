/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxConnector.h>

#include <cstdint>
#include <fmt/core.h>
#include <folly/coro/Timeout.h>
#include <folly/futures/ThreadWheelTimekeeper.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/coro/Transport.h>
#include <folly/logging/xlog.h>
#include <quic/folly_utils/Utils.h>
#include <stdexcept>

namespace {
using namespace proxygen;
using namespace proxygen::qmux;
using folly::coro::co_error;

std::string_view toString(QmuxErrorCode code) {
  switch (code) {
    case QmuxErrorCode::PARSE_UNDERFLOW:
      return "PARSE_UNDERFLOW";
    case QmuxErrorCode::PARSE_ERROR:
      return "PARSE_ERROR";
    case QmuxErrorCode::PROTOCOL_VIOLATION:
      return "PROTOCOL_VIOLATION";
    case QmuxErrorCode::FRAME_ENCODING_ERROR:
      return "FRAME_ENCODING_ERROR";
    case QmuxErrorCode::TRANSPORT_PARAMETER_ERROR:
      return "TRANSPORT_PARAMETER_ERROR";
  }
  return "UNKNOWN";
}

enum class PeelStatus : std::uint8_t { NeedMore, GotParams, Error };

struct PeelOutcome {
  PeelStatus status{PeelStatus::NeedMore};
  QxTransportParams params{};
  std::string errorMsg;
};

// Parses the first record at the front of `buf`, expecting its first frame
// to be QX_TRANSPORT_PARAMETERS. On success:
//   - Removes the size header and TP frame from `buf`.
//   - If the record contained additional frames after the TP frame, rewraps
//     just those bytes in a fresh `writeRecord(...)` envelope and pushes
//     that rewrapped record back onto the front of `buf` ahead of any
//     subsequent records the same read returned.
// The caller can then hand `buf.move()` to QmuxSession as initialIngress;
// the session's codec parses it as a normal record stream.
PeelOutcome peelTransportParams(folly::IOBufQueue& buf) {
  PeelOutcome out;
  if (buf.empty()) {
    return out;
  }
  folly::io::Cursor cursor(buf.front());
  auto available = buf.chainLength();

  auto sizeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  if (!sizeOpt) {
    return out;
  }
  uint64_t recordSize = sizeOpt->first;
  size_t sizeFieldLen = sizeOpt->second;

  if (recordSize > kDefaultMaxRecordSize) {
    out.status = PeelStatus::Error;
    out.errorMsg = "QMUX record exceeds max size";
    return out;
  }
  if (available - sizeFieldLen < recordSize) {
    return out; // need more bytes
  }

  size_t recordRemaining = recordSize;
  auto maybeFrameType =
      quic::follyutils::decodeQuicInteger(cursor, recordRemaining);
  if (!maybeFrameType) {
    out.status = PeelStatus::Error;
    out.errorMsg = "malformed first frame type in QMUX record";
    return out;
  }
  if (maybeFrameType->first != qval(QmuxFrameType::QX_TRANSPORT_PARAMS)) {
    out.status = PeelStatus::Error;
    out.errorMsg = "first frame must be QX_TRANSPORT_PARAMETERS";
    return out;
  }
  recordRemaining -= maybeFrameType->second;

  auto maybeTpPayloadLen =
      quic::follyutils::decodeQuicInteger(cursor, recordRemaining);
  if (!maybeTpPayloadLen) {
    out.status = PeelStatus::Error;
    out.errorMsg = "malformed QX_TRANSPORT_PARAMETERS frame header";
    return out;
  }
  recordRemaining -= maybeTpPayloadLen->second;
  size_t payloadLen = maybeTpPayloadLen->first;
  if (payloadLen > recordRemaining) {
    out.status = PeelStatus::Error;
    out.errorMsg = "QX_TRANSPORT_PARAMETERS payload exceeds record";
    return out;
  }

  auto parsed = parseTransportParams(cursor, payloadLen);
  if (!parsed) {
    out.status = PeelStatus::Error;
    out.errorMsg =
        fmt::format("failed to parse QX_TRANSPORT_PARAMETERS payload: {}",
                    toString(parsed.error()));
    return out;
  }
  recordRemaining -= payloadLen;

  // Trim the consumed prefix (size header + TP frame). Anything after this
  // belongs either to the same record (trailing frames) or to subsequent
  // records that arrived in the same read.
  buf.trimStart(sizeFieldLen + (recordSize - recordRemaining));

  if (recordRemaining > 0) {
    // The peer pipelined non-TP frames into the TP record. Rewrap them as
    // their own record so the session's codec can parse them normally.
    auto sameRecordRemainder = buf.split(recordRemaining);
    folly::IOBufQueue rewrap{folly::IOBufQueue::cacheChainLength()};
    writeRecord(rewrap, std::move(sameRecordRemainder));

    auto subsequentRecords = buf.move();
    buf.append(rewrap.move());
    if (subsequentRecords) {
      buf.append(std::move(subsequentRecords));
    }
  }

  out.status = PeelStatus::GotParams;
  out.params = std::move(*parsed);
  return out;
}

folly::coro::Task<QxTransportParams> readPeerTransportParams(
    folly::coro::TransportIf& transport,
    folly::IOBufQueue& ingressBuf,
    std::chrono::milliseconds timeout) {
  while (true) {
    auto readRes = co_await folly::coro::co_awaitTry(
        transport.read(ingressBuf,
                       /*minReadSize=*/1,
                       /*newAllocationSize=*/4096,
                       timeout));
    if (readRes.hasException()) {
      co_yield co_error(readRes.exception());
    }
    const bool eof = (*readRes == 0);

    auto outcome = peelTransportParams(ingressBuf);
    if (outcome.status == PeelStatus::Error) {
      co_yield co_error(std::runtime_error(outcome.errorMsg));
    }
    if (outcome.status == PeelStatus::GotParams) {
      co_return std::move(outcome.params);
    }
    if (eof) {
      co_yield co_error(
          std::runtime_error("peer EOF before QX_TRANSPORT_PARAMETERS"));
    }
  }
}

} // namespace

namespace proxygen::qmux {

WtStreamManager::WtConfig makeWtConfig(
    const QxTransportParams& selfParams,
    const QxTransportParams& peerParams) noexcept {
  return WtStreamManager::WtConfig{
      .selfMaxStreamsBidi = selfParams.initialMaxStreamsBidi,
      .selfMaxStreamsUni = selfParams.initialMaxStreamsUni,
      .selfMaxConnData = selfParams.initialMaxData,
      .selfMaxStreamDataBidi = selfParams.initialMaxStreamDataBidiLocal,
      .selfMaxStreamDataUni = selfParams.initialMaxStreamDataUni,
      .peerMaxStreamsBidi = peerParams.initialMaxStreamsBidi,
      .peerMaxStreamsUni = peerParams.initialMaxStreamsUni,
      .peerMaxConnData = peerParams.initialMaxData,
      .peerMaxStreamDataBidi = peerParams.initialMaxStreamDataBidiRemote,
      .peerMaxStreamDataUni = peerParams.initialMaxStreamDataUni};
}

folly::coro::Task<QmuxSession::Ptr> QmuxConnector::connect(
    folly::EventBase* evb,
    WtDir dir,
    QxTransportParams selfParams,
    std::unique_ptr<folly::coro::TransportIf> transport,
    std::chrono::milliseconds timeout,
    QmuxSession::Config sessionConfig) {
  XLOG(DBG4) << "QmuxConnector::connect dir=" << static_cast<int>(dir)
             << " timeout=" << timeout.count() << "ms";

  // 1) Send our QX_TRANSPORT_PARAMETERS, wrapped as a QMux record.
  folly::IOBufQueue framesBuf{folly::IOBufQueue::cacheChainLength()};
  writeTransportParams(framesBuf, selfParams);
  folly::IOBufQueue recordBuf{folly::IOBufQueue::cacheChainLength()};
  writeRecord(recordBuf, framesBuf.move());

  auto writeRes =
      co_await folly::coro::co_awaitTry(transport->write(recordBuf, timeout));
  if (writeRes.hasException()) {
    XLOG(ERR) << "QmuxConnector: TP write failed: " << writeRes.exception();
    co_yield co_error(writeRes.exception());
  }

  // 2) Read until peer's QX_TRANSPORT_PARAMETERS is parsed, bounded by
  //    timeout. peelTransportParams trims the TP frame and rewraps any
  //    same-record trailing bytes so they parse cleanly downstream.
  folly::IOBufQueue ingressBuf{folly::IOBufQueue::cacheChainLength()};
  folly::EventBaseThreadTimekeeper tk{*evb};

  auto peerParams = co_await folly::coro::co_awaitTry(folly::coro::timeout(
      readPeerTransportParams(*transport, ingressBuf, timeout), timeout, &tk));
  if (peerParams.hasException()) {
    XLOG(ERR) << "QmuxConnector: TP read failed: " << peerParams.exception();
    co_yield co_error(peerParams.exception());
  }

  // 3) Construct the session, forwarding any post-TP bytes as initialIngress.
  auto wtConfig = makeWtConfig(selfParams, *peerParams);
  auto initialIngress = ingressBuf.empty() ? nullptr : ingressBuf.move();
  const uint64_t peerMaxRecordSize = peerParams->maxRecordSize;

  co_return std::make_shared<QmuxSession>(evb,
                                          dir,
                                          std::move(selfParams),
                                          std::move(transport),
                                          std::move(wtConfig),
                                          peerMaxRecordSize,
                                          std::move(initialIngress),
                                          sessionConfig);
}

} // namespace proxygen::qmux
