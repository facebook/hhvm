/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <tuple>
#include <unordered_set>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/compress/Huffman.h>
#include <proxygen/lib/http/codec/compress/Logging.h>

using namespace folly::io;
using namespace folly;
using namespace proxygen::huffman;
using namespace std;

class HuffmanTests : public testing::Test {
 protected:
  const HuffTree& tree_ = huffTree();
};

TEST_F(HuffmanTests, Codes) {
  uint32_t code;
  uint8_t bits;
  // check 'e' for both requests and responses
  tie(code, bits) = tree_.getCode('e');
  EXPECT_EQ(code, 0x05);
  EXPECT_EQ(bits, 5);
  // some extreme cases
  tie(code, bits) = tree_.getCode(0);
  EXPECT_EQ(code, 0x1ff8);
  EXPECT_EQ(bits, 13);
  tie(code, bits) = tree_.getCode(255);
  EXPECT_EQ(code, 0x3ffffee);
  EXPECT_EQ(bits, 26);
}

TEST_F(HuffmanTests, Size) {
  uint32_t size;
  folly::fbstring onebyte("x");
  size = tree_.getEncodeSize(onebyte);
  EXPECT_EQ(size, 1);

  folly::fbstring accept("accept-encoding");
  size = tree_.getEncodeSize(accept);
  EXPECT_EQ(size, 11);
}

TEST_F(HuffmanTests, Encode) {
  uint32_t size;
  // this is going to fit perfectly into 3 bytes
  folly::fbstring gzip("gzip");
  IOBufQueue bufQueue;
  QueueAppender appender(&bufQueue, 512);
  // force the allocation
  appender.ensure(512);

  size = tree_.encode(gzip, appender);
  EXPECT_EQ(size, 3);
  const IOBuf* buf = bufQueue.front();
  const uint8_t* data = buf->data();
  EXPECT_EQ(data[0], 0x9b); // 10011011
  EXPECT_EQ(data[1], 0xd9); // 11011001
  EXPECT_EQ(data[2], 0xab); // 10101011

  // size must equal with the actual encoding
  folly::fbstring accept("accept-encoding");
  size = tree_.getEncodeSize(accept);
  uint32_t encodedSize = tree_.encode(accept, appender);
  EXPECT_EQ(size, encodedSize);
}

TEST_F(HuffmanTests, Decode) {
  uint8_t buffer[3];
  // simple test with one byte
  buffer[0] = 0x60; //
  buffer[1] = 0xbf; //
  folly::fbstring literal;
  tree_.decode(buffer, 2, literal);
  EXPECT_EQ(literal, "/e");

  // simple test with "gzip"
  buffer[0] = 0x9b;
  buffer[1] = 0xd9;
  buffer[2] = 0xab;
  literal.clear();
  tree_.decode(buffer, 3, literal);
  EXPECT_EQ(literal, "gzip");

  // something with padding
  buffer[0] = 0x98;
  buffer[1] = 0xbf;
  literal.clear();
  tree_.decode(buffer, 2, literal);
  EXPECT_EQ(literal, "ge");
}

/*
 * non-printable characters, that use 3 levels
 */
TEST_F(HuffmanTests, NonPrintableDecode) {
  // character code 9 and 38 (&) that have 24 + 8 = 32 bits
  uint8_t buffer1[4] = {0xFF, 0xFF, 0xEA, 0xF8};
  folly::fbstring literal;
  tree_.decode(buffer1, 4, literal);
  EXPECT_EQ(literal.size(), 2);
  EXPECT_EQ((uint8_t)literal[0], 9);
  EXPECT_EQ((uint8_t)literal[1], 38);

  // two weird characters and padding
  // 1 and 240 will have 26 + 23 = 49 bits + 7 bits padding
  uint8_t buffer2[7] = {0xFF, 0xFF, 0xB1, 0xFF, 0xFF, 0xF5, 0xFF};
  literal.clear();
  tree_.decode(buffer2, 7, literal);
  EXPECT_EQ(literal.size(), 2);
  EXPECT_EQ((uint8_t)literal[0], 1);
  EXPECT_EQ((uint8_t)literal[1], 240);
}

TEST_F(HuffmanTests, ExampleCom) {
  // interesting case of one bit with value 0 in the last byte
  IOBufQueue bufQueue;
  QueueAppender appender(&bufQueue, 512);
  appender.ensure(512);

  folly::fbstring example("www.example.com");
  uint32_t size = tree_.getEncodeSize(example);
  EXPECT_EQ(size, 12);
  uint32_t encodedSize = tree_.encode(example, appender);
  EXPECT_EQ(size, encodedSize);

  folly::fbstring decoded;
  tree_.decode(bufQueue.front()->data(), size, decoded);
  CHECK_EQ(example, decoded);
}

TEST_F(HuffmanTests, UserAgent) {
  folly::fbstring user_agent(
      "Mozilla/5.0 (iPhone; CPU iPhone OS 7_0_4 like Mac OS X) "
      "AppleWebKit/537.51"
      ".1 (KHTML, like Gecko) Mobile/11B554a "
      "[FBAN/FBIOS;FBAV/6.7;FBBV/566055;FBD"
      "V/iPhone5,1;FBMD/iPhone;FBSN/iPhone OS;FBSV/7.0.4;FBSS/2; "
      "FBCR/AT&T;FBID/p"
      "hone;FBLC/en_US;FBOP/5]");
  IOBufQueue bufQueue;
  QueueAppender appender(&bufQueue, 512);
  appender.ensure(512);
  const HuffTree& tree = huffTree();
  uint32_t size = tree.getEncodeSize(user_agent);
  uint32_t encodedSize = tree.encode(user_agent, appender);
  EXPECT_EQ(size, encodedSize);

  folly::fbstring decoded;
  tree.decode(bufQueue.front()->data(), size, decoded);
  CHECK_EQ(user_agent, decoded);
}

/*
 * this test is verifying the CHECK for length at the end of huffman::encode()
 */
TEST_F(HuffmanTests, FitInBuffer) {
  IOBufQueue bufQueue;
  QueueAppender appender(&bufQueue, 128);

  // call with an empty string
  folly::fbstring literal("");
  tree_.encode(literal, appender);

  // allow just 1 byte
  appender.ensure(128);
  appender.append(appender.length() - 1);
  literal = "g";
  tree_.encode(literal, appender);
  CHECK_EQ(appender.length(), 0);
}

/*
 * sanity checks of each node in decode tree performed by a depth first search
 *
 * allSnodes is an array of up to 46 SuperHuffNode's, the 46 is hardcoded
 * in creation
 * nodeIndex is the current SuperHuffNode being visited
 * depth is the depth of the current SuperHuffNode being visited
 * fullCode remembers the code from parent HuffNodes
 * eosCode stores the code for End-Of-String characters which the tables
 *   do not store
 * eosCodeBits stores the number of bits for the End-Of-String character
 *   codes
 */
uint32_t treeDfs(const SuperHuffNode* allSnodes,
                 const uint32_t& snodeIndex,
                 const uint32_t& depth,
                 const uint32_t& fullCode,
                 const uint32_t& eosCode,
                 const uint32_t& eosCodeBits) {

  EXPECT_TRUE(depth < 4);

  unordered_set<uint32_t> leaves;
  uint32_t subtreeLeafCount = 0;

  for (uint32_t i = 0; i < 256; i++) {
    const HuffNode& node = allSnodes[snodeIndex].index[i];

    uint32_t newFullCode = fullCode ^ (i << (24 - 8 * depth));
    uint32_t eosCodeDepth = (eosCodeBits - 1) / 8;

    if (eosCodeDepth == depth &&
        (newFullCode >> (32 - eosCodeBits)) == eosCode) {

      // this condition corresponds to an EOS code
      // this should be a leaf that doesn't have supernode or bits set

      EXPECT_TRUE(node.isLeaf());
      EXPECT_TRUE(node.metadata.bits == 0);
    } else if (node.isLeaf()) {

      // this condition is a normal leaf
      // this should have bits set

      EXPECT_TRUE(node.isLeaf());
      EXPECT_TRUE(node.metadata.bits > 0);
      EXPECT_TRUE(node.metadata.bits <= 8);

      // used to count unique leaves at this node
      leaves.insert(node.data.ch);
    } else {

      // this condition is a branching node
      // this should have the superNodeIndex set but not bits should be set

      EXPECT_TRUE(!node.isLeaf());
      EXPECT_TRUE(node.data.superNodeIndex > 0);
      EXPECT_TRUE(node.metadata.bits == 0);
      EXPECT_TRUE(node.data.superNodeIndex < 46);

      // keep track of leaf counts for this subtree
      subtreeLeafCount += treeDfs(allSnodes,
                                  node.data.superNodeIndex,
                                  depth + 1,
                                  newFullCode,
                                  eosCode,
                                  eosCodeBits);
    }
  }
  return subtreeLeafCount + leaves.size();
}

/**
 * Class used in testing to expose the internal tables for requests
 * and responses
 */
class TestingHuffTree : public HuffTree {
 public:
  explicit TestingHuffTree(const HuffTree& tree) : HuffTree(tree) {
  }

  const SuperHuffNode* getInternalTable() {
    return table_;
  }

  static TestingHuffTree getHuffTree() {
    TestingHuffTree reqTree(huffTree());
    return reqTree;
  }
};

TEST_F(HuffmanTests, SanityChecks) {
  TestingHuffTree reqTree = TestingHuffTree::getHuffTree();
  const SuperHuffNode* allSnodesReq = reqTree.getInternalTable();
  uint32_t totalReqChars = treeDfs(allSnodesReq, 0, 0, 0, 0x3fffffff, 30);
  EXPECT_EQ(totalReqChars, 256);
}
