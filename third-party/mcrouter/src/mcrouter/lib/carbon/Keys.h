/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>
#include <string>
#include <type_traits>

#include <folly/Range.h>
#include <folly/hash/SpookyHashV2.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/Thrift.h>

#include "mcrouter/lib/HashFunctionType.h"

namespace carbon {

template <class Storage>
Storage makeKey(folly::StringPiece sp);

template <>
inline std::string makeKey<std::string>(folly::StringPiece sp) {
  return sp.str();
}

template <>
inline folly::IOBuf makeKey<folly::IOBuf>(folly::StringPiece sp) {
  return folly::IOBuf(folly::IOBuf::COPY_BUFFER, sp.data(), sp.size());
}

/**
 * Holds all the references to the various parts of the key.
 *
 *                        /region/cluster/foo:key|#|etc
 * keyData_:              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * keyWithoutRoute:                       ^^^^^^^^^^^^^
 * routingPrefix:         ^^^^^^^^^^^^^^^^
 * routingKey:                            ^^^^^^^
 * afterRoutingKey:                              ^^^^^^
 */
template <class Storage>
class Keys {
 public:
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(rawUnsafe());

  constexpr Keys() = default;

  explicit Keys(Storage&& key) noexcept : key_(std::move(key)) {
    update();
  }

  explicit Keys(folly::StringPiece sp) : key_(makeKey<Storage>(sp)) {
    update();
  }

  explicit Keys(const char* key) : key_(makeKey<Storage>(key)) {
    update();
  }

  Keys(const Keys& other);
  Keys& operator=(const Keys& other);
  Keys(Keys&& other) noexcept;
  Keys& operator=(Keys&& other) noexcept;

  Keys& operator=(const Storage& key) {
    key_ = key;
    update();
    return *this;
  }

  Keys& operator=(Storage&& key) noexcept {
    key_ = std::move(key);
    update();
    return *this;
  }

  Keys& operator=(folly::StringPiece key) {
    key_ = makeKey<Storage>(key);
    update();
    return *this;
  }

  Keys& operator=(const char* key) {
    *this = folly::StringPiece(key);
    return *this;
  }

  size_t size() const {
    return size(key_);
  }

  bool empty() const {
    return key_.empty();
  }

  folly::StringPiece fullKey() const {
    return {reinterpret_cast<const char*>(key_.data()), size()};
  }
  folly::StringPiece keyWithoutRoute() const {
    return keyWithoutRoute_;
  }
  folly::StringPiece routingPrefix() const {
    return routingPrefix_;
  }
  folly::StringPiece routingKey() const {
    return routingKey_;
  }

  folly::StringPiece afterRoutingKey() const {
    return afterRoutingKey_;
  }

  uint32_t routingKeyHash() const {
    if (!routingKeyHash_) {
      const auto keyPiece = routingKey();
      routingKeyHash_ = folly::hash::SpookyHashV2::Hash32(
          keyPiece.begin(), keyPiece.size(), /* seed= */ 0);
    }
    return routingKeyHash_;
  }

  bool hasHashStop() const {
    return routingKey_.size() != keyWithoutRoute_.size();
  }

  // Hack to save some CPU in DestinationRoute. Avoid if possible.
  void stripRoutingPrefix() {
    trimStart(routingPrefix().size());
    routingPrefix_.reset(fullKey().begin(), 0);
  }

  // TODO(jmswen) Would be nice not to expose raw storage. Only needed in
  // asciiKey() in McServerSession-inl.h and SerializationTraits specialization.
  const Storage& raw() const {
    return key_;
  }

  // Usage of this method requires user to call `update()` manually on change of
  // the underlying storage.
  Storage& rawUnsafe() {
    return key_;
  }
  const Storage& rawUnsafe() const {
    return key_;
  }

  void update();

  bool reuseLastHash(size_t size, HashFunctionType typeId) const {
    return (
        lastHash_.size_ > 0 && size == lastHash_.size_ &&
        typeId == lastHash_.typeId_);
  }

  size_t getLastHash() const {
    return lastHash_.hash_;
  }

  void setLastHash(size_t hash, size_t size, HashFunctionType typeId) const {
    lastHash_.hash_ = hash;
    lastHash_.size_ = size;
    lastHash_.typeId_ = typeId;
  }

 private:
  static constexpr bool usingStringStorage =
      std::is_same<Storage, std::string>::value;

  // Assumes that this->key_ has been set to the desired value that StringPiece
  // members of *this should point into.
  void initStringPieces(const Keys& other) {
    if (usingStringStorage &&
        reinterpret_cast<const char*>(key_.data()) !=
            other.routingPrefix().begin()) {
      update();
    } else {
      copyStringPieces(other);
    }
  }

  void copyStringPieces(const Keys& other) {
    keyWithoutRoute_ = other.keyWithoutRoute_;
    routingPrefix_ = other.routingPrefix_;
    routingKey_ = other.routingKey_;
    afterRoutingKey_ = other.afterRoutingKey_;
    routingKeyHash_ = other.routingKeyHash_;
    lastHash_.size_ = other.lastHash_.size_;
    lastHash_.hash_ = other.lastHash_.hash_;
  }

  static size_t size(const folly::IOBuf& buf) {
    return buf.length();
  }
  static size_t size(const std::string& str) {
    return str.size();
  }

  void trimStart(size_t n) {
    return trimStartImpl(key_, n);
  }

  static void trimStartImpl(folly::IOBuf& buf, size_t n) {
    buf.trimStart(n);
  }

  static void trimStartImpl(std::string& s, size_t n) {
    s.erase(0, n);
  }

 protected:
  Storage key_;

 private:
  folly::StringPiece keyWithoutRoute_;
  folly::StringPiece routingPrefix_;
  folly::StringPiece routingKey_;
  folly::StringPiece afterRoutingKey_;
  mutable uint32_t routingKeyHash_{0};

  struct HashData {
    size_t size_{0};
    size_t hash_{0};
    HashFunctionType typeId_{HashFunctionType::Unknown};
  };
  mutable HashData lastHash_;
};

} // namespace carbon

#include "Keys-inl.h"
