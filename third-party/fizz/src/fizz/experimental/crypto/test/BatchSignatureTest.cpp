/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/experimental/crypto/BatchSignature.h>
#include <folly/io/Cursor.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace test {

TEST(BatchSignatureTest, encodeToBeSigned) {
  BatchSignatureMerkleTree<Sha256> mt(4);
  mt.append(folly::range(folly::StringPiece("Message1")));
  mt.append(folly::range(folly::StringPiece("Message2")));
  mt.finalizeAndBuild();
  auto toBeSigned = BatchSignature::encodeToBeSigned(
      mt.getRootValue(), SignatureScheme::ecdsa_secp256r1_sha256_batch);
  EXPECT_EQ(
      folly::hexlify(
          std::string((char*)toBeSigned->data(), toBeSigned->length())),
      "2020202020202020202020202020202020202020202020202020202020202020"
      "2020202020202020202020202020202020202020202020202020202020202020"
      "544c53206261746368207369676e6174757265"
      "00"
      "fe00"
      "db1a452baff0ad476a358efeadb6f70f2c0701e4bc285a198074333b00e765fb");
}

TEST(BatchSignatureTest, signatureEncodeDecode) {
  BatchSignatureMerkleTree<Sha256> mt(4);
  auto index = mt.append(folly::range(folly::StringPiece("Message1")));
  mt.append(folly::range(folly::StringPiece("Message2")));
  mt.finalizeAndBuild();

  std::array<uint8_t, 10> fakeSig = {
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
  BatchSignature sig(
      mt.getPath(index.value()), folly::IOBuf::wrapBuffer(fakeSig));

  auto encoded = sig.encode();
  EXPECT_EQ(
      folly::hexlify(std::string((char*)encoded->data(), encoded->length())),
      "00000000"
      "0020"
      "40385de8d138be26c5525c3396512d4bd361cdb1dc47688660e53ddc768638c0"
      "000a"
      "00010203040506070809");

  folly::io::Cursor cursor(encoded.get());
  auto decoded = BatchSignature::decode(cursor);
  EXPECT_EQ(decoded.getIndex(), sig.getIndex());
  auto decodedSig = decoded.getSignature();
  auto originalSig = sig.getSignature();
  EXPECT_TRUE(
      std::memcmp(
          decodedSig->data(),
          originalSig->data(),
          originalSig->computeChainDataLength()) == 0);
  auto decodedPath = decoded.getPath();
  auto originalPath = sig.getPath();
  EXPECT_TRUE(
      std::memcmp(
          decodedPath->data(),
          originalPath->data(),
          originalPath->computeChainDataLength()) == 0);
}

} // namespace test
} // namespace fizz
