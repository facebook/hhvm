/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/RandomGenerator.h>
#include <fizz/crypto/Sha256.h>
#include <fizz/record/Types.h>
#include <folly/container/F14Map.h>

namespace fizz {

/**
 * The path of a leaf in a Merkle tree, this serves as a proof of existence of a
 * message in a Merkle Tree.
 */
struct MerkleTreePath {
  // The offset of a message's hash in the Merkle tree.
  uint32_t index;
  // A collection of sibling nodes for root calculation.
  Buf path;
};

/**
 * The Merkle Tree Data Structure.
 *
 * A derived class of MerkleTree should provide implementation of three
 * functions:
 *  1. void finalizeTree()
 *    This function is to finalize the tree by balancing the tree, that is, all
 *    layers except the root node should have even number of nodes to ensure all
 *    the leaves are covered by the root. The post condition of the function is
 *    to ensure the merkle tree is properly balanced. The function should not
 *    modify nextRightmostLeaf_.
 *
 *  2. static std::array<uint8_t, Hash::HashLen> constructInternalNode(
 *       folly::ByteRange leftChild,
 *       folly::ByteRange rightChild)
 *    This function is to generate a internal node from the leftchild and right
 *    child. Importantly, proper prepending mechanism should be considered to
 *    prevent second preimage attacks.
 *    @param leftChild, left child of the internal node to generate.
 *    @param rightChild, right child of the internal node to generate.
 *
 *  3. static std::array<uint8_t, Hash::HashLen> constructLeafNode(
 *       folly::ByteRange msg)
 *    This function is to generate a leaf node from a message. Importantly,
 *    proper prepending mechanism should be considered to prevent second
 *    preimage attacks.
 *    @param msg, the original message.
 */
template <typename Derived, typename Hash = Sha256>
class MerkleTree {
 public:
  // key of Hash Map: (height, offset)
  using Key = std::tuple<size_t, size_t>;
  // value of Hash Map: Hash value
  using Value = std::array<uint8_t, Hash::HashLen>;

  /**
   * Constructor of the Merkle Tree.
   *
   * @param maxLeavesSize, maximum number of leaves allowed by the tree.
   */
  MerkleTree(size_t maxLeavesSize)
      : maxLeavesSize_(maxLeavesSize), nextRightmostLeaf_(0), rootHeight_(0) {}

  /**
   * Add a new message into the Merkle tree.
   *
   * This will create a new leaf node and all upper layer nodes if applicable.
   */
  folly::Optional<size_t> append(folly::ByteRange msg) {
    if (nextRightmostLeaf_ >= maxLeavesSize_) {
      return folly::none;
    }
    // generate the leave node with the hash function
    Value leaf = Derived::constructLeafNode(msg);
    addNode(std::move(leaf), 0, nextRightmostLeaf_);
    auto insertionIndex = nextRightmostLeaf_++;
    return insertionIndex;
  }

  /**
   * Balances the tree.
   *
   * Invokes finalizeTree() in derived classes. After this call completes, it is
   * safe to call methods that query information about the tree, such as
   * rootValue() and getPath().
   */
  void finalizeAndBuild() {
    static_cast<Derived*>(this)->finalizeTree();
  }

  /**
   * Get the root value of the Merkle Tree.
   *
   * finalizeAndBuild() should be explicitly called before using this
   * function.
   */
  Buf getRootValue() const {
    return getNodeValue(rootHeight_, 0).value();
  }

  /**
   * Get a tree node with the height and the offset.
   *
   * If one wants to get the root value that covers all the messages, call
   * getRootValue() instead.
   *
   * @param height  the height of the tree starting from 0
   * @param offset  the offset of the node counting from left to right, starting
   *                from 0
   */
  folly::Optional<Buf> getNodeValue(int height, int offset) const {
    auto it = tree_.find(std::make_tuple(height, offset));
    if (it == tree_.end()) {
      return folly::none;
    }
    const auto& value = it->second;
    return folly::IOBuf::copyBuffer(value.data(), value.size());
  }

  /**
   * Get the Merkle tree path for a leaf node.
   *
   * This function will call getRootValue() in order to make each layer has even
   * # of nodes. Note that finalizeAndBuild() should be explicitly called before
   * using this function.
   *
   * @param index the offset of the leaf counting from left to right, starting
   *              from 0
   */
  MerkleTreePath getPath(uint32_t index) const {
    getRootValue();
    struct MerkleTreePath result;
    result.index = index;
    size_t pathSize = rootHeight_ * Hash::HashLen;
    result.path = folly::IOBuf::create(pathSize);
    result.path->append(pathSize);
    int offset = index;
    for (uint8_t height = 0; height < rootHeight_; height++) {
      if (offset % 2 == 0) {
        offset++;
      } else {
        offset--;
      }
      auto it = tree_.find(std::make_tuple(height, offset));
      if (it == tree_.end()) {
        throw std::runtime_error("Merkle Tree is not balanced");
      }
      std::memcpy(
          result.path->writableData() + height * Hash::HashLen,
          it->second.data(),
          it->second.size());
      offset /= 2;
    }
    return result;
  }

  /**
   * Generate the root value from a Merkle Tree path.
   */
  static Buf computeRootFromPath(folly::ByteRange msg, size_t index, Buf path) {
    // start with hashing the message
    auto result = Derived::constructLeafNode(msg);

    // iterate the path to recover the root value
    auto pathRange = path->coalesce();
    if (pathRange.size() % Hash::HashLen != 0) {
      throw std::runtime_error(
          "Merkle Tree path has a bad format: path length must be a multiple of HashLen");
    }
    size_t curIndex = index;
    for (size_t i = 0; i < pathRange.size() / Hash::HashLen; i++) {
      if (curIndex % 2 == 0) {
        result = Derived::constructInternalNode(
            folly::range(result),
            pathRange.subpiece(i * Hash::HashLen, Hash::HashLen));
      } else {
        result = Derived::constructInternalNode(
            pathRange.subpiece(i * Hash::HashLen, Hash::HashLen),
            folly::range(result));
      }
      curIndex /= 2;
    }
    return folly::IOBuf::copyBuffer(result.data(), result.size());
  }
  static Buf computeRootFromPath(
      folly::ByteRange msg,
      struct MerkleTreePath&& path) {
    return computeRootFromPath(msg, path.index, std::move(path.path));
  }

  /**
   * Get the height of the Merkle tree.
   *
   * The returned height counts the leaf layer and root layer. Note that the
   * height of the tree can change after calling getRootValue() or getPath()
   * because these two functions can increase the height by 1 when previous root
   * value cannot cover all leaves.
   */
  size_t countHeight() const {
    return rootHeight_ + 1;
  }

  /**
   * Get the # of leaves in the tree.
   */
  size_t countLeaves() const {
    return nextRightmostLeaf_;
  }

  /**
   * Clear the underlying Merkle Tree data structure.
   */
  void clear() {
    nextRightmostLeaf_ = 0;
    rootHeight_ = 0;
    tree_.clear();
  }

 protected:
  /**
   * Add a new value into the tree.
   *
   * This is a function which will recursively add upper layer nodes when the
   * current layer has even # of nodes.
   *
   * @param hash    the tree node's value, which is an output of the hash
   *                function.
   * @param height  (height, offset) uniquely identifies a tree node to insert
   *                the new value.
   * @param offset  (height, offset) uniquely identifies a tree node to insert
   *                the new value.
   */
  void addNode(Value&& hash, size_t height, size_t offset) {
    // insert the leaf into the tree
    tree_[std::make_tuple(height, offset)] = std::move(hash);
    if (height > rootHeight_) {
      rootHeight_ = height;
    }
    // if the index is an odd number, a upper layer node is ready
    if (offset % 2 == 1) {
      const auto& leftChild = tree_[std::make_tuple(height, offset - 1)];
      const auto& rightChild = tree_[std::make_tuple(height, offset)];
      Value parentHash = Derived::constructInternalNode(leftChild, rightChild);
      addNode(std::move(parentHash), height + 1, offset / 2);
    }
  }

  // maximum leaves allowed by the tree
  const size_t maxLeavesSize_;
  // the next index of the rightmost leaf
  size_t nextRightmostLeaf_;
  // the height of the tree
  size_t rootHeight_;
  // tree structure
  folly::F14FastMap<Key, Value> tree_;
};

/**
 * Merkle Tree for TLS Batch Signature.
 *
 * Refer to https://datatracker.ietf.org/doc/draft-ietf-tls-batch-signing for a
 * detailed description.
 */
template <class Hash = Sha256>
class BatchSignatureMerkleTree
    : public MerkleTree<BatchSignatureMerkleTree<Hash>, Hash> {
 public:
  /**
   * Constructor of the Merkle Tree.
   *
   * @param maxMsgSize  maximum number of messages allowed by the tree. The
   *                    underlying Merkle Tree can at most have 2 * maxMsgSize
   *                    leaves.
   */
  BatchSignatureMerkleTree(size_t maxMsgSize)
      : MerkleTree<BatchSignatureMerkleTree<Hash>, Hash>(2 * maxMsgSize) {}

  /**
   * Append a new TLS handshake transcript into the Merkle Tree for TLS Batch
   * Signature.
   *
   * This function will add two leaves into the tree: one is the hash
   * of the input @p msg and the other is the hash of a randomness.  All
   * appended messages will be hashed into a left leaf node (with even index
   * number) while the right leaf node is a random value.
   *
   * @param msg the message to append into the Merkle Tree.
   * @return the index of the message if it is successfully appended; -1 if
   * the Merkle Tree is full (2^31 message already).
   */
  folly::Optional<size_t> appendTranscript(folly::ByteRange msg) {
    // first add the message into the Merkle tree
    auto messageIndex = this->append(msg);
    // then generate a randomness
    auto random = RandomGenerator<64>().generateRandom();
    // insert the random message into the Merkle tree
    this->append(folly::range(random));
    return messageIndex;
  }

  /**
   * Get the number of messages that have been appended into the tree.
   */
  size_t countMessages() const {
    return this->countLeaves() / 2;
  }

  /**
   * Balance the tree by copying the leftmost leaf/node to make each layer
   * contain even # of leaves/nodes.
   */
  void finalizeTree() {
    if (this->nextRightmostLeaf_ > 1) {
      // compensate the layers with odd # of nodes (except the root)
      int curRightmostIndex = this->nextRightmostLeaf_ - 1;
      for (size_t i = 0; i < this->rootHeight_; i++) {
        if (curRightmostIndex % 2 == 0) {
          // need compensate
          auto leftmostSibling = this->tree_[std::make_tuple(i, 0)];
          this->addNode(std::move(leftmostSibling), i, curRightmostIndex + 1);
        }
        curRightmostIndex /= 2;
      }
    }
  }

  /**
   * Construct an internal node from children and a prefix 0x01.
   *
   * Hash(0x01 || leftChild || rightChild)
   */
  static std::array<uint8_t, Hash::HashLen> constructInternalNode(
      folly::ByteRange leftChild,
      folly::ByteRange rightChild) {
    // ensure the children are of right size
    DCHECK(leftChild.size() == Hash::HashLen);
    DCHECK(rightChild.size() == Hash::HashLen);

    std::array<uint8_t, Hash::HashLen> result;
    Hash hasher;
    hasher.hash_init();
    constexpr std::array<uint8_t, 1> prefix = {0x01};
    hasher.hash_update(folly::range(prefix));
    hasher.hash_update(leftChild);
    hasher.hash_update(rightChild);
    hasher.hash_final(folly::range(result));
    return result;
  }

  /**
   * Construct a leaf node from the message and a prefix 0x00.
   *
   * Hash(0x00 || msg)
   */
  static std::array<uint8_t, Hash::HashLen> constructLeafNode(
      folly::ByteRange msg) {
    std::array<uint8_t, Hash::HashLen> result;
    Hash hasher;
    hasher.hash_init();
    constexpr std::array<uint8_t, 1> prefix = {0x00};
    hasher.hash_update(folly::range(prefix));
    hasher.hash_update(msg);
    hasher.hash_final(folly::range(result));
    return result;
  }
};

} // namespace fizz
