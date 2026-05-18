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

Status HandshakeContextImpl::appendToTranscript(Error& err, const Buf& data) {
  FIZZ_RETURN_ON_ERROR(hashState_->hash_update(err, *data));
  return Status::Success;
}

Status HandshakeContextImpl::getHandshakeContext(Buf& ret, Error& err) const {
  std::unique_ptr<Hasher> copied;
  FIZZ_RETURN_ON_ERROR(hashState_->clone(copied, err));
  size_t len = copied->getHashLen();
  ret = folly::IOBuf::create(len);
  ret->append(len);
  folly::MutableByteRange outRange(ret->writableData(), ret->length());
  FIZZ_RETURN_ON_ERROR(copied->hash_final(err, outRange));
  return Status::Success;
}

Status HandshakeContextImpl::getFinishedData(
    Buf& ret,
    Error& err,
    folly::ByteRange baseKey) const {
  Buf context;
  FIZZ_RETURN_ON_ERROR(getHandshakeContext(context, err));
  auto hashLen = makeHasher_->hashLength();
  Buf finishedKey;
  FIZZ_RETURN_ON_ERROR(KeyDerivationImpl(makeHasher_)
                           .expandLabel(
                               finishedKey,
                               err,
                               baseKey,
                               "finished",
                               folly::IOBuf::create(0),
                               hashLen));
  ret = folly::IOBuf::create(hashLen);
  ret->append(hashLen);
  auto outRange = folly::MutableByteRange(ret->writableData(), ret->length());
  FIZZ_RETURN_ON_ERROR(
      hmac(err, makeHasher_, finishedKey->coalesce(), *context, outRange));
  return Status::Success;
}
} // namespace fizz
