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

#include <wangle/ssl/SSLUtil.h>

#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/portability/OpenSSL.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace wangle {

SSLException::SSLException(
    SSLErrorEnum const error,
    std::chrono::milliseconds const& latency,
    uint64_t const bytesRead)
    : std::runtime_error(folly::sformat(
          "SSL error: {}; Elapsed time: {} ms; Bytes read: {}",
          static_cast<int>(error),
          latency.count(),
          bytesRead)),
      error_(error),
      latency_(latency),
      bytesRead_(bytesRead) {}

std::mutex SSLUtil::sIndexLock_;

SSLResumeEnum SSLUtil::getResumeState(folly::AsyncSSLSocket* sslSocket) {
  return sslSocket->getSSLSessionReused()
      ? (sslSocket->sessionIDResumed() ? SSLResumeEnum::RESUME_SESSION_ID
                                       : SSLResumeEnum::RESUME_TICKET)
      : SSLResumeEnum::HANDSHAKE;
}

std::unique_ptr<std::string> SSLUtil::getCommonName(const X509* cert) {
  X509_NAME* subject = X509_get_subject_name((X509*)cert);
  if (!subject) {
    return nullptr;
  }
  char cn[ub_common_name + 1];
  int res =
      X509_NAME_get_text_by_NID(subject, NID_commonName, cn, ub_common_name);
  if (res <= 0) {
    return nullptr;
  } else {
    cn[ub_common_name] = '\0';
    return std::make_unique<std::string>(cn);
  }
}

std::unique_ptr<std::list<std::string>> SSLUtil::getSubjectAltName(
    const X509* cert) {
  auto nameList = std::make_unique<std::list<std::string>>();
  GENERAL_NAMES* names = (GENERAL_NAMES*)X509_get_ext_d2i(
      (X509*)cert, NID_subject_alt_name, nullptr, nullptr);
  if (names) {
    auto guard = folly::makeGuard([names] { GENERAL_NAMES_free(names); });
    size_t count = sk_GENERAL_NAME_num(names);
    CHECK(count < std::numeric_limits<int>::max());
    for (int i = 0; i < (int)count; ++i) {
      GENERAL_NAME* generalName = sk_GENERAL_NAME_value(names, i);
      if (generalName->type == GEN_DNS) {
        ASN1_STRING* s = generalName->d.dNSName;
        const char* name = (const char*)ASN1_STRING_get0_data(s);
        // I can't find any docs on what a negative return value here
        // would mean, so I'm going to ignore it.
        auto len = ASN1_STRING_length(s);
        DCHECK(len >= 0);
        if (size_t(len) != strlen(name)) {
          // Null byte(s) in the name; return an error rather than depending on
          // the caller to safely handle this case.
          return nullptr;
        }
        nameList->emplace_back(name);
      }
    }
  }
  return nameList;
}

folly::ssl::X509UniquePtr SSLUtil::getX509FromCertificate(
    const std::string& certificateData) {
  // BIO_new_mem_buf creates a bio pointing to a read-only buffer. However,
  // older versions of OpenSSL fail to mark the first argument `const`.
  DCHECK_LE(certificateData.length(), std::numeric_limits<int>::max());
  folly::ssl::BioUniquePtr bio(BIO_new_mem_buf(
      (void*)certificateData.data(),
      folly::to_narrow(folly::to_signed(certificateData.length()))));
  if (!bio) {
    throw std::runtime_error("Cannot create mem BIO");
  }

  auto x509 = folly::ssl::X509UniquePtr(
      PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));
  if (!x509) {
    throw std::runtime_error("Cannot read X509 from PEM bio");
  }
  return x509;
}

std::string SSLUtil::decrypt(
    folly::ByteRange ciphertext,
    folly::ByteRange key,
    folly::ByteRange iv,
    const EVP_CIPHER* cipher) {
  auto ctx = folly::ssl::EvpCipherCtxUniquePtr(EVP_CIPHER_CTX_new());
  // Plaintext will be at most the same length as ciphertext
  std::unique_ptr<unsigned char[]> plaintext(
      new unsigned char[ciphertext.size() + EVP_CIPHER_block_size(cipher)]());
  int offset1, offset2;

  /* Initialize the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher. The IV size for *most* modes is
   * the same as the block size. */
  if (EVP_DecryptInit_ex(ctx.get(), cipher, nullptr, key.data(), iv.data()) !=
      1) {
    throw std::runtime_error("Failure when initializing file decryption.");
  }

  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary. */
  DCHECK_LE(ciphertext.size(), std::numeric_limits<int>::max());
  if (EVP_DecryptUpdate(
          ctx.get(),
          plaintext.get(),
          &offset1,
          const_cast<unsigned char*>(ciphertext.data()),
          folly::to_narrow(folly::to_signed(ciphertext.size()))) != 1) {
    throw std::runtime_error("Failure when decrypting file.");
  }

  /* Finalize the decryption. Further plaintext bytes may be written at
   * this stage. */
  if (EVP_DecryptFinal_ex(ctx.get(), plaintext.get() + offset1, &offset2) !=
      1) {
    throw std::runtime_error("Failure when finalizing decryption operation.");
  }
  return std::string(
      reinterpret_cast<char*>(plaintext.get()), offset1 + offset2);
}

folly::Optional<std::string> SSLUtil::decryptOpenSSLEncFilePassString(
    const std::string& filename,
    const std::string& password,
    const EVP_CIPHER* cipher,
    const EVP_MD* digest) {
  // Most of this code adapted from openssl/apps/enc.c
  const std::string magic = "Salted__";
  std::array<unsigned char, EVP_MAX_KEY_LENGTH> key;
  std::array<unsigned char, EVP_MAX_IV_LENGTH> iv;

  // Read encrypted file into string
  std::string fileData;
  if (!folly::readFile(filename.c_str(), fileData)) {
    LOG(ERROR) << "Error reading file: " << filename;
    return folly::none;
  }
  if (fileData.size() < magic.size() + PKCS5_SALT_LEN) {
    LOG(ERROR) << "Not a valid encrypted file.";
    return folly::none;
  }

  // Parse file contents into magic number, salt, and encrypted content
  auto fileMagic = fileData.substr(0, magic.size());
  if (fileMagic.compare(magic) != 0) {
    LOG(ERROR) << "Incorrect magic number in file.";
    return folly::none;
  }
  auto salt = fileData.substr(magic.size(), PKCS5_SALT_LEN);
  auto ciphertext = fileData.substr(magic.size() + PKCS5_SALT_LEN);

  // Construct key and iv from password
  DCHECK_LE(password.size(), std::numeric_limits<int>::max());
  EVP_BytesToKey(
      cipher,
      digest,
      reinterpret_cast<const unsigned char*>(salt.data()),
      reinterpret_cast<const unsigned char*>(password.data()),
      folly::to_narrow(folly::to_signed(password.size())),
      1 /* one round */,
      key.data(),
      iv.data());

  // Decrypt content using key and iv
  return decrypt(
      folly::range(folly::StringPiece(ciphertext)),
      folly::range(key),
      folly::range(iv),
      cipher);
}

folly::Optional<std::string> SSLUtil::decryptOpenSSLEncFilePassFile(
    const std::string& filename,
    const folly::ssl::PasswordCollector& pwdCollector,
    const EVP_CIPHER* cipher,
    const EVP_MD* digest) {
  // Get password as a string and call decryptFilePasswordString()
  std::string password;
  pwdCollector.getPassword(password, 0);
  if (password.empty()) {
    LOG(ERROR) << "Error getting encryption password from collector "
               << pwdCollector;
    return folly::none;
  }
  return decryptOpenSSLEncFilePassString(filename, password, cipher, digest);
}

} // namespace wangle
