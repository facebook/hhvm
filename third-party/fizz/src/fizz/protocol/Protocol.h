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
  static void setAead(
      Type& recordLayer,
      CipherSuite cipher,
      folly::ByteRange secret,
      const Factory& factory,
      const KeyScheduler& scheduler) {
    auto aead = deriveRecordAead(factory, scheduler, cipher, secret);
    recordLayer.setAead(secret, std::move(aead));
  }

  static std::unique_ptr<Aead> deriveRecordAead(
      const Factory& factory,
      const KeyScheduler& scheduler,
      CipherSuite cipher,
      folly::ByteRange secret) {
    auto aead = factory.makeAead(cipher);
    auto trafficKey =
        scheduler.getTrafficKey(secret, aead->keyLength(), aead->ivLength());
    aead->setKey(std::move(trafficKey));
    return aead;
  }

  static std::unique_ptr<Aead> deriveRecordAeadWithLabel(
      const Factory& factory,
      const KeyScheduler& scheduler,
      CipherSuite cipher,
      folly::ByteRange secret,
      folly::StringPiece keyLabel,
      folly::StringPiece ivLabel) {
    auto aead = factory.makeAead(cipher);
    auto trafficKey = scheduler.getTrafficKeyWithLabel(
        secret, keyLabel, ivLabel, aead->keyLength(), aead->ivLength());
    aead->setKey(std::move(trafficKey));
    return aead;
  }

  static Buf getFinished(
      folly::ByteRange handshakeWriteSecret,
      HandshakeContext& handshakeContext) {
    Finished finished;
    finished.verify_data =
        handshakeContext.getFinishedData(handshakeWriteSecret);
    auto encodedFinished = encodeHandshake(std::move(finished));
    handshakeContext.appendToTranscript(encodedFinished);
    return encodedFinished;
  }

  static Buf getKeyUpdated(KeyUpdateRequest request_update) {
    KeyUpdate keyUpdated;
    keyUpdated.request_update = request_update;
    return encodeHandshake(std::move(keyUpdated));
  }

  static void checkAllowedExtensions(
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
          throw FizzException(
              "unexpected extension in ee: " + toString(ext.extension_type),
              AlertDescription::illegal_parameter);
        default:
          if (std::find(
                  requestedExtensions.begin(),
                  requestedExtensions.end(),
                  ext.extension_type) == requestedExtensions.end()) {
            throw FizzException(
                "unexpected extension in ee: " + toString(ext.extension_type),
                AlertDescription::illegal_parameter);
          }
          break;
      }
    }
    checkDuplicateExtensions(ee.extensions);
  }

  static void checkAllowedExtensions(
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
        throw FizzException(
            "unexpected extension in shlo: " + toString(ext.extension_type),
            AlertDescription::illegal_parameter);
      }
    }
    checkDuplicateExtensions(shlo.extensions);
  }

  static void checkAllowedExtensions(const HelloRetryRequest& hrr) {
    // HRR is allowed to send 'cookie' unprompted. Otherwise only other allowed
    // extensions are key_share and supported_versions, which we always send.
    for (const auto& ext : hrr.extensions) {
      if (ext.extension_type != ExtensionType::cookie &&
          ext.extension_type != ExtensionType::key_share &&
          ext.extension_type != ExtensionType::supported_versions) {
        throw FizzException(
            "unexpected extension in hrr: " + toString(ext.extension_type),
            AlertDescription::illegal_parameter);
      }
    }
    checkDuplicateExtensions(hrr.extensions);
  }

  static void checkDuplicateExtensions(const std::vector<Extension>& exts) {
    std::vector<ExtensionType> extensionList;
    for (const auto& extension : exts) {
      extensionList.push_back(extension.extension_type);
    }
    std::sort(extensionList.begin(), extensionList.end());
    if (std::unique(extensionList.begin(), extensionList.end()) !=
        extensionList.end()) {
      throw FizzException(
          "duplicate extension", AlertDescription::illegal_parameter);
    }
  }
};
} // namespace fizz
