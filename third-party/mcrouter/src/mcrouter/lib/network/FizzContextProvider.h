/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/Range.h>
#include <wangle/ssl/TLSTicketKeySeeds.h>

namespace fizz {
class CertificateVerifier;
namespace client {
class FizzClientContext;
}
namespace server {
class FizzServerContext;
}
} // namespace fizz

namespace facebook {
namespace memcache {

using FizzContextAndVerifier = std::pair<
    std::shared_ptr<const fizz::client::FizzClientContext>,
    std::shared_ptr<const fizz::CertificateVerifier>>;

FizzContextAndVerifier createClientFizzContextAndVerifier(
    std::string certData,
    std::string keyData,
    folly::StringPiece pemCaPath,
    bool preferOcbCipher);

std::shared_ptr<fizz::server::FizzServerContext> createFizzServerContext(
    folly::StringPiece pemCertPath,
    folly::StringPiece certData,
    folly::StringPiece pemKeyPath,
    folly::StringPiece keyData,
    folly::StringPiece pemCaPath,
    bool requireClientVerification,
    bool preferOcbCipher,
    wangle::TLSTicketKeySeeds* ticketKeySeeds);
} // namespace memcache
} // namespace facebook
