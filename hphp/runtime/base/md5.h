/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_BASE_MD5_H_
#define incl_BASE_MD5_H_

#include <runtime/base/zend/zend_string.h>
#include <util/util.h>

namespace HPHP {
/*
 * Most of php's infrastructure for md5sums treats them as strings. They
 * are 128-bits, though, and fit economically in a fixed-size struct.
 */

static const char* kDefaultMD5 = "00000000000000000000000000000000";
struct MD5 {
  uint64 q[2];
  MD5(const char* str = kDefaultMD5) {
    if (str == kDefaultMD5) {
      q[0] = q[1] = 0;
      return;
    }
    // We expect our input to be null-terminated output from PHP::md5().
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

  std::string toString() const {
    int len = 16;
    char md5nbo[16];
    nbo((void *)md5nbo);
    return std::string(string_bin2hex(md5nbo, len));
  }

  // blob is assumed to be in network byte order.
  MD5(const void* blob) {
    q[0] = ntohq(((const uint64*)blob)[0]);
    q[1] = ntohq(((const uint64*)blob)[1]);
  }

  // Copy out in network byte order.
  void nbo(void* blob) const {
    ((uint64_t*)blob)[0] = htonq(q[0]);
    ((uint64_t*)blob)[1] = htonq(q[1]);
  }

  bool isValid() const {
    /*
     * We arbitrarily choose kDefaultMD5 to be an "impossible" md5 value.
     * If someone manages to construct a PHP compilation unit that
     * collides, well, we owe them a pizza dinner or something.
     */
    return q[0] || q[1];
  }

  bool operator==(const MD5& r) const {
    return q[0] == r.q[0] && q[1] == r.q[1];
  }
  bool operator<(const MD5& r) const {
    return q[0] < r.q[0] || (q[0] == r.q[0] && q[1] < r.q[1]);
  }
  bool operator>(const MD5& r) const {
    return q[0] > r.q[0] || (q[0] == r.q[0] && q[1] > r.q[1]);
  }
  bool operator!=(const MD5& r) const {
    return !operator==(r);
  }

  uint64 hash() const {
    // hash_int64_pair does way more work than necessary; all the bits here
    // are fantastically good.
    return q[0];
  }
};

} // HPHP
#endif

