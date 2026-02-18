/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/record/Types.h>

#include <folly/String.h>
#include <folly/io/Cursor.h>

using namespace fizz::extensions;

namespace fizz {
namespace detail {

template <>
Status read<TokenBindingID>(
    size_t& ret,
    Error& err,
    TokenBindingID& id,
    folly::io::Cursor& cursor) {
  size_t len;
  FIZZ_RETURN_ON_ERROR(read(len, err, id.key_parameters, cursor));
  len += readBuf<uint16_t>(id.key, cursor);
  ret = len;
  return Status::Success;
}

template <>
void write<TokenBindingID>(const TokenBindingID& id, folly::io::Appender& out) {
  write(id.key_parameters, out);
  writeBuf<uint16_t>(id.key, out);
}

template <>
size_t getSize<TokenBindingID>(const TokenBindingID& id) {
  return sizeof(TokenBindingKeyParameters) + getBufSize<uint16_t>(id.key);
}

template <>
Status read<TokenBinding>(
    size_t& ret,
    Error& err,
    TokenBinding& tokenBinding,
    folly::io::Cursor& cursor) {
  size_t len;
  FIZZ_RETURN_ON_ERROR(read(len, err, tokenBinding.tokenbinding_type, cursor));
  size_t idLen;
  FIZZ_RETURN_ON_ERROR(read(idLen, err, tokenBinding.tokenbindingid, cursor));
  len += idLen;
  len += readBuf<uint16_t>(tokenBinding.signature, cursor);
  len += readBuf<uint16_t>(tokenBinding.extensions, cursor);
  ret = len;
  return Status::Success;
}

template <>
void write<TokenBinding>(
    const TokenBinding& tokenBinding,
    folly::io::Appender& out) {
  write(tokenBinding.tokenbinding_type, out);
  write(tokenBinding.tokenbindingid, out);
  writeBuf<uint16_t>(tokenBinding.signature, out);
  writeBuf<uint16_t>(tokenBinding.extensions, out);
}

template <>
size_t getSize<TokenBinding>(const TokenBinding& tokenBinding) {
  return sizeof(TokenBindingType) + getSize(tokenBinding.tokenbindingid) +
      getBufSize<uint16_t>(tokenBinding.signature) +
      getBufSize<uint16_t>(tokenBinding.extensions);
}
} // namespace detail

std::string extensions::toString(TokenBindingProtocolVersion version) {
  switch (version) {
    case TokenBindingProtocolVersion::token_binding_1_0:
      return "Token Binding v1";
  }

  return enumToHex(version);
}

std::string extensions::toString(TokenBindingKeyParameters keyParams) {
  switch (keyParams) {
    case TokenBindingKeyParameters::rsa2048_pkcs1_5:
      return "RSA2048 pkcs";
    case TokenBindingKeyParameters::rsa2048_pss:
      return "RSA2048 pss";
    case TokenBindingKeyParameters::ecdsap256:
      return "ECDSA p256";
    case TokenBindingKeyParameters::ed25519_experimental:
      return "Ed25519 (Experimental)";
  }
  return enumToHex(keyParams);
}

template <>
Status getExtension(
    folly::Optional<TokenBindingParameters>& ret,
    Error& err,
    const std::vector<Extension>& extensions) {
  auto it = findExtension(extensions, ExtensionType::token_binding);
  if (it == extensions.end()) {
    ret = folly::none;
    return Status::Success;
  }
  TokenBindingParameters params;
  folly::io::Cursor cursor(it->extension_data.get());
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, params.version, cursor));
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint8_t>(
          len, err, params.key_parameters_list, cursor));
  ret = params;
  return Status::Success;
}

namespace extensions {

Status encodeExtension(
    Extension& ret,
    Error& /* err */,
    const TokenBindingParameters& params) {
  Extension ext;
  ext.extension_type = ExtensionType::token_binding;
  ext.extension_data = folly::IOBuf::create(0);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  detail::write(params.version, appender);
  detail::writeVector<uint8_t>(params.key_parameters_list, appender);
  ret = std::move(ext);
  return Status::Success;
}
} // namespace extensions

template <>
Status encode(Buf& ret, Error& /* err */, TokenBindingMessage&& message) {
  auto buf = folly::IOBuf::create(20);
  folly::io::Appender appender(buf.get(), 20);
  detail::writeVector<uint16_t>(message.tokenbindings, appender);
  ret = std::move(buf);
  return Status::Success;
}

template <>
Status decode<TokenBindingMessage>(
    TokenBindingMessage& ret,
    Error& err,
    folly::io::Cursor& cursor) {
  TokenBindingMessage message;
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(len, err, message.tokenbindings, cursor));
  ret = std::move(message);
  return Status::Success;
}
} // namespace fizz
