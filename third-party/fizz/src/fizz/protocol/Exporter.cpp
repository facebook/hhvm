/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hasher.h>
#include <fizz/protocol/Exporter.h>

namespace fizz {

Status Exporter::getExportedKeyingMaterial(
    Buf& ret,
    Error& err,
    const Factory& factory,
    CipherSuite cipher,
    folly::ByteRange exporterMaster,
    folly::StringPiece label,
    Buf context,
    uint16_t length) {
  if (!context) {
    context = folly::IOBuf::create(0);
  }

  std::unique_ptr<KeyDerivation> deriver;
  FIZZ_RETURN_ON_ERROR(factory.makeKeyDeriver(deriver, err, cipher));

  std::vector<uint8_t> base(deriver->hashLength());
  folly::MutableByteRange hashedContext(base.data(), base.size());
  HashFunction hash;
  const HasherFactoryWithMetadata* hasherFactory = nullptr;
  FIZZ_RETURN_ON_ERROR(getHashFunction(hash, err, cipher));
  FIZZ_RETURN_ON_ERROR(factory.makeHasherFactory(hasherFactory, err, hash));
  fizz::hash(hasherFactory, *context, hashedContext);

  auto secret = deriver->deriveSecret(
      exporterMaster, label, deriver->blankHash(), deriver->hashLength());
  FIZZ_RETURN_ON_ERROR(deriver->expandLabel(
      ret,
      err,
      folly::range(secret),
      "exporter",
      folly::IOBuf::wrapBuffer(hashedContext),
      length));
  return Status::Success;
}
} // namespace fizz
