/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/CryptUtil.h>

#include <folly/portability/OpenSSL.h>
#include <iomanip>
#include <openssl/buffer.h>
#include <openssl/md5.h>
#include <sstream>

namespace proxygen {

// Base64 encode using openssl
std::string base64Encode(folly::ByteRange text) {
  std::string result;
  BIO *b64 = BIO_new(BIO_f_base64());
  if (b64 == nullptr) {
    return result;
  }
  BIO *bmem = BIO_new(BIO_s_mem());
  if (bmem == nullptr) {
    BIO_free_all(b64);
    return result;
  }
  BUF_MEM *bptr;

  // chain base64 filter with the memory buffer
  // so that text will be encoded by base64 and flushed to buffer
  BIO *chain = BIO_push(b64, bmem);
  if (chain == nullptr) {
    BIO_free_all(b64);
    return result;
  }
  BIO_set_flags(chain, BIO_FLAGS_BASE64_NO_NL);
  BIO_write(chain, text.begin(), text.size());
  if (BIO_flush(chain) != 1) {
    BIO_free_all(chain);
    return result;
  }

  BIO_get_mem_ptr(chain, &bptr);

  if (bptr && bptr->length > 0) {
    result = std::string((char *)bptr->data, bptr->length);
  }

  // free the whole BIO chain (b64 and mem)
  BIO_free_all(chain);
  return result;
}

// MD5 encode using openssl
std::string md5Encode(folly::ByteRange text) {
  static_assert(MD5_DIGEST_LENGTH == 16, "");

  unsigned char digest[MD5_DIGEST_LENGTH];
  MD5(text.begin(), text.size(), digest);

  // convert digest to hex string
  std::ostringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    ss << std::setw(2) << (unsigned int)digest[i];
  }
  return ss.str();
}

} // namespace proxygen
