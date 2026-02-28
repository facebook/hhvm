/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/zend/zend-string.h"

#include <cstdlib>

#include <folly/ssl/OpenSSLHash.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string string_sha1(folly::StringPiece s) {
  SHA_CTX context;
  SHA1_Init(&context);
  SHA1_Update(&context, s.data(), s.size());

  unsigned char digest[SHA_DIGEST_LENGTH];
  SHA1_Final(digest, &context);
  char hex[sizeof(digest) * 2 + 1];
  string_bin2hex((const char*)digest, sizeof(digest), hex);
  return std::string{hex, sizeof(digest) * 2};
}

char *string_sha1(const char *arg, size_t arg_len, bool raw, int &out_len) {
  SHA_CTX context;
  SHA1_Init(&context);
  SHA1_Update(&context, arg, arg_len);

  unsigned char digest[SHA_DIGEST_LENGTH];
  SHA1_Final(digest, &context);
  out_len = sizeof(digest);
  if (raw) {
    return string_duplicate((const char *)digest, out_len);
  }
  return string_bin2hex((const char*)digest, out_len);
}

///////////////////////////////////////////////////////////////////////////////

struct SHA1Hasher::Impl {
  SHA_CTX ctx;
};

SHA1Hasher::SHA1Hasher()
  : m_impl{std::make_unique<Impl>()}
{
  SHA1_Init(&m_impl->ctx);
}

SHA1Hasher::~SHA1Hasher() {}

void SHA1Hasher::update(const char* p, size_t s) {
  SHA1_Update(&m_impl->ctx, p, s);
}

SHA1 SHA1Hasher::finish() {
  static_assert(SHA_DIGEST_LENGTH == 20);
  std::array<uint8_t, 20> digest;
  SHA1_Final(digest.data(), &m_impl->ctx);
  return SHA1{digest};
}

///////////////////////////////////////////////////////////////////////////////
}
