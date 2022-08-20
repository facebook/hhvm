/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/experimental/crypto/MerkleTree.h>
#include <folly/portability/GTest.h>

namespace fizz {
namespace test {

TEST(MerkleTreeTest, TestTree1) {
  /**
   *   Layer 1:       H(0)
   *   Layer 0:  H(0)     H(1)
   * Messages: Message1 Message2
   */
  BatchSignatureMerkleTree<Sha256> mt(2);
  auto index1 = mt.append(folly::range(folly::StringPiece("Message1")));
  auto index2 = mt.append(folly::range(folly::StringPiece("Message2")));
  auto node1 = mt.getNodeValue(0, 0);
  auto node2 = mt.getNodeValue(0, 1);
  EXPECT_EQ(
      folly::hexlify(
          std::string((char*)node1.value()->data(), node1.value()->length())),
      "785bc5e5d7da71a3734ea21f64dd2752c82fbd323748292a8b1ba50afe99bbc6");
  EXPECT_EQ(
      folly::hexlify(
          std::string((char*)node2.value()->data(), node2.value()->length())),
      "40385de8d138be26c5525c3396512d4bd361cdb1dc47688660e53ddc768638c0");
  EXPECT_EQ(mt.countHeight(), 2);
  EXPECT_EQ(mt.countLeaves(), 2);
  EXPECT_EQ(index1.value(), 0);
  EXPECT_EQ(index2.value(), 1);
  mt.finalizeAndBuild();
  auto root = mt.getRootValue();
  EXPECT_EQ(mt.countHeight(), 2);
  EXPECT_EQ(mt.countLeaves(), 2);
  size_t hashLen = Sha256::HashLen;
  EXPECT_EQ(root->length(), hashLen);
  EXPECT_EQ(
      folly::hexlify(std::string((char*)root->data(), root->length())),
      "db1a452baff0ad476a358efeadb6f70f2c0701e4bc285a198074333b00e765fb");
}

TEST(MerkleTreeTest, TestTree2) {
  /**
   *   Layer 0:  H(0)
   * Messages: Message1
   */
  BatchSignatureMerkleTree<Sha256> mt(2);
  auto index1 = mt.append(folly::range(folly::StringPiece("Message1")));
  EXPECT_EQ(mt.countHeight(), 1);
  EXPECT_EQ(mt.countLeaves(), 1);
  EXPECT_EQ(index1.value(), 0);
  mt.finalizeAndBuild();
  auto root = mt.getRootValue();
  EXPECT_EQ(
      folly::hexlify(std::string((char*)root->data(), root->length())),
      "785bc5e5d7da71a3734ea21f64dd2752c82fbd323748292a8b1ba50afe99bbc6");
}

TEST(MerkleTreeTest, TestTree3) {
  /**
   *   Layer 2:               H(0)*
   *   Layer 1:       H(0)             H(1)*
   *   Layer 0:  H(0)     H(1)     H(2)     H(3)*=H(0)
   * Messages: Message1 Message1 Message1
   */
  BatchSignatureMerkleTree<Sha256> mt(2);
  mt.append(folly::range(folly::StringPiece("Message1")));
  mt.append(folly::range(folly::StringPiece("Message1")));
  mt.append(folly::range(folly::StringPiece("Message1")));
  EXPECT_EQ(mt.countHeight(), 2);
  EXPECT_EQ(mt.countLeaves(), 3);
  mt.finalizeAndBuild();
  auto root = mt.getRootValue();
  EXPECT_EQ(
      folly::hexlify(std::string((char*)root->data(), root->length())),
      "afb6987ffbd8fce1d5db50c8ea9b598ad82870eb27885b2f47109bb84d86b025");
  EXPECT_EQ(mt.countHeight(), 3); // height increase because of compensation
  EXPECT_EQ(mt.countLeaves(), 3);
  auto newlyAddedNode1 = mt.getNodeValue(0, 3);
  EXPECT_EQ(
      folly::hexlify(std::string(
          (char*)newlyAddedNode1.value()->data(),
          newlyAddedNode1.value()->length())),
      "785bc5e5d7da71a3734ea21f64dd2752c82fbd323748292a8b1ba50afe99bbc6");
  auto newlyAddedNode2 = mt.getNodeValue(1, 1);
  EXPECT_EQ(
      folly::hexlify(std::string(
          (char*)newlyAddedNode2.value()->data(),
          newlyAddedNode2.value()->length())),
      "e51583b571942ea1f7a1da34fdf66c310ed05b339f7c8d1bf495949fcce4437f");
  auto newlyAddedNode3 = mt.getNodeValue(2, 0);
  EXPECT_EQ(
      folly::hexlify(std::string(
          (char*)newlyAddedNode3.value()->data(),
          newlyAddedNode3.value()->length())),
      "afb6987ffbd8fce1d5db50c8ea9b598ad82870eb27885b2f47109bb84d86b025");
}

TEST(MerkleTreeTest, TestTLSTree) {
  /**
   *   Layer 2:                  H(0)
   *   Layer 1:        H(0)                 H(1)
   *   Layer 0:  H(0)       H(1)       H(2)      H(3)
   * Messages: Message1 Randomness1 Message2 Randomness2
   */
  BatchSignatureMerkleTree<Sha256> mt(2);
  auto index1 =
      mt.appendTranscript(folly::range(folly::StringPiece("Message1")));
  auto index2 =
      mt.appendTranscript(folly::range(folly::StringPiece("Message2")));
  EXPECT_EQ(mt.countHeight(), 3);
  EXPECT_EQ(mt.countLeaves(), 4);
  EXPECT_EQ(index1.value(), 0);
  EXPECT_EQ(index2.value(), 2);
  EXPECT_EQ(mt.countMessages(), 2);

  // generate a path used for reconstruct the root
  auto path = mt.getPath(index2.value());
  EXPECT_EQ(path.path->computeChainDataLength() / Sha256::HashLen, 2);
  auto expectedPathNode1 = mt.getNodeValue(0, 3);
  EXPECT_TRUE(std::equal(
      path.path->data(),
      path.path->data() + Sha256::HashLen,
      expectedPathNode1.value()->data()));
  auto expectedPathNode2 = mt.getNodeValue(1, 0);
  EXPECT_TRUE(std::equal(
      path.path->data() + Sha256::HashLen,
      path.path->data() + 2 * Sha256::HashLen,
      expectedPathNode2.value()->data()));
  // compute the root from message, index, and path
  mt.finalizeAndBuild();
  auto root = mt.getRootValue();
  auto root2 = BatchSignatureMerkleTree<Sha256>::computeRootFromPath(
      folly::range(folly::StringPiece("Message2")), std::move(path));
  EXPECT_TRUE(std::equal(root->data(), root->tail(), root2->data()));
}

} // namespace test
} // namespace fizz
