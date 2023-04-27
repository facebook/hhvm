/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <tuple>

#include <fizz/crypto/KeyDerivation.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/record/Types.h>

namespace fizz {
namespace detail {

std::tuple<Buf, std::vector<fizz::Extension>> decodeAuthRequest(
    const Buf& authRequest);

Buf computeTranscriptHash(
    std::unique_ptr<KeyDerivation>& deriver,
    const Buf& toBeHashed);

void writeBuf(const Buf& buf, folly::io::Appender& out);

Buf computeTranscript(
    const Buf& handshakeContext,
    const Buf& authenticatorRequest,
    const Buf& certificate);

Buf computeFinishedTranscript(const Buf& crTranscript, const Buf& certVerify);

Buf getFinishedData(
    std::unique_ptr<KeyDerivation>& deriver,
    Buf& finishedMacKey,
    const Buf& finishedTranscript);

folly::Optional<std::vector<SignatureScheme>> getRequestedSchemes(
    const std::vector<fizz::Extension>& authRequestExtensions);

folly::Optional<SignatureScheme> getSignatureScheme(
    const std::vector<SignatureScheme>& supportedSchemes,
    const SelfCert& cert,
    const std::vector<fizz::Extension>& authRequestExtensions);

Buf getEmptyAuthenticator(
    std::unique_ptr<KeyDerivation>& kderiver,
    Buf authRequest,
    Buf handshakeContext,
    Buf finishedMacKey);

} // namespace detail
} // namespace fizz
