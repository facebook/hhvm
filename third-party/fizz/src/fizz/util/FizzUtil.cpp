/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/server/TicketTypes.h>
#include <fizz/util/FizzUtil.h>

#include <fizz/backend/libsodium/crypto/exchange/X25519.h>
#include <fmt/core.h>
#include <folly/FileUtil.h>
#include <folly/io/async/SSLContext.h>
#include <folly/portability/OpenSSL.h>
#include <sodium.h>

#include <openssl/bio.h>
#include <fstream>

namespace fizz {

static int passwordCallback(char* password, int size, int, void* data) {
  if (!password || !data || size < 1) {
    FIZZ_LOG(ERROR) << "invalid password buffer, size is " << size;
    return 0;
  }
  std::string userPassword;
  static_cast<folly::PasswordInFile*>(data)->getPassword(userPassword, size);
  if (userPassword.empty()) {
    FIZZ_LOG(ERROR) << "empty private key password";
    return 0;
  }
  auto length = std::min(static_cast<int>(userPassword.size()), size - 1);
  memcpy(password, userPassword.data(), length);
  password[length] = '\0';
  return length;
}

Status FizzUtil::readChainFile(
    std::vector<folly::ssl::X509UniquePtr>& ret,
    Error& err,
    const std::string& filename) {
  std::string certData;
  if (!folly::readFile(filename.c_str(), certData)) {
    return err.error(
        folly::to<std::string>("couldn't read cert file: ", filename));
  }
  auto certRange = folly::ByteRange(folly::StringPiece(certData));
  ret = folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(certRange);
  if (ret.empty()) {
    return err.error(
        folly::to<std::string>("couldn't read any cert from: ", filename));
  }
  return Status::Success;
}

Status FizzUtil::readPrivateKeyFromBuf(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    folly::ByteRange privateKey,
    const std::string& passwordFilename) {
  std::shared_ptr<folly::PasswordInFile> pf;
  if (!passwordFilename.empty()) {
    pf = std::make_shared<folly::PasswordInFile>(passwordFilename);
  }
  try {
    if (FizzUtil::decryptPrivateKey(ret, err, privateKey, pf.get()) ==
        Status::Fail) {
      std::string pwFile = pf ? pf->describe().c_str() : "(none)";
      return err.error(
          fmt::format("failed to decrypt private key; pwFile: {}", pwFile));
    }
  } catch (std::runtime_error&) {
    std::string pwFile = pf ? pf->describe().c_str() : "(none)";
    return err.error(
        fmt::format("failed to decrypt private key; pwFile: {}", pwFile));
  }
  return Status::Success;
}

Status FizzUtil::readPrivateKey(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    const std::string& filename,
    const std::shared_ptr<folly::PasswordInFile>& pf) {
  std::string data;
  folly::readFile(filename.c_str(), data);
  try {
    if (FizzUtil::decryptPrivateKey(ret, err, data, pf.get()) == Status::Fail) {
      std::string pwFile = pf ? pf->describe().c_str() : "(none)";
      return err.error(
          fmt::format(
              "Failed to read private key from file: {}, password file: {}",
              filename,
              pwFile));
    }
  } catch (std::runtime_error&) {
    std::string pwFile = pf ? pf->describe().c_str() : "(none)";
    return err.error(
        fmt::format(
            "Failed to read private key from file: {}, password file: {}",
            filename,
            pwFile));
  }
  return Status::Success;
}

Status FizzUtil::readPrivateKey(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    const std::string& filename,
    const std::string& passwordFilename) {
  std::shared_ptr<folly::PasswordInFile> pf;
  if (!passwordFilename.empty()) {
    pf = std::make_shared<folly::PasswordInFile>(passwordFilename);
  }
  return readPrivateKey(ret, err, filename, pf);
}

Status FizzUtil::decryptPrivateKey(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    const std::string& data,
    folly::PasswordInFile* pf) {
  folly::ByteRange keyBuf((folly::StringPiece(data)));
  return FizzUtil::decryptPrivateKey(ret, err, keyBuf, pf);
}

Status FizzUtil::decryptPrivateKey(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    folly::ByteRange data,
    folly::PasswordInFile* pf) {
  folly::ssl::BioUniquePtr keyBio(BIO_new_mem_buf(
      const_cast<void*>( // needed by openssl 1.0.2d
          reinterpret_cast<const void*>(data.data())),
      data.size()));
  if (!keyBio) {
    return err.error("couldn't create bio");
  }

  if (pf) {
    ret.reset(
        PEM_read_bio_PrivateKey(keyBio.get(), nullptr, passwordCallback, pf));
  } else {
    ret.reset(PEM_read_bio_PrivateKey(keyBio.get(), nullptr, nullptr, nullptr));
  }

  if (!ret) {
    return err.error("couldn't read private key");
  }

  return Status::Success;
}

Status FizzUtil::createKeyExchangeFromBuf(
    std::unique_ptr<KeyExchange>& ret,
    Error& err,
    hpke::KEMId kemId,
    folly::ByteRange privKey) {
  switch (kemId) {
    case hpke::KEMId::secp256r1: {
      std::unique_ptr<openssl::OpenSSLECKeyExchange> kex =
          openssl::makeOpenSSLECKeyExchange<fizz::P256>();
      folly::ssl::EvpPkeyUniquePtr pkey;
      FIZZ_RETURN_ON_ERROR(readPrivateKeyFromBuf(pkey, err, privKey, ""));
      FIZZ_RETURN_ON_ERROR(kex->setPrivateKey(err, std::move(pkey)));
      ret = std::move(kex);
      return Status::Success;
    }
    case hpke::KEMId::secp384r1: {
      std::unique_ptr<openssl::OpenSSLECKeyExchange> kex =
          openssl::makeOpenSSLECKeyExchange<fizz::P384>();
      folly::ssl::EvpPkeyUniquePtr pkey;
      FIZZ_RETURN_ON_ERROR(readPrivateKeyFromBuf(pkey, err, privKey, ""));
      FIZZ_RETURN_ON_ERROR(kex->setPrivateKey(err, std::move(pkey)));
      ret = std::move(kex);
      return Status::Success;
    }
    case hpke::KEMId::secp521r1: {
      std::unique_ptr<openssl::OpenSSLECKeyExchange> kex =
          openssl::makeOpenSSLECKeyExchange<fizz::P521>();
      folly::ssl::EvpPkeyUniquePtr pkey;
      FIZZ_RETURN_ON_ERROR(readPrivateKeyFromBuf(pkey, err, privKey, ""));
      FIZZ_RETURN_ON_ERROR(kex->setPrivateKey(err, std::move(pkey)));
      ret = std::move(kex);
      return Status::Success;
    }
    case hpke::KEMId::x25519: {
      auto kex = std::make_unique<libsodium::X25519KeyExchange>();
      FIZZ_RETURN_ON_ERROR(kex->setPrivateKey(
          err,
          folly::IOBuf::copyBuffer(
              folly::unhexlify(folly::StringPiece(privKey)))));
      ret = std::move(kex);
      return Status::Success;
    }
    default: {
      // We don't support other key exchanges right now.
      break;
    }
  }
  ret = nullptr;
  return Status::Success;
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
Status FizzUtil::generateKeypairCurve25519(
    std::tuple<std::string, std::string>& ret,
    Error& err) {
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
  auto result = crypto_box_curve25519xsalsa20poly1305_keypair(
      pubKey.data(), privKey.data());
  if (result != 0) {
    return err.error(
        folly::to<std::string>("Could not generate keys ", result));
  }

  std::string pubKeyHex =
      folly::hexlify(std::string_view((char*)pubKey.data(), pubKey.size()));
  std::string privKeyHex =
      folly::hexlify(std::string_view((char*)privKey.data(), privKey.size()));

  ret = std::make_tuple(pubKeyHex, privKeyHex);
  return Status::Success;
}

} // namespace fizz
