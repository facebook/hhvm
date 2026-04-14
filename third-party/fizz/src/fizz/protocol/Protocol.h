/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Factory.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/record/Types.h>

namespace fizz {

class Protocol {
 public:
  template <typename Type>
  static Status setAead(
      Error& err,
      Type& recordLayer,
      CipherSuite cipher,
      folly::ByteRange secret,
      const Factory& factory,
      const KeyScheduler& scheduler) {
    std::unique_ptr<Aead> aead;
    FIZZ_RETURN_ON_ERROR(
        deriveRecordAead(aead, err, factory, scheduler, cipher, secret));
    FIZZ_RETURN_ON_ERROR(recordLayer.setAead(err, secret, std::move(aead)));
    return Status::Success;
  }

  static Status deriveRecordAead(
      std::unique_ptr<Aead>& ret,
      Error& err,
      const Factory& factory,
      const KeyScheduler& scheduler,
      CipherSuite cipher,
      folly::ByteRange secret) {
    FIZZ_RETURN_ON_ERROR(factory.makeAead(ret, err, cipher));
    auto trafficKey =
        scheduler.getTrafficKey(secret, ret->keyLength(), ret->ivLength());
    ret->setKey(std::move(trafficKey));
    return Status::Success;
  }

  static Status deriveRecordAeadWithLabel(
      std::unique_ptr<Aead>& ret,
      Error& err,
      const Factory& factory,
      const KeyScheduler& scheduler,
      CipherSuite cipher,
      folly::ByteRange secret,
      folly::StringPiece keyLabel,
      folly::StringPiece ivLabel) {
    FIZZ_RETURN_ON_ERROR(factory.makeAead(ret, err, cipher));
    auto trafficKey = scheduler.getTrafficKeyWithLabel(
        secret, keyLabel, ivLabel, ret->keyLength(), ret->ivLength());
    ret->setKey(std::move(trafficKey));
    return Status::Success;
  }

  static Status getFinished(
      Buf& ret,
      Error& err,
      folly::ByteRange handshakeWriteSecret,
      HandshakeContext& handshakeContext) {
    Finished finished;
    finished.verify_data =
        handshakeContext.getFinishedData(handshakeWriteSecret);
    FIZZ_RETURN_ON_ERROR(encodeHandshake(ret, err, std::move(finished)));
    handshakeContext.appendToTranscript(ret);
    return Status::Success;
  }

  static Status
  getKeyUpdated(Buf& ret, Error& err, KeyUpdateRequest request_update) {
    KeyUpdate keyUpdated;
    keyUpdated.request_update = request_update;
    FIZZ_RETURN_ON_ERROR(encodeHandshake(ret, err, std::move(keyUpdated)));
    return Status::Success;
  }

  static Status checkAllowedExtensions(
      Error& err,
      const EncryptedExtensions& ee,
      const std::vector<ExtensionType>& requestedExtensions) {
    for (const auto& ext : ee.extensions) {
      // These extensions are not allowed in EE
      switch (ext.extension_type) {
        case ExtensionType::signature_algorithms:
        case ExtensionType::key_share:
        case ExtensionType::pre_shared_key:
        case ExtensionType::psk_key_exchange_modes:
        case ExtensionType::cookie:
        case ExtensionType::supported_versions:
          return err.error(
              "unexpected extension in ee: " + toString(ext.extension_type),
              AlertDescription::illegal_parameter);
        default:
          if (std::find(
                  requestedExtensions.begin(),
                  requestedExtensions.end(),
                  ext.extension_type) == requestedExtensions.end()) {
            return err.error(
                "unexpected extension in ee: " + toString(ext.extension_type),
                AlertDescription::illegal_parameter);
          }
          break;
      }
    }
    return checkDuplicateExtensions(err, ee.extensions);
  }

  static Status checkAllowedExtensions(
      Error& err,
      const ServerHello& shlo,
      const std::vector<ExtensionType>& requestedExtensions) {
    for (const auto& ext : shlo.extensions) {
      if (std::find(
              requestedExtensions.begin(),
              requestedExtensions.end(),
              ext.extension_type) == requestedExtensions.end() ||
          (ext.extension_type != ExtensionType::key_share &&
           ext.extension_type != ExtensionType::pre_shared_key &&
           ext.extension_type != ExtensionType::supported_versions)) {
        return err.error(
            "unexpected extension in shlo: " + toString(ext.extension_type),
            AlertDescription::illegal_parameter);
      }
    }
    return checkDuplicateExtensions(err, shlo.extensions);
  }

  static Status checkAllowedExtensions(
      Error& err,
      const HelloRetryRequest& hrr,
      const std::vector<ExtensionType>& requestedExtensions) {
    // HRR is allowed to send 'cookie' unprompted, and we always send key_share
    // and supported_versions. ech is the only other allowed extension here, and
    // only when we sent it before.
    for (const auto& ext : hrr.extensions) {
      if (ext.extension_type != ExtensionType::cookie &&
          ext.extension_type != ExtensionType::key_share &&
          ext.extension_type != ExtensionType::supported_versions &&
          (ext.extension_type != ExtensionType::encrypted_client_hello ||
           std::find(
               requestedExtensions.begin(),
               requestedExtensions.end(),
               ext.extension_type) == requestedExtensions.end())) {
        return err.error(
            "unexpected extension in hrr: " + toString(ext.extension_type),
            AlertDescription::illegal_parameter);
      }
    }
    return checkDuplicateExtensions(err, hrr.extensions);
  }

  /*
   * Checks that the presented extensions in the cert are a subset of the
   * requested. Used for the response to the servers cert req msg
   */
  static Status checkAllowedExtensions(
      Error& err,
      const CertificateEntry& certEntry,
      const std::vector<ExtensionType>& requestedExtensions) {
    // Extensions in cert must be a subset of extensions sent in the cert
    // request msg
    if (!std::all_of(
            certEntry.extensions.begin(),
            certEntry.extensions.end(),
            [&requestedExtensions](const auto& ext) {
              return std::find(
                         requestedExtensions.begin(),
                         requestedExtensions.end(),
                         ext.extension_type) != requestedExtensions.end();
            })) {
      return err.error(
          "Unexpected extension in cert", AlertDescription::illegal_parameter);
    }
    return checkDuplicateExtensions(err, certEntry.extensions);
  }

  static Status checkDuplicateExtensions(
      Error& err,
      const std::vector<Extension>& exts) {
    std::vector<ExtensionType> extensionList;
    extensionList.reserve(exts.size());
    for (const auto& extension : exts) {
      extensionList.push_back(extension.extension_type);
    }
    std::sort(extensionList.begin(), extensionList.end());
    if (std::unique(extensionList.begin(), extensionList.end()) !=
        extensionList.end()) {
      return err.error(
          "duplicate extension", AlertDescription::illegal_parameter);
    }
    return Status::Success;
  }
};
} // namespace fizz
