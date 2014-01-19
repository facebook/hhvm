/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_BASE_MD5_H_
#define incl_HPHP_BASE_MD5_H_

#include <cstring>
#include <memory>

#include <boost/operators.hpp>

#include "folly/String.h"
#include "folly/Range.h"

#include "hphp/util/byte-order.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct MD5 : private boost::totally_ordered<MD5> {
  uint64_t q[2];

  MD5() : q{} {}

  // Input should be null-terminated output from PHP::md5().
  explicit MD5(const char* str) {
    assert(strlen(str) == 32);
    const int kQWordAsciiLen = 16;
    char buf[kQWordAsciiLen + 1];
    buf[kQWordAsciiLen] = 0;
    memcpy(buf, str, kQWordAsciiLen);
    assert(strlen(buf) == 16);
    q[0] = strtoull(buf, nullptr, 16);

    memcpy(buf, str + kQWordAsciiLen, 16);
    assert(strlen(buf) == 16);
    q[1] = strtoull(buf, nullptr, 16);
  }

  // Blob is assumed to be in network byte order.
  explicit MD5(const void* blob) {
    q[0] = ntohq(((const uint64_t*)blob)[0]);
    q[1] = ntohq(((const uint64_t*)blob)[1]);
  }

  // Copy out in network byte order.
  void nbo(void* blob) const {
    ((uint64_t*)blob)[0] = htonq(q[0]);
    ((uint64_t*)blob)[1] = htonq(q[1]);
  }

  // Convert to a std::string with hex representation of the md5.
  std::string toString() const {
    std::string ret;
    char md5nbo[16];
    nbo(md5nbo);
    folly::hexlify(folly::StringPiece(md5nbo, sizeof md5nbo), ret);
    return ret;
  }

  bool operator==(const MD5& r) const {
    return q[0] == r.q[0] && q[1] == r.q[1];
  }

  bool operator<(const MD5& r) const {
    return q[0] < r.q[0] || (q[0] == r.q[0] && q[1] < r.q[1]);
  }

  uint64_t hash() const {
    // hash_int64_pair does way more work than necessary; all the bits here
    // are fantastically good.
    return q[0];
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif

