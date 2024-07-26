/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/KeyDerivation.h>

namespace fizz {

template <typename Hash>
HandshakeContextImpl<Hash>::HandshakeContextImpl() {
  hashState_.hash_init();
}

template <typename Hash>
void HandshakeContextImpl<Hash>::appendToTranscript(const Buf& data) {
  hashState_.hash_update(*data);
}

template <typename Hash>
Buf HandshakeContextImpl<Hash>::getHandshakeContext() const {
  openssl::Hasher<Hash> copied(hashState_);
  auto out = folly::IOBuf::create(Hash::HashLen);
  out->append(Hash::HashLen);
  folly::MutableByteRange outRange(out->writableData(), out->length());
  copied.hash_final(outRange);
  return out;
}

template <typename Hash>
Buf HandshakeContextImpl<Hash>::getFinishedData(
    folly::ByteRange baseKey) const {
  auto context = getHandshakeContext();
  auto finishedKey =
      KeyDerivationImpl(
          Hash::HashLen,
          &openssl::Hasher<Hash>::hash,
          &openssl::Hasher<Hash>::hmac,
          HkdfImpl(Hash::HashLen, &openssl::Hasher<Hash>::hmac),
          Hash::BlankHash)
          .expandLabel(
              baseKey, "finished", folly::IOBuf::create(0), Hash::HashLen);
  auto data = folly::IOBuf::create(Hash::HashLen);
  data->append(Hash::HashLen);
  auto outRange = folly::MutableByteRange(data->writableData(), data->length());
  openssl::Hasher<Hash>::hmac(finishedKey->coalesce(), *context, outRange);
  return data;
}
} // namespace fizz
