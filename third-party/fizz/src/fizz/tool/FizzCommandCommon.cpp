/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/ECCurve.h>
#include <fizz/crypto/exchange/OpenSSLKeyExchange.h>
#include <fizz/crypto/exchange/X25519.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/ech/Types.h>
#include <fizz/tool/FizzCommandCommon.h>
#include <folly/FileUtil.h>
#include <folly/String.h>
#include <folly/base64.h>

#include <fstream>

using namespace folly;

namespace fizz {
namespace tool {

int parseArguments(
    std::vector<std::string> argv,
    FizzArgHandlerMap handlers,
    std::function<void()> usageFunc) {
  for (size_t idx = 2; idx < argv.size(); idx++) {
    auto& argument = argv[idx];
    auto handlerIter = handlers.find(argument);

    // Ignore these.
    if (argument == "-v" || argument == "-vmodule") {
      idx++;
      continue;
    }

    if (handlerIter != handlers.end()) {
      auto& handlerInfo = handlerIter->second;
      std::string variable;
      if (handlerInfo.hasVariable) {
        if (idx + 1 >= argv.size()) {
          std::cerr << "Argument " << argument << " requires an parameter."
                    << std::endl;
          usageFunc();
          return 1;
        } else {
          idx++;
          variable = argv[idx];
        }
      }
      handlerInfo.handler(variable);
    } else {
      std::cerr << "Unknown argument: " << argument << std::endl;
      usageFunc();
      return 1;
    }
  }
  return 0;
}
std::unique_ptr<folly::EventBaseBackendBase> setupBackend(
    FOLLY_MAYBE_UNUSED bool uring,
    FOLLY_MAYBE_UNUSED bool uringAsync,
    FOLLY_MAYBE_UNUSED bool uringRegisterFds,
    FOLLY_MAYBE_UNUSED int32_t uringCapacity,
    FOLLY_MAYBE_UNUSED int32_t uringMaxSubmit,
    FOLLY_MAYBE_UNUSED int32_t uringMaxGet) {
#ifdef FIZZ_TOOL_ENABLE_IO_URING
  if (uring) {
    try {
      folly::PollIoBackend::Options options;
      options.setCapacity(static_cast<size_t>(uringCapacity))
          .setMaxSubmit(static_cast<size_t>(uringMaxSubmit))
          .setMaxGet(static_cast<size_t>(uringMaxGet))
          .setUseRegisteredFds(uringRegisterFds);
      return std::make_unique<folly::IoUringBackend>(options);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Failed to create io_uring backend: " << e.what();
      std::exit(1);
    }
  }
#endif
  return folly::EventBase::getDefaultBackend();
}

TerminalInputHandler::TerminalInputHandler(
    EventBase* evb,
    InputHandlerCallback* cb)
    : EventHandler(evb, folly::NetworkSocket::fromFd(0)), cb_(cb), evb_(evb) {
  registerHandler(EventHandler::READ | EventHandler::PERSIST);
}

void TerminalInputHandler::handlerReady(uint16_t events) noexcept {
  // Handle read ready on stdin, but only once we're connected.
  if (events & EventHandler::READ && cb_->connected()) {
    std::array<char, 512> buf;
    int result = read(0, buf.data(), buf.size());

    if (result > 0) {
      cb_->write(IOBuf::wrapBuffer(buf.data(), result));
    } else {
      if (result < 0) {
        LOG(ERROR) << "Error on terminal read: " << folly::errnoStr(errno);
      }
      hitEOF();
    }
  }
}

void TerminalInputHandler::hitEOF() {
  evb_->runInLoop([cb = cb_]() { cb->close(); });
}

std::vector<Extension> getExtensions(folly::StringPiece hex) {
  auto buf = folly::IOBuf::copyBuffer(folly::unhexlify(hex.toString()));
  auto outBuf = folly::IOBuf::create(0);
  {
    folly::io::Appender appender(outBuf.get(), 16);
    detail::writeBuf<uint16_t>(buf, appender);
  }
  folly::io::Cursor cursor(outBuf.get());
  std::vector<Extension> extensions;
  if (!cursor.isAtEnd()) {
    detail::readVector<uint16_t>(extensions, cursor);
  }
  CHECK(cursor.isAtEnd());
  return extensions;
}

hpke::KEMId getKEMId(std::string kemStr) {
  if (kemStr == "secp256r1") {
    return hpke::KEMId::secp256r1;
  } else if (kemStr == "secp384r1") {
    return hpke::KEMId::secp384r1;
  } else if (kemStr == "secp521r1") {
    return hpke::KEMId::secp521r1;
  } else if (kemStr == "x25519") {
    return hpke::KEMId::x25519;
  }

  // Input doesn't match any KEM id.
  throw std::runtime_error("Input doesn't match any KEM id");
}

std::string tryReadFile(const std::string& echFile) {
  if (echFile.empty()) {
    throw std::runtime_error("No file provided");
  }

  std::string echConfigStr;
  if (!folly::readFile(echFile.c_str(), echConfigStr)) {
    throw std::runtime_error("Unable to read file provided");
  }
  return echConfigStr;
}

folly::Optional<folly::dynamic> readECHConfigsJson(std::string echFile) {
  auto echConfigJson = tryReadFile(echFile);

  auto json = folly::parseJson(echConfigJson);
  if (!json.isObject()) {
    throw std::runtime_error(
        "Unable to parse ECH configs JSON. Please ensure the file matches what is expected."
        "The format is roughly { echconfigs: [ ECHConfig{fields..} ] }."
        "For an actual example, see the test file.");
  }

  return json;
}

folly::Optional<ech::ECHConfigList> parseECHConfigsBase64(
    std::string echConfigListBase64) {
  std::vector<ech::ECHConfig> echConfigs;
  auto configBuf = folly::IOBuf::copyBuffer(
      folly::base64Decode(folly::trimWhitespace(echConfigListBase64)));
  folly::io::Cursor cursor(configBuf.get());
  return decode<ech::ECHConfigList>(cursor);
}

folly::Optional<ech::ECHConfigList> parseECHConfigs(folly::dynamic json) {
  auto getKDFId = [](std::string kdfStr) {
    if (kdfStr == "Sha256") {
      return hpke::KDFId::Sha256;
    } else if (kdfStr == "Sha384") {
      return hpke::KDFId::Sha384;
    } else if (kdfStr == "Sha512") {
      return hpke::KDFId::Sha512;
    }

    // Input doesn't match any KDF id.
    throw std::runtime_error("Input doesn't match any KDF id");
  };

  auto getAeadId = [](std::string aeadStr) {
    if (aeadStr == "TLS_AES_128_GCM_SHA256") {
      return hpke::AeadId::TLS_AES_128_GCM_SHA256;
    } else if (aeadStr == "TLS_AES_256_GCM_SHA384") {
      return hpke::AeadId::TLS_AES_256_GCM_SHA384;
    } else if (aeadStr == "TLS_CHACHA20_POLY1305_SHA256") {
      return hpke::AeadId::TLS_CHACHA20_POLY1305_SHA256;
    }

    // Input doesn't match any Aead id.
    throw std::runtime_error("Input doesn't match any Aead id");
  };

  auto strToNum = [](const std::string& str) {
    size_t processed;
    try {
      auto num = std::stoul(str, &processed, 0);
      if (processed != str.size()) {
        // The outer catch swallows it anyway
        throw std::exception();
      }
      return num;
    } catch (...) {
      throw std::runtime_error("Input is not a valid number");
    }
  };

  auto echConfigs = std::vector<ech::ECHConfig>();
  for (const auto& config : json["echconfigs"]) {
    std::string version = config["version"].asString();

    ech::ECHVersion echVersion;
    if (version == "Draft15") {
      echVersion = ech::ECHVersion::Draft15;
    } else {
      continue;
    }

    ech::ECHConfigContentDraft configContent;
    configContent.public_name =
        folly::IOBuf::copyBuffer(config["public_name"].asString());

    configContent.key_config.config_id =
        folly::to<uint8_t>(strToNum(config["config_id"].asString()));

    configContent.key_config.public_key = folly::IOBuf::copyBuffer(
        folly::unhexlify(config["public_key"].asString()));
    configContent.key_config.kem_id = getKEMId(config["kem_id"].asString());
    configContent.maximum_name_length =
        folly::to<uint8_t>(strToNum(config["maximum_name_length"].asString()));

    // Get ciphersuites.
    auto ciphersuites = std::vector<ech::HpkeSymmetricCipherSuite>();
    for (size_t suiteIndex = 0; suiteIndex < config["cipher_suites"].size();
         ++suiteIndex) {
      const auto& suite = config["cipher_suites"][suiteIndex];

      ech::HpkeSymmetricCipherSuite parsedSuite;
      parsedSuite.kdf_id = getKDFId(suite["kdf_id"].asString());
      parsedSuite.aead_id = getAeadId(suite["aead_id"].asString());

      ciphersuites.push_back(parsedSuite);
    }

    configContent.key_config.cipher_suites = ciphersuites;

    // Get extensions.
    configContent.extensions = getExtensions(config["extensions"].asString());

    ech::ECHConfig parsedConfig;
    parsedConfig.version = echVersion;
    parsedConfig.ech_config_content = encode(std::move(configContent));
    echConfigs.push_back(std::move(parsedConfig));
  }
  if (echConfigs.empty()) {
    return folly::none;
  } else {
    ech::ECHConfigList echConfigList;
    echConfigList.configs = echConfigs;
    return echConfigList;
  }
}

std::vector<ech::ECHConfig> getDefaultECHConfigs() {
  // TODO: Generate a public and private key pair each time,
  // and output it so the user can use it on the client side.
  // This allows the user to more easily use ECH without needing
  // to generate ECH configs and private keys themselves, without having
  // the keys hardcoded.
  LOG(INFO) << "Using default ECH configs.";

  // Set the ECH config content.
  ech::ECHConfigContentDraft echConfigContent;
  echConfigContent.public_name = folly::IOBuf::copyBuffer("publicname");

  echConfigContent.key_config.cipher_suites = {ech::HpkeSymmetricCipherSuite{
      hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256}};
  echConfigContent.maximum_name_length = 100;
  folly::StringPiece cookie{"002c00080006636f6f6b6965"};
  echConfigContent.extensions = getExtensions(cookie);

  // Set default public key (which corresponds to the private key set on the
  // server side).
  echConfigContent.key_config.public_key =
      folly::IOBuf::copyBuffer(folly::unhexlify(
          "8a07563949fac6232936ed6f36c4fa735930ecdeaef6734e314aeac35a56fd0a"));

  // Corresponds to the public key set above.
  echConfigContent.key_config.kem_id = hpke::KEMId::x25519;

  // Assign a config id
  echConfigContent.key_config.config_id = 0xE6;

  // Construct an ECH config to pass in to the client.
  ech::ECHConfig echConfig;
  echConfig.version = ech::ECHVersion::Draft15;
  echConfig.ech_config_content = encode(std::move(echConfigContent));
  auto configs = std::vector<ech::ECHConfig>();
  configs.push_back(std::move(echConfig));

  return configs;
}

} // namespace tool
} // namespace fizz
