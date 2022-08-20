/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/Huffman.h>

#include <folly/Indestructible.h>
#include <folly/portability/Sockets.h>

using std::pair;

namespace proxygen { namespace huffman {

HuffTree::HuffTree(const uint32_t* codes, const uint8_t* bits)
    : codes_(codes), bits_(bits) {
  buildTree();
}

HuffTree::HuffTree(const HuffTree& tree)
    : codes_(tree.codes_), bits_(tree.bits_) {
  buildTree();
}

bool HuffTree::decode(const uint8_t* buf,
                      uint32_t size,
                      folly::fbstring& literal) const {
  const SuperHuffNode* snode = &table_[0];
  uint32_t w = 0;
  uint32_t wbits = 0;
  uint32_t i = 0;
  while (i < size || wbits > 0) {
    // decide if we need to load more bits using an 8-bit chunk
    if (i < size && wbits < 8) {
      w = (w << 8) | buf[i];
      wbits += 8;
      i++;
    }
    // key is used for performing the indexed lookup
    uint32_t key;
    if (wbits >= 8) {
      key = w >> (wbits - 8);
    } else {
      // this the case we're at the end of the buffer
      uint8_t xbits = 8 - wbits;
      w = (w << xbits) | ((1 << xbits) - 1);
      key = w;
      wbits = 8;
    }
    // perform the indexed lookup
    const HuffNode& node = snode->index[key];
    if (node.isLeaf()) {
      // final node, we can emit the character
      literal.push_back(node.data.ch);
      wbits -= node.metadata.bits;
      snode = &table_[0];
    } else {
      // this is a branch, so we just need to move one level
      wbits -= 8;
      snode = &table_[node.data.superNodeIndex];
    }
    // remove what we've just used
    w = w & ((1 << wbits) - 1);
  }
  return true;
}

/**
 * insert a new character into the tree, identified by an unique code,
 * a number of bits to represent it. The code is aligned at LSB.
 */
void HuffTree::insert(uint32_t code, uint8_t bits, uint8_t ch) {
  SuperHuffNode* snode = &table_[0];
  while (bits > 8) {
    uint32_t mask = 0xFF << (bits - 8);
    uint32_t x = (code & mask) >> (bits - 8);
    // mark this node as branch
    if (snode->index[x].isLeaf()) {
      nodes_++;
      HuffNode& node = snode->index[x];
      node.metadata.isSuperNode = true;
      node.data.superNodeIndex = nodes_;
    }
    snode = &table_[snode->index[x].data.superNodeIndex];
    bits -= 8;
    code = code & ~mask;
  }
  // fill the node with all the suffixes
  fillIndex(*snode, code, bits, ch, bits);
}

const uint32_t* HuffTree::codesTable() const {
  return codes_;
}

const uint8_t* HuffTree::bitsTable() const {
  return bits_;
}

/**
 * recursive function for generating subtrees
 */
void HuffTree::fillIndex(SuperHuffNode& snode,
                         uint32_t code,
                         uint8_t bits,
                         uint8_t ch,
                         uint8_t level) {
  if (level == 8) {
    snode.index[code].data.ch = ch;
    snode.index[code].metadata.bits = bits;
    return;
  }
  // generate the bit at the current level
  code = code << 1;
  for (uint8_t bit = 0; bit <= 1; bit++) {
    fillIndex(snode, code | bit, bits, ch, level + 1);
  }
}

/**
 * initializes and builds the huffman tree
 */
void HuffTree::buildTree() {
  // create the indexed table
  for (uint32_t i = 0; i < kTableSize; i++) {
    insert(codes_[i], bits_[i], i);
  }
}

uint32_t HuffTree::encode(folly::StringPiece literal,
                          folly::io::QueueAppender& buf) const {
  uint32_t code;     // the huffman code of a given character
  uint8_t bits;      // on how many bits code is represented
  uint32_t w = 0;    // 4-byte word used for packing bits and write it to memory
  uint8_t wbits = 0; // how many bits we have in 'w'
  uint32_t totalBytes = 0;
  for (size_t i = 0; i < literal.size(); i++) {
    uint8_t ch = literal[i];
    code = codes_[ch];
    bits = bits_[ch];

    if (wbits + bits < 32) {
      w = (w << bits) | code;
      wbits += bits;
    } else {
      uint8_t xbits = wbits + bits - 32;
      w = (w << (bits - xbits)) | (code >> xbits);
      // write the word into the buffer by converting to network order, which
      // takes care of the endianness problems
      buf.writeBE<uint32_t>(w);
      totalBytes += 4;
      // carry for next batch
      wbits = xbits;
      w = code & ((1 << xbits) - 1);
    }
  }
  // we might have some padding at the byte level
  if (wbits & 0x7) {
    // padding bits
    uint8_t padbits = 8 - (wbits & 0x7);
    w = (w << padbits) | ((1 << padbits) - 1);

    wbits += padbits;
  }
  // we need to write the leftover bytes, from 1 to 4 bytes
  if (wbits > 0) {
    uint8_t bytes = wbits >> 3;
    // align the bits to the MSB
    w = w << (32 - wbits);
    // set the bytes in the network order and copy w[0], w[1]...
    w = htonl(w);
    // we need to use memcpy because we might write less than 4 bytes
    buf.push((uint8_t*)&w, bytes);
    totalBytes += bytes;
  }
  return totalBytes;
}

uint32_t HuffTree::getEncodeSize(folly::StringPiece literal) const {
  uint32_t totalBits = 0;
  for (size_t i = 0; i < literal.size(); i++) {
    // we just need the number of bits
    uint8_t ch = literal[i];
    totalBits += bits_[ch];
  }
  uint32_t size = totalBits >> 3;
  if (totalBits & 0x07) {
    ++size;
  }
  return size;
}

pair<uint32_t, uint8_t> HuffTree::getCode(uint8_t ch) const {
  return std::make_pair(codes_[ch], bits_[ch]);
}

// http://tools.ietf.org/html/draft-ietf-httpbis-header-compression-09#appendix-C
const uint32_t s_codesTable[kTableSize] = {
    0x1ff8,    0x7fffd8,   0xfffffe2, 0xfffffe3, 0xfffffe4,  0xfffffe5,
    0xfffffe6, 0xfffffe7,  0xfffffe8, 0xffffea,  0x3ffffffc, 0xfffffe9,
    0xfffffea, 0x3ffffffd, 0xfffffeb, 0xfffffec, 0xfffffed,  0xfffffee,
    0xfffffef, 0xffffff0,  0xffffff1, 0xffffff2, 0x3ffffffe, 0xffffff3,
    0xffffff4, 0xffffff5,  0xffffff6, 0xffffff7, 0xffffff8,  0xffffff9,
    0xffffffa, 0xffffffb,  0x14,      0x3f8,     0x3f9,      0xffa,
    0x1ff9,    0x15,       0xf8,      0x7fa,     0x3fa,      0x3fb,
    0xf9,      0x7fb,      0xfa,      0x16,      0x17,       0x18,
    0x0,       0x1,        0x2,       0x19,      0x1a,       0x1b,
    0x1c,      0x1d,       0x1e,      0x1f,      0x5c,       0xfb,
    0x7ffc,    0x20,       0xffb,     0x3fc,     0x1ffa,     0x21,
    0x5d,      0x5e,       0x5f,      0x60,      0x61,       0x62,
    0x63,      0x64,       0x65,      0x66,      0x67,       0x68,
    0x69,      0x6a,       0x6b,      0x6c,      0x6d,       0x6e,
    0x6f,      0x70,       0x71,      0x72,      0xfc,       0x73,
    0xfd,      0x1ffb,     0x7fff0,   0x1ffc,    0x3ffc,     0x22,
    0x7ffd,    0x3,        0x23,      0x4,       0x24,       0x5,
    0x25,      0x26,       0x27,      0x6,       0x74,       0x75,
    0x28,      0x29,       0x2a,      0x7,       0x2b,       0x76,
    0x2c,      0x8,        0x9,       0x2d,      0x77,       0x78,
    0x79,      0x7a,       0x7b,      0x7ffe,    0x7fc,      0x3ffd,
    0x1ffd,    0xffffffc,  0xfffe6,   0x3fffd2,  0xfffe7,    0xfffe8,
    0x3fffd3,  0x3fffd4,   0x3fffd5,  0x7fffd9,  0x3fffd6,   0x7fffda,
    0x7fffdb,  0x7fffdc,   0x7fffdd,  0x7fffde,  0xffffeb,   0x7fffdf,
    0xffffec,  0xffffed,   0x3fffd7,  0x7fffe0,  0xffffee,   0x7fffe1,
    0x7fffe2,  0x7fffe3,   0x7fffe4,  0x1fffdc,  0x3fffd8,   0x7fffe5,
    0x3fffd9,  0x7fffe6,   0x7fffe7,  0xffffef,  0x3fffda,   0x1fffdd,
    0xfffe9,   0x3fffdb,   0x3fffdc,  0x7fffe8,  0x7fffe9,   0x1fffde,
    0x7fffea,  0x3fffdd,   0x3fffde,  0xfffff0,  0x1fffdf,   0x3fffdf,
    0x7fffeb,  0x7fffec,   0x1fffe0,  0x1fffe1,  0x3fffe0,   0x1fffe2,
    0x7fffed,  0x3fffe1,   0x7fffee,  0x7fffef,  0xfffea,    0x3fffe2,
    0x3fffe3,  0x3fffe4,   0x7ffff0,  0x3fffe5,  0x3fffe6,   0x7ffff1,
    0x3ffffe0, 0x3ffffe1,  0xfffeb,   0x7fff1,   0x3fffe7,   0x7ffff2,
    0x3fffe8,  0x1ffffec,  0x3ffffe2, 0x3ffffe3, 0x3ffffe4,  0x7ffffde,
    0x7ffffdf, 0x3ffffe5,  0xfffff1,  0x1ffffed, 0x7fff2,    0x1fffe3,
    0x3ffffe6, 0x7ffffe0,  0x7ffffe1, 0x3ffffe7, 0x7ffffe2,  0xfffff2,
    0x1fffe4,  0x1fffe5,   0x3ffffe8, 0x3ffffe9, 0xffffffd,  0x7ffffe3,
    0x7ffffe4, 0x7ffffe5,  0xfffec,   0xfffff3,  0xfffed,    0x1fffe6,
    0x3fffe9,  0x1fffe7,   0x1fffe8,  0x7ffff3,  0x3fffea,   0x3fffeb,
    0x1ffffee, 0x1ffffef,  0xfffff4,  0xfffff5,  0x3ffffea,  0x7ffff4,
    0x3ffffeb, 0x7ffffe6,  0x3ffffec, 0x3ffffed, 0x7ffffe7,  0x7ffffe8,
    0x7ffffe9, 0x7ffffea,  0x7ffffeb, 0xffffffe, 0x7ffffec,  0x7ffffed,
    0x7ffffee, 0x7ffffef,  0x7fffff0, 0x3ffffee};

const uint8_t s_bitsTable[kTableSize] = {
    13, 23, 28, 28, 28, 28, 28, 28, 28, 24, 30, 28, 28, 30, 28, 28, 28, 28, 28,
    28, 28, 28, 30, 28, 28, 28, 28, 28, 28, 28, 28, 28, 6,  10, 10, 12, 13, 6,
    8,  11, 10, 10, 8,  11, 8,  6,  6,  6,  5,  5,  5,  6,  6,  6,  6,  6,  6,
    6,  7,  8,  15, 6,  12, 10, 13, 6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  7,  8,  13, 19, 13, 14,
    6,  15, 5,  6,  5,  6,  5,  6,  6,  6,  5,  7,  7,  6,  6,  6,  5,  6,  7,
    6,  5,  5,  6,  7,  7,  7,  7,  7,  15, 11, 14, 13, 28, 20, 22, 20, 20, 22,
    22, 22, 23, 22, 23, 23, 23, 23, 23, 24, 23, 24, 24, 22, 23, 24, 23, 23, 23,
    23, 21, 22, 23, 22, 23, 23, 24, 22, 21, 20, 22, 22, 23, 23, 21, 23, 22, 22,
    24, 21, 22, 23, 23, 21, 21, 22, 21, 23, 22, 23, 23, 20, 22, 22, 22, 23, 22,
    22, 23, 26, 26, 20, 19, 22, 23, 22, 25, 26, 26, 26, 27, 27, 26, 24, 25, 19,
    21, 26, 27, 27, 26, 27, 24, 21, 21, 26, 26, 28, 27, 27, 27, 20, 24, 20, 21,
    22, 21, 21, 23, 22, 22, 25, 25, 24, 24, 26, 23, 26, 27, 26, 26, 27, 27, 27,
    27, 27, 28, 27, 27, 27, 27, 27, 26};

const HuffTree& huffTree() {
  static const folly::Indestructible<HuffTree> huffTree{
      HuffTree{s_codesTable, s_bitsTable}};
  return *huffTree;
}

}} // namespace proxygen::huffman
