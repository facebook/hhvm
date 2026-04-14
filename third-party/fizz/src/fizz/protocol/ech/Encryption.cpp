/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/ech/Encryption.h>
#include "fizz/record/Types.h"

#include <fizz/crypto/Utils.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/protocol/Protocol.h>
#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/Types.h>
#include <iterator>

namespace fizz {
namespace ech {

namespace {

static const size_t kGreasePSKIdentitySize = 16;

Status makeClientHelloOuterForAad(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const ClientHello& clientHelloOuter) {
  // Copy client hello outer
  ClientHello chloCopy = clientHelloOuter.clone();

  // Zero the ech extension from the copy
  auto it = std::find_if(
      chloCopy.extensions.begin(), chloCopy.extensions.end(), [](auto& e) {
        return e.extension_type == ExtensionType::encrypted_client_hello;
      });

  folly::io::Cursor cursor(it->extension_data.get());
  OuterECHClientHello echExtension;
  FIZZ_RETURN_ON_ERROR(
      getExtension<OuterECHClientHello>(echExtension, err, cursor));

  // Create a zeroed out version of the payload
  size_t payloadSize = echExtension.payload->computeChainDataLength();
  echExtension.payload = folly::IOBuf::create(payloadSize);
  memset(echExtension.payload->writableData(), 0, payloadSize);
  echExtension.payload->append(payloadSize);

  FIZZ_RETURN_ON_ERROR(encodeExtension(*it, err, echExtension));

  // Get the serialized version of the client hello outer
  // without the ECH extension to use
  FIZZ_RETURN_ON_ERROR(encode(ret, err, chloCopy));
  return Status::Success;
}

Status extractEncodedClientHelloInner(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    ECHVersion version,
    std::unique_ptr<folly::IOBuf> encryptedCh,
    std::unique_ptr<hpke::HpkeContext>& context,
    const ClientHello& clientHelloOuter) {
  switch (version) {
    case ECHVersion::Draft15: {
      std::unique_ptr<folly::IOBuf> aadCH;
      FIZZ_RETURN_ON_ERROR(
          makeClientHelloOuterForAad(aadCH, err, clientHelloOuter));
      ret = context->open(aadCH.get(), std::move(encryptedCh));
    }
  }
  return Status::Success;
}

Status makeHpkeContextInfoParam(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const ParsedECHConfig& echConfig) {
  // The "info" parameter to setupWithEncap is the
  // concatenation of "tls ech", a zero byte, and the serialized
  // ECHConfig.
  std::string tlsEchPrefix = "tls ech";
  tlsEchPrefix += '\0';
  auto bufContents = folly::IOBuf::copyBuffer(tlsEchPrefix);
  auto config = ECHConfig{};
  config.version = ECHVersion::Draft15;
  FIZZ_RETURN_ON_ERROR(encode(config.ech_config_content, err, echConfig));
  std::unique_ptr<folly::IOBuf> encodedConfig;
  FIZZ_RETURN_ON_ERROR(encode(encodedConfig, err, std::move(config)));
  bufContents->prependChain(std::move(encodedConfig));
  ret = std::move(bufContents);
  return Status::Success;
}

bool isValidPublicName(const std::string& publicName) {
  if (publicName.empty()) {
    return false;
  }

  // Starts/ends with a dot.
  if (publicName.front() == '.' || publicName.back() == '.') {
    return false;
  }

  std::vector<folly::StringPiece> parts;
  folly::split('.', publicName, parts, false);

  // Check that each part is a valid LDH label ([a-z,A-Z,0-9,-])
  for (auto& part : parts) {
    if (part.empty()) {
      return false;
    }
    if (std::any_of(part.begin(), part.end(), [](char c) {
          return !std::isalnum(c) && c != '-';
        })) {
      return false;
    }
  }
  return true;
}

} // namespace

// We currently don't support any extensions to alter ECH behavior. As such,
// just check that there are no mandatory extensions. (Extensions with the high
// order bit set). Since the integer has been converted from network order to
// native already, we just have to generate a native integer with the highest
// order bit set and compare.
//
// If there are any mandatory extensions, we have to skip this config.
static bool echConfigHasMandatoryExtension(const ParsedECHConfig& config) {
  return std::any_of(
      config.extensions.begin(),
      config.extensions.end(),
      [](const auto& echExt) {
        // Bitwise operators work the same independent of endianness (left
        // shift will consume msb)
        static const uint16_t msb = 1 << ((sizeof(uint16_t) * 8) - 1);
        const auto extType = static_cast<uint16_t>(echExt.extension_type);
        return (msb & extType) != 0;
      });
}

Status negotiateECHConfig(
    folly::Optional<NegotiatedECHConfig>& ret,
    Error& err,
    const std::vector<ParsedECHConfig>& configs,
    std::vector<hpke::KEMId> supportedKEMs,
    std::vector<hpke::AeadId> supportedAeads) {
  // Received set of configs is in order of preference so
  // we should be selecting the first one that we can support.
  for (const auto& config : configs) {
    // Before anything else, check if the config has mandatory extensions.
    // We don't support any extensions, so if any are mandatory, skip this
    // config.
    if (echConfigHasMandatoryExtension(config)) {
      FIZZ_VLOG(8) << "ECH config has mandatory extension, skipping...";
      continue;
    }

    // Check for an invalid public name and skip if found.
    if (!isValidPublicName(config.public_name)) {
      FIZZ_VLOG(8) << config.public_name << " isn't a valid public name";
      continue;
    }

    // Check if we (client) support the server's chosen KEM.
    auto result = std::find(
        supportedKEMs.begin(), supportedKEMs.end(), config.key_config.kem_id);
    if (result == supportedKEMs.end()) {
      continue;
    }

    // Check if we (client) support the HPKE cipher suite.
    auto& cipherSuites = config.key_config.cipher_suites;
    for (const auto& suite : cipherSuites) {
      auto isCipherSupported =
          std::find(
              supportedAeads.begin(), supportedAeads.end(), suite.aead_id) !=
          supportedAeads.end();
      if (isCipherSupported) {
        HashFunction hashFunc;
        FIZZ_RETURN_ON_ERROR(
            getHashFunction(hashFunc, err, getCipherSuite(suite.aead_id)));
        auto associatedCipherKdf = hpke::getKDFId(hashFunc);
        if (suite.kdf_id == associatedCipherKdf) {
          auto negotiatedECHConfig = config;
          auto configId = config.key_config.config_id;
          auto maxLen = config.maximum_name_length;
          ret =
              NegotiatedECHConfig{negotiatedECHConfig, configId, maxLen, suite};
          return Status::Success;
        }
      }
    }
  }
  ret = folly::none;
  return Status::Success;
}

static Status getSetupParam(
    hpke::SetupParam& ret,
    Error& err,
    const fizz::Factory& factory,
    std::unique_ptr<DHKEM> dhkem,
    hpke::KEMId kemId,
    const HpkeSymmetricCipherSuite& cipherSuite) {
  // Get suite id
  auto group = getKexGroup(kemId);
  auto hash = getHashFunction(cipherSuite.kdf_id);
  auto suite = getCipherSuite(cipherSuite.aead_id);
  auto suiteId = hpke::generateHpkeSuiteId(group, hash, suite);

  const HasherFactoryWithMetadata* hasherFactory = nullptr;
  FIZZ_RETURN_ON_ERROR(factory.makeHasherFactory(hasherFactory, err, hash));
  auto hkdf =
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::v1(hasherFactory));

  std::unique_ptr<Aead> aead;
  FIZZ_RETURN_ON_ERROR(
      factory.makeAead(aead, err, getCipherSuite(cipherSuite.aead_id)));

  ret = hpke::SetupParam{
      std::move(dhkem),
      std::move(aead),
      std::move(hkdf),
      std::move(suiteId),
      0};
  return Status::Success;
}

Status constructHpkeSetupResult(
    hpke::SetupResult& ret,
    Error& err,
    const fizz::Factory& factory,
    std::unique_ptr<KeyExchange> kex,
    const NegotiatedECHConfig& negotiatedECHConfig) {
  const auto& echConfigContent = negotiatedECHConfig.config;
  auto cipherSuite = negotiatedECHConfig.cipherSuite;
  auto hash = getHashFunction(cipherSuite.kdf_id);

  // Get shared secret
  const HasherFactoryWithMetadata* hasherFactory = nullptr;
  FIZZ_RETURN_ON_ERROR(factory.makeHasherFactory(hasherFactory, err, hash));
  auto hkdf =
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::v1(hasherFactory));
  std::unique_ptr<DHKEM> dhkem = std::make_unique<DHKEM>(
      std::move(kex),
      getKexGroup(echConfigContent.key_config.kem_id),
      std::move(hkdf));

  // Get context
  std::unique_ptr<folly::IOBuf> info;
  FIZZ_RETURN_ON_ERROR(
      makeHpkeContextInfoParam(info, err, negotiatedECHConfig.config));

  hpke::SetupParam setupParam;
  FIZZ_RETURN_ON_ERROR(getSetupParam(
      setupParam,
      err,
      factory,
      std::move(dhkem),
      echConfigContent.key_config.kem_id,
      cipherSuite));
  ret = setupWithEncap(
      hpke::Mode::Base,
      echConfigContent.key_config.public_key->clone()->coalesce(),
      std::move(info),
      folly::none,
      std::move(setupParam));
  return Status::Success;
}

ServerHello makeDummyServerHello(const ServerHello& shlo) {
  std::vector<Extension> extensionCopies;
  for (auto& ext : shlo.extensions) {
    extensionCopies.push_back(ext.clone());
  }
  ServerHello shloEch;
  shloEch.legacy_version = shlo.legacy_version;
  shloEch.random = shlo.random;
  shloEch.legacy_session_id_echo = shlo.legacy_session_id_echo->clone();
  shloEch.cipher_suite = shlo.cipher_suite;
  shloEch.legacy_compression_method = shlo.legacy_compression_method;
  shloEch.extensions = std::move(extensionCopies);
  // Zero the acceptance confirmation bytes
  memset(
      shloEch.random.data() +
          (shloEch.random.size() - kEchAcceptConfirmationSize),
      0,
      kEchAcceptConfirmationSize);
  return shloEch;
}

HelloRetryRequest makeDummyHRR(const HelloRetryRequest& hrr) {
  // Dummy HRR is identical to original HRR except the contents of the ECH
  // extension it sent is zeroed out.
  std::vector<Extension> extensionCopies;
  for (auto& ext : hrr.extensions) {
    if (ext.extension_type != ExtensionType::encrypted_client_hello) {
      // Not ECH, just clone it.
      extensionCopies.push_back(ext.clone());
    } else {
      // This is an ECH extension, replace it with one that has the content
      // zeroed out.
      Extension e;
      e.extension_type = ExtensionType::encrypted_client_hello;
      e.extension_data = folly::IOBuf::create(kEchAcceptConfirmationSize);
      memset(e.extension_data->writableData(), 0, kEchAcceptConfirmationSize);
      e.extension_data->append(kEchAcceptConfirmationSize);
      extensionCopies.push_back(std::move(e));
    }
  }
  HelloRetryRequest hrrEch;
  hrrEch.legacy_version = hrr.legacy_version;
  hrrEch.legacy_session_id_echo = hrr.legacy_session_id_echo->clone();
  hrrEch.cipher_suite = hrr.cipher_suite;
  hrrEch.legacy_compression_method = hrr.legacy_compression_method;
  hrrEch.extensions = std::move(extensionCopies);
  return hrrEch;
}

namespace {

Status calculateAcceptConfirmation(
    std::vector<uint8_t>& ret,
    Error& err,
    const ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  // Acceptance is done by feeding a dummy hello into the transcript and
  // deriving a secret from it.
  auto shloEch = makeDummyServerHello(shlo);
  Buf encodedShloEch;
  FIZZ_RETURN_ON_ERROR(
      encodeHandshake(encodedShloEch, err, std::move(shloEch)));
  context->appendToTranscript(encodedShloEch);

  auto hsc = context->getHandshakeContext();
  auto echAcceptance = scheduler->getSecret(
      EarlySecrets::ECHAcceptConfirmation, hsc->coalesce());

  ret = std::move(echAcceptance.secret);
  return Status::Success;
}

Status calculateAcceptConfirmation(
    std::vector<uint8_t>& ret,
    Error& err,
    const HelloRetryRequest& hrr,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  // Acceptance is done by zeroing the confirmation extension,
  // putting it into the transcript, and deriving a secret.
  auto hrrEch = makeDummyHRR(hrr);
  Buf encodedHrrEch;
  FIZZ_RETURN_ON_ERROR(encodeHandshake(encodedHrrEch, err, std::move(hrrEch)));
  context->appendToTranscript(encodedHrrEch);

  auto hsc = context->getHandshakeContext();
  auto echAcceptance = scheduler->getSecret(
      EarlySecrets::HRRECHAcceptConfirmation, hsc->coalesce());

  if (echAcceptance.secret.size() < kEchAcceptConfirmationSize) {
    FIZZ_VLOG(8) << "ECH acceptance secret too small?";
    return err.error("ech acceptance secret too small");
  }

  ret = std::move(echAcceptance.secret);
  return Status::Success;
}

} // namespace

Status checkECHAccepted(
    bool& ret,
    Error& err,
    const ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  std::vector<uint8_t> acceptConfirmation;
  FIZZ_RETURN_ON_ERROR(calculateAcceptConfirmation(
      acceptConfirmation, err, shlo, std::move(context), std::move(scheduler)));
  // ECH accepted if the 8 bytes match the accept_confirmation
  ret = CryptoUtils::equal(
      folly::ByteRange(
          shlo.random.data() +
              (shlo.random.size() - kEchAcceptConfirmationSize),
          kEchAcceptConfirmationSize),
      folly::ByteRange(acceptConfirmation.data(), kEchAcceptConfirmationSize));
  return Status::Success;
}

Status checkECHAccepted(
    bool& ret,
    Error& err,
    const HelloRetryRequest& hrr,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  std::vector<uint8_t> acceptConfirmation;
  FIZZ_RETURN_ON_ERROR(calculateAcceptConfirmation(
      acceptConfirmation, err, hrr, std::move(context), std::move(scheduler)));

  // ECH accepted if the 8 bytes match the accept_confirmation in the
  // extension
  folly::Optional<ECHHelloRetryRequest> echConf;
  FIZZ_RETURN_ON_ERROR(
      getExtension<ECHHelloRetryRequest>(echConf, err, hrr.extensions));
  if (!echConf) {
    FIZZ_VLOG(8) << "HRR ECH extension missing, rejected...";
    ret = false;
    return Status::Success;
  }

  ret = CryptoUtils::equal(
      folly::ByteRange(
          echConf->confirmation.data(), kEchAcceptConfirmationSize),
      folly::ByteRange(acceptConfirmation.data(), kEchAcceptConfirmationSize));
  return Status::Success;
}

Status setAcceptConfirmation(
    Error& err,
    ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  std::vector<uint8_t> acceptConfirmation;
  FIZZ_RETURN_ON_ERROR(calculateAcceptConfirmation(
      acceptConfirmation, err, shlo, std::move(context), std::move(scheduler)));

  // Copy the acceptance confirmation bytes to the end
  memcpy(
      shlo.random.data() + (shlo.random.size() - kEchAcceptConfirmationSize),
      acceptConfirmation.data(),
      kEchAcceptConfirmationSize);
  return Status::Success;
}

Status setAcceptConfirmation(
    Error& err,
    HelloRetryRequest& hrr,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  // Add an ECH confirmation extension. The calculation code will ignore its
  // contents but expects it to be there.
  Extension ext;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, ECHHelloRetryRequest()));
  hrr.extensions.push_back(std::move(ext));

  // Calculate it.
  std::vector<uint8_t> acceptConfirmation;
  FIZZ_RETURN_ON_ERROR(calculateAcceptConfirmation(
      acceptConfirmation, err, hrr, std::move(context), std::move(scheduler)));

  // Copy the acceptance confirmation bytes to the payload
  memcpy(
      hrr.extensions.back().extension_data->writableData(),
      acceptConfirmation.data(),
      kEchAcceptConfirmationSize);
  return Status::Success;
}

namespace {

// GREASE PSKs are essentially the same size as the source PSK with the actual
// contents of all fields replaced with random data. For the HRR case, the PSK
// identity is preserved.
ClientPresharedKey generateGreasePskCommon(
    const ClientPresharedKey& source,
    const Factory* factory,
    bool keepIdentity) {
  ClientPresharedKey grease;
  for (size_t i = 0; i < source.identities.size(); i++) {
    const auto& identity = source.identities.at(i);
    PskIdentity greaseIdentity;
    if (keepIdentity) {
      greaseIdentity.psk_identity = identity.psk_identity->clone();
    } else {
      greaseIdentity.psk_identity =
          factory->makeRandomIOBuf(kGreasePSKIdentitySize);
    }

    factory->makeRandomBytes(
        reinterpret_cast<unsigned char*>(&greaseIdentity.obfuscated_ticket_age),
        sizeof(greaseIdentity.obfuscated_ticket_age));
    grease.identities.push_back(std::move(greaseIdentity));

    const auto& binder = source.binders.at(i);
    PskBinder greaseBinder;
    size_t binderSize = binder.binder->computeChainDataLength();
    greaseBinder.binder = factory->makeRandomIOBuf(binderSize);
    grease.binders.push_back(std::move(greaseBinder));
  }
  return grease;
}
} // namespace

Status generateGreasePSK(
    folly::Optional<ClientPresharedKey>& ret,
    Error& err,
    const ClientHello& chloInner,
    const Factory* factory) {
  folly::Optional<ClientPresharedKey> innerPsk;
  FIZZ_RETURN_ON_ERROR(
      getExtension<ClientPresharedKey>(innerPsk, err, chloInner.extensions));
  if (!innerPsk) {
    ret = folly::none;
    return Status::Success;
  }

  // For client hello, don't preserve identity.
  ret = generateGreasePskCommon(*innerPsk, factory, false);
  return Status::Success;
}

ClientPresharedKey generateGreasePSKForHRR(
    const ClientPresharedKey& previousPsk,
    const Factory* factory) {
  // This PSK was the one sent before (i.e. with a random identity). We want to
  // keep it.
  return generateGreasePskCommon(previousPsk, factory, true);
}

Status calculateECHPadding(
    size_t& ret,
    Error& err,
    const ClientHello& chlo,
    size_t encodedSize,
    size_t maxLen) {
  size_t padding = 0;
  folly::Optional<ServerNameList> sni;
  FIZZ_RETURN_ON_ERROR(getExtension<ServerNameList>(sni, err, chlo.extensions));
  if (sni) {
    // Add max(0, maxLen - len(server_name))
    size_t sniLen =
        sni->server_name_list.at(0).hostname->computeChainDataLength();
    if (sniLen < maxLen) {
      padding = maxLen - sniLen;
    }
  } else {
    // Add maxLen + 9, the size of an SNI extension of size maxLen
    padding = maxLen + 9;
  }

  // Now, add the final padding.
  // L = len(encodedClientHelloInner) + currentPadding
  // N = 31 - ((L - 1) % 32)
  size_t currentLen = encodedSize + padding;
  padding += 31 - ((currentLen - 1) % 32);
  ret = padding;
  return Status::Success;
}

Status generateAndReplaceOuterExtensions(
    std::vector<Extension>& ret,
    Error& err,
    std::vector<Extension>&& chloInnerExt,
    const std::vector<ExtensionType>& outerExtensionTypes) {
  std::vector<ExtensionType> extTypes;
  for (const auto& ext : chloInnerExt) {
    if (std::find(
            outerExtensionTypes.begin(),
            outerExtensionTypes.end(),
            ext.extension_type) != outerExtensionTypes.end()) {
      extTypes.push_back(ext.extension_type);
    }
  }
  if (extTypes.size() == 0) {
    ret = std::move(chloInnerExt);
    return Status::Success;
  }

  OuterExtensions outerExt;
  outerExt.types = extTypes;

  bool outerExtensionsInserted = false;
  for (const auto extType : extTypes) {
    auto it = std::find_if(
        chloInnerExt.begin(), chloInnerExt.end(), [extType](const auto& ext) {
          return ext.extension_type == extType;
        });
    if (!outerExtensionsInserted) {
      FIZZ_RETURN_ON_ERROR(encodeExtension(*it, err, outerExt));
      outerExtensionsInserted = true;
    } else {
      chloInnerExt.erase(it);
    }
  }

  ret = std::move(chloInnerExt);
  return Status::Success;
}

namespace {

Status encryptClientHelloImpl(
    Error& err,
    OuterECHClientHello& echExtension,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult,
    const folly::Optional<ClientPresharedKey>& greasePsk,
    size_t maxLen,
    const std::vector<ExtensionType>& outerExtensionTypes) {
  // Remove legacy_session_id and serialize the client hello inner
  auto chloInnerCopy = clientHelloInner.clone();
  chloInnerCopy.legacy_session_id = folly::IOBuf::copyBuffer("");
  std::vector<Extension> processedExtensions;
  FIZZ_RETURN_ON_ERROR(generateAndReplaceOuterExtensions(
      processedExtensions,
      err,
      std::move(chloInnerCopy.extensions),
      outerExtensionTypes));
  chloInnerCopy.extensions = std::move(processedExtensions);
  std::unique_ptr<folly::IOBuf> encodedClientHelloInner;
  FIZZ_RETURN_ON_ERROR(encode(encodedClientHelloInner, err, chloInnerCopy));

  size_t padding;
  FIZZ_RETURN_ON_ERROR(calculateECHPadding(
      padding,
      err,
      clientHelloInner,
      encodedClientHelloInner->computeChainDataLength(),
      maxLen));
  if (padding > 0) {
    auto paddingBuf = folly::IOBuf::create(padding);
    memset(paddingBuf->writableData(), 0, padding);
    paddingBuf->append(padding);
    encodedClientHelloInner->prependChain(std::move(paddingBuf));
  }

  // Compute the AAD for sealing
  auto chloOuterForAAD = clientHelloOuter.clone();

  // Get cipher overhead
  size_t dummyPayloadSize = encodedClientHelloInner->computeChainDataLength() +
      hpke::getCipherOverhead(echExtension.cipher_suite.aead_id);

  // Make dummy payload.
  echExtension.payload = folly::IOBuf::create(dummyPayloadSize);
  memset(echExtension.payload->writableData(), 0, dummyPayloadSize);
  echExtension.payload->append(dummyPayloadSize);
  Extension ext1;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext1, err, echExtension));
  chloOuterForAAD.extensions.push_back(std::move(ext1));

  // Add grease PSK if passed in
  if (greasePsk.has_value()) {
    Extension ext2;
    FIZZ_RETURN_ON_ERROR(encodeExtension(ext2, err, *greasePsk));
    chloOuterForAAD.extensions.push_back(std::move(ext2));
  }

  // Serialize for AAD
  std::unique_ptr<folly::IOBuf> clientHelloOuterAad;
  FIZZ_RETURN_ON_ERROR(encode(clientHelloOuterAad, err, chloOuterForAAD));

  // Encrypt inner client hello
  echExtension.payload = setupResult.context->seal(
      clientHelloOuterAad.get(), std::move(encodedClientHelloInner));
  return Status::Success;
}

} // namespace

Status encryptClientHelloHRR(
    OuterECHClientHello& ret,
    Error& err,
    const NegotiatedECHConfig& negotiatedECHConfig,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult,
    const folly::Optional<ClientPresharedKey>& greasePsk,
    const std::vector<ExtensionType>& outerExtensionTypes) {
  // Create ECH extension with blank config ID and enc for HRR
  ret.cipher_suite = negotiatedECHConfig.cipherSuite;
  ret.config_id = negotiatedECHConfig.configId;
  ret.enc = folly::IOBuf::create(0);

  FIZZ_RETURN_ON_ERROR(encryptClientHelloImpl(
      err,
      ret,
      clientHelloInner,
      clientHelloOuter,
      setupResult,
      greasePsk,
      negotiatedECHConfig.maxLen,
      outerExtensionTypes));

  return Status::Success;
}

Status encryptClientHello(
    OuterECHClientHello& ret,
    Error& err,
    const NegotiatedECHConfig& negotiatedECHConfig,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult,
    const folly::Optional<ClientPresharedKey>& greasePsk,
    const std::vector<ExtensionType>& outerExtensionTypes) {
  // Create ECH extension
  ret.cipher_suite = negotiatedECHConfig.cipherSuite;
  ret.config_id = negotiatedECHConfig.configId;
  ret.enc = setupResult.enc->clone();

  FIZZ_RETURN_ON_ERROR(encryptClientHelloImpl(
      err,
      ret,
      clientHelloInner,
      clientHelloOuter,
      setupResult,
      greasePsk,
      negotiatedECHConfig.maxLen,
      outerExtensionTypes));

  return Status::Success;
}

Status decryptECHWithContext(
    ClientHello& ret,
    Error& err,
    const ClientHello& clientHelloOuter,
    const ParsedECHConfig& /*echConfig*/,
    HpkeSymmetricCipherSuite& /*cipherSuite*/,
    std::unique_ptr<folly::IOBuf> /*encapsulatedKey*/,
    uint8_t /*configId*/,
    std::unique_ptr<folly::IOBuf> encryptedCh,
    ECHVersion version,
    std::unique_ptr<hpke::HpkeContext>& context) {
  std::unique_ptr<folly::IOBuf> encodedClientHelloInner;
  FIZZ_RETURN_ON_ERROR(extractEncodedClientHelloInner(
      encodedClientHelloInner,
      err,
      version,
      std::move(encryptedCh),
      context,
      clientHelloOuter));

  // Set actual client hello, ECH acceptance
  folly::io::Cursor encodedECHInnerCursor(encodedClientHelloInner.get());
  ClientHello decodedChlo;
  FIZZ_RETURN_ON_ERROR(
      decode<ClientHello>(decodedChlo, err, encodedECHInnerCursor));

  // Skip any padding and check that we don't have any data left.
  encodedECHInnerCursor.skipWhile([](uint8_t b) { return b == 0; });
  if (!encodedECHInnerCursor.isAtEnd()) {
    return err.error("ech padding contains nonzero byte");
  }

  // Replace legacy_session_id that got removed during encryption
  decodedChlo.legacy_session_id = clientHelloOuter.legacy_session_id->clone();

  // Expand extensions
  std::vector<Extension> expandedExtensions;
  FIZZ_RETURN_ON_ERROR(substituteOuterExtensions(
      expandedExtensions,
      err,
      std::move(decodedChlo.extensions),
      clientHelloOuter.extensions));
  decodedChlo.extensions = std::move(expandedExtensions);

  // Update encoding
  Buf originalEncoding;
  FIZZ_RETURN_ON_ERROR(encodeHandshake(originalEncoding, err, decodedChlo));
  decodedChlo.originalEncoding = std::move(originalEncoding);

  ret = std::move(decodedChlo);
  return Status::Success;
}

Status setupDecryptionContext(
    std::unique_ptr<hpke::HpkeContext>& ret,
    Error& err,
    const fizz::Factory& factory,
    const ParsedECHConfig& echConfig,
    HpkeSymmetricCipherSuite cipherSuite,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey,
    std::unique_ptr<KeyExchange> kex,
    uint64_t seqNum) {
  // Get crypto primitive types used for decrypting
  hpke::KDFId kdfId = cipherSuite.kdf_id;
  auto kemId = echConfig.key_config.kem_id;
  NamedGroup group = hpke::getKexGroup(kemId);
  auto hash = getHashFunction(cipherSuite.kdf_id);
  const HasherFactoryWithMetadata* hasherFactory = nullptr;
  FIZZ_RETURN_ON_ERROR(factory.makeHasherFactory(hasherFactory, err, hash));
  auto hkdf =
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::v1(hasherFactory));

  auto dhkem = std::make_unique<DHKEM>(std::move(kex), group, std::move(hkdf));
  auto aeadId = cipherSuite.aead_id;
  auto suiteId = hpke::generateHpkeSuiteId(
      group, hpke::getHashFunction(kdfId), hpke::getCipherSuite(aeadId));

  std::unique_ptr<Aead> aead;
  FIZZ_RETURN_ON_ERROR(factory.makeAead(aead, err, getCipherSuite(aeadId)));

  const HasherFactoryWithMetadata* hasherFactory2 = nullptr;
  FIZZ_RETURN_ON_ERROR(factory.makeHasherFactory(hasherFactory2, err, hash));
  hpke::SetupParam setupParam{
      std::move(dhkem),
      std::move(aead),
      std::make_unique<fizz::hpke::Hkdf>(fizz::hpke::Hkdf::v1(hasherFactory2)),
      std::move(suiteId),
      seqNum};

  std::unique_ptr<folly::IOBuf> info;
  FIZZ_RETURN_ON_ERROR(makeHpkeContextInfoParam(info, err, echConfig));

  ret = hpke::setupWithDecap(
      hpke::Mode::Base,
      encapsulatedKey->coalesce(),
      folly::none,
      std::move(info),
      folly::none,
      std::move(setupParam));
  return Status::Success;
}

Status substituteOuterExtensions(
    std::vector<Extension>& ret,
    Error& err,
    std::vector<Extension>&& chloInnerExt,
    const std::vector<Extension>& chloOuterExt) {
  std::vector<Extension> expandedInnerExt;

  // Track seen types to detect duplicates
  std::unordered_set<ExtensionType> seenTypes;
  auto dupeCheck = [&seenTypes](Error& errInner, ExtensionType t) {
    if (seenTypes.count(t) != 0) {
      return errInner.error(
          "inner client hello has duplicate extensions",
          folly::none,
          Error::Category::OuterExtensions);
    }
    seenTypes.insert(t);
    return Status::Success;
  };

  for (auto& ext : chloInnerExt) {
    // Check for duplicate extension types
    FIZZ_RETURN_ON_ERROR(dupeCheck(err, ext.extension_type));

    if (ExtensionType::ech_outer_extensions != ext.extension_type) {
      expandedInnerExt.push_back(std::move(ext));
    } else {
      // Parse the extension
      OuterExtensions outerExtensions;
      try {
        folly::io::Cursor cursor(ext.extension_data.get());
        FIZZ_RETURN_ON_ERROR(
            getExtension<OuterExtensions>(outerExtensions, err, cursor));
      } catch (...) {
        return err.error(
            "ech_outer_extensions malformed",
            folly::none,
            Error::Category::OuterExtensions);
      }

      // Use the linear approach suggested by the RFC.
      auto outerIt = chloOuterExt.cbegin();
      auto outerEnd = chloOuterExt.cend();
      for (const auto extType : outerExtensions.types) {
        // Check types for dupes
        FIZZ_RETURN_ON_ERROR(dupeCheck(err, extType));

        // Check for ech in outer extensions
        if (extType == ExtensionType::encrypted_client_hello) {
          return err.error(
              "ech is not allowed in outer extensions",
              folly::none,
              Error::Category::OuterExtensions);
        }

        // Scan
        while (outerIt != outerEnd && outerIt->extension_type != extType) {
          outerIt++;
        }

        // If at end, error
        if (outerIt == outerEnd) {
          return err.error(
              "ech outer extensions references a missing extension",
              folly::none,
              Error::Category::OuterExtensions);
        }

        // Add it and increment
        expandedInnerExt.push_back(outerIt->clone());
        outerIt++;
      }
    }
  }

  ret = std::move(expandedInnerExt);
  return Status::Success;
}

} // namespace ech
} // namespace fizz
