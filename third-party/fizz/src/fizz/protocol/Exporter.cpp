/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Exporter.h>

namespace fizz {

Buf Exporter::getExportedKeyingMaterial(
    const Factory& factory,
    CipherSuite cipher,
    folly::ByteRange exporterMaster,
    folly::StringPiece label,
    Buf context,
    uint16_t length) {
  if (!context) {
    context = folly::IOBuf::create(0);
  }
  auto deriver = factory.makeKeyDeriver(cipher);
  std::vector<uint8_t> base(deriver->hashLength());
  folly::MutableByteRange hashedContext(base.data(), base.size());
  deriver->hash(*context, hashedContext);
  auto secret = deriver->deriveSecret(
      exporterMaster, label, deriver->blankHash(), deriver->hashLength());
  return deriver->expandLabel(
      folly::range(secret),
      "exporter",
      folly::IOBuf::wrapBuffer(hashedContext),
      length);
}
} // namespace fizz
