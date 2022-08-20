/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/uio.h>

#include <type_traits>
#include <utility>

#include <folly/Optional.h>
#include <folly/Varint.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Bits.h>

namespace folly {
class IOBuf;
} // namespace folly

namespace carbon {

class CarbonQueueAppenderStorage {
 public:
  CarbonQueueAppenderStorage() {
    iovs_[0] = {embeddedStorage_.data(), 0};
  }

  CarbonQueueAppenderStorage(const CarbonQueueAppenderStorage&) = delete;
  CarbonQueueAppenderStorage& operator=(const CarbonQueueAppenderStorage&) =
      delete;

  void append(const folly::IOBuf& buf) {
    // IOBuf copy is a very expensive procedure (64 bytes object + atomic
    // operation), avoid incuring that cost for small buffers.
    if (!buf.empty() && !buf.isChained() && buf.length() <= kInlineIOBufLen &&
        storageIdx_ + buf.length() <= sizeOfStorage()) {
      push(buf.data(), buf.length());
      return;
    }

    finalizeLastIovec();

    if (nIovsUsed_ == kMaxIovecs) {
      coalesce();
    }

    assert(nIovsUsed_ < kMaxIovecs);

    struct iovec* nextIov = iovs_.data() + nIovsUsed_;
    const auto nFilled =
        buf.fillIov(nextIov, kMaxIovecs - nIovsUsed_).numIovecs;

    if (tcpZeroCopyThreshold_ && !applyZeroCopy_ &&
        buf.capacity() >= tcpZeroCopyThreshold_) {
      applyZeroCopy_ = true;
    }

    if (nFilled > 0) {
      nIovsUsed_ += nFilled;
      if (!head_) {
        head_ = buf;
      } else {
        head_->prependChain(buf.clone());
      }
    } else {
      if (buf.empty()) {
        return;
      }
      appendSlow(buf);
    }

    // If a push() comes after, it should not use the iovec we just filled in
    canUsePreviousIov_ = false;
  }

  void push(const uint8_t* buf, size_t len) {
    if (storageIdx_ + len > sizeOfStorage()) {
      growStorage(len);
    }

    if (nIovsUsed_ == kMaxIovecs) {
      // In this case, it would be possible to use the last iovec if
      // canUsePreviousIov_ is true, but we simplify logic by foregoing this
      // optimization.
      coalesce();
    }
    assert(nIovsUsed_ < kMaxIovecs);

    if (!canUsePreviousIov_) {
      // Note, we will be updating iov_len once we're done with this iovec,
      // i.e. in finalizeLastIovec()
      iovs_[nIovsUsed_++].iov_base = &storage()[storageIdx_];

      // If the next push() comes before the next append(), and if we still
      // have room left in storage,  then we can just extend the last iovec
      // used since we will write to storage where we left off.
      canUsePreviousIov_ = true;
    }

    std::memcpy(&storage()[storageIdx_], buf, len);
    storageIdx_ += len;
  }

  void coalesce();

  bool setFullBuffer(const folly::IOBuf& buf) {
    struct iovec* nextIov = iovs_.data() + nIovsUsed_;
    const auto nFilled =
        buf.fillIov(nextIov, kMaxIovecs - nIovsUsed_).numIovecs;

    if (nFilled > 0) {
      nIovsUsed_ += nFilled;
      return true;
    }
    return false;
  }

  void reset() {
    storageIdx_ = kMaxHeaderLength;
    head_.reset();
    iobufStorage_.reset();
    lastStorageSize_ = kInitMsgStoreLen;
    // Reserve first element of iovs_ for header, which won't be filled in
    // until after body data is serialized.
    iovs_[0] = {embeddedStorage_.data(), 0};
    nIovsUsed_ = 1;
    canUsePreviousIov_ = false;
    headerOverlap_ = 0;
    applyZeroCopy_ = false;
  }

  std::pair<const struct iovec*, size_t> getIovecs() {
    finalizeLastIovec();

    // First iovec optimization. Merge header into first iovec if currently
    // using the embedded storage, and have headroom available.
    if (nIovsUsed_ > 1 &&
        iovs_[1].iov_base == (embeddedStorage_.data() + kMaxHeaderLength)) {
      auto headerSize = iovs_[0].iov_len;
      iovs_[1].iov_base =
          embeddedStorage_.data() + (kMaxHeaderLength - headerSize);
      memmove(iovs_[1].iov_base, embeddedStorage_.data(), headerSize);
      iovs_[1].iov_len += headerSize;
      iovs_[0].iov_len = 0;
      headerOverlap_ = headerSize;
    }

    return iovs_[0].iov_len == 0
        ? std::make_pair(iovs_.data() + 1, nIovsUsed_ - 1)
        : std::make_pair(iovs_.data(), nIovsUsed_);
  }

  size_t computeBodySize() {
    finalizeLastIovec();
    size_t bodySize = 0;
    // Skip iovs_[0], which refers to message header
    for (size_t i = 1; i < nIovsUsed_; ++i) {
      bodySize += iovs_[i].iov_len;
    }
    return bodySize - headerOverlap_;
  }

  // Hack: we expose header buffer so users can write directly to it.
  // It is the responsibility of the user to report how much data was written
  // via reportHeaderSize().
  uint8_t* getHeaderBuf() {
    assert(iovs_[0].iov_base == embeddedStorage_.data());
    return embeddedStorage_.data();
  }

  void reportHeaderSize(size_t headerSize) {
    iovs_[0].iov_len = headerSize;
  }

  // If an IOBuf exceeds this size threshold, zero copy will be enabled
  void setTCPZeroCopyThreshold(size_t threshold) {
    tcpZeroCopyThreshold_ = threshold;
  }

  bool shouldApplyZeroCopy() const {
    return applyZeroCopy_;
  }

 private:
  static constexpr size_t kMaxIovecs{32};
  static constexpr size_t kInlineIOBufLen{128};
  static constexpr size_t kInitMsgStoreLen{512};

  static constexpr size_t kMaxAdditionalFields = 6;
  static constexpr size_t kMaxHeaderLength = 1 /* magic byte */ +
      1 /* GroupVarint header (lengths of 4 ints) */ +
      4 * sizeof(uint32_t) /* body size, typeId, reqId, num addl fields */ +
      2 * kMaxAdditionalFields *
          folly::kMaxVarintLength64; /* key and value for additional fields */

  size_t storageIdx_{kMaxHeaderLength};
  size_t lastStorageSize_{kInitMsgStoreLen};
  size_t nIovsUsed_{1};
  size_t headerOverlap_{0};
  bool canUsePreviousIov_{false};
  bool applyZeroCopy_{false};
  size_t tcpZeroCopyThreshold_{0};

  // Buffer used for non-IOBuf data, e.g., ints, strings, and protocol
  // data. The buffer can be a static small buffer embeded in here (i.e.
  // embeddedStorage) or a dynamically allocated buffer which grows as needed
  // (i.e. iobufStorage_).
  //
  // The appender provide two interfaces: push & append. Push accept a pointer
  // and length and can utilize these storage buffers while building iovecs.
  // Append accepts an IOBuf and is placed directly into a new iovec using
  // the fillIov method.
  //
  // It is possible for multiple iovecs to point into a storage buffer in
  // case the buffer has more space, and push() calls are intermingled with
  // append() calls.
  std::array<uint8_t, kInitMsgStoreLen + kMaxHeaderLength> embeddedStorage_;

  // Once static embded storage runs out extend the storage with dynamically
  // growing iobuf(s). This is especially important for very large message body
  // which is composed over many small push() calls. (e.g. large vector)
  std::unique_ptr<folly::IOBuf> iobufStorage_;

  // The first iovec in iovs_ points to Caret message header data, and nothing
  // else. The remaining iovecs are used for the message body. Note that we do
  // not share iovs_[0] with body data, even if it would be possible, e.g., we
  // do not append the CT_STRUCT (struct beginning delimiter) to iovs_[0].
  std::array<struct iovec, kMaxIovecs> iovs_;

  // Chain of IOBufs used for IOBuf fields, like key and value. Note that we
  // also maintain views into this data via iovs_.
  folly::Optional<folly::IOBuf> head_;

  void growStorage(size_t len) {
    // Switching over to new storage, finalize current iovec used.
    finalizeLastIovec();
    canUsePreviousIov_ = false;

    // We ran out of size before, so start with at least 2x the previous size.
    do {
      lastStorageSize_ = lastStorageSize_ * 2;
    } while (lastStorageSize_ < len);

    auto buf = folly::IOBuf::createCombined(lastStorageSize_);
    if (!iobufStorage_) {
      iobufStorage_ = std::move(buf);
    } else {
      iobufStorage_->prependChain(std::move(buf));
    }

    resetStorageIdx();
  }

  std::size_t sizeOfStorage() {
    if (iobufStorage_) {
      assert(iobufStorage_->prev()->capacity() >= lastStorageSize_);
      return lastStorageSize_;
    }
    return sizeof(embeddedStorage_);
  }

  uint8_t* storage() {
    if (iobufStorage_) {
      return const_cast<uint8_t*>(iobufStorage_->prev()->data());
    }
    return embeddedStorage_.data();
  }

  void resetStorageIdx() {
    storageIdx_ = kMaxHeaderLength;
    if (iobufStorage_) {
      // We aren't merging the header into dynamic storage, so index can point
      // to the first byte.
      storageIdx_ = 0;
    }
  }

  FOLLY_NOINLINE void appendNoInline(const folly::IOBuf& buf) {
    append(buf);
  }

  FOLLY_NOINLINE void appendSlow(const folly::IOBuf& buf) {
    struct iovec* nextIov = iovs_.data() + nIovsUsed_;
    auto bufCopy = buf;
    bufCopy.coalesce();
    const auto nFilledRetry =
        bufCopy.fillIov(nextIov, kMaxIovecs - nIovsUsed_).numIovecs;
    assert(nFilledRetry == 1);
    (void)nFilledRetry;
    ++nIovsUsed_;
    if (!head_) {
      head_ = std::move(bufCopy);
    } else {
      head_->prependChain(bufCopy.clone());
    }
  }

  void finalizeLastIovec() {
    if (canUsePreviousIov_) {
      auto& iov = iovs_[nIovsUsed_ - 1];
      iov.iov_len =
          &storage()[storageIdx_] - static_cast<const uint8_t*>(iov.iov_base);
    }
  }
};

/**
 * Mcrouter's own implementation of folly's QueueAppender.  CarbonQueueAppender
 * implements the portion of the folly::io::QueueAppender interface needed by
 * carbon::CarbonProtocolWriter.
 * We have our own version of QueueAppender in order to support more efficient
 * memory management for mcrouter's use case.
 */
template <class TS = CarbonQueueAppenderStorage>
class CarbonQueueAppender {
 public:
  CarbonQueueAppender(TS* storage, uint64_t unused) {
    reset(storage, unused);
  }

  template <class T>
  typename std::enable_if<std::is_arithmetic<T>::value>::type write(T value) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    push(bytes, sizeof(T));
  }

  template <class T>
  void writeBE(T value) {
    write(folly::Endian::big(value));
  }

  template <class T>
  void writeLE(T value) {
    write(folly::Endian::little(value));
  }

  void reset(TS* storage, uint64_t /* unused */) {
    storage_ = storage;
  }

  // The user is responsible for ensuring storage_ is valid before calling
  // push() or insert()
  void push(const uint8_t* buf, size_t len) {
    assert(storage_);
    storage_->push(buf, len);
  }

  void insert(std::unique_ptr<folly::IOBuf> buf) {
    assert(storage_);
    assert(buf);
    storage_->append(*buf);
  }

  void insert(const folly::IOBuf& buf) {
    assert(storage_);
    storage_->append(buf);
  }

 private:
  TS* storage_{nullptr};
};

} // namespace carbon
