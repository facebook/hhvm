/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/server/TicketTypes.h>
#include <fizz/util/FizzUtil.h>

#include <fizz/crypto/exchange/X25519.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/io/async/SSLContext.h>
#include <folly/portability/OpenSSL.h>
#include <folly/ssl/Init.h>
#include <sodium.h>

#include <openssl/bio.h>
#include <fstream>

namespace fizz {

static int passwordCallback(char* password, int size, int, void* data) {
  if (!password || !data || size < 1) {
    LOG(ERROR) << "invalid password buffer, size is " << size;
    return 0;
  }
  std::string userPassword;
  static_cast<folly::PasswordInFile*>(data)->getPassword(userPassword, size);
  if (userPassword.empty()) {
    LOG(ERROR) << "empty private key password";
    return 0;
  }
  auto length = std::min(static_cast<int>(userPassword.size()), size - 1);
  memcpy(password, userPassword.data(), length);
  password[length] = '\0';
  return length;
}

std::vector<folly::ssl::X509UniquePtr> FizzUtil::readChainFile(
    const std::string& filename) {
  std::string certData;
  if (!folly::readFile(filename.c_str(), certData)) {
    throw std::runtime_error(
        folly::to<std::string>("couldn't read cert file: ", filename));
  }
  auto certRange = folly::ByteRange(folly::StringPiece(certData));
  auto certs = folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(certRange);
  if (certs.empty()) {
    throw std::runtime_error(
        folly::to<std::string>("couldn't read any cert from: ", filename));
  }
  return certs;
}

folly::ssl::EvpPkeyUniquePtr FizzUtil::readPrivateKeyFromBuf(
    folly::ByteRange privateKey,
    const std::string& passwordFilename) {
  std::shared_ptr<folly::PasswordInFile> pf;
  if (!passwordFilename.empty()) {
    pf = std::make_shared<folly::PasswordInFile>(passwordFilename);
  }
  try {
    return FizzUtil::decryptPrivateKey(privateKey, pf.get());
  } catch (std::runtime_error&) {
    std::string pwFile = pf ? pf->describe().c_str() : "(none)";
    throw std::runtime_error(
        folly::sformat("failed to decrypt private key; pwFile: {}", pwFile));
  }
}

folly::ssl::EvpPkeyUniquePtr FizzUtil::readPrivateKey(
    const std::string& filename,
    const std::shared_ptr<folly::PasswordInFile>& pf) {
  std::string data;
  folly::readFile(filename.c_str(), data);
  try {
    return FizzUtil::decryptPrivateKey(data, pf.get());
  } catch (std::runtime_error&) {
    std::string pwFile = pf ? pf->describe().c_str() : "(none)";
    auto ex = folly::sformat(
        "Failed to read private key from file: {}, password file: {}",
        filename,
        pwFile);
    std::throw_with_nested(std::runtime_error(ex));
  }
}

folly::ssl::EvpPkeyUniquePtr FizzUtil::readPrivateKey(
    const std::string& filename,
    const std::string& passwordFilename) {
  std::shared_ptr<folly::PasswordInFile> pf;
  if (!passwordFilename.empty()) {
    pf = std::make_shared<folly::PasswordInFile>(passwordFilename);
  }
  return readPrivateKey(filename, pf);
}

folly::ssl::EvpPkeyUniquePtr FizzUtil::decryptPrivateKey(
    const std::string& data,
    folly::PasswordInFile* pf) {
  folly::ByteRange keyBuf((folly::StringPiece(data)));
  return FizzUtil::decryptPrivateKey(keyBuf, pf);
}

folly::ssl::EvpPkeyUniquePtr FizzUtil::decryptPrivateKey(
    folly::ByteRange data,
    folly::PasswordInFile* pf) {
  folly::ssl::BioUniquePtr keyBio(BIO_new_mem_buf(
      const_cast<void*>( // needed by openssl 1.0.2d
          reinterpret_cast<const void*>(data.data())),
      data.size()));
  if (!keyBio) {
    throw std::runtime_error("couldn't create bio");
  }

  folly::ssl::EvpPkeyUniquePtr pkey;
  if (pf) {
    pkey.reset(
        PEM_read_bio_PrivateKey(keyBio.get(), nullptr, passwordCallback, pf));
  } else {
    pkey.reset(
        PEM_read_bio_PrivateKey(keyBio.get(), nullptr, nullptr, nullptr));
  }

  if (!pkey) {
    throw std::runtime_error("couldn't read private key");
  }

  return pkey;
}

std::unique_ptr<KeyExchange> FizzUtil::createKeyExchangeFromBuf(
    hpke::KEMId kemId,
    folly::ByteRange privKey) {
  switch (kemId) {
    case hpke::KEMId::secp256r1: {
      auto kex = std::make_unique<OpenSSLECKeyExchange<P256>>();
      kex->setPrivateKey(readPrivateKeyFromBuf(privKey, ""));
      return kex;
    }
    case hpke::KEMId::secp384r1: {
      auto kex = std::make_unique<OpenSSLECKeyExchange<P384>>();
      kex->setPrivateKey(readPrivateKeyFromBuf(privKey, ""));
      return kex;
    }
    case hpke::KEMId::secp521r1: {
      auto kex = std::make_unique<OpenSSLECKeyExchange<P521>>();
      kex->setPrivateKey(readPrivateKeyFromBuf(privKey, ""));
      return kex;
    }
    case hpke::KEMId::x25519: {
      auto kex = std::make_unique<X25519KeyExchange>();
      kex->setPrivateKey(folly::IOBuf::copyBuffer(
          folly::unhexlify(folly::StringPiece(privKey))));
      return kex;
    }
    default: {
      // We don't support other key exchanges right now.
      break;
    }
  }
  return nullptr;
}

std::vector<std::string> FizzUtil::getAlpnsFromNpnList(
    const std::list<folly::SSLContext::NextProtocolsItem>& list) {
  CHECK(!list.empty());
  auto maxWeight = list.front().weight;
  auto protoList = &list.front().protocols;
  for (const auto& item : list) {
    if (item.weight > maxWeight) {
      protoList = &item.protocols;
    }
  }
  return std::vector<std::string>(protoList->begin(), protoList->end());
}

/**
 * Generates a curve25519 keypair encoded in hex.
 * @return a tuple of keys: {publicKey, privateKey}
 */
[[maybe_unused]] std::tuple<std::string, std::string>
FizzUtil::generateKeypairCurve25519() {
  constexpr static size_t kCurve25519PubBytes = 32;
  constexpr static size_t kCurve25519PrivBytes = 32;
  using PrivKey = std::array<uint8_t, kCurve25519PubBytes>;
  using PubKey = std::array<uint8_t, kCurve25519PrivBytes>;
  static_assert(
      PrivKey().size() == crypto_scalarmult_SCALARBYTES,
      "Incorrect size of the private key");
  static_assert(
      PubKey().size() == crypto_scalarmult_BYTES,
      "Incorrect size of the public key");
  auto privKey = PrivKey();
  auto pubKey = PubKey();
  auto err = crypto_box_curve25519xsalsa20poly1305_keypair(
      pubKey.data(), privKey.data());
  if (err != 0) {
    throw std::runtime_error(
        folly::to<std::string>("Could not generate keys ", err));
  }

  std::string pubKeyHex =
      folly::hexlify(std::string_view((char*)pubKey.data(), pubKey.size()));
  std::string privKeyHex =
      folly::hexlify(std::string_view((char*)privKey.data(), privKey.size()));

  return std::make_tuple(pubKeyHex, privKeyHex);
}

} // namespace fizz
