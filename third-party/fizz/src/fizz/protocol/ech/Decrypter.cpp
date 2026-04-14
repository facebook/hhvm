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
Status decodeAndGetParam(
    folly::Optional<DecrypterLookupResult>& ret,
    Error& err,
    const Extension& encodedECHExtension,
    const std::vector<DecrypterParams>& decrypterParams) {
  folly::io::Cursor cursor(encodedECHExtension.extension_data.get());
  ech::OuterECHClientHello echExtension;
  FIZZ_RETURN_ON_ERROR(
      getExtension<ech::OuterECHClientHello>(echExtension, err, cursor));
  for (const auto& param : decrypterParams) {
    if (echExtension.config_id == param.echConfig.key_config.config_id) {
      ret.emplace(DecrypterLookupResult{std::move(echExtension), param});
      return Status::Success;
    }
  }
  // No match
  ret = folly::none;
  return Status::Success;
}

Status tryToDecodeECH(
    folly::Optional<DecrypterResult>& ret,
    Error& err,
    const fizz::Factory& factory,
    const ClientHello& clientHelloOuter,
    const Extension& encodedECHExtension,
    const std::vector<DecrypterParams>& decrypterParams) {
  folly::Optional<DecrypterLookupResult> configIdResult;
  FIZZ_RETURN_ON_ERROR(decodeAndGetParam(
      configIdResult, err, encodedECHExtension, decrypterParams));

  if (!configIdResult.has_value()) {
    ret = folly::none;
    return Status::Success;
  }

  try {
    std::unique_ptr<hpke::HpkeContext> context;
    FIZZ_THROW_ON_ERROR(
        setupDecryptionContext(
            context,
            err,
            factory,
            configIdResult->matchingParam.echConfig,
            configIdResult->echExtension.cipher_suite,
            configIdResult->echExtension.enc,
            configIdResult->matchingParam.kex->clone(),
            0),
        err);
    ClientHello chlo;
    FIZZ_THROW_ON_ERROR(
        decryptECHWithContext(
            chlo,
            err,
            clientHelloOuter,
            configIdResult->matchingParam.echConfig,
            configIdResult->echExtension.cipher_suite,
            configIdResult->echExtension.enc->clone(),
            configIdResult->echExtension.config_id,
            configIdResult->echExtension.payload->clone(),
            ECHVersion::Draft15,
            context),
        err);
    ret = DecrypterResult{
        std::move(chlo),
        configIdResult->echExtension.config_id,
        std::move(context)};
    return Status::Success;
  } catch (const OuterExtensionsError& e) {
    return err.error(
        std::string(e.what()), AlertDescription::illegal_parameter);
  } catch (const std::exception&) {
    ret = folly::none;
    return Status::Success;
  }
}

Status decodeClientHelloHRR(
    ClientHello& ret,
    Error& err,
    const fizz::Factory& factory,
    const ClientHello& chlo,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey,
    std::unique_ptr<hpke::HpkeContext>& context,
    const std::vector<DecrypterParams>& decrypterParams) {
  // Check for the ECH extension. If not found, return error.
  auto it =
      findExtension(chlo.extensions, ExtensionType::encrypted_client_hello);
  if (it == chlo.extensions.end()) {
    return err.error(
        "ech not sent for hrr", AlertDescription::missing_extension);
  }

  folly::Optional<DecrypterLookupResult> configIdResult;
  FIZZ_RETURN_ON_ERROR(
      decodeAndGetParam(configIdResult, err, *it, decrypterParams));

  if (!configIdResult.has_value()) {
    return err.error(
        "failed to decrypt hrr ech", AlertDescription::decrypt_error);
  }

  try {
    if (context) {
      FIZZ_THROW_ON_ERROR(
          decryptECHWithContext(
              ret,
              err,
              chlo,
              configIdResult->matchingParam.echConfig,
              configIdResult->echExtension.cipher_suite,
              configIdResult->echExtension.enc->clone(),
              configIdResult->echExtension.config_id,
              configIdResult->echExtension.payload->clone(),
              ECHVersion::Draft15,
              context),
          err);
      return Status::Success;
    } else {
      std::unique_ptr<hpke::HpkeContext> recreatedContext;
      FIZZ_THROW_ON_ERROR(
          setupDecryptionContext(
              recreatedContext,
              err,
              factory,
              configIdResult->matchingParam.echConfig,
              configIdResult->echExtension.cipher_suite,
              encapsulatedKey,
              configIdResult->matchingParam.kex->clone(),
              1),
          err);
      FIZZ_THROW_ON_ERROR(
          decryptECHWithContext(
              ret,
              err,
              chlo,
              configIdResult->matchingParam.echConfig,
              configIdResult->echExtension.cipher_suite,
              configIdResult->echExtension.enc->clone(),
              configIdResult->echExtension.config_id,
              configIdResult->echExtension.payload->clone(),
              ECHVersion::Draft15,
              recreatedContext),
          err);
      return Status::Success;
    }
  } catch (const OuterExtensionsError& e) {
    return err.error(e.what(), AlertDescription::illegal_parameter);
  }
}

} // namespace

void ECHConfigManager::addDecryptionConfig(DecrypterParams decrypterParams) {
  configs_.push_back(std::move(decrypterParams));
}

Status ECHConfigManager::decryptClientHello(
    folly::Optional<DecrypterResult>& ret,
    Error& err,
    const ClientHello& chlo) {
  // Check for the ECH extension
  auto it =
      findExtension(chlo.extensions, ExtensionType::encrypted_client_hello);
  if (it != chlo.extensions.end()) {
    return tryToDecodeECH(ret, err, *factory_, chlo, *it, configs_);
  }

  ret = folly::none;
  return Status::Success;
}

Status ECHConfigManager::decryptClientHelloHRR(
    ClientHello& ret,
    Error& err,
    const ClientHello& chlo,
    std::unique_ptr<hpke::HpkeContext>& context) {
  return decodeClientHelloHRR(
      ret, err, *factory_, chlo, nullptr, context, configs_);
}

Status ECHConfigManager::decryptClientHelloHRR(
    ClientHello& ret,
    Error& err,
    const ClientHello& chlo,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey) {
  std::unique_ptr<hpke::HpkeContext> dummy;
  return decodeClientHelloHRR(
      ret, err, *factory_, chlo, encapsulatedKey, dummy, configs_);
}

Status ECHConfigManager::getRetryConfigs(
    std::vector<ech::ECHConfig>& ret,
    Error& err,
    const folly::Optional<std::string>& maybeSni) const {
  std::vector<ech::ECHConfig> retryConfigs;
  std::vector<ech::ECHConfig> nonMatchingConfigs;
  for (const auto& config : configs_) {
    auto echConfig = ech::ECHConfig{};
    echConfig.version = ech::ECHVersion::Draft15;
    FIZZ_RETURN_ON_ERROR(
        encode(echConfig.ech_config_content, err, config.echConfig));
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
  ret = std::move(retryConfigs);
  return Status::Success;
}

} // namespace ech
} // namespace fizz
