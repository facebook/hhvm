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

#include <list>
#include <mutex>

#include <folly/String.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <folly/ssl/PasswordCollector.h>

namespace folly {
class AsyncSSLSocket;
} // namespace folly

namespace wangle {

/**
 * SSL session establish/resume status
 *
 * changing these values will break logging pipelines
 */
enum class SSLResumeEnum : uint8_t {
  HANDSHAKE = 0,
  RESUME_SESSION_ID = 1,
  RESUME_TICKET = 3,
  NA = 2,
};

enum class SSLErrorEnum {
  NO_ERROR,
  TIMEOUT,
  DROPPED,
};

class SSLException : public std::runtime_error {
 public:
  SSLException(
      SSLErrorEnum error,
      const std::chrono::milliseconds& latency,
      uint64_t bytesRead);

  SSLErrorEnum getError() const {
    return error_;
  }
  std::chrono::milliseconds getLatency() const {
    return latency_;
  }
  uint64_t getBytesRead() const {
    return bytesRead_;
  }

 private:
  SSLErrorEnum error_{SSLErrorEnum::NO_ERROR};
  std::chrono::milliseconds latency_;
  uint64_t bytesRead_{0};
};

class SSLUtil {
 private:
  static std::mutex sIndexLock_;

  /*
   * This helper function was taken and modified from:
   * https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
   */
  static std::string decrypt(
      folly::ByteRange ciphertext,
      folly::ByteRange key,
      folly::ByteRange iv,
      const EVP_CIPHER* cipher);

 public:
  /**
   * Ensures only one caller will allocate an ex_data index for a given static
   * or global.
   */
  static void getSSLCtxExIndex(int* pindex) {
    std::lock_guard<std::mutex> g(sIndexLock_);
    if (*pindex < 0) {
      *pindex = SSL_CTX_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);
    }
  }

  static void getRSAExIndex(int* pindex) {
    std::lock_guard<std::mutex> g(sIndexLock_);
    if (*pindex < 0) {
      *pindex = RSA_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);
    }
  }

 private:
  // The following typedefs are needed for compatibility across various OpenSSL
  // versions since each change the dup function param types ever so slightly
#if FOLLY_OPENSSL_IS_110 || defined(OPENSSL_IS_BORINGSSL)
  using ex_data_dup_from_arg_t = const CRYPTO_EX_DATA*;
#else
  using ex_data_dup_from_arg_t = CRYPTO_EX_DATA*;
#endif

#if defined(OPENSSL_IS_BORINGSSL) || FOLLY_OPENSSL_PREREQ(3, 0, 0)
  using ex_data_dup_ptr_arg_t = void**;
#else
  using ex_data_dup_ptr_arg_t = void*;
#endif

 public:
  // ex data string dup func
  static int exDataStdStringDup(
      CRYPTO_EX_DATA* /* to */,
      ex_data_dup_from_arg_t /* from */,
      ex_data_dup_ptr_arg_t ptr,
      int /* idx */,
      long /* argl */,
      void* /* argp */) {
    // TODO: With OpenSSL, ptr is passed in as a void* but is actually a void**
    // So we need to convert it and then set to the duped data.
    // see int_dup_ex_data in ex_data.c
    // BoringSSL is saner and uses a void**
    void** dataPtr = reinterpret_cast<void**>(ptr);
    std::string* strData = reinterpret_cast<std::string*>(*dataPtr);
    if (strData) {
      *dataPtr = new std::string(*strData);
    }
    return 1;
  }

  // ex data string free func
  static void exDataStdStringFree(
      void* /* parent */,
      void* ptr,
      CRYPTO_EX_DATA* /* ad */,
      int /* idx */,
      long /* argl */,
      void* /* argp */) {
    if (ptr) {
      auto strPtr = reinterpret_cast<std::string*>(ptr);
      delete strPtr;
    }
  }
  // get an index that will store a std::string*
  static void getSSLSessionExStrIndex(int* pindex) {
    std::lock_guard<std::mutex> g(sIndexLock_);
    if (*pindex < 0) {
      *pindex = SSL_SESSION_get_ex_new_index(
          0, nullptr, nullptr, exDataStdStringDup, exDataStdStringFree);
    }
  }

  static inline std::string hexlify(const std::string& binary) {
    std::string hex;
    folly::hexlify<std::string, std::string>(binary, hex);

    return hex;
  }

  static inline const std::string& hexlify(
      const std::string& binary,
      std::string& hex) {
    folly::hexlify<std::string, std::string>(binary, hex);

    return hex;
  }

  /**
   * Return the SSL resume type for the given socket.
   */
  static SSLResumeEnum getResumeState(folly::AsyncSSLSocket* sslSocket);

  /**
   * Get the Common Name from an X.509 certificate
   * @param cert  certificate to inspect
   * @return  common name, or null if an error occurs
   */
  static std::unique_ptr<std::string> getCommonName(const X509* cert);

  /**
   * Get the Subject Alternative Name value(s) from an X.509 certificate
   * @param cert  certificate to inspect
   * @return  set of zero or more alternative names, or null if
   *            an error occurs
   */
  static std::unique_ptr<std::list<std::string>> getSubjectAltName(
      const X509* cert);

  /**
   * Parse an X509 out of a certificate buffer (usually read from the cert file)
   * @param certificateData  Buffer containing certificate data from file
   * @return  unique_ptr to X509 (may be null)
   */
  static folly::ssl::X509UniquePtr getX509FromCertificate(
      const std::string& certificateData);

  /**
   * Same as decryptOpenSSLEncFilePassFile except the password is a string
   * instead of being stored in a file.
   */
  static folly::Optional<std::string> decryptOpenSSLEncFilePassString(
      const std::string& filename,
      const std::string& password,
      const EVP_CIPHER* cipher,
      const EVP_MD* digest);

  /**
   * Function for decrypting files encrypted with openssl enc as:
   *
   * openssl enc -e -md sha256 -aes-256-cbc -pass file:<passwordFile>
   */
  static folly::Optional<std::string> decryptOpenSSLEncFilePassFile(
      const std::string& filename,
      const folly::ssl::PasswordCollector& pwdCollector,
      const EVP_CIPHER* cipher,
      const EVP_MD* digest);
};

} // namespace wangle
