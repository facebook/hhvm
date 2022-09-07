/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/Base64.h>

#include <glog/logging.h>

#include <folly/Range.h>
#include <folly/portability/OpenSSL.h>
#include <openssl/buffer.h>

namespace {
struct BIODeleter {
 public:
  void operator()(BIO* bio) const {
    BIO_free_all(bio);
  };
};

} // namespace

namespace proxygen {

// Decodes a base64url encoded string
std::string Base64::urlDecode(const std::string& urlB64message) {
  uint8_t padding = (4 - urlB64message.length() % 4) % 4;
  if (padding == 3) {
    return std::string();
  }

  std::string b64message(urlB64message.length() + padding, '=');
  std::transform(urlB64message.begin(),
                 urlB64message.end(),
                 b64message.begin(),
                 [](char c) {
                   if (c == '-') {
                     return '+';
                   } else if (c == '_') {
                     return '/';
                   }
                   return c;
                 });
  return decode(b64message, padding);
}

std::string Base64::decode(const std::string& b64message, int padding) {
  if (b64message.length() % 4 != 0 || padding >= 3) {
    return std::string();
  }

  std::unique_ptr<BIO, BIODeleter> bio, b64;
  size_t decodeLen = b64message.length() * 3 / 4 - padding;
  std::string result(decodeLen, '\0');

  bio.reset(BIO_new_mem_buf((void*)b64message.data(), -1));
  if (!bio) {
    return std::string();
  }
  b64.reset(BIO_new(BIO_f_base64()));
  if (!b64) {
    return std::string();
  }
  bio.reset(BIO_push(b64.release(), bio.release()));

  // Do not use newlines to flush buffer
  BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
  BIO_read(bio.get(), (char*)result.data(), b64message.length());
  DCHECK_LE(result.length(), decodeLen);
  if (result.length() < decodeLen) {
    return std::string();
  }
  return result;
}

std::string Base64::encode(folly::ByteRange buffer) {
  std::unique_ptr<BIO, BIODeleter> bio, b64;
  BUF_MEM* bufferPtr;

  b64.reset(BIO_new(BIO_f_base64()));
  if (!b64) {
    throw std::bad_alloc();
  }
  bio.reset(BIO_new(BIO_s_mem()));
  if (!bio) {
    throw std::bad_alloc();
  }
  bio.reset(BIO_push(b64.release(), bio.release()));

  // Ignore newlines - write everything in one line
  BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
  BIO_write(bio.get(), buffer.data(), buffer.size());
  (void)BIO_flush(bio.get());
  BIO_get_mem_ptr(bio.get(), &bufferPtr);

  std::string result(bufferPtr->data, bufferPtr->length);
  return result;
}

// Encodes a binary safe base 64 string
std::string Base64::urlEncode(folly::ByteRange buffer) {
  std::string result = encode(buffer);
  folly::StringPiece sp(result.data(), result.length());
  uint8_t padding = 0;
  std::transform(sp.begin(), sp.end(), result.begin(), [&padding](char c) {
    if (c == '+') {
      return '-';
    } else if (c == '/') {
      return '_';
    } else if (c == '=') {
      padding++;
    }
    return c;
  });
  DCHECK_LE(padding, result.length());
  result.resize(result.length() - padding);
  return result;
}

} // namespace proxygen
