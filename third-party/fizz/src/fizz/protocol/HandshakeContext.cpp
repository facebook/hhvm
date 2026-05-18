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
  Error err;
  FIZZ_THROW_ON_ERROR(hashState_->hash_update(err, *data), err);
}

Buf HandshakeContextImpl::getHandshakeContext() const {
  Error err;
  std::unique_ptr<Hasher> copied;
  FIZZ_THROW_ON_ERROR(hashState_->clone(copied, err), err);
  size_t len = copied->getHashLen();
  auto out = folly::IOBuf::create(len);
  out->append(len);
  folly::MutableByteRange outRange(out->writableData(), out->length());
  FIZZ_THROW_ON_ERROR(copied->hash_final(err, outRange), err);
  return out;
}

Buf HandshakeContextImpl::getFinishedData(folly::ByteRange baseKey) const {
  auto context = getHandshakeContext();
  auto hashLen = makeHasher_->hashLength();
  Buf finishedKey;
  Error err;
  FIZZ_THROW_ON_ERROR(
      KeyDerivationImpl(makeHasher_)
          .expandLabel(
              finishedKey,
              err,
              baseKey,
              "finished",
              folly::IOBuf::create(0),
              hashLen),
      err);
  auto data = folly::IOBuf::create(hashLen);
  data->append(hashLen);
  auto outRange = folly::MutableByteRange(data->writableData(), data->length());
  FIZZ_THROW_ON_ERROR(
      hmac(err, makeHasher_, finishedKey->coalesce(), *context, outRange), err);
  return data;
}
} // namespace fizz
