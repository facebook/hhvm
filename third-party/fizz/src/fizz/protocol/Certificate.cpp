/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Certificate.h>

namespace fizz {

IdentityCert::IdentityCert(std::string identity) : identity_(identity) {}

std::string IdentityCert::getIdentity() const {
  return identity_;
}

std::optional<std::string> IdentityCert::getDER() const {
  return std::nullopt;
}

namespace certverify {

Buf prepareSignData(
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) {
  static constexpr folly::StringPiece kServerLabel =
      "TLS 1.3, server CertificateVerify";
  static constexpr folly::StringPiece kClientLabel =
      "TLS 1.3, client CertificateVerify";
  static constexpr folly::StringPiece kAuthLabel = "Exported Authenticator";
  static constexpr folly::StringPiece kServerDelegatedCredLabel =
      "TLS, server delegated credentials";
  static constexpr folly::StringPiece kClientDelegatedCredLabel =
      "TLS, client delegated credentials";
  static constexpr size_t kSigPrefixLen = 64;
  static constexpr uint8_t kSigPrefix = 32;

  folly::StringPiece label;
  switch (context) {
    case CertificateVerifyContext::Server:
      label = kServerLabel;
      break;
    case CertificateVerifyContext::Client:
      label = kClientLabel;
      break;
    case CertificateVerifyContext::Authenticator:
      label = kAuthLabel;
      break;
    case CertificateVerifyContext::ClientDelegatedCredential:
      label = kClientDelegatedCredLabel;
      break;
    case CertificateVerifyContext::ServerDelegatedCredential:
      label = kServerDelegatedCredLabel;
      break;
  }

  size_t sigDataLen = kSigPrefixLen + label.size() + 1 + toBeSigned.size();
  auto buf = folly::IOBuf::create(sigDataLen);
  buf->append(sigDataLen);

  // Place bytes in the right order.
  size_t offset = 0;
  memset(buf->writableData(), kSigPrefix, kSigPrefixLen);
  offset += kSigPrefixLen;
  memcpy(buf->writableData() + offset, label.data(), label.size());
  offset += label.size();
  memset(buf->writableData() + offset, 0, 1);
  offset += 1;
  memcpy(buf->writableData() + offset, toBeSigned.data(), toBeSigned.size());
  return buf;
}
} // namespace certverify
} // namespace fizz
