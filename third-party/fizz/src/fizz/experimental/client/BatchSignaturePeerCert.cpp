/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/experimental/client/BatchSignaturePeerCert.h>

namespace fizz {

void BatchSignaturePeerCert::batchSigVerify(
    SignatureScheme scheme,
    BatchSchemeInfo batchInfo,
    CertificateVerifyContext context,
    folly::ByteRange message,
    folly::ByteRange signature) const {
  // decode batch signature
  auto sig = folly::IOBuf::wrapBuffer(signature);
  folly::io::Cursor cursor(sig.get());
  auto decoded = BatchSignature::decode(cursor);
  // verify index
  constexpr uint64_t maxIndex = 1ULL << 32;
  if (decoded.getIndex() > maxIndex) {
    throw std::runtime_error(
        "Verification failure: batch signature index must be less than 2^31");
  }
  // verify path element size
  auto pathBuf = decoded.getPath();
  if (pathBuf->computeChainDataLength() / Sha256::HashLen > 32) {
    throw std::runtime_error(
        "Verification failure: batch signature number of path elements must be less than 32");
  }
  // verify signature
  auto rootValue = BatchSignatureMerkleTree<Sha256>::computeRootFromPath(
      message, decoded.getIndex(), std::move(pathBuf));
  auto toBeSigned =
      BatchSignature::encodeToBeSigned(std::move(rootValue), scheme);
  verifier_->verify(
      batchInfo.baseScheme,
      context,
      toBeSigned->coalesce(),
      decoded.getSignature()->coalesce());
}

} // namespace fizz
