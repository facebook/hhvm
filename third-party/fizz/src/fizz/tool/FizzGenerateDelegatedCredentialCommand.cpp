/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialUtils.h>
#include <fizz/extensions/delegatedcred/Serialization.h>
#include <fizz/tool/FizzCommandCommon.h>
#include <fizz/util/Parse.h>
#include <folly/FileUtil.h>

using namespace fizz::extensions;
using namespace folly;

namespace fizz {
namespace tool {
namespace {

void printUsage() {
  // clang-format off
  std::cerr
    << "Usage: gendc args\n"
    << "\n"
    << "Supported arguments:\n"
    << " -cert cert           (PEM format parent certificate. Required)\n"
    << " -key key             (PEM format private key for parent certificate. Required.)\n"
    << " -pass password       (certificate private key password. Default: none)\n"
    << " -credkey key         (PEM format public/private key for credential. Required.)\n"
    << " -credpass password   (credential private key password. Default: none)\n"
    << " -signscheme scheme   (signature scheme to sign the credential with. Default: first scheme supported by cert)\n"
    << " -verifscheme scheme  (signature scheme the credential can use. Default: first scheme supported by credential key)\n"
    << " -validsec seconds    (credential validity in seconds. Default: one day)\n"
    << " -out path            (file to output credential to. Default: none, prints to standard out)\n";
  // clang-format on
}
} // namespace

int fizzGenerateDelegatedCredentialCommand(
    const std::vector<std::string>& args) {
  // clang-format off
  std::string certPath;
  std::string certKeyPath;
  std::string credKeyPath;
  std::string certKeyPass;
  std::string credKeyPass;
  Optional<SignatureScheme> credSignScheme;
  Optional<SignatureScheme> credVerifScheme;
  std::chrono::seconds validSec{86400}; // 1 day
  std::string outPath;

  FizzArgHandlerMap handlers = {
    {"-cert", {true, [&certPath](const std::string& arg) { certPath = arg; }}},
    {"-key", {true, [&certKeyPath](const std::string& arg) { certKeyPath = arg; }}},
    {"-pass", {true, [&certKeyPass](const std::string& arg) { certKeyPass = arg; }}},
    {"-credkey", {true, [&credKeyPath](const std::string& arg) { credKeyPath = arg; }}},
    {"-credpass", {true, [&credKeyPass](const std::string& arg) { credKeyPass = arg; }}},
    {"-signscheme", {true, [&credSignScheme](const std::string& arg) {
        credSignScheme = parse<SignatureScheme>(arg);
    }}},
    {"-verifscheme", {true, [&credVerifScheme](const std::string& arg) {
        credVerifScheme = parse<SignatureScheme>(arg);
    }}},
    {"-validsec", {true, [&validSec](const std::string& arg) {
        validSec = std::chrono::seconds(std::stoul(arg));
    }}},
    {"-out", {true, [&outPath](const std::string& arg) {
        outPath = arg;
    }}}
  };
  // clang-format on

  try {
    if (parseArguments(args, handlers, printUsage)) {
      // Parsing failed, return
      return 1;
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
    return 1;
  }

  if (certPath.empty()) {
    LOG(ERROR) << "-cert is a required argument for gendc";
    printUsage();
    return 1;
  }

  if (certKeyPath.empty()) {
    LOG(ERROR) << "-key is a required argument for gendc";
    printUsage();
    return 1;
  }

  if (credKeyPath.empty()) {
    LOG(ERROR) << "-credkey is a required argument for gendc";
    printUsage();
    return 1;
  }

  std::string certData;
  std::string certKeyData;
  std::string credKeyData;
  if (!readFile(certPath.c_str(), certData)) {
    LOG(ERROR) << "Failed to read certificate";
    return 1;
  } else if (!readFile(certKeyPath.c_str(), certKeyData)) {
    LOG(ERROR) << "Failed to read cert private key";
    return 1;
  } else if (!readFile(credKeyPath.c_str(), credKeyData)) {
    LOG(ERROR) << "Failed to read credential private key";
    return 1;
  }

  try {
    std::shared_ptr<SelfCert> cert;
    if (!certKeyPass.empty()) {
      cert =
          openssl::CertUtils::makeSelfCert(certData, certKeyData, certKeyPass);
    } else {
      cert = openssl::CertUtils::makeSelfCert(certData, certKeyData);
    }

    folly::ssl::EvpPkeyUniquePtr certPrivKey =
        openssl::CertUtils::readPrivateKeyFromBuffer(
            certKeyData, certKeyPass.empty() ? nullptr : &certKeyPass[0]);

    folly::ssl::EvpPkeyUniquePtr credPrivKey =
        openssl::CertUtils::readPrivateKeyFromBuffer(
            credKeyData, credKeyPass.empty() ? nullptr : &credKeyPass[0]);

    if (!credSignScheme) {
      credSignScheme = cert->getSigSchemes().front();
    }

    if (!credVerifScheme) {
      switch (openssl::CertUtils::getKeyType(credPrivKey)) {
        case openssl::KeyType::RSA:
          credVerifScheme =
              openssl::CertUtils::getSigSchemes<openssl::KeyType::RSA>()
                  .front();
          break;
        case openssl::KeyType::P256:
          credVerifScheme =
              openssl::CertUtils::getSigSchemes<openssl::KeyType::P256>()
                  .front();
          break;
        case openssl::KeyType::P384:
          credVerifScheme =
              openssl::CertUtils::getSigSchemes<openssl::KeyType::P384>()
                  .front();
          break;
        case openssl::KeyType::P521:
          credVerifScheme =
              openssl::CertUtils::getSigSchemes<openssl::KeyType::P521>()
                  .front();
          break;
        case openssl::KeyType::ED25519:
          credVerifScheme =
              openssl::CertUtils::getSigSchemes<openssl::KeyType::ED25519>()
                  .front();
          break;
      }
    }

    folly::ssl::BioUniquePtr bio(BIO_new(BIO_s_mem()));
    BUF_MEM* bptr = nullptr;
    if (!PEM_write_bio_X509(bio.get(), cert->getX509().get())) {
      throw std::runtime_error("Failed to re-encode cert");
    }
    BIO_get_mem_ptr(bio.get(), &bptr);
    auto certPem = std::string(bptr->data, bptr->length);
    auto clientCredential = DelegatedCredentialUtils::generateCredential(
        cert,
        certPrivKey,
        credPrivKey,
        *credSignScheme,
        *credVerifScheme,
        CertificateVerifyContext::ClientDelegatedCredential,
        validSec);

    auto serverCredential = DelegatedCredentialUtils::generateCredential(
        std::move(cert),
        certPrivKey,
        credPrivKey,
        *credSignScheme,
        *credVerifScheme,
        CertificateVerifyContext::ServerDelegatedCredential,
        validSec);

    auto clientPem = fizz::extensions::generateDelegatedCredentialPEM(
        fizz::extensions::DelegatedCredentialMode::Client,
        std::move(clientCredential),
        credKeyData);

    auto serverPem = fizz::extensions::generateDelegatedCredentialPEM(
        fizz::extensions::DelegatedCredentialMode::Server,
        std::move(serverCredential),
        credKeyData);

    // Create Combined PEM
    auto pem = clientPem + serverPem + certPem;

    if (outPath.empty()) {
      std::cout << pem;
    } else {
      if (!writeFile(pem, outPath.c_str())) {
        LOG(ERROR) << "Failed to write out credential: " << errnoStr(errno);
        return 1;
      }
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to generate credential: " << e.what();
    return 1;
  }

  return 0;
}

} // namespace tool
} // namespace fizz
