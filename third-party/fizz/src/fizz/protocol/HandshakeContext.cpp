/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/Hmac.h>
#include <fizz/crypto/KeyDerivation.h>
#include <fizz/protocol/HandshakeContext.h>

namespace fizz {

void HandshakeContextImpl::appendToTranscript(const Buf& data) {
  hashState_->hash_update(*data);
}

Buf HandshakeContextImpl::getHandshakeContext() const {
  auto copied = hashState_->clone();
  size_t len = copied->getHashLen();
  auto out = folly::IOBuf::create(len);
  out->append(len);
  folly::MutableByteRange outRange(out->writableData(), out->length());
  copied->hash_final(outRange);
  return out;
}

Buf HandshakeContextImpl::getFinishedData(folly::ByteRange baseKey) const {
  auto context = getHandshakeContext();
  auto hashLen = makeHasher_->hashLength();
  auto finishedKey =
      KeyDerivationImpl(makeHasher_)
          .expandLabel(baseKey, "finished", folly::IOBuf::create(0), hashLen);
  auto data = folly::IOBuf::create(hashLen);
  data->append(hashLen);
  auto outRange = folly::MutableByteRange(data->writableData(), data->length());
  hmac(makeHasher_, finishedKey->coalesce(), *context, outRange);
  return data;
}
} // namespace fizz
