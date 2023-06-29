/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/PersistentQuicPskCache.h>

#include <folly/Conv.h>
#include <folly/dynamic.h>

namespace {
constexpr auto FIZZ_PSK = "psk";
constexpr auto QUIC_PARAMS = "quic1";
constexpr auto USES = "uses";
} // namespace

namespace proxygen {
PersistentQuicPskCache::PersistentQuicPskCache(
    const std::string& filename,
    wangle::PersistentCacheConfig config,
    std::unique_ptr<fizz::Factory> factory)
    : cache_(filename, std::move(config)), factory_(std::move(factory)) {
}

void PersistentQuicPskCache::setMaxPskUses(size_t maxUses) {
  maxPskUses_ = maxUses;
}

folly::Optional<size_t> PersistentQuicPskCache::getPskUses(
    const std::string& identity) {
  auto cachedPsk = cache_.get(identity);
  if (cachedPsk) {
    return cachedPsk->uses;
  }
  return folly::none;
}

folly::Optional<quic::QuicCachedPsk> PersistentQuicPskCache::getPsk(
    const std::string& identity) {
  auto cachedPsk = cache_.get(identity);
  if (!cachedPsk) {
    return folly::none;
  }
  try {
    quic::QuicCachedPsk quicCachedPsk;
    quicCachedPsk.cachedPsk =
        fizz::client::deserializePsk(cachedPsk->fizzPsk, *factory_);

    auto buf = folly::IOBuf::wrapBuffer(cachedPsk->quicParams.data(),
                                        cachedPsk->quicParams.length());
    folly::io::Cursor cursor(buf.get());
    fizz::detail::read(quicCachedPsk.transportParams.idleTimeout, cursor);
    fizz::detail::read(quicCachedPsk.transportParams.maxRecvPacketSize, cursor);
    fizz::detail::read(quicCachedPsk.transportParams.initialMaxData, cursor);
    fizz::detail::read(
        quicCachedPsk.transportParams.initialMaxStreamDataBidiLocal, cursor);
    fizz::detail::read(
        quicCachedPsk.transportParams.initialMaxStreamDataBidiRemote, cursor);
    fizz::detail::read(quicCachedPsk.transportParams.initialMaxStreamDataUni,
                       cursor);
    fizz::detail::read(quicCachedPsk.transportParams.initialMaxStreamsBidi,
                       cursor);
    fizz::detail::read(quicCachedPsk.transportParams.initialMaxStreamsUni,
                       cursor);
    uint8_t knobFrameSupport;
    fizz::detail::read(knobFrameSupport, cursor);
    quicCachedPsk.transportParams.knobFrameSupport = knobFrameSupport > 0;

    std::unique_ptr<folly::IOBuf> appParams;
    fizz::detail::readBuf<uint16_t>(appParams, cursor);
    quicCachedPsk.appParams = appParams->moveToFbString().toStdString();

    cachedPsk->uses++;
    if (maxPskUses_ != 0 && cachedPsk->uses >= maxPskUses_) {
      cache_.remove(identity);
    } else {
      cache_.put(identity, *cachedPsk);
    }
    return std::move(quicCachedPsk);
  } catch (const std::exception& ex) {
    LOG(ERROR) << "Error deserializing PSK: " << ex.what();
    cache_.remove(identity);
    return folly::none;
  }
}

void PersistentQuicPskCache::putPsk(const std::string& identity,
                                    quic::QuicCachedPsk quicCachedPsk) {
  PersistentQuicCachedPsk cachedPsk;
  cachedPsk.fizzPsk = fizz::client::serializePsk(quicCachedPsk.cachedPsk);

  auto quicParams = folly::IOBuf::create(0);
  folly::io::Appender appender(quicParams.get(), 512);
  fizz::detail::write(quicCachedPsk.transportParams.idleTimeout, appender);
  fizz::detail::write(quicCachedPsk.transportParams.maxRecvPacketSize,
                      appender);
  fizz::detail::write(quicCachedPsk.transportParams.initialMaxData, appender);
  fizz::detail::write(
      quicCachedPsk.transportParams.initialMaxStreamDataBidiLocal, appender);
  fizz::detail::write(
      quicCachedPsk.transportParams.initialMaxStreamDataBidiRemote, appender);
  fizz::detail::write(quicCachedPsk.transportParams.initialMaxStreamDataUni,
                      appender);
  fizz::detail::write(quicCachedPsk.transportParams.initialMaxStreamsBidi,
                      appender);
  fizz::detail::write(quicCachedPsk.transportParams.initialMaxStreamsUni,
                      appender);
  uint8_t knobSupport = quicCachedPsk.transportParams.knobFrameSupport ? 1 : 0;
  fizz::detail::write(knobSupport, appender);

  fizz::detail::writeBuf<uint16_t>(
      folly::IOBuf::wrapBuffer(folly::StringPiece(quicCachedPsk.appParams)),
      appender);
  cachedPsk.quicParams = quicParams->moveToFbString().toStdString();
  cachedPsk.uses = 0;
  cache_.put(identity, cachedPsk);
}

void PersistentQuicPskCache::removePsk(const std::string& identity) {
  cache_.remove(identity);
}
} // namespace proxygen

namespace folly {

template <>
dynamic toDynamic(const proxygen::PersistentQuicCachedPsk& cached) {
  dynamic d = dynamic::object;
  d[FIZZ_PSK] = cached.fizzPsk;
  d[QUIC_PARAMS] = cached.quicParams;
  d[USES] = cached.uses;
  return d;
}

template <>
proxygen::PersistentQuicCachedPsk convertTo(const dynamic& d) {
  proxygen::PersistentQuicCachedPsk psk;
  psk.fizzPsk = d[FIZZ_PSK].asString();
  psk.quicParams = d[QUIC_PARAMS].asString();
  psk.uses = folly::to<size_t>(d[USES].asInt());
  return psk;
}
} // namespace folly
