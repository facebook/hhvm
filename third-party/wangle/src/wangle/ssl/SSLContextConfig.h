/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/SSLOptions.h>
#include <set>
#include <string>
#include <vector>

/**
 * SSLContextConfig helps to describe the configs/options for
 * a SSL_CTX. For example:
 *
 *   1. Filename of X509, private key and its password.
 *   2. ciphers list
 *   3. NPN list
 *   4. Is session cache enabled?
 *   5. Is it the default X509 in SNI operation?
 *   6. .... and a few more
 */
namespace wangle {

struct SSLContextConfig {
  SSLContextConfig() = default;
  virtual ~SSLContextConfig() = default;

  struct CertificateInfo {
    CertificateInfo(
        const std::string& crtPath,
        const std::string& kyPath,
        const std::string& passwdPath)
        : certPath(crtPath), keyPath(kyPath), passwordPath(passwdPath) {}

    CertificateInfo(const std::string& crtBuf, const std::string& kyBuf)
        : certPath(crtBuf), keyPath(kyBuf), isBuffer(true) {}

    std::string certPath;
    std::string keyPath;
    std::string passwordPath;
    bool isBuffer{false};
  };

  static const std::string& getDefaultCiphers() {
    static const std::string& defaultCiphers =
        folly::join(':', folly::ssl::SSLServerOptions::ciphers());
    return defaultCiphers;
  }

  static const std::string& getDefaultCiphersuites() {
    static const std::string& defaultCiphersuites =
        folly::join(':', folly::ssl::SSLServerOptions::ciphersuites());
    return defaultCiphersuites;
  }

  struct KeyOffloadParams {
    // What keys do we want to offload
    // Currently supported values: "rsa", "ec" (can also be empty)
    // Note that the corresponding thrift IDL has a list instead
    std::set<std::string> offloadType;
    // An identifier for the service to which we are offloading.
    std::string serviceId{"default"};
    // Whether we want to offload certificates
    bool enableCertOffload{false};
  };

  /**
   * Helpers to set/add a certificate
   */
  virtual void setCertificate(
      const std::string& certPath,
      const std::string& keyPath,
      const std::string& passwordPath) {
    certificates.clear();
    addCertificate(certPath, keyPath, passwordPath);
  }

  void setCertificateBuf(const std::string& cert, const std::string& key) {
    certificates.clear();
    addCertificateBuf(cert, key);
  }

  void addCertificate(
      const std::string& certPath,
      const std::string& keyPath,
      const std::string& passwordPath) {
    certificates.emplace_back(certPath, keyPath, passwordPath);
  }

  void addCertificateBuf(const std::string& cert, const std::string& key) {
    certificates.emplace_back(cert, key);
  }

  /**
   * Set the optional list of protocols to advertise via TLS
   * Next Protocol Negotiation. An empty list means NPN is not enabled.
   */
  void setNextProtocols(const std::list<std::string>& inNextProtocols) {
    nextProtocols.clear();
    nextProtocols.emplace_back(1, inNextProtocols);
  }

  typedef std::function<bool(char const* server_name)> SNINoMatchFn;

  std::vector<CertificateInfo> certificates;
  folly::SSLContext::SSLVersion sslVersion{folly::SSLContext::TLSv1_2};
  bool sessionCacheEnabled{true};
  bool sessionTicketEnabled{true};
  std::string sslCiphers{getDefaultCiphers()};
  std::string sslCiphersuites{getDefaultCiphersuites()};
  folly::Optional<std::string> sigAlgs;
  std::string eccCurveName{"prime256v1"};

  // Weighted lists of NPN strings to advertise
  std::list<folly::SSLContext::NextProtocolsItem> nextProtocols;
  bool isLocalPrivateKey{true};
  // Should this SSLContextConfig be the default for SNI purposes
  bool isDefault{false};
  // File containing trusted CA's to validate client certificates
  std::string clientCAFile;
  // List of files containing trusted CA's to validate client certificates
  std::vector<std::string> clientCAFiles;

  // Verification method to use for client certificates.
  folly::SSLContext::VerifyClientCertificate clientVerification{
      folly::SSLContext::VerifyClientCertificate::ALWAYS};

  // Key offload configuration
  KeyOffloadParams keyOffloadParams;

  // If true, read cert-key files locally. Otherwise, fetch them from cryptossl
  bool offloadDisabled{true};

  // Load cert-key pairs corresponding to these domains
  std::vector<std::string> domains;

  // If true, the certs for this domain is signed by our internal CA
  bool isProdCASigned{false};

  // A namespace to use for sessions generated from this context so that
  // they will not be shared between other sessions generated from the
  // same context. If not specified the vip name will be used by default
  folly::Optional<std::string> sessionContext;

  bool alpnAllowMismatch{true};
};

} // namespace wangle
