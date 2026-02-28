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

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <folly/portability/Sockets.h>

#include <openssl/ssl.h>

#include <folly/SocketAddress.h>
#include <folly/io/async/SSLContext.h>

using folly::SocketAddress;
using folly::SSLContext;
using folly::ssl::OpenSSLUtils;

class X509Cert {
 public:
  explicit X509Cert(folly::StringPiece cert) {
    ctx_.loadCertificateFromBufferPEM(cert);
    ssl_ = ctx_.createSSL();
    CHECK(ssl_ != nullptr);
  }
  ~X509Cert() { SSL_free(ssl_); }
  X509* getX509() { return SSL_get_certificate(ssl_); }

 private:
  X509Cert(const X509Cert&) = delete;
  X509Cert& operator=(const X509Cert&) = delete;

  SSLContext ctx_;
  SSL* ssl_;
};

TEST(SSLUtilsTest, ValidatePeerCertNamesIPSanityTest) {
  X509Cert cert(R"(
-----BEGIN CERTIFICATE-----
MIIDKzCCAhOgAwIBAgIBCjANBgkqhkiG9w0BAQUFADBFMQswCQYDVQQGEwJVUzEP
MA0GA1UECgwGVGhyaWZ0MSUwIwYDVQQDDBxUaHJpZnQgQ2VydGlmaWNhdGUgQXV0
aG9yaXR5MB4XDTE0MDUxNjIwMjg1MloXDTQxMTAwMTIwMjg1MlowRjELMAkGA1UE
BhMCVVMxDTALBgNVBAgTBE9oaW8xETAPBgNVBAcTCEhpbGxpYXJkMRUwEwYDVQQD
EwxBc294IENvbXBhbnkwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCz
ZGrJ5XQHAuMYHlBgn32OOc9l0n3RjXccio2ceeWctXkSxDP3vFyZ4kBILF1lsY1g
o8UTjMkqSDYcytCLK0qavrv9BZRLB9FcpqJ9o4V9feaI/HsHa8DYHyEs8qyNTTNG
YQ3i4j+AA9iDSpezIYy/tyAOAjrSquUW1jI4tzKTBh8hk8MAMvR2/NPHPkrp4gI+
EMH6u4vWdr4F9bbriLFWoU04T9mWOMk7G+h8BS9sgINg2+v5cWvl3BC4kLk5L1yJ
FEyuofSSCEEe6dDf7uVh+RPKa4hEkIYo31AEOPFrN56d+pCj/5l67HTWXoQx3rjy
dNXMvgU75urm6TQe8dB5AgMBAAGjJTAjMCEGA1UdEQQaMBiHBH8AAAGHEAAAAAAA
AAAAAAAAAAAAAAEwDQYJKoZIhvcNAQEFBQADggEBAD26XYInaEvlWZJYgtl3yQyC
3NRQc3LG7XxWg4aFdXCxYLPRAL2HLoarKYH8GPFso57t5xnhA8WfP7iJxmgsKdCS
0pNIicOWsMmXvYLib0j9tMCFR+a8rn3f4n+clwnqas4w/vWBJUoMgyxtkP8NNNZO
kIl02JKRhuyiFyPLilVp5tu0e+lmyUER+ak53WjLq2yoytYAlHkzkOpc4MZ/TNt5
UTEtx/WVlZvlrPi3dsi7QikkjQgo1wCnm7owtuAHlPDMAB8wKk4+vvIOjsGM33T/
8ffq/4X1HeYM0w0fM+SVlX1rwkXA1RW/jn48VWFHpWbE10+m196OdiToGfm2OJI=
-----END CERTIFICATE-----
  )");
  folly::SocketAddress addr;

  addr.setFromIpPort("127.0.0.1", 1);
  sockaddr_storage addrStorage;
  sockaddr* addr_ptr = reinterpret_cast<sockaddr*>(&addrStorage);
  addr.getAddress(&addrStorage);
  EXPECT_TRUE(
      OpenSSLUtils::validatePeerCertNames(
          cert.getX509(), addr_ptr, addr.getActualSize()));
  addr.setFromIpPort("::1", 1);
  addr.getAddress(&addrStorage);
  EXPECT_TRUE(
      OpenSSLUtils::validatePeerCertNames(
          cert.getX509(), addr_ptr, addr.getActualSize()));
  addr.setFromIpPort("127.0.0.2", 1);
  addr.getAddress(&addrStorage);
  EXPECT_FALSE(
      OpenSSLUtils::validatePeerCertNames(
          cert.getX509(), addr_ptr, addr.getActualSize()));
  addr.setFromIpPort("::2", 1);
  addr.getAddress(&addrStorage);
  EXPECT_FALSE(
      OpenSSLUtils::validatePeerCertNames(
          cert.getX509(), addr_ptr, addr.getActualSize()));
}
