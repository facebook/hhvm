/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/util/Status.h>
#include <folly/Optional.h>
#include <folly/io/IOBuf.h>

namespace fizz {

struct TrafficKey {
  std::unique_ptr<folly::IOBuf> key;
  std::unique_ptr<folly::IOBuf> iv;

  TrafficKey clone() const {
    return TrafficKey{key->clone(), iv->clone()};
  }
};

/**
 * Interface for aead algorithms (RFC 5116).
 */
class Aead {
 public:
  enum class BufferOption {
    RespectSharedPolicy, // Assume shared = no in-place
    AllowInPlace, // Assume in-place editing is safe
    AllowFullModification, // Assume in-place editing and growing into
                           // head/tailroom are safe.
  };

  enum class AllocationOption {
    Allow, // Allow allocating new buffers
    Deny, // Disallow allocating new buffers
  };

  struct AeadOptions {
    BufferOption bufferOpt = BufferOption::RespectSharedPolicy;
    AllocationOption allocOpt = AllocationOption::Allow;
  };

  virtual ~Aead() = default;

  /**
   * Returns the number of key bytes needed by this aead.
   */
  virtual size_t keyLength() const = 0;

  /**
   * Returns the number of iv bytes needed by this aead.
   */
  virtual size_t ivLength() const = 0;

  /**
   * Sets the key and iv for this aead. The length of the key and iv must match
   * keyLength() and ivLength().
   */
  virtual Status setKey(Error& err, TrafficKey key) = 0;

  /**
   * Retrieves a shallow copy (IOBuf cloned) version of the TrafficKey
   * corresponding to this AEAD, if set. Otherwise, returns none.
   */
  virtual folly::Optional<TrafficKey> getKey() const = 0;

  /**
   * Encrypts plaintext. Will throw on error.
   *
   * Uses BufferOption::RespectSharedPolicy and AllocationOption::Allow by
   * default.
   */
  Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const {
    return encrypt(
        ret,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(plaintext),
        associatedData,
        seqNum,
        {BufferOption::RespectSharedPolicy, AllocationOption::Allow});
  }

  /**
   * Encrypts plaintext with nonce passed explicitly by the caller. Will throw
   * on error.
   *
   * Uses BufferOption::RespectSharedPolicy and AllocationOption::Allow by
   * default.
   */
  Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce) const {
    return encrypt(
        ret,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(plaintext),
        associatedData,
        nonce,
        {BufferOption::RespectSharedPolicy, AllocationOption::Allow});
  }

  /**
   * `encrypt` performs authenticated encryption.
   *
   *  This version of encrypt generates the nonce used for
   *  encryption using the TLS record number to nonce construction
   *  as specified in RFC 8446.
   */
  virtual Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      AeadOptions options) const = 0;

  /**
   * `encrypt` performs authenticated encryption.
   *
   * This version of encrypt uses a nonce passed in explicitly
   * by the caller; consequently, this interface can be used
   * to perform AEAD outside of a TLS specific application.
   */
  virtual Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const = 0;

  /**
   * Version of encrypt which is guaranteed to be inplace. Will throw an
   * exception if the inplace encryption cannot be done.
   *
   * Equivalent of calling encrypt() with BufferOption::AllowFullModification
   * and AllocationOption::Deny.
   */
  virtual Status inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const = 0;

  /**
   * Set a hint to the AEAD about how much space to try to leave as headroom for
   * ciphertexts returned from encrypt.  Implementations may or may not honor
   * this.
   */
  virtual void setEncryptedBufferHeadroom(size_t headroom) = 0;

  /**
   * Decrypt ciphertext. Will throw if the ciphertext does not decrypt
   * successfully.
   *
   * Uses BufferOption::RespectSharedPolicy and AllocationOption::Allow by
   * default.
   */
  Status decrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const {
    return decrypt(
        ret,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(ciphertext),
        associatedData,
        seqNum,
        {BufferOption::RespectSharedPolicy, AllocationOption::Allow});
  }

  /**
   * Decrypt ciphertext. Will throw if the ciphertext does not decrypt
   * successfully.
   *
   * This version of decrypt uses a nonce passed in explicitly
   * by the caller; consequently, this interface can be used
   * to perform AEAD outside of a TLS specific application.
   *
   * Uses BufferOption::RespectSharedPolicy and AllocationOption::Allow by
   * default.
   */
  Status decrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce) const {
    return decrypt(
        ret,
        err,
        std::move(ciphertext),
        associatedData,
        nonce,
        {BufferOption::RespectSharedPolicy, AllocationOption::Allow});
  }

  virtual Status decrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      AeadOptions options) const {
    folly::Optional<std::unique_ptr<folly::IOBuf>> plaintext;
    FIZZ_RETURN_ON_ERROR(tryDecrypt(
        plaintext,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(ciphertext),
        associatedData,
        seqNum,
        options));
    if (!plaintext) {
      return err.error("decryption failed");
    }
    ret = std::move(*plaintext);
    return Status::Success;
  }

  virtual Status decrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const {
    folly::Optional<std::unique_ptr<folly::IOBuf>> plaintext;
    FIZZ_RETURN_ON_ERROR(tryDecrypt(
        plaintext,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(ciphertext),
        associatedData,
        nonce,
        options));
    if (!plaintext) {
      return err.error("decryption failed");
    }
    ret = std::move(*plaintext);
    return Status::Success;
  }

  /**
   * Decrypt ciphertext. Will return none if the ciphertext does not decrypt
   * successfully. May still throw from errors unrelated to ciphertext.
   *
   * Uses BufferOption::RespectSharedPolicy and AllocationOption::Allow by
   * default.
   */
  Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const {
    return tryDecrypt(
        ret,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(ciphertext),
        associatedData,
        seqNum,
        {BufferOption::RespectSharedPolicy, AllocationOption::Allow});
  }

  /**
   * Decrypt ciphertext. Will return none if the ciphertext does not decrypt
   * successfully. May still throw from errors unrelated to ciphertext.
   *
   * This version of tryDecrypt uses a nonce passed in explicitly
   * by the caller; consequently, this interface can be used
   * to perform AEAD outside of a TLS specific application.
   *
   * Uses BufferOption::RespectSharedPolicy and AllocationOption::Allow by
   * default.
   */
  Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce) const {
    return tryDecrypt(
        ret,
        err,
        std::forward<std::unique_ptr<folly::IOBuf>>(ciphertext),
        associatedData,
        nonce,
        {BufferOption::RespectSharedPolicy, AllocationOption::Allow});
  }

  virtual Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      AeadOptions options) const = 0;

  virtual Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const = 0;

  /**
   * Returns the number of bytes the aead will add to the plaintext (size of
   * ciphertext - size of plaintext).
   */
  virtual size_t getCipherOverhead() const = 0;
};
} // namespace fizz
