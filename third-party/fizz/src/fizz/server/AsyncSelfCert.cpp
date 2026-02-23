/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/AsyncSelfCert.h>

#include <fizz/record/Types.h>
#include <fizz/util/Status.h>

namespace fizz {

folly::SemiFuture<folly::Optional<AsyncSelfCert::CertificateAndSignature>>
AsyncSelfCert::getCertificateAndSign(
    folly::Optional<CertificateCompressionAlgorithm> algo,
    SignatureScheme sigScheme,
    CertificateVerifyContext verifyContext,
    std::unique_ptr<HandshakeContext> handshakeContext) const {
  Buf encodedCertificate;
  Error err;
  if (algo) {
    FIZZ_THROW_ON_ERROR(
        encodeHandshake(encodedCertificate, err, getCompressedCert(*algo)),
        err);
  } else {
    FIZZ_THROW_ON_ERROR(
        encodeHandshake(encodedCertificate, err, getCertMessage()), err);
  }
  handshakeContext->appendToTranscript(encodedCertificate);

  auto toBeSigned = handshakeContext->getHandshakeContext();
  return signFuture(sigScheme, verifyContext, std::move(toBeSigned))
      .deferValue(
          [encodedCertificate =
               std::move(encodedCertificate)](folly::Optional<Buf> sig) mutable
              -> folly::Optional<CertificateAndSignature> {
            if (!sig) {
              return folly::none;
            }
            return CertificateAndSignature{
                std::move(encodedCertificate), std::move(*sig)};
          });
}

} // namespace fizz
