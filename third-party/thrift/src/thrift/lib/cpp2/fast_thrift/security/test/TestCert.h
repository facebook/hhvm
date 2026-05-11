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

#include <string>
#include <vector>

#include <openssl/pem.h>
#include <fizz/protocol/test/CertUtil.h>
#include <folly/portability/OpenSSL.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace apache::thrift::fast_thrift::security::test {

struct TestCert {
  std::string certPem;
  std::string keyPem;
};

inline std::string toPem(::X509* cert) {
  folly::ssl::BioUniquePtr bio(BIO_new(BIO_s_mem()));
  PEM_write_bio_X509(bio.get(), cert);
  BUF_MEM* mem = nullptr;
  BIO_get_mem_ptr(bio.get(), &mem);
  return std::string(mem->data, mem->length);
}

inline std::string toPem(::EVP_PKEY* key) {
  folly::ssl::BioUniquePtr bio(BIO_new(BIO_s_mem()));
  PEM_write_bio_PrivateKey(
      bio.get(), key, nullptr, nullptr, 0, nullptr, nullptr);
  BUF_MEM* mem = nullptr;
  BIO_get_mem_ptr(bio.get(), &mem);
  return std::string(mem->data, mem->length);
}

// Returns a freshly-generated self-signed P256 cert as PEM strings.
// Cheap enough (~milliseconds) to call per-test.
inline TestCert makeTestCert() {
  auto ck = fizz::test::createCert(
      "fast-thrift-test",
      /*ca=*/false,
      /*issuer=*/nullptr,
      fizz::KeyType::P256);
  return {toPem(ck.cert.get()), toPem(ck.key.get())};
}

} // namespace apache::thrift::fast_thrift::security::test
