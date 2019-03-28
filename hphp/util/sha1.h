/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <array>
#include <cstdlib>

#include <boost/operators.hpp>

#include <folly/Range.h>
#include <folly/String.h>

#include "hphp/util/assertions.h"
#include "hphp/util/byte-order.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct SHA1 : private boost::totally_ordered<SHA1> {
  static constexpr size_t kQNumWords = 5;
  static constexpr size_t kQWordLen = sizeof(uint32_t);

  // One byte = two ascii hex characters
  static constexpr size_t kQWordHexLen = 2 * kQWordLen;

  std::array<uint32_t, kQNumWords> q;

  SHA1() : q{} {}

  /**
   * Build from a SHA1 hexadecimal string
   */
  explicit SHA1(folly::StringPiece str) {
    assertx(str.size() == kQNumWords * kQWordHexLen);

    char buf[kQWordHexLen + 1];
    buf[kQWordHexLen] = '\0';
    for (auto i = 0; i < kQNumWords; i++) {
      memcpy(buf, str.begin() + (i * kQWordHexLen), kQWordHexLen);
      q.at(i) = strtoul(buf, nullptr, 16);
    }
  }

  // Blob is assumed to be in network byte order.
  explicit SHA1(const void* blob, DEBUG_ONLY size_t len) {
    assertx(len == kQNumWords * kQWordLen);

    for (auto i = 0; i < kQNumWords; i++) {
      q.at(i) = ntohl(reinterpret_cast<const uint32_t*>(blob)[i]);
    }
  }

  explicit SHA1(uint64_t x) {
    q.at(kQNumWords - 2) = x >> 32;
    q.at(kQNumWords - 1) = x;
  }

  // Copy out in network byte order.
  void nbo(void* blob) const {
    for (auto i = 0; i < kQNumWords; i++) {
      reinterpret_cast<uint32_t*>(blob)[i] = htonl(q.at(i));
    }
  }

  std::string toString() const {
    std::string ret;
    char sha1nbo[kQNumWords * kQWordLen];
    nbo(sha1nbo);
    folly::hexlify(folly::StringPiece(sha1nbo, sizeof sha1nbo), ret);
    return ret;
  }

  bool operator==(const SHA1& r) const {
    for (auto i = 0; i < kQNumWords; i++) {
      if (q.at(i) != r.q.at(i)) {
        return false;
      }
    }
    return true;
  }

  bool operator<(const SHA1& r) const {
    for (auto i = 0; i < kQNumWords; i++) {
      if (q.at(i) < r.q.at(i)) {
        return true;
      }

      if (q.at(i) > r.q.at(i)) {
        return false;
      }
    }
    return false;
  }

  uint64_t hash() const {
    // All the bits here are fantastically good.
    return (uint64_t(q[0]) << 32) + q[1];
  }
};

//////////////////////////////////////////////////////////////////////

}
