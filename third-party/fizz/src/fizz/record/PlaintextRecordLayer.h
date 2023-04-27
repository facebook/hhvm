/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/RecordLayer.h>

namespace fizz {

class PlaintextReadRecordLayer : public ReadRecordLayer {
 public:
  ~PlaintextReadRecordLayer() override = default;

  /**
   * Skip over received encrypted records until a plaintext record is received.
   */
  virtual void setSkipEncryptedRecords(bool enabled) {
    skipEncryptedRecords_ = enabled;
  }

  ReadResult<TLSMessage> read(folly::IOBufQueue& buf, Aead::AeadOptions options)
      override;

  /**
   * Get the record protocol version of the most recent received record.
   * Should only be used for logging.
   */
  folly::Optional<ProtocolVersion> getReceivedRecordVersion() const {
    return receivedRecordVersion_;
  }

  EncryptionLevel getEncryptionLevel() const override;

 private:
  bool skipEncryptedRecords_{false};

  folly::Optional<ProtocolVersion> receivedRecordVersion_;
};

class PlaintextWriteRecordLayer : public WriteRecordLayer {
 public:
  ~PlaintextWriteRecordLayer() override = default;

  TLSContent write(TLSMessage&& msg, Aead::AeadOptions options) const override;

  /**
   * Write the initial ClientHello handshake message. This is a separate method
   * as the record encoding can be slightly different since the version has not
   * been negotiated yet.
   */
  virtual TLSContent writeInitialClientHello(Buf encodedClientHello) const;

  EncryptionLevel getEncryptionLevel() const override;

 private:
  TLSContent write(TLSMessage msg, ProtocolVersion recordVersion) const;
};
} // namespace fizz
