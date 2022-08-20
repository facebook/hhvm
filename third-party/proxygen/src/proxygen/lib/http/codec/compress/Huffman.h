/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/FBString.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <proxygen/lib/http/codec/compress/HPACKConstants.h>
#include <string>

namespace proxygen { namespace huffman {

// size of the huffman tables (codes and bits)
const uint32_t kTableSize = 256;

// not used explicitly, since the prefixes are all 1's and they are
// used only for padding of up to 7 bits
const uint32_t kEOSHpack = 0x3fffffff;

/**
 * node from the huffman tree
 *
 * A leaf has no index table, or index == nullptr
 */
struct HuffNode {
  union {
    uint8_t ch; // leafs hold characters
    uint8_t superNodeIndex;
  } data{0};
  struct {
    uint8_t bits : 4; // how many bits are used for representing ch, range is
                      // 0-8
    bool isSuperNode : 1;
  } metadata{0, false};

  bool isLeaf() const {
    return !metadata.isSuperNode;
  }
};

/**
 * a super node from the condensed Huffman Tree representation with 8-bit level
 */
struct SuperHuffNode {
  HuffNode index[256];
};

/**
 * Immutable Huffman tree used in the process of decoding. Traditionally the
 * huffman tree is binary, but using that approach leads to major inefficiencies
 * since it's using per-bit level processing and needs to perform several memory
 * accesses and bit operations for every single bit.
 * This implementation is using 8-bit level indexing and uses aggregated nodes
 * that link up to 256 other nodes. The complexity for lookup is reduced from
 * O(bits) to O(Bytes) which is 1 or 2 for most of the printable characters.
 * The tradeoff of using this approach is using more memory and generating the
 * tree is more laborious since we need to fill all the subtrees denoted by a
 * character code, which is an unique prefix.
 *
 * Example
 *
 * bit stream:
 * 00001111 1111010
 * 1. our lookup key is 00001111 which will point to character '/', since the
 * entire subtree with prefix 0000 points to it. We know the subtree has just
 * 4 bits, we remove just those from the current key.
 * bit stream:
 * 11111111 010
 *
 * 2. key is 11111111 which points to a branch, so we go down one level
 * bit stream:
 * 010
 *
 * 3. we don't have enough bits, so we use paddding and we get a key of
 * 01011111, which points to '(' character, like any other node under the
 * subtree '010'.
 */
class HuffTree {
 public:
  /**
   * the constructor assumes the codes and bits tables will not be freed,
   * ideally they are static
   */
  explicit HuffTree(const uint32_t* codes, const uint8_t* bits);
  explicit HuffTree(HuffTree&& tree) = default;
  ~HuffTree() {
  }

  /**
   * decode bitstream into a string literal
   *
   * @param buf start of a huffman-encoded bit stream
   * @param size size of the buffer
   * @param literal where to append decoded characters
   *
   * @return true if the decode process was successful
   */
  bool decode(const uint8_t* buf,
              uint32_t size,
              folly::fbstring& literal) const;

  /**
   * encode string literal into huffman encoded bit stream
   *
   * @param literal string to encode
   * @param buf where to append the encoded binary data
   */
  uint32_t encode(folly::StringPiece literal,
                  folly::io::QueueAppender& buf) const;

  /**
   * get the encode size for a string literal, works as a dry-run for the encode
   * useful to allocate enough buffer space before doing the actual encode
   *
   * @param literal string literal
   * @return size how many bytes it will take to encode the given string
   */
  uint32_t getEncodeSize(folly::StringPiece literal) const;

  /**
   * get the binary representation for a given character, as a 32-bit word and
   * a number of bits is represented on (<32). The code is aligned to LSB.
   *
   * @param ch ASCII character
   * @return pair<word, bits>
   *
   * Example:
   * 'e' will be encoded as 1 using 4 bits: 0001
   */
  std::pair<uint32_t, uint8_t> getCode(uint8_t ch) const;

  // get internal tables for codes and bit lengths, useful for testing
  const uint32_t* codesTable() const;
  const uint8_t* bitsTable() const;

 private:
  void fillIndex(SuperHuffNode& snode,
                 uint32_t code,
                 uint8_t bits,
                 uint8_t ch,
                 uint8_t level);
  void buildTree();
  void insert(uint32_t code, uint8_t bits, uint8_t ch);

  uint32_t nodes_{0};
  const uint32_t* codes_;
  const uint8_t* bits_;

 protected:
  explicit HuffTree(const HuffTree& tree);
  SuperHuffNode table_[46];
};

const HuffTree& huffTree();

}} // namespace proxygen::huffman
