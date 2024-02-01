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

#pragma once

#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/protocol/AsyncFizzBase.h>
#include <fizz/record/Types.h>

namespace wangle {

struct FizzConfig {
  bool enableFizz{true};
  std::vector<fizz::ProtocolVersion> supportedVersions;
  std::vector<std::vector<fizz::CipherSuite>> supportedCiphers;
  std::vector<fizz::SignatureScheme> supportedSigSchemes;
  std::vector<fizz::NamedGroup> supportedGroups;
  std::vector<fizz::PskKeyExchangeMode> supportedPskModes;
  bool acceptEarlyData{false};
  bool earlyDataFbOnly{false};

  // EXPERIMENTAL: Attempt to switch to kTLS based I/O on successful
  // fizz handshakes. This may or may not work depending on platform support
  // and connection parameters negotiated by the connection.
  bool preferKTLS{false};
  // EXPERIMENTAL: Attempt to switch to kTLS Rx only
  // Requires preferKTLS to be enabled
  bool preferKTLSRx{false};
  // EXPERIMENTAL: Attempt opportunistic zero-copy
  // Requires preferKTLS to be enabled
  bool expectNoPadKTLSRx{false};

  folly::Optional<uint16_t> maxRecord;
  folly::Optional<uint16_t> paddingModulo;
  std::vector<fizz::CertificateCompressionAlgorithm>
      supportedCompressionAlgorithms;
  fizz::AsyncFizzBase::TransportOptions transportOptions;

  bool dropClientX509Cert{false};
};

struct FizzClientConfig {
  bool enableFizz{false};
  std::vector<fizz::ProtocolVersion> supportedVersions;
  std::vector<fizz::CipherSuite> supportedCiphers;
  std::vector<fizz::SignatureScheme> supportedSigSchemes;
  std::vector<fizz::NamedGroup> supportedGroups;
  std::vector<fizz::PskKeyExchangeMode> supportedPskModes;
  bool sendEarlyData{false};
};

struct TokenBindingConfig {
  bool enableTokenBinding{false};
  std::vector<fizz::extensions::TokenBindingProtocolVersion> supportedVersions;
  std::vector<fizz::extensions::TokenBindingKeyParameters>
      supportedKeyParameters;
};

} // namespace wangle
