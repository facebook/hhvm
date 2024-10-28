/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/test/HashTest.h>

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/Hasher.h>
#include <folly/String.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

namespace fizz::test {

const std::vector<HashTestVector> kHashTestVectors = {
    HashTestVector{
        .message = "Message",
        .digest =
            {
                //{fizz::HashFunction::Sha224,
                // "3dc6a3fd912b08bf15170296c4f1694f512ffa1dc9bddb8b9e1c8d38"},
                {fizz::HashFunction::Sha256,
                 "2f77668a9dfbf8d5848b9eeb4a7145ca94c6ed9236e4a773f6dcafa5132b2f91"},
                {fizz::HashFunction::Sha384,
                 "b526d8394134b853bd071719bc99d42b669bc9252baa82dcafabc1f322a3841c57cc0c82f080fd331b1666112b27a329"},
                {fizz::HashFunction::Sha512,
                 "4fb472dfc43def7a46ad442c58ac532f89e0c8a96f23b672f5fd637652eab158d4d589444ef7530a34e6626b40830b4e1ec5364611ae31c599bffa958e8b4c4e"},
            },
    },

    HashTestVector{
        .message = "fizzy fizz fizz",
        .digest =
            {
                //{fizz::HashFunction::Sha224,
                // "d6ef892ca392de5edd1b2b904cff86d39ba935630da2bd92f4322244"},
                {fizz::HashFunction::Sha256,
                 "716980d0f8ddb8a8ee134e44ef191149328a058e02ce6b6400beae7ac71dc40a"},
                {fizz::HashFunction::Sha384,
                 "029a11ad1d072c680b4bf1cf316d47e97710bb380b100f2cbdbbceffaddb7b3d54b90de50443ee2ef3a63c3295ee880a"},
                {fizz::HashFunction::Sha512,
                 "bbaf446db8fb501b078f0ffbef048f6cc625ee1dbc1e96eb9763934a6b792735b7f606f552ac0d892b84d71b2780a8a9071ddd8dc6a9c4e52dd868c54df57764"},
            },
    },

};

void runHashTestWithFizzHasher(
    const fizz::HasherFactoryWithMetadata* makeHasher) {
  size_t hashLen = makeHasher->hashLength();

  for (auto& testVector : kHashTestVectors) {
    folly::IOBuf messageBuf(
        folly::IOBuf::COPY_BUFFER,
        reinterpret_cast<const uint8_t*>(testVector.message.c_str()),
        testVector.message.size());

    std::vector<unsigned char> out(hashLen, 0);
    folly::MutableByteRange outRange(out.data(), out.size());

    fizz::hash(makeHasher, messageBuf, outRange);

    ASSERT_EQ(folly::hexlify(out), testVector.digest.at(makeHasher->id()));
  }
}

} // namespace fizz::test
