/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/SecondaryAuthManager.h>

#include <fizz/crypto/test/TestUtil.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>
#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/ssl/Init.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace proxygen;
using namespace fizz;
using namespace fizz::test;
using namespace folly;

StringPiece expected_auth_request = {
    "120000303132333435363738396162636465660008000d000400020403"};

StringPiece expected_cert = {
    "308201ee30820195a003020102020900c569eec901ce86d9300a06082a8648ce3d04030230"
    "54310b3009060355040613025553310b300906035504080c024e59310b300906035504070c"
    "024e59310d300b060355040a0c0446697a7a310d300b060355040b0c0446697a7a310d300b"
    "06035504030c0446697a7a301e170d3137303430343138323930395a170d34313131323431"
    "38323930395a3054310b3009060355040613025553310b300906035504080c024e59310b30"
    "0906035504070c024e59310d300b060355040a0c0446697a7a310d300b060355040b0c0446"
    "697a7a310d300b06035504030c0446697a7a3059301306072a8648ce3d020106082a8648ce"
    "3d030107034200049d87bcaddb65d8dcf6df8b148a9679b5b710db19c95a9badfff13468cb"
    "358b4e21d24a5c826112658ebb96d64e2985dfb41c1948334391a4aa81b67837e2dbf0a350"
    "304e301d0603551d0e041604143c5b8ba954d9752faf3c8ad6d1a62449dccaa850301f0603"
    "551d230418301680143c5b8ba954d9752faf3c8ad6d1a62449dccaa850300c0603551d1304"
    "0530030101ff300a06082a8648ce3d04030203470030440220349b7d34d7132fb2756576e0"
    "bfa36cbe1723337a7a6f5ef9c8d3bf1aa7efa4a5022025c50a91e0aa4272f1f52c3d5583a7"
    "d7cee14b178835273a0bd814303e62d714"};

TEST(SecondaryAuthManagerTest, AuthenticatorRequest) {
  auto certRequestContext = folly::IOBuf::copyBuffer("0123456789abcdef");
  fizz::SignatureAlgorithms sigAlgs;
  sigAlgs.supported_signature_algorithms.push_back(
      SignatureScheme::ecdsa_secp256r1_sha256);
  std::vector<fizz::Extension> extensions;
  extensions.push_back(encodeExtension(std::move(sigAlgs)));
  SecondaryAuthManager authManager;
  auto authRequestPair = authManager.createAuthRequest(
      std::move(certRequestContext), std::move(extensions));
  auto requestId = authRequestPair.first;
  auto authRequest = std::move(authRequestPair.second);
  EXPECT_EQ(requestId, 0);
  EXPECT_EQ(expected_auth_request,
            StringPiece(hexlify(authRequest->coalesce())));
}

TEST(SecondaryAuthManagerTest, Authenticator) {
  folly::ssl::init();
  // Instantiate a SecondaryAuthManager.
  auto cert = fizz::test::getCert(kP256Certificate);
  auto key = fizz::test::getPrivateKey(kP256Key);
  std::vector<folly::ssl::X509UniquePtr> certs;
  certs.push_back(std::move(cert));
  std::unique_ptr<fizz::SelfCert> certPtr =
      std::make_unique<OpenSSLSelfCertImpl<KeyType::P256>>(std::move(key),
                                                           std::move(certs));
  EXPECT_NE(certPtr, nullptr);
  SecondaryAuthManager authManager(std::move(certPtr));
  // Genearte an authenticator request.
  auto certRequestContext = folly::IOBuf::copyBuffer("0123456789abcdef");
  fizz::SignatureAlgorithms sigAlgs;
  sigAlgs.supported_signature_algorithms.push_back(
      SignatureScheme::ecdsa_secp256r1_sha256);
  std::vector<fizz::Extension> extensions;
  extensions.push_back(encodeExtension(std::move(sigAlgs)));
  auto authRequestPair = authManager.createAuthRequest(
      std::move(certRequestContext), std::move(extensions));
  auto requestId = authRequestPair.first;
  auto authRequest = std::move(authRequestPair.second);

  // Generate an authenticator.
  MockAsyncFizzBase fizzBase;
  EXPECT_CALL(fizzBase, getCipher()).WillRepeatedly(InvokeWithoutArgs([]() {
    folly::Optional<CipherSuite> cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
    return cipher;
  }));
  EXPECT_CALL(fizzBase, getSupportedSigSchemes())
      .WillRepeatedly(InvokeWithoutArgs([]() {
        std::vector<SignatureScheme> schemes = {
            SignatureScheme::ecdsa_secp256r1_sha256};
        return schemes;
      }));
  EXPECT_CALL(fizzBase, _getExportedKeyingMaterial(_, _, _))
      .WillRepeatedly(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("exportedmaterial"); }));
  auto authenticatorPair =
      authManager.getAuthenticator(fizzBase,
                                   TransportDirection::UPSTREAM,
                                   requestId,
                                   std::move(authRequest));
  auto certId = authenticatorPair.first;
  auto authenticator = std::move(authenticatorPair.second);

  // Validate the authenticator.
  auto isValid = authManager.validateAuthenticator(
      fizzBase, TransportDirection::UPSTREAM, certId, std::move(authenticator));
  auto cachedCertId = authManager.getCertId(requestId);
  EXPECT_TRUE(cachedCertId.has_value());
  EXPECT_EQ(*cachedCertId, certId);
  auto peerCert = authManager.getPeerCert(certId);
  EXPECT_TRUE(peerCert.has_value());
  EXPECT_EQ((*peerCert).size(), 1);
  EXPECT_EQ(expected_cert,
            StringPiece(hexlify(((*peerCert)[0].cert_data)->coalesce())));
  EXPECT_TRUE(isValid);
}
