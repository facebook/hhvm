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

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/experimental/ktls/LinuxKTLS.h>
#include <fizz/record/RecordLayer.h>
#include <fizz/record/Types.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/net/NetworkSocket.h>

namespace fizz {

/**
 * platformCapableOfKTLS is a compile time check on whether or not the target
 * is capable of supporting kTLS. This does not actually mean the target
 * platform will allow kTLS usage. Use fizz::platformSupportsKTLS() for a
 * runtime check
 */

constexpr bool platformCapableOfKTLS =
    FIZZ_PLATFORM_CAPABLE_KTLS ? true : false;

/**
 * KTLSCryptoParams represents the cryptographic parameters required to enable
 * kTLS on a socket.
 */
struct KTLSCryptoParams {
  CipherSuite ciphersuite;
  TrafficKey key;
  uint64_t recordSeq;

  static KTLSCryptoParams fromRecordState(
      CipherSuite suite,
      const RecordLayerState& recordState);
  /**
   * toSockoptFormat encodes `KTLSCryptoParams` into a format that is
   * suitable to pass to the kernel through `setsockopt(2)`
   *
   * Returns `nullptr` if the ciphersuite is unsupported, or this represents
   * an invalid parameter configuration
   */
#if FIZZ_PLATFORM_CAPABLE_KTLS
  Buf toSockoptFormat() const;
#else
  Buf toSockoptFormat() const {
    throw std::runtime_error("platform not capable of ktls");
  }
#endif
};

/**
 * TrafficDirection indicates the direction of I/O.
 */
enum class TrafficDirection {
  Transmit,
  Receive,
};

/**
 * TLS 1.3 only. Expect the sender to not pad records if enabled
 * This allows the data to be decrypted directly into user space buffers with
 * TLS 1.3.
 */

enum class KTLSRxPad { RxPadUnknown, RxExpectNoPad };

/**
 * KTLSDirectionalCryptoParams is the same as KTLSCryptoParams, except tagged
 * with a compile time `TrafficDirection`.
 *
 * This is used in some APIs to help prevent specifying the wrong parameters
 */
template <TrafficDirection D>
struct KTLSDirectionalCryptoParams : public KTLSCryptoParams {
  KTLSDirectionalCryptoParams() = default;

  /* implicit */ KTLSDirectionalCryptoParams(KTLSCryptoParams&& params)
      : KTLSCryptoParams{std::move(params)} {}
};

/*
 * platformSupportKTLS probes the running system for KTLS support. It checks
 * to ensure that:
 *
 *    1) TLS_TX and TLS_RX are both available. This requires the `tls.ko` module
 *       to be loaded.
 *    2) TLS 1.3 support is available
 */
#if FIZZ_PLATFORM_CAPABLE_KTLS
bool platformSupportsKTLS();
#else
constexpr bool platformSupportsKTLS() {
  return false;
}
#endif

/**
 * KTLSNetworkSocket represents a file descriptor that has been successfully
 * configured with kTLS.
 */
class KTLSNetworkSocket : public folly::NetworkSocket {
 public:
  /**
   * tryEnableKTLS attempts to enable kTLS on a file descriptor.
   *
   * Upon success, an instance of KTLSNetworkSocket is returned. This can be
   * passed to the constructor of AsyncKTLSSocket, or it may be used directly.
   * Direct usage of this socket is discouraged, as the kernel does not
   * handle post handshake messages (such as NewSessionTicket) that may occur
   * on the connection.
   *
   * Upon error, an exception will be returned through the `folly::Expected`
   * error channel. The original socket may continue to be used.
   */
  static folly::Expected<KTLSNetworkSocket, folly::exception_wrapper>
  tryEnableKTLS(
      NetworkSocket fd,
      const KTLSDirectionalCryptoParams<TrafficDirection::Receive>& rx,
      const KTLSDirectionalCryptoParams<TrafficDirection::Transmit>& tx,
      KTLSRxPad rxPad = KTLSRxPad::RxPadUnknown)
#if FIZZ_PLATFORM_CAPABLE_KTLS
      ;
#else
  {
    return folly::makeUnexpected<folly::exception_wrapper>(
        std::runtime_error("platform not capable of ktls"));
  }
#endif

  static folly::Expected<KTLSNetworkSocket, folly::exception_wrapper>
  tryEnableKTLS(
      NetworkSocket fd,
      const KTLSDirectionalCryptoParams<TrafficDirection::Receive>& rx,
      KTLSRxPad rxPad = KTLSRxPad::RxPadUnknown)
#if FIZZ_PLATFORM_CAPABLE_KTLS
      ;
#else
  {
    return folly::makeUnexpected<folly::exception_wrapper>(
        std::runtime_error("platform not capable of ktls"));
  }
#endif

  /**
   * unsafeFromExistingKTLSSocket constructs a KTLSNetworkSocket from a socket
   * that has already been configured with kTLS.
   *
   * The caller guarantees that `socket` refers to a valid file descriptor
   * with kTLS configured.
   */
  static KTLSNetworkSocket unsafeFromExistingKTLSSocket(
      folly::NetworkSocket socket) {
    return KTLSNetworkSocket{socket};
  }

 private:
  explicit KTLSNetworkSocket(folly::NetworkSocket socket)
      : folly::NetworkSocket(std::move(socket)) {}
};
} // namespace fizz
