/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fizz/experimental/ktls/KTLS.h>

#if FIZZ_PLATFORM_CAPABLE_KTLS
#include <fizz/experimental/ktls/LinuxKTLS.h>
#include <folly/File.h>
#include <folly/portability/Sockets.h>

#include <sys/socket.h>

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/AESGCM256.h>

namespace fizz {

namespace {

struct KTLSParameterLayout {
  size_t keyLength;
  size_t ivLength;

  size_t ktlsAllocationSize;
  uint16_t ktlsCipherType;
};
} // namespace

/**
 * sizeof() the parameter struct for the given cipher.
 *
 * Normally, one would simply do `sizeof(tls12_crypto_info_aes_gcm128)`, however
 * we have to assume that current platform kernel headers may be out of date.
 *
 * The layout of the parameter struct (again, stable because it is part of the
 * user-kernel ABI) is a `struct tls_crypto_info` header followed by the (iv,
 * key, salt, rec_seq), which can vary based on the ciphersuite.
 */

#define CRYPTO_INFO_SIZE(cipher)                                           \
  (sizeof(struct tls_crypto_info) + cipher##_IV_SIZE + cipher##_KEY_SIZE + \
   cipher##_SALT_SIZE + cipher##_REC_SEQ_SIZE)

static folly::Optional<KTLSParameterLayout> getKTLSLayout(CipherSuite suite) {
  KTLSParameterLayout ret;

  // kTLS only works with specific ciphers. Explicitly itemize the ones that
  // are supported here.
  //
  // TODO: Newer kernels support chacha20
  switch (suite) {
    case CipherSuite::TLS_AES_128_GCM_SHA256:
      ret.keyLength = AESGCM128::kKeyLength;
      ret.ivLength = AESGCM128::kIVLength;
      ret.ktlsAllocationSize = CRYPTO_INFO_SIZE(TLS_CIPHER_AES_GCM_128);
      ret.ktlsCipherType = TLS_CIPHER_AES_GCM_128;
      break;
    case CipherSuite::TLS_AES_256_GCM_SHA384:
      ret.keyLength = AESGCM256::kKeyLength;
      ret.ivLength = AESGCM256::kIVLength;
      ret.ktlsAllocationSize = CRYPTO_INFO_SIZE(TLS_CIPHER_AES_GCM_256);
      ret.ktlsCipherType = TLS_CIPHER_AES_GCM_256;
      break;
    default:
      return folly::none;
  }
  return ret;
}

KTLSCryptoParams KTLSCryptoParams::fromRecordState(
    CipherSuite suite,
    const RecordLayerState& recordState) {
  if (!recordState.key.has_value() || !recordState.sequence.has_value()) {
    throw std::runtime_error("invalid record state for ktls");
  }

  KTLSCryptoParams params;
  params.ciphersuite = suite;
  params.key = recordState.key->clone();
  params.recordSeq = *recordState.sequence;
  return params;
}

Buf KTLSCryptoParams::toSockoptFormat() const {
  auto layout = getKTLSLayout(ciphersuite);
  if (!layout) {
    return nullptr;
  }
  auto& layoutParams = layout.value();

  if (!key.iv || !key.key) {
    return nullptr;
  }
  if (key.iv->computeChainDataLength() != layoutParams.ivLength) {
    return nullptr;
  }
  if (key.key->computeChainDataLength() != layoutParams.keyLength) {
    return nullptr;
  }

  auto buf = folly::IOBuf::create(layoutParams.ktlsAllocationSize);
  CHECK_GT(layoutParams.ktlsAllocationSize, sizeof(struct tls_crypto_info));

  // Header
  struct tls_crypto_info* info =
      reinterpret_cast<struct tls_crypto_info*>(buf->writableTail());
  info->version = TLS_1_3_VERSION;
  info->cipher_type = layoutParams.ktlsCipherType;

  buf->append(sizeof(struct tls_crypto_info));

  // Body. Format is (iv, key, salt, sequence). We have allocated enough
  // space above so that we can safely write out this tuple for all supported
  // ciphers, even if the ciphers have different key lengths.
  //
  //
  // Some clarification on iv and salt:
  //
  // AES-128-GCM and AES-256-GCM require 12 byte nonces.
  //
  // Nonce construction has changed between TLS 1.2 and TLS 1.3
  //
  // For TLS 1.2, the 12 byte nonce used for AES-GCM is constructed as follows:
  //    (1) 4 bytes of "fixed IV" (which is the 4 byte
  //        client_write_iv/server_write_iv derived from the master secret). It
  //        is called "fixed" because it is "fixed" for a given TLS session
  //        (derived once during the handshake).
  //
  //        kTLS calls this "salt". It is also called "implicit" IV in some
  //        contexts (because it is implicitly known by both parties, and it
  //        is in contrast to IVs that are sent explicitly as part of the
  //        record).
  //
  //    (2) 8 bytes of "explicit IV". This is "explicit" because it is
  //        explicitly sent in the record (see RFC 5246 6.2.3.3).
  //
  // In TLS 1.3, the client_write_iv and server_write_iv, as derived in
  // the key schedule, is already sizeof(nonce) (12 bytes for AES-GCM).
  //
  // We need to pass this 12 byte value to kTLS via the (4 byte "salt", 8 byte
  // "iv") fields.

  folly::io::Appender cursor(buf.get(), 0);

  // The "iv", which is trafficKey.iv[4:]
  auto realIV = key.iv->coalesce();

  constexpr size_t kLegacyFixedIVSize = 4;
  static_assert(
      kLegacyFixedIVSize == TLS_CIPHER_AES_GCM_128_SALT_SIZE,
      "mismatched sizes");
  static_assert(
      kLegacyFixedIVSize == TLS_CIPHER_AES_GCM_256_SALT_SIZE,
      "mismatched sizes");
  cursor.push(folly::ByteRange(
      realIV.data() + kLegacyFixedIVSize, realIV.size() - kLegacyFixedIVSize));

  // The key
  cursor.push(key.key->coalesce());

  // The "salt" which is the trafficKey.iv[:4]
  cursor.push(folly::ByteRange(realIV.data(), kLegacyFixedIVSize));

  // Finally, the record sequence number in network byte order.
  cursor.writeBE(recordSeq);

  return buf;
}

static_assert(
    sizeof(std::declval<KTLSCryptoParams>().recordSeq) ==
        TLS_CIPHER_AES_GCM_128_REC_SEQ_SIZE,
    "unexpected record sequence size");

namespace {
struct ConnectedPair {
  folly::File client;
  folly::File server;
};
} // namespace

static folly::Optional<ConnectedPair> makeConnectedTCPPair() {
  folly::Optional<ConnectedPair> result = folly::none;
  /**
   * Probing for tls module support requires a pair of connected TCP
   * sockets. This cannot be done over socketpair() (which only supports
   * AF_UNIX), so we need to actually bind a socket and connect to it.
   */
  folly::File client(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), true);
  folly::File listener(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), true);
  folly::File server;

  if (client.fd() < 0 || listener.fd() < 0) {
    return result;
  }
  int yes = 1;
  if (::setsockopt(listener.fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    return result;
  }

  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (::bind(listener.fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
      0) {
    return result;
  }
  socklen_t len = sizeof(addr);
  if (::getsockname(listener.fd(), reinterpret_cast<sockaddr*>(&addr), &len) <
      0) {
    return result;
  }

  if (::listen(listener.fd(), 1) < 0) {
    return result;
  }

  if (::connect(client.fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
      0) {
    return result;
  }

  server = folly::File(::accept(listener.fd(), nullptr, nullptr), true);
  if (server.fd() < 0) {
    return result;
  }

  result = ConnectedPair{std::move(client), std::move(server)};
  return result;
}

bool platformSupportsKTLS() {
  static bool supported = []() {
    using namespace folly::portability::sockets;
    auto connectedPair = makeConnectedTCPPair();
    if (!connectedPair) {
      return false;
    }

    int fd = connectedPair->client.fd();

    // Check for the TLS ulp option
    if (setsockopt(fd, SOL_TCP, TCP_ULP, "tls", sizeof("tls")) < 0) {
      return false;
    }

    // Check for both TX and RX support for TLS 1.3
    struct tls12_crypto_info_aes_gcm_128 fake_info {};
    fake_info.info.version = TLS_1_3_VERSION;
    fake_info.info.cipher_type = TLS_CIPHER_AES_GCM_128;
    if (setsockopt(fd, SOL_TLS, TLS_RX, &fake_info, sizeof(fake_info)) < 0) {
      return false;
    }
    if (setsockopt(fd, SOL_TLS, TLS_TX, &fake_info, sizeof(fake_info)) < 0) {
      return false;
    }

    return true;
  }();
  return supported;
}

folly::Expected<KTLSNetworkSocket, folly::exception_wrapper>
KTLSNetworkSocket::tryEnableKTLS(
    NetworkSocket fd,
    const KTLSDirectionalCryptoParams<TrafficDirection::Receive>& rx,
    const KTLSDirectionalCryptoParams<TrafficDirection::Transmit>& tx) {
  if (!platformSupportsKTLS()) {
    return folly::makeUnexpected<folly::exception_wrapper>(
        std::runtime_error("platform does not support ktls"));
  }
  if (rx.ciphersuite != tx.ciphersuite) {
    return folly::makeUnexpected<folly::exception_wrapper>(std::logic_error(
        "conflicting cipher suites in ktls cryptographic parameters"));
  }

  auto rxs = rx.toSockoptFormat();
  auto txs = tx.toSockoptFormat();
  if (!rxs || !txs) {
    return folly::makeUnexpected<folly::exception_wrapper>(
        std::runtime_error("invalid ktls cryptographic parameters"));
  }

  // Unrevertable side effect: install the TLS ULP on the socket.
  //
  // Given that we check that TCP_ULP, TLS_RX, and TLS_TX are supported on
  // a dummy socket (platformSupportsKTLS()), this is unlikely to fail.
  // However, even if it does, as long as we don't set TLS keys on the socket,
  // the socket behaves normally.
  if (folly::netops::setsockopt(fd, SOL_TCP, TCP_ULP, "tls", sizeof("tls")) <
      0) {
    return folly::makeUnexpected<folly::exception_wrapper>(std::system_error(
        errno, std::system_category(), "failed to enable tls ulp"));
  }

  // Unrevertable side effect: install the TLS_RX an TLX_TX parameters.
  if (folly::netops::setsockopt(
          fd, SOL_TLS, TLS_RX, rxs->data(), rxs->length()) < 0) {
    return folly::makeUnexpected<folly::exception_wrapper>(std::system_error(
        errno, std::system_category(), "could not configure ktls rx"));
  }

  if (folly::netops::setsockopt(
          fd, SOL_TLS, TLS_TX, txs->data(), txs->length()) < 0) {
    return folly::makeUnexpected<folly::exception_wrapper>(std::system_error(
        errno, std::system_category(), "could not configure ktls tx"));
  }

  return folly::makeExpected<folly::exception_wrapper>(KTLSNetworkSocket(fd));
}

folly::Expected<KTLSNetworkSocket, folly::exception_wrapper>
KTLSNetworkSocket::tryEnableKTLS(
    NetworkSocket fd,
    const KTLSDirectionalCryptoParams<TrafficDirection::Receive>& rx) {
  if (!platformSupportsKTLS()) {
    return folly::makeUnexpected<folly::exception_wrapper>(
        std::runtime_error("platform does not support ktls"));
  }

  auto rxs = rx.toSockoptFormat();
  if (!rxs) {
    return folly::makeUnexpected<folly::exception_wrapper>(
        std::runtime_error("invalid ktls cryptographic parameters"));
  }

  // Unrevertable side effect: install the TLS ULP on the socket.
  //
  // Given that we check that TCP_ULP, TLS_RX, and TLS_TX are supported on
  // a dummy socket (platformSupportsKTLS()), this is unlikely to fail.
  // However, even if it does, as long as we don't set TLS keys on the socket,
  // the socket behaves normally.
  if (folly::netops::setsockopt(fd, SOL_TCP, TCP_ULP, "tls", sizeof("tls")) <
      0) {
    return folly::makeUnexpected<folly::exception_wrapper>(std::system_error(
        errno, std::system_category(), "failed to enable tls ulp"));
  }

  // Unrevertable side effect: install the TLS_RX parameters.
  if (folly::netops::setsockopt(
          fd, SOL_TLS, TLS_RX, rxs->data(), rxs->length()) < 0) {
    return folly::makeUnexpected<folly::exception_wrapper>(std::system_error(
        errno, std::system_category(), "could not configure ktls rx"));
  }

  return folly::makeExpected<folly::exception_wrapper>(KTLSNetworkSocket(fd));
}
} // namespace fizz

#endif
