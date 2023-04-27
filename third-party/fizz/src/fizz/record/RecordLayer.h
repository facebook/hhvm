/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/protocol/Params.h>
#include <fizz/record/Types.h>
#include <folly/Optional.h>
#include <folly/io/IOBufQueue.h>

namespace fizz {

struct TLSContent {
  Buf data;
  ContentType contentType;
  EncryptionLevel encryptionLevel;
};

/**
 * RecordLayerState contains the state of the record layer -- all data
 * that is needed in order to decrypt/encrypt the _next_ record from the wire/
 * from the application.
 */
struct RecordLayerState {
  folly::Optional<TrafficKey> key;
  folly::Optional<uint64_t> sequence;
};

class ReadRecordLayer {
 public:
  template <class T>
  struct ReadResult {
    ReadResult() : message(folly::none), sizeHint{0} {}
    /* implicit */ ReadResult(const folly::None&)
        : message(folly::none), sizeHint{0} {}

    folly::Optional<T> message;

    // A non-zero size hint indicates the amount of bytes needed to continue
    // record processing.
    size_t sizeHint{0};

    static ReadResult from(T&& t) {
      return from(std::forward<T>(t), 0);
    }

    static ReadResult from(T&& t, size_t sizeHint) {
      ReadResult r;
      r.message = std::forward<T>(t);
      r.sizeHint = sizeHint;
      return r;
    }

    static ReadResult none() {
      return noneWithSizeHint(0);
    }

    static ReadResult noneWithSizeHint(size_t s) {
      ReadResult r;
      r.sizeHint = s;
      return r;
    }

    operator bool() const {
      return bool(message);
    }

    [[nodiscard]] bool has_value() const {
      return message.has_value();
    }

    auto operator->() -> decltype(this->message.operator->()) {
      return message.operator->();
    }

    auto operator->() const -> decltype(this->message.operator->()) {
      return message.operator->();
    }
    auto operator*() -> decltype(this->message.operator*()) {
      return message.operator*();
    }
  };

  virtual ~ReadRecordLayer() = default;

  /**
   * Reads a fragment from the record layer. Returns an empty optional if
   * insuficient data available. Throws if data malformed. On success, advances
   * buf the amount read.
   */
  virtual ReadResult<TLSMessage> read(
      folly::IOBufQueue& buf,
      Aead::AeadOptions options) = 0;

  /**
   * Get a message from the record layer. Returns none if insufficient data was
   * available on the socket. Throws on parse error.
   */
  virtual ReadResult<Param> readEvent(
      folly::IOBufQueue& socketBuf,
      Aead::AeadOptions options);

  /**
   * Check if there is decrypted but unparsed handshake data buffered.
   */
  virtual bool hasUnparsedHandshakeData() const;

  /**
   * Returns the current encryption level of the data that the read record layer
   * can process.
   */
  virtual EncryptionLevel getEncryptionLevel() const = 0;

  /**
   * Returns a snapshot of the state of the record layer.
   *
   * `key`, if set, indicates the keying parameters for the AEAD associated
   * with this ReadRecordLayer.
   *
   * `sequence`, if set, indicates the sequence number of the next expected
   * record to be read.
   */
  virtual RecordLayerState getRecordLayerState() const {
    return RecordLayerState{};
  }

  static folly::Optional<Param> decodeHandshakeMessage(folly::IOBufQueue& buf);

 private:
  folly::IOBufQueue unparsedHandshakeData_{
      folly::IOBufQueue::cacheChainLength()};
};

class WriteRecordLayer {
 public:
  virtual ~WriteRecordLayer() = default;

  virtual TLSContent write(TLSMessage&& msg, Aead::AeadOptions options)
      const = 0;

  TLSContent writeAlert(Alert&& alert) const {
    return write(
        TLSMessage{ContentType::alert, encode(std::move(alert))},
        Aead::AeadOptions());
  }

  TLSContent writeAppData(
      std::unique_ptr<folly::IOBuf>&& appData,
      Aead::AeadOptions options) const {
    return write(
        TLSMessage{ContentType::application_data, std::move(appData)}, options);
  }

  template <typename... Args>
  TLSContent writeHandshake(Buf&& encodedHandshakeMsg, Args&&... args) const {
    TLSMessage msg{ContentType::handshake, std::move(encodedHandshakeMsg)};
    addMessage(msg.fragment, std::forward<Args>(args)...);
    return write(std::move(msg), Aead::AeadOptions());
  }

  void setProtocolVersion(ProtocolVersion version) const {
    auto realVersion = getRealDraftVersion(version);
    if (realVersion == ProtocolVersion::tls_1_3_23) {
      useAdditionalData_ = false;
    } else {
      useAdditionalData_ = true;
    }
  }

  /**
   * Returns the current encryption level of the data that the write record
   * layer writes at.
   */
  virtual EncryptionLevel getEncryptionLevel() const = 0;

  /**
   * Returns a snapshot of the state of the record layer.
   *
   * `key`, if set, indicates the keying parameters for the AEAD associated
   * with this WriteRecordLayer.
   *
   * `sequence`, if set, indicates the sequence number of the next expected
   * record to be written.
   */
  virtual RecordLayerState getRecordLayerState() const {
    return RecordLayerState{};
  }

 protected:
  mutable bool useAdditionalData_{true};

 private:
  template <typename... Args>
  static void addMessage(Buf& buf, Buf&& add, Args&&... args) {
    buf->prependChain(std::move(add));
    addMessage(buf, std::forward<Args>(args)...);
  }

  static void addMessage(Buf& /*buf*/) {}
};
} // namespace fizz
