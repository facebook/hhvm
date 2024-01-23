/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/ech/Encryption.h>
#include "fizz/record/Types.h"

#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/Sha384.h>
#include <fizz/crypto/Sha512.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/protocol/Protocol.h>
#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/Types.h>
#include <iterator>

namespace fizz {
namespace ech {

namespace {

std::unique_ptr<folly::IOBuf> makeClientHelloOuterForAad(
    const ClientHello& clientHelloOuter) {
  // Copy client hello outer
  ClientHello chloCopy = clientHelloOuter.clone();

  // Zero the ech extension from the copy
  auto it = std::find_if(
      chloCopy.extensions.begin(), chloCopy.extensions.end(), [](auto& e) {
        return e.extension_type == ExtensionType::encrypted_client_hello;
      });

  folly::io::Cursor cursor(it->extension_data.get());
  auto echExtension = getExtension<OuterECHClientHello>(cursor);

  // Create a zeroed out version of the payload
  size_t payloadSize = echExtension.payload->computeChainDataLength();
  echExtension.payload = folly::IOBuf::create(payloadSize);
  memset(echExtension.payload->writableData(), 0, payloadSize);
  echExtension.payload->append(payloadSize);

  *it = encodeExtension(echExtension);

  // Get the serialized version of the client hello outer
  // without the ECH extension to use
  return encode(chloCopy);
}

std::unique_ptr<folly::IOBuf> extractEncodedClientHelloInner(
    ECHVersion version,
    std::unique_ptr<folly::IOBuf> encryptedCh,
    std::unique_ptr<hpke::HpkeContext>& context,
    const ClientHello& clientHelloOuter) {
  std::unique_ptr<folly::IOBuf> encodedClientHelloInner;
  switch (version) {
    case ECHVersion::Draft15: {
      auto aadCH = makeClientHelloOuterForAad(clientHelloOuter);
      encodedClientHelloInner =
          context->open(aadCH.get(), std::move(encryptedCh));
    }
  }
  return encodedClientHelloInner;
}

std::unique_ptr<folly::IOBuf> makeHpkeContextInfoParam(
    const ECHConfig& echConfig) {
  switch (echConfig.version) {
    case ECHVersion::Draft15: {
      // The "info" parameter to setupWithEncap is the
      // concatenation of "tls ech", a zero byte, and the serialized
      // ECHConfig.
      std::string tlsEchPrefix = "tls ech";
      tlsEchPrefix += '\0';
      auto bufContents = folly::IOBuf::copyBuffer(tlsEchPrefix);
      bufContents->prependChain(encode(echConfig));

      return bufContents;
    }
  }
  return nullptr;
}

bool isValidPublicName(const std::string& publicName) {
  // Starts/ends with a dot.
  if (publicName.front() == '.' || publicName.back() == '.') {
    return false;
  }

  std::vector<std::string> parts;
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
static bool echConfigHasMandatoryExtension(
    const ECHConfigContentDraft& config) {
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

folly::Optional<SupportedECHConfig> selectECHConfig(
    const std::vector<ECHConfig>& configs,
    std::vector<hpke::KEMId> supportedKEMs,
    std::vector<hpke::AeadId> supportedAeads) {
  // Received set of configs is in order of server preference so
  // we should be selecting the first one that we can support.
  for (const auto& config : configs) {
    folly::io::Cursor cursor(config.ech_config_content.get());
    if (config.version == ECHVersion::Draft15) {
      auto echConfig = decode<ECHConfigContentDraft>(cursor);

      // Before anything else, check if the config has mandatory extensions.
      // We don't support any extensions, so if any are mandatory, skip this
      // config.
      if (echConfigHasMandatoryExtension(echConfig)) {
        VLOG(8) << "ECH config has mandatory extension, skipping...";
        continue;
      }

      // Check for an invalid public name and skip if found.
      std::string publicName = echConfig.public_name->cloneCoalescedAsValue()
                                   .moveToFbString()
                                   .toStdString();
      if (!isValidPublicName(publicName)) {
        VLOG(8) << publicName << " isn't a valid public name";
        continue;
      }

      // Check if we (client) support the server's chosen KEM.
      auto result = std::find(
          supportedKEMs.begin(),
          supportedKEMs.end(),
          echConfig.key_config.kem_id);
      if (result == supportedKEMs.end()) {
        continue;
      }

      // Check if we (client) support the HPKE cipher suite.
      auto& cipherSuites = echConfig.key_config.cipher_suites;
      for (const auto& suite : cipherSuites) {
        auto isCipherSupported =
            std::find(
                supportedAeads.begin(), supportedAeads.end(), suite.aead_id) !=
            supportedAeads.end();
        if (isCipherSupported) {
          auto associatedCipherKdf =
              hpke::getKDFId(getHashFunction(getCipherSuite(suite.aead_id)));
          if (suite.kdf_id == associatedCipherKdf) {
            auto supportedConfig = config;
            auto configId = echConfig.key_config.config_id;
            auto maxLen = echConfig.maximum_name_length;
            return SupportedECHConfig{supportedConfig, configId, maxLen, suite};
          }
        }
      }
    }
  }
  return folly::none;
}

static hpke::SetupParam getSetupParam(
    std::unique_ptr<DHKEM> dhkem,
    std::unique_ptr<folly::IOBuf> prefix,
    hpke::KEMId kemId,
    const HpkeSymmetricCipherSuite& cipherSuite) {
  // Get suite id
  auto group = getKexGroup(kemId);
  auto hash = getHashFunction(cipherSuite.kdf_id);
  auto suite = getCipherSuite(cipherSuite.aead_id);
  auto suiteId = hpke::generateHpkeSuiteId(group, hash, suite);

  auto hkdf = hpke::makeHpkeHkdf(std::move(prefix), cipherSuite.kdf_id);

  return hpke::SetupParam{
      std::move(dhkem),
      makeCipher(cipherSuite.aead_id),
      std::move(hkdf),
      std::move(suiteId),
      0};
}

std::unique_ptr<folly::IOBuf> getRecordDigest(
    const ECHConfig& echConfig,
    hpke::KDFId id) {
  switch (id) {
    case hpke::KDFId::Sha256: {
      std::array<uint8_t, fizz::Sha256::HashLen> recordDigest;
      fizz::Sha256::hash(
          *encode(echConfig),
          folly::MutableByteRange(recordDigest.data(), recordDigest.size()));
      return folly::IOBuf::copyBuffer(recordDigest);
    }
    case hpke::KDFId::Sha384: {
      std::array<uint8_t, fizz::Sha384::HashLen> recordDigest;
      fizz::Sha384::hash(
          *encode(echConfig),
          folly::MutableByteRange(recordDigest.data(), recordDigest.size()));
      return folly::IOBuf::copyBuffer(recordDigest);
    }
    case hpke::KDFId::Sha512: {
      std::array<uint8_t, fizz::Sha512::HashLen> recordDigest;
      fizz::Sha512::hash(
          *encode(echConfig),
          folly::MutableByteRange(recordDigest.data(), recordDigest.size()));
      return folly::IOBuf::copyBuffer(recordDigest);
    }
    default:
      throw std::runtime_error("kdf: not implemented");
  }
}

hpke::SetupResult constructHpkeSetupResult(
    std::unique_ptr<KeyExchange> kex,
    const SupportedECHConfig& supportedConfig) {
  const std::unique_ptr<folly::IOBuf> prefix =
      folly::IOBuf::copyBuffer("HPKE-v1");

  folly::io::Cursor cursor(supportedConfig.config.ech_config_content.get());
  auto config = decode<ECHConfigContentDraft>(cursor);
  auto cipherSuite = supportedConfig.cipherSuite;

  // Get shared secret
  auto hkdf = hpke::makeHpkeHkdf(prefix->clone(), cipherSuite.kdf_id);
  std::unique_ptr<DHKEM> dhkem = std::make_unique<DHKEM>(
      std::move(kex), getKexGroup(config.key_config.kem_id), std::move(hkdf));

  // Get context
  std::unique_ptr<folly::IOBuf> info =
      makeHpkeContextInfoParam(supportedConfig.config);

  return setupWithEncap(
      hpke::Mode::Base,
      config.key_config.public_key->clone()->coalesce(),
      std::move(info),
      folly::none,
      getSetupParam(
          std::move(dhkem),
          prefix->clone(),
          config.key_config.kem_id,
          cipherSuite));
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

std::vector<uint8_t> calculateAcceptConfirmation(
    const ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  // Acceptance is done by feeding a dummy hello into the transcript and
  // deriving a secret from it.
  auto shloEch = makeDummyServerHello(shlo);
  context->appendToTranscript(encodeHandshake(std::move(shloEch)));

  auto hsc = context->getHandshakeContext();
  auto echAcceptance = scheduler->getSecret(
      EarlySecrets::ECHAcceptConfirmation, hsc->coalesce());

  return std::move(echAcceptance.secret);
}

std::vector<uint8_t> calculateAcceptConfirmation(
    const HelloRetryRequest& hrr,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  // Acceptance is done by zeroing the confirmation extension,
  // putting it into the transcript, and deriving a secret.
  auto hrrEch = makeDummyHRR(hrr);
  context->appendToTranscript(encodeHandshake(std::move(hrrEch)));

  auto hsc = context->getHandshakeContext();
  auto echAcceptance = scheduler->getSecret(
      EarlySecrets::HRRECHAcceptConfirmation, hsc->coalesce());

  if (echAcceptance.secret.size() < kEchAcceptConfirmationSize) {
    VLOG(8) << "ECH acceptance secret too small?";
    throw std::runtime_error("ech acceptance secret too small");
  }

  return std::move(echAcceptance.secret);
}

} // namespace

bool checkECHAccepted(
    const ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  auto acceptConfirmation = calculateAcceptConfirmation(
      shlo, std::move(context), std::move(scheduler));
  // ECH accepted if the 8 bytes match the accept_confirmation
  return memcmp(
             shlo.random.data() +
                 (shlo.random.size() - kEchAcceptConfirmationSize),
             acceptConfirmation.data(),
             kEchAcceptConfirmationSize) == 0;
}

bool checkECHAccepted(
    const HelloRetryRequest& hrr,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  auto acceptConfirmation = calculateAcceptConfirmation(
      hrr, std::move(context), std::move(scheduler));

  // ECH accepted if the 8 bytes match the accept_confirmation in the
  // extension
  auto echConf = getExtension<ECHHelloRetryRequest>(hrr.extensions);
  if (!echConf) {
    VLOG(8) << "HRR ECH extension missing, rejected...";
    return false;
  }

  return memcmp(
             echConf->confirmation.data(),
             acceptConfirmation.data(),
             kEchAcceptConfirmationSize) == 0;
}

void setAcceptConfirmation(
    ServerHello& shlo,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  auto acceptConfirmation = calculateAcceptConfirmation(
      shlo, std::move(context), std::move(scheduler));

  // Copy the acceptance confirmation bytes to the end
  memcpy(
      shlo.random.data() + (shlo.random.size() - kEchAcceptConfirmationSize),
      acceptConfirmation.data(),
      kEchAcceptConfirmationSize);
}

void setAcceptConfirmation(
    HelloRetryRequest& hrr,
    std::unique_ptr<HandshakeContext> context,
    std::unique_ptr<KeyScheduler> scheduler) {
  // Add an ECH confirmation extension. The calculation code will ignore its
  // contents but expects it to be there.
  hrr.extensions.push_back(encodeExtension(ECHHelloRetryRequest()));

  // Calculate it.
  auto acceptConfirmation = calculateAcceptConfirmation(
      hrr, std::move(context), std::move(scheduler));

  // Copy the acceptance confirmation bytes to the payload
  memcpy(
      hrr.extensions.back().extension_data->writableData(),
      acceptConfirmation.data(),
      kEchAcceptConfirmationSize);
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
      size_t identitySize = identity.psk_identity->computeChainDataLength();
      greaseIdentity.psk_identity = factory->makeRandomBytes(identitySize);
    }
    greaseIdentity.obfuscated_ticket_age = factory->makeTicketAgeAdd();
    grease.identities.push_back(std::move(greaseIdentity));

    const auto& binder = source.binders.at(i);
    PskBinder greaseBinder;
    size_t binderSize = binder.binder->computeChainDataLength();
    greaseBinder.binder = factory->makeRandomBytes(binderSize);
    grease.binders.push_back(std::move(greaseBinder));
  }
  return grease;
}
} // namespace

folly::Optional<ClientPresharedKey> generateGreasePSK(
    const ClientHello& chloInner,
    const Factory* factory) {
  auto innerPsk = getExtension<ClientPresharedKey>(chloInner.extensions);
  if (!innerPsk) {
    return folly::none;
  }

  // For client hello, don't preserve identity.
  return generateGreasePskCommon(*innerPsk, factory, false);
}

ClientPresharedKey generateGreasePSKForHRR(
    const ClientPresharedKey& previousPsk,
    const Factory* factory) {
  // This PSK was the one sent before (i.e. with a random identity). We want to
  // keep it.
  return generateGreasePskCommon(previousPsk, factory, true);
}

size_t calculateECHPadding(
    const ClientHello& chlo,
    size_t encodedSize,
    size_t maxLen) {
  size_t padding = 0;
  auto sni = getExtension<ServerNameList>(chlo.extensions);
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
  return padding;
}

namespace {

void encryptClientHelloShared(
    OuterECHClientHello& echExtension,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult,
    const folly::Optional<ClientPresharedKey>& greasePsk,
    size_t maxLen) {
  // Remove legacy_session_id and serialize the client hello inner
  auto chloInnerCopy = clientHelloInner.clone();
  chloInnerCopy.legacy_session_id = folly::IOBuf::copyBuffer("");
  auto encodedClientHelloInner = encode(chloInnerCopy);

  size_t padding = calculateECHPadding(
      clientHelloInner,
      encodedClientHelloInner->computeChainDataLength(),
      maxLen);
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
      makeCipher(echExtension.cipher_suite.aead_id)->getCipherOverhead();

  // Make dummy payload.
  echExtension.payload = folly::IOBuf::create(dummyPayloadSize);
  memset(echExtension.payload->writableData(), 0, dummyPayloadSize);
  echExtension.payload->append(dummyPayloadSize);
  chloOuterForAAD.extensions.push_back(encodeExtension(echExtension));

  // Add grease PSK if passed in
  if (greasePsk.has_value()) {
    chloOuterForAAD.extensions.push_back(encodeExtension(*greasePsk));
  }

  // Serialize for AAD
  auto clientHelloOuterAad = encode(chloOuterForAAD);

  // Encrypt inner client hello
  echExtension.payload = setupResult.context->seal(
      clientHelloOuterAad.get(), std::move(encodedClientHelloInner));
}

} // namespace

OuterECHClientHello encryptClientHelloHRR(
    const SupportedECHConfig& supportedConfig,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult,
    const folly::Optional<ClientPresharedKey>& greasePsk) {
  // Create ECH extension with blank config ID and enc for HRR
  OuterECHClientHello echExtension;
  echExtension.cipher_suite = supportedConfig.cipherSuite;
  echExtension.config_id = supportedConfig.configId;
  echExtension.enc = folly::IOBuf::create(0);

  encryptClientHelloShared(
      echExtension,
      clientHelloInner,
      clientHelloOuter,
      setupResult,
      greasePsk,
      supportedConfig.maxLen);

  return echExtension;
}

OuterECHClientHello encryptClientHello(
    const SupportedECHConfig& supportedConfig,
    const ClientHello& clientHelloInner,
    const ClientHello& clientHelloOuter,
    hpke::SetupResult& setupResult,
    const folly::Optional<ClientPresharedKey>& greasePsk) {
  // Create ECH extension
  OuterECHClientHello echExtension;
  echExtension.cipher_suite = supportedConfig.cipherSuite;
  echExtension.config_id = supportedConfig.configId;
  echExtension.enc = setupResult.enc->clone();

  encryptClientHelloShared(
      echExtension,
      clientHelloInner,
      clientHelloOuter,
      setupResult,
      greasePsk,
      supportedConfig.maxLen);

  return echExtension;
}

ClientHello decryptECHWithContext(
    const ClientHello& clientHelloOuter,
    const ECHConfig& /*echConfig*/,
    HpkeSymmetricCipherSuite& /*cipherSuite*/,
    std::unique_ptr<folly::IOBuf> /*encapsulatedKey*/,
    uint8_t /*configId*/,
    std::unique_ptr<folly::IOBuf> encryptedCh,
    ECHVersion version,
    std::unique_ptr<hpke::HpkeContext>& context) {
  auto encodedClientHelloInner = extractEncodedClientHelloInner(
      version, std::move(encryptedCh), context, clientHelloOuter);

  // Set actual client hello, ECH acceptance
  folly::io::Cursor encodedECHInnerCursor(encodedClientHelloInner.get());
  auto decodedChlo = decode<ClientHello>(encodedECHInnerCursor);

  // Skip any padding and check that we don't have any data left.
  encodedECHInnerCursor.skipWhile([](uint8_t b) { return b == 0; });
  if (!encodedECHInnerCursor.isAtEnd()) {
    throw std::runtime_error("ech padding contains nonzero byte");
  }

  // Replace legacy_session_id that got removed during encryption
  decodedChlo.legacy_session_id = clientHelloOuter.legacy_session_id->clone();

  // Expand extensions
  auto expandedExtensions = substituteOuterExtensions(
      std::move(decodedChlo.extensions), clientHelloOuter.extensions);
  decodedChlo.extensions = std::move(expandedExtensions);

  // Update encoding
  decodedChlo.originalEncoding = encodeHandshake(decodedChlo);

  return decodedChlo;
}

std::unique_ptr<hpke::HpkeContext> setupDecryptionContext(
    const ECHConfig& echConfig,
    HpkeSymmetricCipherSuite cipherSuite,
    const std::unique_ptr<folly::IOBuf>& encapsulatedKey,
    std::unique_ptr<KeyExchange> kex,
    uint64_t seqNum) {
  const std::unique_ptr<folly::IOBuf> prefix =
      folly::IOBuf::copyBuffer("HPKE-v1");

  // Get crypto primitive types used for decrypting
  hpke::KDFId kdfId = cipherSuite.kdf_id;
  folly::io::Cursor echConfigCursor(echConfig.ech_config_content.get());
  auto decodedConfigContent = decode<ECHConfigContentDraft>(echConfigCursor);
  auto kemId = decodedConfigContent.key_config.kem_id;
  NamedGroup group = hpke::getKexGroup(kemId);

  auto dhkem = std::make_unique<DHKEM>(
      std::move(kex), group, hpke::makeHpkeHkdf(prefix->clone(), kdfId));
  auto aeadId = cipherSuite.aead_id;
  auto suiteId = hpke::generateHpkeSuiteId(
      group, hpke::getHashFunction(kdfId), hpke::getCipherSuite(aeadId));

  hpke::SetupParam setupParam{
      std::move(dhkem),
      makeCipher(aeadId),
      hpke::makeHpkeHkdf(prefix->clone(), kdfId),
      std::move(suiteId),
      seqNum};

  std::unique_ptr<folly::IOBuf> info = makeHpkeContextInfoParam(echConfig);

  return hpke::setupWithDecap(
      hpke::Mode::Base,
      encapsulatedKey->coalesce(),
      folly::none,
      std::move(info),
      folly::none,
      std::move(setupParam));
}

std::vector<Extension> substituteOuterExtensions(
    std::vector<Extension>&& innerExt,
    const std::vector<Extension>& outerExt) {
  std::vector<Extension> expandedInnerExt;

  // This will throw if we duplicate an extension (or if we try to put an
  // ech_outer_extensions in the resulting inner chlo)
  std::unordered_set<ExtensionType> seenTypes;
  auto dupeCheck = [&seenTypes](ExtensionType t) {
    if (seenTypes.count(t) != 0) {
      throw OuterExtensionsError("inner client hello has duplicate extensions");
    }
    seenTypes.insert(t);
  };

  for (auto& ext : innerExt) {
    dupeCheck(ext.extension_type);
    if (ExtensionType::ech_outer_extensions != ext.extension_type) {
      expandedInnerExt.push_back(std::move(ext));
    } else {
      // Parse the extension
      OuterExtensions outerExtensions;
      try {
        folly::io::Cursor cursor(ext.extension_data.get());
        outerExtensions = getExtension<OuterExtensions>(cursor);
      } catch (...) {
        throw OuterExtensionsError("ech_outer_extensions malformed");
      }

      // Use the linear approach suggested by the RFC.
      auto outerIt = outerExt.cbegin();
      auto outerEnd = outerExt.cend();
      for (const auto extType : outerExtensions.types) {
        // Check types for dupes and ech
        dupeCheck(extType);
        if (extType == ExtensionType::encrypted_client_hello) {
          throw OuterExtensionsError("ech is not allowed in outer extensions");
        }

        // Scan
        while (outerIt != outerEnd && outerIt->extension_type != extType) {
          outerIt++;
        }

        // If at end, error
        if (outerIt == outerEnd) {
          throw OuterExtensionsError(
              "ech outer extensions references a missing extension");
        }

        // Add it and increment
        expandedInnerExt.push_back(outerIt->clone());
        outerIt++;
      }
    }
  }

  return expandedInnerExt;
}

} // namespace ech
} // namespace fizz
