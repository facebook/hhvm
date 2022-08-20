/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/ech/Decrypter.h>

namespace fizz {
namespace ech {

namespace {

// Internal result return type. The param reference is valid since we're only
// ever using it while holding a const reference to its parent container,
// guaranteeing its lifetime while we use it (since it's internal to this unit)
struct DecrypterLookupResult {
  Buf configId;
  ech::ClientECH echExtension;
  const DecrypterParams& matchingParam;
};

// Tries to find a config in our decrypter params that matches the passed in
// config ID if given or the config ID in the encoded extension if no config
// ID is explicitly passed in.
//
// Can return folly::none if no matching config is found.
folly::Optional<DecrypterLookupResult> getMatchingConfigIdAndECH(
    const Extension& encodedECHExtension,
    const Buf& overrideConfigId,
    const std::vector<DecrypterParams>& decrypterParams) {
  folly::io::Cursor cursor(encodedECHExtension.extension_data.get());
  for (const auto& param : decrypterParams) {
    switch (param.echConfig.version) {
      case ECHVersion::Draft9: {
        auto echExtension = getExtension<ech::ClientECH>(cursor);
        auto& echConfig = param.echConfig;
        const Buf& configId = overrideConfigId == nullptr
            ? echExtension.config_id
            : overrideConfigId;
        auto currentConfigId =
            constructConfigId(echExtension.cipher_suite.kdf_id, echConfig);
        if (!folly::IOBufEqualTo()(currentConfigId, configId)) {
          continue;
        }
        return DecrypterLookupResult{
            std::move(currentConfigId), std::move(echExtension), param};
      }
      default: {
        continue;
      }
    }
  }
  // No match
  return folly::none;
}

folly::Optional<DecrypterResult> tryToDecodeECH(
    const ClientHello& clientHelloOuter,
    const Extension& encodedECHExtension,
    const std::vector<DecrypterParams>& decrypterParams) {
  auto configIdResult =
      getMatchingConfigIdAndECH(encodedECHExtension, nullptr, decrypterParams);

  if (!configIdResult.has_value()) {
    return folly::none;
  }

  try {
    auto context = setupDecryptionContext(
        configIdResult->matchingParam.echConfig,
        configIdResult->echExtension.cipher_suite,
        configIdResult->echExtension.enc,
        configIdResult->matchingParam.kex->clone(),
        0);
    auto chlo = decryptECHWithContext(
        clientHelloOuter,
        configIdResult->matchingParam.echConfig,
        configIdResult->echExtension.cipher_suite,
        configIdResult->echExtension.enc->clone(),
        configIdResult->configId->clone(),
        configIdResult->echExtension.payload->clone(),
        ECHVersion::Draft9,
        context);
    return DecrypterResult{
        std::move(chlo),
        std::move(configIdResult->configId),
        std::move(context)};
  } catch (const OuterExtensionsError& e) {
    throw FizzException(e.what(), AlertDescription::illegal_parameter);
  } catch (const std::exception&) {
    return folly::none;
  }
}

ClientHello decodeClientHelloHRR(
    const ClientHello& chlo,
    const std::unique_ptr<folly::IOBuf>& configId,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey,
    std::unique_ptr<hpke::HpkeContext>& context,
    const std::vector<DecrypterParams>& decrypterParams) {
  // Check for the ECH extension. If not found, throw.
  auto it =
      findExtension(chlo.extensions, ExtensionType::encrypted_client_hello);
  if (it == chlo.extensions.end()) {
    throw FizzException(
        "ech not sent for hrr", AlertDescription::missing_extension);
  }

  auto configIdResult =
      getMatchingConfigIdAndECH(*it, configId, decrypterParams);

  if (!configIdResult.has_value()) {
    throw FizzException(
        "failed to decrypt hrr ech", AlertDescription::decrypt_error);
  }

  try {
    if (context) {
      return decryptECHWithContext(
          chlo,
          configIdResult->matchingParam.echConfig,
          configIdResult->echExtension.cipher_suite,
          configIdResult->echExtension.enc->clone(),
          configIdResult->echExtension.config_id->clone(),
          configIdResult->echExtension.payload->clone(),
          ECHVersion::Draft9,
          context);
    } else {
      auto recreatedContext = setupDecryptionContext(
          configIdResult->matchingParam.echConfig,
          configIdResult->echExtension.cipher_suite,
          encapsulatedKey,
          configIdResult->matchingParam.kex->clone(),
          1);
      return decryptECHWithContext(
          chlo,
          configIdResult->matchingParam.echConfig,
          configIdResult->echExtension.cipher_suite,
          configIdResult->echExtension.enc->clone(),
          configIdResult->echExtension.config_id->clone(),
          configIdResult->echExtension.payload->clone(),
          ECHVersion::Draft9,
          recreatedContext);
    }
  } catch (const OuterExtensionsError& e) {
    throw FizzException(e.what(), AlertDescription::illegal_parameter);
  }

  throw FizzException(
      "failed to decrypt hrr ech", AlertDescription::decrypt_error);
}

} // namespace

void ECHConfigManager::addDecryptionConfig(DecrypterParams decrypterParams) {
  configs_.push_back(std::move(decrypterParams));
}

folly::Optional<DecrypterResult> ECHConfigManager::decryptClientHello(
    const ClientHello& chlo) {
  // Check for the ECH extension
  auto it =
      findExtension(chlo.extensions, ExtensionType::encrypted_client_hello);
  if (it != chlo.extensions.end()) {
    return tryToDecodeECH(chlo, *it, configs_);
  }

  return folly::none;
}

ClientHello ECHConfigManager::decryptClientHelloHRR(
    const ClientHello& chlo,
    const std::unique_ptr<folly::IOBuf>& configId,
    std::unique_ptr<hpke::HpkeContext>& context) {
  return decodeClientHelloHRR(chlo, configId, nullptr, context, configs_);
}

ClientHello ECHConfigManager::decryptClientHelloHRR(
    const ClientHello& chlo,
    const std::unique_ptr<folly::IOBuf>& configId,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey) {
  std::unique_ptr<hpke::HpkeContext> dummy;
  return decodeClientHelloHRR(chlo, configId, encapsulatedKey, dummy, configs_);
}

std::vector<ech::ECHConfig> ECHConfigManager::getRetryConfigs() const {
  std::vector<ech::ECHConfig> retryConfigs;
  for (const auto& cfg : configs_) {
    retryConfigs.push_back(cfg.echConfig);
  }
  return retryConfigs;
}

} // namespace ech
} // namespace fizz
