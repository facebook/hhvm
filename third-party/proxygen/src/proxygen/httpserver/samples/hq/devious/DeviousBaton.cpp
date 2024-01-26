/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "DeviousBaton.h"

#include <quic/codec/QuicInteger.h>

using namespace proxygen;

namespace {
std::unique_ptr<folly::IOBuf> makeBatonMessage(uint64_t padLen, uint8_t baton) {
  auto buf = folly::IOBuf::create(padLen + 9);
  folly::io::Appender cursor(buf.get(), 1);
  quic::encodeQuicInteger(padLen, [&](auto val) { cursor.writeBE(val); });
  memset(buf->writableTail(), 'a', padLen);
  buf->append(padLen);
  buf->writableTail()[0] = baton;
  buf->append(1);
  return buf;
}

constexpr uint64_t kStreamPadLen = 2000;
constexpr uint64_t kDatagramPadLen = 1000;
constexpr uint64_t kMaxParallelBatons = 10;
} // namespace

namespace devious {

folly::Expected<folly::Unit, uint16_t> DeviousBaton::onRequest(
    const HTTPMessage& request) {
  if (request.getMethod() != HTTPMethod::CONNECT) {
    LOG(ERROR) << "Invalid method=" << request.getMethodString();
    return folly::makeUnexpected(uint16_t(400));
  }
  if (request.getPathAsStringPiece() != "/webtransport/devious-baton") {
    LOG(ERROR) << "Invalid path=" << request.getPathAsStringPiece();
    return folly::makeUnexpected(uint16_t(404));
  }
  try {
    auto queryParams = request.getQueryParams();
    uint64_t count = 1;
    for (auto it : queryParams) {
      if (it.first == "version") {
        if (folly::to<uint64_t>(it.second) != 0) {
          throw std::runtime_error("Unsupported version");
        }
      }
      if (it.first == "count") {
        count = folly::to<uint64_t>(it.second);
        if (count > kMaxParallelBatons) {
          throw std::runtime_error("Exceed max parallel batons");
        }
      }
      if (it.first == "baton") {
        batons_.push_back(folly::to<uint8_t>(it.second));
        if (batons_.back() == 0) {
          throw std::runtime_error("Invalid starting baton = 0");
        }
      }
    }
    for (auto i = batons_.size(); i < count; i++) {
      batons_.push_back(255 - i);
    }
    activeBatons_ = count;
    return folly::unit;
  } catch (const std::exception& ex) {
    LOG(ERROR) << "Invalid query parameters: " << ex.what();
    return folly::makeUnexpected(uint16_t(404));
  }
  return folly::makeUnexpected(uint16_t(500));
}

void DeviousBaton::start() {
  for (auto baton : batons_) {
    auto handle = wt_->createUniStream();
    if (!handle) {
      wt_->closeSession(uint32_t(BatonSessionError::DA_YAMN));
    }
    auto id = handle.value()->getID();
    wt_->writeStreamData(
        id, makeBatonMessage(kStreamPadLen, baton), /*fin=*/true);
  }
}

HTTPMessage DeviousBaton::makeRequest(uint64_t version,
                                      uint64_t count,
                                      std::vector<uint8_t> batons) {
  HTTPMessage request;
  request.setMethod(HTTPMethod::CONNECT);
  request.setHTTPVersion(1, 1);
  request.setUpgradeProtocol("webtransport");
  request.setURL("/webtransport/devious-baton");
  request.setQueryParam("version", folly::to<std::string>(version));
  request.setQueryParam("count", folly::to<std::string>(count));
  for (auto baton : batons) {
    request.setQueryParam("baton", folly::to<std::string>(baton));
  }
  activeBatons_ = count;
  return request;
}

folly::Expected<folly::Unit, BatonSessionError>
DeviousBaton::onBatonMessageData(BatonMessageState& state,
                                 std::unique_ptr<folly::IOBuf> data,
                                 bool fin) {
  state.bufQueue.append(std::move(data));
  folly::io::Cursor cursor(state.bufQueue.front());
  uint64_t consumed = 0;
  bool underflow = false;
  switch (state.state) {
    case BatonMessageState::PAD_LEN: {
      auto padLen = quic::decodeQuicInteger(cursor);
      if (!padLen) {
        underflow = true;
        break;
      }
      consumed += padLen->second;
      state.paddingRemaining = padLen->first;
      state.state = BatonMessageState::PAD;
      [[fallthrough]];
    }
    case BatonMessageState::PAD: {
      auto skipped = cursor.skipAtMost(state.paddingRemaining);
      state.paddingRemaining -= skipped;
      consumed += skipped;
      if (state.paddingRemaining > 0) {
        underflow = true;
        break;
      }
      state.state = BatonMessageState::BATON;
      [[fallthrough]];
    }
    case BatonMessageState::BATON: {
      if (cursor.isAtEnd()) {
        underflow = true;
        break;
      }
      state.baton = cursor.read<uint8_t>();
      LOG(INFO) << "Parsed baton=" << uint64_t(state.baton);
      consumed += 1;
      state.state = BatonMessageState::DONE;
      [[fallthrough]];
    }
    case BatonMessageState::DONE:
      if (!state.bufQueue.empty() && !cursor.isAtEnd()) {
        return folly::makeUnexpected(BatonSessionError::BRUH);
      }
  };
  if (underflow && fin) {
    return folly::makeUnexpected(BatonSessionError::BRUH);
  }
  state.bufQueue.trimStartAtMost(consumed);
  return folly::unit;
}

void DeviousBaton::onStreamData(uint64_t streamId,
                                BatonMessageState& state,
                                std::unique_ptr<folly::IOBuf> data,
                                bool fin) {
  if (state.state == BatonMessageState::DONE) {
    // can only be a FIN
    if (data && data->computeChainDataLength() > 0) {
      closeSession(100);
    }
    return;
  }
  auto res = onBatonMessageData(state, std::move(data), fin);
  if (res.hasError()) {
    closeSession(uint32_t(res.error()));
    return;
  }
  if (state.state == BatonMessageState::DONE) {
    MessageSource arrivedOn;
    if (streamId & 0x2) {
      arrivedOn = UNI;
    } else if (bool(streamId & 0x01) == (mode_ == Mode::SERVER)) {
      arrivedOn = SELF_BIDI;
    } else {
      arrivedOn = PEER_BIDI;
    }
    auto who = onBatonMessage(streamId, arrivedOn, state.baton);
    if (who.hasError()) {
      closeSession(uint32_t(who.error()));
      return;
    }
    onBatonFinished(*who, /*reset=*/false);
  }
}

folly::Expected<DeviousBaton::WhoFinished, BatonSessionError>
DeviousBaton::onBatonMessage(uint64_t inStreamId,
                             MessageSource arrivedOn,
                             uint8_t baton) {
  if (baton % 7 == ((mode_ == Mode::SERVER) ? 0 : 1)) {
    LOG(INFO) << "Sending datagram on baton=" << uint64_t(baton);
    wt_->sendDatagram(makeBatonMessage(kDatagramPadLen, baton));
  }
  if (baton == 0) {
    return PEER;
  }
  uint64_t outStreamId = 0;
  switch (arrivedOn) {
    case UNI: {
      auto res = wt_->createBidiStream();
      if (!res) {
        return folly::makeUnexpected(BatonSessionError::DA_YAMN);
      }
      outStreamId = res->writeHandle->getID();
      startReadFn_(res->readHandle);
      break;
    }
    case PEER_BIDI:
      outStreamId = inStreamId;
      break;
    case SELF_BIDI: {
      auto res = wt_->createUniStream();
      if (!res) {
        return folly::makeUnexpected(BatonSessionError::DA_YAMN);
      }
      outStreamId = res.value()->getID();
      break;
    }
  }
  wt_->writeStreamData(
      outStreamId, makeBatonMessage(kStreamPadLen, baton + 1), /*fin=*/true);
  if (baton + 1 == 0) {
    return WhoFinished::SELF;
  }
  return WhoFinished::NO_ONE;
}

void DeviousBaton::onBatonFinished(WhoFinished who, bool reset) {
  if (who == WhoFinished::NO_ONE) {
    return;
  }
  if (activeBatons_ == 0) {
    closeSession(uint32_t(BatonSessionError::BRUH));
    return;
  }
  activeBatons_--;
  if (reset) {
    resetBatons_++;
  } else {
    finishedBatons_++;
  }
  if (activeBatons_ == 0 && who == WhoFinished::PEER) {
    if (finishedBatons_ > 0) {
      closeSession(folly::none);
    } else {
      closeSession(uint32_t(BatonSessionError::GAME_OVER));
    }
  }
}

} // namespace devious
