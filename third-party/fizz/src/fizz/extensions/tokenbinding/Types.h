/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>
#include <folly/Optional.h>

namespace fizz {
namespace extensions {

constexpr uint8_t kTokenBindingEkmSize = 32;
constexpr folly::StringPiece kTokenBindingExporterLabel{
    "EXPORTER-Token-Binding"};

enum class TokenBindingProtocolVersion : uint16_t {
  token_binding_1_0 = 0x0100,
};

std::string toString(TokenBindingProtocolVersion);

enum class TokenBindingKeyParameters : uint8_t {
  rsa2048_pkcs1_5 = 0,
  rsa2048_pss = 1,
  ecdsap256 = 2,
  ed25519_experimental = 239,
};

std::string toString(TokenBindingKeyParameters);

struct TokenBindingParameters {
  TokenBindingProtocolVersion version;
  std::vector<TokenBindingKeyParameters> key_parameters_list;
};

struct TokenBindingID {
  TokenBindingKeyParameters key_parameters;
  Buf key;
};

enum class TokenBindingType : uint8_t {
  provided_token_binding = 0,
  referred_token_binding = 1,
};

struct TokenBinding {
  TokenBindingType tokenbinding_type;
  TokenBindingID tokenbindingid;
  Buf signature;
  // TODO: if extensions are added to the token binding protocol, make an
  // extensions class that is only for token binding
  Buf extensions;
};

struct TokenBindingMessage {
  std::vector<TokenBinding> tokenbindings;
};

Extension encodeExtension(const extensions::TokenBindingParameters& params);
} // namespace extensions

template <>
folly::Optional<extensions::TokenBindingParameters> getExtension(
    const std::vector<Extension>& extensions);

template <>
Buf encode(extensions::TokenBindingMessage&& message);
template <>
extensions::TokenBindingMessage decode(folly::io::Cursor& cursor);
} // namespace fizz
