/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>

#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionSimulator.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/CompressionUtils.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/HPACKScheme.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/QMINScheme.h>
#include <proxygen/lib/http/codec/compress/experimental/simulator/QPACKScheme.h>

#include <proxygen/lib/http/codec/compress/test/HTTPArchive.h>
#include <proxygen/lib/utils/TestUtils.h>
#include <proxygen/lib/utils/Time.h>

using namespace std;
using namespace folly;

namespace {

// This needs to be synchronized with HPACKEncoder::kAutoFlushThreshold.
const size_t kMTU = 1400;

const std::string kTestDir = getContainingDirectory(XLOG_FILENAME).str();

} // namespace

namespace proxygen { namespace compress {

bool CompressionSimulator::readInputFromFileAndSchedule(
    const string& filename) {
  unique_ptr<HTTPArchive> har;
  try {
    har = HTTPArchive::fromFile(kTestDir + filename);
  } catch (const std::exception& ex) {
    LOG(ERROR) << folly::exceptionStr(ex);
  }
  if (!har || har->requests.size() == 0) {
    return false;
  }
  // Sort by start time (har ordered by finish time?)
  std::sort(har->requests.begin(),
            har->requests.end(),
            [](const HTTPMessage& a, const HTTPMessage& b) {
              return a.getStartTime() < b.getStartTime();
            });
  TimePoint last = har->requests[0].getStartTime();
  std::chrono::milliseconds cumulativeDelay(0);
  uint16_t index = 0;
  for (HTTPMessage& msg : har->requests) {
    auto delayFromPrevious = millisecondsBetween(msg.getStartTime(), last);
    // If there was a quiescent gap in the HAR of at least some value, shrink
    // it so the test doesn't last forever
    if (delayFromPrevious > std::chrono::milliseconds(1000)) {
      delayFromPrevious = std::chrono::milliseconds(1000);
    }
    last = msg.getStartTime();
    cumulativeDelay += delayFromPrevious;
    setupRequest(index++, std::move(msg), cumulativeDelay);
  }
  for (auto& kv : domains_) {
    flushRequests(kv.second.get());
  }
  return true;
}

void CompressionSimulator::run() {
#ifndef HAVE_REAL_QMIN
  if (params_.type == SchemeType::QMIN) {
    LOG(INFO) << "QMIN not available";
    return;
  }
#endif

  LOG(INFO) << "Starting run";
  eventBase_.loop();
  uint32_t holBlockCount = 0;
  for (auto& scheme : domains_) {
    holBlockCount += scheme.second->getHolBlockCount();
  }
  LOG(INFO) << "Complete"
            << "\nStats:"
               "\nSeed: "
            << params_.seed << "\nBlocks sent: " << requests_.size()
            << "\nAllowed OOO: " << stats_.allowedOOO
            << "\nPackets: " << stats_.packets
            << "\nPacket Losses: " << stats_.packetLosses
            << "\nHOL Block Count: " << holBlockCount
            << "\nHOL Delay (ms): " << stats_.holDelay.count()
            << "\nMax Queue Buffer Bytes: " << stats_.maxQueueBufferBytes
            << "\nUncompressed Bytes: " << stats_.uncompressed
            << "\nCompressed Bytes: " << stats_.compressed
            << "\nCompression Ratio: "
            << int(100 - double(100 * stats_.compressed) / stats_.uncompressed);
}

void CompressionSimulator::flushRequests(CompressionScheme* scheme) {
  VLOG(5) << "schedule encode for " << scheme->packetIndices.size()
          << " blocks at " << scheme->prev.count();
  // Flush previous train
  scheduleEvent(
      [this, scheme, indices = std::move(scheme->packetIndices)]() mutable {
        bool newPacket = true;
        while (!indices.empty()) {
          int16_t index = indices.front();
          indices.pop_front();
          auto schemeIndex = scheme->index;
          auto encodeRes = encode(scheme, newPacket, index);
          FrameFlags flags = encodeRes.first;
          bool allowOOO = flags.allowOOO;
          if (schemeIndex < minOOOThresh()) {
            allowOOO = false;
            auto ack = scheme->getAck(schemeIndex);
            if (ack) {
              scheme->recvAck(std::move(ack));
            }
          }
          stats_.allowedOOO += (allowOOO) ? 1 : 0;
          flags.allowOOO = allowOOO;
          scheme->encodedBlocks.emplace_back(flags,
                                             newPacket,
                                             std::move(encodeRes.second),
                                             &callbacks_[index]);
          newPacket = false;
        }
        eventBase_.runInLoop(scheme, true);
      },
      scheme->prev);
}

void CompressionSimulator::setupRequest(uint16_t index,
                                        HTTPMessage&& msg,
                                        std::chrono::milliseconds encodeDelay) {
  // Normalize to relative paths
  auto query = msg.getQueryStringAsStringPiece();
  if (query.empty()) {
    msg.setURL(msg.getPathAsStringPiece());
  } else {
    msg.setURL(folly::to<string>(msg.getPathAsStringPiece(), "?", query));
  }

  auto scheme = getScheme(msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST));
  requests_.emplace_back(msg);
  auto decodeCompleteCB =
      [index, this, scheme](std::chrono::milliseconds holDelay) {
        // record processed timestamp
        CHECK(!callbacks_[index].getResult().hasError());
        DCHECK_EQ(requests_[index], *callbacks_[index].getResult().value());
        stats_.holDelay += holDelay;
        VLOG(1) << "Finished decoding request=" << index
                << " with holDelay=" << holDelay.count()
                << " cumulative HoL delay=" << stats_.holDelay.count();
        if (callbacks_[index].acknowledge) {
          sendAck(scheme, scheme->getAck(callbacks_[index].seqn));
        }
      };
  callbacks_.emplace_back(index, decodeCompleteCB);

  // Assume that all packets with same encodeDelay will form a packet
  // train.  Encode them as a group, so can we can emulate packet
  // boundaries more realistically, telling the encoder which blocks
  // start such trains.
  if (scheme->packetIndices.size() > 0) {
    auto delayFromPrevious = encodeDelay - scheme->prev;
    VLOG(1) << "request " << index << " delay " << delayFromPrevious.count();
    if (delayFromPrevious > std::chrono::milliseconds(1)) {
      flushRequests(scheme);
    }
  }
  scheme->prev = encodeDelay;
  scheme->packetIndices.push_back(index);
}

// Once per loop, each connection flushes it's encode blocks and schedules
// decodes based on how many packets the block occupies
void CompressionScheme::runLoopCallback() noexcept {
  simulator_->flushSchemePackets(this);
}

void CompressionSimulator::flushPacket(CompressionScheme* scheme) {
  if (scheme->packetBlocks.empty()) {
    return;
  }

  stats_.packets++;
  VLOG(1) << "schedule decode for " << scheme->packetBlocks.size()
          << " blocks at " << scheme->decodeDelay.count();
  scheduleEvent(
      {[this, scheme, blocks = std::move(scheme->packetBlocks)]() mutable {
        decodePacket(scheme, blocks);
      }},
      scheme->decodeDelay);
  scheme->packetBytes = 0;
}

void CompressionSimulator::flushSchemePackets(CompressionScheme* scheme) {
  CHECK(!scheme->encodedBlocks.empty());
  VLOG(2) << "Flushing " << scheme->encodedBlocks.size() << " requests";
  // tracks the number of bytes in the current simulated packet
  auto encodeRes = &scheme->encodedBlocks.front();
  bool newPacket = std::get<1>(*encodeRes);
  size_t headerBlockBytesRemaining =
      std::get<2>(*encodeRes)->computeChainDataLength();
  std::chrono::milliseconds packetDelay = deliveryDelay();
  scheme->decodeDelay = packetDelay;
  while (true) {
    if (newPacket) {
      flushPacket(scheme);
    }
    newPacket = false;
    // precondition packetBytes < kMTU
    if (scheme->packetBytes + headerBlockBytesRemaining >= kMTU) {
      // Header block filled current packet, triggering a flush
      VLOG(2) << "Request(s) spanned multiple packets";
      newPacket = true;
    } else {
      scheme->packetBytes += headerBlockBytesRemaining;
    }
    headerBlockBytesRemaining -=
        std::min(headerBlockBytesRemaining, kMTU - scheme->packetBytes);
    if (headerBlockBytesRemaining == 0) {
      // Move from the first element of encodedBlocks to the last
      // element of packetBlocks.
      scheme->packetBlocks.splice(scheme->packetBlocks.end(),
                                  scheme->encodedBlocks,
                                  scheme->encodedBlocks.begin());
      if (scheme->encodedBlocks.empty()) {
        // All done
        break;
      }
      // Grab the next request
      encodeRes = &scheme->encodedBlocks.front();
      newPacket = std::get<1>(*encodeRes);
      headerBlockBytesRemaining =
          std::get<2>(*encodeRes)->computeChainDataLength();
    }
    if (newPacket) {
      packetDelay = deliveryDelay();
      scheme->decodeDelay = std::max(scheme->decodeDelay, packetDelay);
    }
  }
  flushPacket(scheme);
  CHECK(scheme->encodedBlocks.empty());
}

CompressionScheme* CompressionSimulator::getScheme(StringPiece domain) {
  static string blended("\"Facebook\"");
  if (params_.blend &&
      (domain.endsWith("facebook.com") || domain.endsWith("fbcdn.net"))) {
    domain = blended;
  }

  auto it = domains_.find(domain.str());
  CompressionScheme* scheme = nullptr;
  if (it == domains_.end()) {
    LOG(INFO) << "Creating scheme for domain=" << domain;
    auto schemePtr = makeScheme();
    scheme = schemePtr.get();
    domains_.emplace(domain.str(), std::move(schemePtr));
  } else {
    scheme = it->second.get();
  }
  return scheme;
}

unique_ptr<CompressionScheme> CompressionSimulator::makeScheme() {
  switch (params_.type) {
    case SchemeType::QPACK:
      return make_unique<QPACKScheme>(
          this, params_.tableSize, params_.maxBlocking);
    case SchemeType::QMIN:
      return make_unique<QMINScheme>(this, params_.tableSize);
    case SchemeType::HPACK:
      return make_unique<HPACKScheme>(this, params_.tableSize);
  }
  LOG(FATAL) << "Bad scheme";
  return nullptr;
}

std::pair<FrameFlags, unique_ptr<IOBuf>> CompressionSimulator::encode(
    CompressionScheme* scheme, bool newPacket, uint16_t index) {
  VLOG(1) << "Start encoding request=" << index;
  // vector to hold cookie crumbs
  vector<string> cookies;
  vector<compress::Header> allHeaders =
      prepareMessageForCompression(requests_[index], cookies);

  auto before = stats_.uncompressed;
  auto res = scheme->encode(newPacket, std::move(allHeaders), stats_);
  VLOG(1) << "Encoded request=" << index << " for host="
          << requests_[index].getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST)
          << " orig size=" << (stats_.uncompressed - before)
          << " block size=" << res.second->computeChainDataLength()
          << " cumulative bytes=" << stats_.compressed
          << " cumulative compression ratio="
          << int(100 - double(100 * stats_.compressed) / stats_.uncompressed);
  return res;
}

void CompressionSimulator::decode(CompressionScheme* scheme,
                                  FrameFlags flags,
                                  unique_ptr<IOBuf> encodedReq,
                                  SimStreamingCallback& cb) {
  scheme->decode(flags, std::move(encodedReq), stats_, cb);
}

void CompressionSimulator::decodePacket(
    CompressionScheme* scheme,
    std::list<CompressionScheme::BlockInfo>& blocks) {
  VLOG(1) << "decode packet with " << blocks.size() << " blocks";
  while (!blocks.empty()) {
    auto encodeRes = &blocks.front();
    // TODO(ckrasic) - to get packet coordination correct, could plumb
    // through "start of packet" flag here.  Probably not worth it,
    // since it seems to make only a very small difference (about a
    // 0.1% compressiondifference on my facebook har).
    decode(scheme,
           std::get<0>(*encodeRes),
           std::move(std::get<2>(*encodeRes)),
           *std::get<3>(*encodeRes));
    blocks.pop_front();
  }
}

void CompressionSimulator::scheduleEvent(folly::Function<void()> f,
                                         std::chrono::milliseconds ms) {
  eventBase_.runAfterDelay(std::move(f), ms.count());
}

void CompressionSimulator::sendAck(CompressionScheme* scheme,
                                   unique_ptr<CompressionScheme::Ack> ack) {
  if (!ack) {
    return;
  }
  // An ack is a packet
  stats_.packets++;
  scheduleEvent([a = std::move(ack),
                 this,
                 scheme]() mutable { recvAck(scheme, std::move(a)); },
                deliveryDelay());
}

void CompressionSimulator::recvAck(CompressionScheme* scheme,
                                   unique_ptr<CompressionScheme::Ack> ack) {
  scheme->recvAck(std::move(ack));
}

std::chrono::milliseconds CompressionSimulator::deliveryDelay() {
  std::chrono::milliseconds delay = one_half_rtt();
  while (loss()) {
    stats_.packetLosses++;
    scheduleEvent([] { VLOG(4) << "Packet lost!"; }, delay);
    std::chrono::milliseconds rxmit = rxmitDelay();
    delay += rxmit;
    scheduleEvent(
        [rxmit] {
          VLOG(4) << "Packet loss detected, retransmitting with additional "
                  << rxmit.count();
        },
        delay - one_half_rtt());
  }
  if (delayed()) {
    scheduleEvent([] { VLOG(4) << "Packet delayed in network"; }, delay);
    delay += extraDelay();
  }
  return delay;
}

std::chrono::milliseconds CompressionSimulator::rtt() {
  return params_.rtt;
}

std::chrono::milliseconds CompressionSimulator::one_half_rtt() {
  return params_.rtt / 2;
}

std::chrono::milliseconds CompressionSimulator::rxmitDelay() {
  uint32_t ms = rtt().count() * Random::randDouble(1.1, 2, rng_);
  return std::chrono::milliseconds(ms);
}

bool CompressionSimulator::loss() {
  return Random::randDouble01(rng_) < params_.lossProbability;
}

bool CompressionSimulator::delayed() {
  return Random::randDouble01(rng_) < params_.delayProbability;
}

std::chrono::milliseconds CompressionSimulator::extraDelay() {
  uint32_t ms = params_.maxDelay.count() * Random::randDouble01(rng_);
  return std::chrono::milliseconds(ms);
}

uint32_t CompressionSimulator::minOOOThresh() {
  return params_.minOOOThresh;
}
}} // namespace proxygen::compress
