/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include <fizz/protocol/Types.h>
#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/record/PlaintextRecordLayer.h>

namespace fizz {

/* using override */
using namespace testing;

template <typename T>
void setWriteDefaults(T* obj) {
  ON_CALL(*obj, _write(_, _))
      .WillByDefault(Invoke([obj](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = obj->getEncryptionLevel();

        if (msg.type == ContentType::application_data) {
          content.data = folly::IOBuf::copyBuffer("appdata");
        } else if (msg.type == ContentType::handshake) {
          content.data = folly::IOBuf::copyBuffer("handshake");
        } else if (msg.type == ContentType::alert) {
          auto buf = folly::IOBuf::copyBuffer("alert");
          buf->prependChain(std::move(msg.fragment));
          buf->coalesce();
          content.data = std::move(buf);
        } else {
          content.data = std::unique_ptr<folly::IOBuf>();
        }
        return content;
      }));
}

class MockPlaintextReadRecordLayer : public PlaintextReadRecordLayer {
 public:
  MOCK_METHOD(
      ReadResult<TLSMessage>,
      read,
      (folly::IOBufQueue & buf, Aead::AeadOptions));
  MOCK_METHOD(bool, hasUnparsedHandshakeData, (), (const));
  MOCK_METHOD(void, setSkipEncryptedRecords, (bool));
  MOCK_METHOD(ReadResult<Param>, mockReadEvent, ());

  ReadResult<Param> readEvent(folly::IOBufQueue& buf, Aead::AeadOptions options)
      override {
    if (useMockReadEvent_) {
      return mockReadEvent();
    } else {
      return PlaintextReadRecordLayer::readEvent(buf, options);
    }
  }

  void useMockReadEvent(bool b) {
    useMockReadEvent_ = b;
  }

 private:
  bool useMockReadEvent_{false};
};

class MockEncryptedReadRecordLayer : public EncryptedReadRecordLayer {
 public:
  explicit MockEncryptedReadRecordLayer(EncryptionLevel encryptionLevel)
      : EncryptedReadRecordLayer(encryptionLevel) {}

  MOCK_METHOD(
      ReadResult<TLSMessage>,
      read,
      (folly::IOBufQueue & buf, Aead::AeadOptions options));
  MOCK_METHOD(bool, hasUnparsedHandshakeData, (), (const));

  MOCK_METHOD(void, _setAead, (folly::ByteRange, Aead*));
  void setAead(folly::ByteRange baseSecret, std::unique_ptr<Aead> aead)
      override {
    _setAead(baseSecret, aead.get());
  }

  MOCK_METHOD(void, setSkipFailedDecryption, (bool));
  MOCK_METHOD(ReadResult<Param>, mockReadEvent, ());

  ReadResult<Param> readEvent(folly::IOBufQueue& buf, Aead::AeadOptions options)
      override {
    if (useMockReadEvent_) {
      return mockReadEvent();
    } else {
      return EncryptedReadRecordLayer::readEvent(buf, options);
    }
  }

  void useMockReadEvent(bool b) {
    useMockReadEvent_ = b;
  }

 private:
  bool useMockReadEvent_{false};
};

class MockPlaintextWriteRecordLayer : public PlaintextWriteRecordLayer {
 public:
  MOCK_METHOD(
      TLSContent,
      _write,
      (TLSMessage & msg, Aead::AeadOptions options),
      (const));
  TLSContent write(TLSMessage&& msg, Aead::AeadOptions options) const override {
    return _write(msg, options);
  }

  MOCK_METHOD(TLSContent, _writeInitialClientHello, (Buf&), (const));
  TLSContent writeInitialClientHello(Buf encoded) const override {
    return _writeInitialClientHello(encoded);
  }

  void setDefaults() {
    setWriteDefaults(this);
    ON_CALL(*this, _writeInitialClientHello(_))
        .WillByDefault(InvokeWithoutArgs([]() {
          TLSContent record;
          record.contentType = ContentType::handshake;
          record.data = folly::IOBuf::copyBuffer("handshake");
          record.encryptionLevel = EncryptionLevel::Plaintext;
          return record;
        }));
  }
};

class MockEncryptedWriteRecordLayer : public EncryptedWriteRecordLayer {
 public:
  MockEncryptedWriteRecordLayer(EncryptionLevel encryptionLevel)
      : EncryptedWriteRecordLayer(encryptionLevel) {}

  MOCK_METHOD(
      TLSContent,
      _write,
      (TLSMessage & msg, Aead::AeadOptions options),
      (const));
  TLSContent write(TLSMessage&& msg, Aead::AeadOptions options) const override {
    return _write(msg, options);
  }

  MOCK_METHOD(void, _setAead, (folly::ByteRange, Aead*));
  void setAead(folly::ByteRange baseSecret, std::unique_ptr<Aead> aead)
      override {
    _setAead(baseSecret, aead.get());
  }

  void setDefaults() {
    setWriteDefaults(this);
  }
};
} // namespace fizz
