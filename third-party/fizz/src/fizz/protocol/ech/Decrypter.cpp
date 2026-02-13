/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/ech/Decrypter.h>

#include <iterator>

namespace fizz {
namespace ech {

namespace {

// Internal result return type. The param reference is valid since we're only
// ever using it while holding a const reference to its parent container,
// guaranteeing its lifetime while we use it (since it's internal to this unit)
struct DecrypterLookupResult {
  ech::OuterECHClientHello echExtension;
  const DecrypterParams& matchingParam;
};

// Tries to find a config in our decrypter params that matches the config ID
// in the passed in extension. If one is found, returns the extension and the
// matching config.
//
// Can return folly::none if no matching config is found.
folly::Optional<DecrypterLookupResult> decodeAndGetParam(
    const Extension& encodedECHExtension,
    const std::vector<DecrypterParams>& decrypterParams) {
  folly::io::Cursor cursor(encodedECHExtension.extension_data.get());
  ech::OuterECHClientHello echExtension;
  Error err;
  FIZZ_THROW_ON_ERROR(
      getExtension<ech::OuterECHClientHello>(echExtension, err, cursor), err);
  for (const auto& param : decrypterParams) {
    if (echExtension.config_id == param.echConfig.key_config.config_id) {
      return DecrypterLookupResult{std::move(echExtension), param};
    }
  }
  // No match
  return folly::none;
}

folly::Optional<DecrypterResult> tryToDecodeECH(
    const fizz::Factory& factory,
    const ClientHello& clientHelloOuter,
    const Extension& encodedECHExtension,
    const std::vector<DecrypterParams>& decrypterParams) {
  auto configIdResult = decodeAndGetParam(encodedECHExtension, decrypterParams);

  if (!configIdResult.has_value()) {
    return folly::none;
  }

  try {
    auto context = setupDecryptionContext(
        factory,
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
        configIdResult->echExtension.config_id,
        configIdResult->echExtension.payload->clone(),
        ECHVersion::Draft15,
        context);
    return DecrypterResult{
        std::move(chlo),
        configIdResult->echExtension.config_id,
        std::move(context)};
  } catch (const OuterExtensionsError& e) {
    throw FizzException(e.what(), AlertDescription::illegal_parameter);
  } catch (const std::exception&) {
    return folly::none;
  }
}

ClientHello decodeClientHelloHRR(
    const fizz::Factory& factory,
    const ClientHello& chlo,
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

  auto configIdResult = decodeAndGetParam(*it, decrypterParams);

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
          configIdResult->echExtension.config_id,
          configIdResult->echExtension.payload->clone(),
          ECHVersion::Draft15,
          context);
    } else {
      auto recreatedContext = setupDecryptionContext(
          factory,
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
          configIdResult->echExtension.config_id,
          configIdResult->echExtension.payload->clone(),
          ECHVersion::Draft15,
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
    return tryToDecodeECH(*factory_, chlo, *it, configs_);
  }

  return folly::none;
}

ClientHello ECHConfigManager::decryptClientHelloHRR(
    const ClientHello& chlo,
    std::unique_ptr<hpke::HpkeContext>& context) {
  return decodeClientHelloHRR(*factory_, chlo, nullptr, context, configs_);
}

ClientHello ECHConfigManager::decryptClientHelloHRR(
    const ClientHello& chlo,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey) {
  std::unique_ptr<hpke::HpkeContext> dummy;
  return decodeClientHelloHRR(
      *factory_, chlo, encapsulatedKey, dummy, configs_);
}

std::vector<ech::ECHConfig> ECHConfigManager::getRetryConfigs(
    const folly::Optional<std::string>& maybeSni) const {
  std::vector<ech::ECHConfig> retryConfigs;
  std::vector<ech::ECHConfig> nonMatchingConfigs;
  for (const auto& config : configs_) {
    auto echConfig = ech::ECHConfig{};
    echConfig.version = ech::ECHVersion::Draft15;
    Error err;
    FIZZ_THROW_ON_ERROR(
        encode(echConfig.ech_config_content, err, config.echConfig), err);
    if (maybeSni.hasValue() &&
        maybeSni.value() == config.echConfig.public_name) {
      retryConfigs.push_back(std::move(echConfig));
    } else {
      nonMatchingConfigs.push_back(std::move(echConfig));
    }
  }
  retryConfigs.insert(
      retryConfigs.end(),
      std::make_move_iterator(nonMatchingConfigs.begin()),
      std::make_move_iterator(nonMatchingConfigs.end()));
  return retryConfigs;
}

} // namespace ech
} // namespace fizz
