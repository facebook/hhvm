/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/Extensions.h>
#include <fizz/server/AeadCookieCipher.h>

namespace fizz {
namespace server {
namespace detail {

enum class CookieHasGroup : uint8_t {
  No = 0,
  Yes = 1,
};

enum class CookieHasEch : uint8_t {
  No = 0,
  Yes = 1,
};

inline Buf encodeCookie(const CookieState& state) {
  auto buf = folly::IOBuf::create(100);
  folly::io::Appender appender(buf.get(), 100);

  fizz::detail::write(state.version, appender);
  fizz::detail::write(state.cipher, appender);

  if (state.group) {
    fizz::detail::write(CookieHasGroup::Yes, appender);
    fizz::detail::write(*state.group, appender);
  } else {
    fizz::detail::write(CookieHasGroup::No, appender);
  }

  fizz::detail::writeBuf<uint16_t>(state.chloHash, appender);
  fizz::detail::writeBuf<uint16_t>(state.appToken, appender);

  if (state.echCipherSuite) {
    fizz::detail::write(CookieHasEch::Yes, appender);
    fizz::detail::write(*state.echCipherSuite, appender);
    fizz::detail::write(*state.echConfigId, appender);
    fizz::detail::writeBuf<uint16_t>(state.echEnc, appender);
  } else {
    fizz::detail::write(CookieHasEch::No, appender);
  }
  return buf;
}

inline CookieState decodeCookie(Buf cookie) {
  folly::io::Cursor cursor(cookie.get());

  CookieState state;
  fizz::detail::read(state.version, cursor);
  fizz::detail::read(state.cipher, cursor);

  CookieHasGroup hasGroup;
  fizz::detail::read(hasGroup, cursor);
  if (hasGroup == CookieHasGroup::Yes) {
    NamedGroup group;
    fizz::detail::read(group, cursor);
    state.group = group;
  }

  fizz::detail::readBuf<uint16_t>(state.chloHash, cursor);
  fizz::detail::readBuf<uint16_t>(state.appToken, cursor);

  CookieHasEch hasEch;
  fizz::detail::read(hasEch, cursor);
  if (hasEch == CookieHasEch::Yes) {
    ech::HpkeSymmetricCipherSuite cs;
    fizz::detail::read(cs, cursor);
    state.echCipherSuite = std::move(cs);
    uint8_t configId;
    fizz::detail::read(configId, cursor);
    state.echConfigId = configId;
    fizz::detail::readBuf<uint16_t>(state.echEnc, cursor);
  }
  return state;
}
} // namespace detail

boost::variant<AppToken, StatelessHelloRetryRequest>
AeadCookieCipher::getTokenOrRetry(Buf clientHello, Buf appToken) const {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  queue.append(std::move(clientHello));
  auto msg = PlaintextReadRecordLayer().readEvent(queue, Aead::AeadOptions());
  if (!msg) {
    throw std::runtime_error("no TLS message in initial");
  }

  auto chloPtr = msg->asClientHello();
  if (!chloPtr) {
    throw std::runtime_error("Msg isn't client hello");
  }
  auto chlo = std::move(*chloPtr);

  auto cookie = getExtension<Cookie>(chlo.extensions);
  if (cookie) {
    auto state = decrypt(std::move(cookie->cookie));
    if (!state) {
      throw std::runtime_error("cookie could not be decrypted");
    }
    AppToken token;
    token.token = std::move(state->appToken);
    return std::move(token);
  } else {
    StatelessHelloRetryRequest hrr;
    hrr.data = getStatelessResponse(chlo, std::move(appToken));
    return std::move(hrr);
  }
}

folly::Optional<CookieState> AeadCookieCipher::decrypt(Buf cookie) const {
  auto plaintext = tokenCipher_->decrypt(std::move(cookie));
  if (plaintext) {
    return detail::decodeCookie(std::move(*plaintext));
  } else {
    return folly::none;
  }
}

Buf AeadCookieCipher::getStatelessResponse(
    const ClientHello& chlo,
    Buf appToken) const {
  auto state = getCookieState(
      *context_->getFactory(),
      context_->getSupportedVersions(),
      context_->getSupportedCiphers(),
      context_->getSupportedGroups(),
      chlo,
      std::move(appToken));

  auto encoded = detail::encodeCookie(state);
  auto cookie = tokenCipher_->encrypt(std::move(encoded));
  if (!cookie) {
    throw std::runtime_error("could not encrypt cookie");
  }

  auto statelessMessage = getStatelessHelloRetryRequest(
      state.version, state.cipher, state.group, std::move(*cookie));

  return PlaintextWriteRecordLayer()
      .writeHandshake(std::move(statelessMessage))
      .data;
}
} // namespace server
} // namespace fizz
