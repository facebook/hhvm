/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/record/BufAndPaddingPolicy.h>
#include <fizz/record/RecordLayer.h>
#include <fizz/util/Logging.h>

namespace fizz {

constexpr size_t kMaxPlaintextRecordSize = 0x4000; // 16k

class EncryptedReadRecordLayer : public ReadRecordLayer {
 public:
  ~EncryptedReadRecordLayer() override = default;

  explicit EncryptedReadRecordLayer(EncryptionLevel encryptionLevel)
      : encryptionLevel_(encryptionLevel) {}

  Status read(
      ReadResult<TLSMessage>& ret,
      Error& err,
      folly::IOBufQueue& buf,
      Aead::AeadOptions options) override;

  virtual Status setAead(
      Error& err,
      folly::ByteRange /* baseSecret */,
      std::unique_ptr<Aead> aead) {
    if (seqNum_ != 0) {
      return err.error("aead set after read");
    }
    aead_ = std::move(aead);
    return Status::Success;
  }

  virtual void setSkipFailedDecryption(bool enabled) {
    skipFailedDecryption_ = enabled;
  }

  void setSequenceNumber(uint64_t seq) {
    seqNum_ = seq;
  }

  Status setProtocolVersion(Error& err, ProtocolVersion version) {
    ProtocolVersion realVersion;
    FIZZ_RETURN_ON_ERROR(getRealDraftVersion(realVersion, err, version));
    if (realVersion == ProtocolVersion::tls_1_3_23) {
      useAdditionalData_ = false;
    } else {
      useAdditionalData_ = true;
    }
    return Status::Success;
  }

  EncryptionLevel getEncryptionLevel() const override;

  RecordLayerState getRecordLayerState() const override {
    auto key = [&]() -> folly::Optional<TrafficKey> {
      if (!aead_) {
        return folly::none;
      }
      return aead_->getKey();
    }();

    auto sequence = seqNum_;

    return RecordLayerState{std::move(key), sequence};
  }

 private:
  Status getDecryptedBuf(
      ReadResult<Buf>& ret,
      Error& err,
      folly::IOBufQueue& buf,
      Aead::AeadOptions options);

  EncryptionLevel encryptionLevel_;
  std::unique_ptr<Aead> aead_;
  mutable uint64_t seqNum_{0};

  bool skipFailedDecryption_{false};
  bool useAdditionalData_{true};
};

class EncryptedWriteRecordLayer : public WriteRecordLayer {
 public:
  ~EncryptedWriteRecordLayer() override = default;

  explicit EncryptedWriteRecordLayer(EncryptionLevel encryptionLevel)
      : encryptionLevel_(encryptionLevel) {}

  Status write(
      TLSContent& ret,
      Error& err,
      TLSMessage&& msg,
      Aead::AeadOptions options) const override;

  virtual Status setAead(
      Error& err,
      folly::ByteRange /* baseSecret */,
      std::unique_ptr<Aead> aead) {
    if (seqNum_ != 0) {
      return err.error("aead set after write");
    }
    aead_ = std::move(aead);
    return Status::Success;
  }

  virtual void setBufAndPaddingPolicy(
      std::unique_ptr<const BufAndPaddingPolicy> bufAndPaddingPolicy) {
    bufAndPaddingPolicy_ = std::move(bufAndPaddingPolicy);
  }

  void setMaxRecord(uint16_t size) {
    FIZZ_CHECK_GT(size, 0);
    FIZZ_DCHECK_LE(size, kMaxPlaintextRecordSize);
    maxRecord_ = size;
  }

  void setSequenceNumber(uint64_t seq) {
    seqNum_ = seq;
  }

  EncryptionLevel getEncryptionLevel() const override;

  RecordLayerState getRecordLayerState() const override {
    auto key = [&]() -> folly::Optional<TrafficKey> {
      if (!aead_) {
        return folly::none;
      }
      return aead_->getKey();
    }();

    auto sequence = seqNum_;

    return RecordLayerState{std::move(key), sequence};
  }

 private:
  EncryptionLevel encryptionLevel_;
  std::unique_ptr<Aead> aead_;
  std::unique_ptr<const BufAndPaddingPolicy> bufAndPaddingPolicy_{
      std::make_unique<BufAndConstPaddingPolicy>(0)};
  mutable uint64_t seqNum_{0};

  uint16_t maxRecord_{kMaxPlaintextRecordSize};
};
} // namespace fizz
