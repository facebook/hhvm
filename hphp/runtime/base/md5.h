/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_BASE_MD5_H_
#define incl_HPHP_BASE_MD5_H_

#include "hphp/runtime/base/zend_string.h"
#include "hphp/util/util.h"

namespace HPHP {
/*
 * Most of php's infrastructure for md5sums treats them as strings. They
 * are 128-bits, though, and fit economically in a fixed-size struct.
 */

struct MD5 {
  uint64_t q[2];
  MD5() {
    q[0] = q[1] = 0;
  }

  explicit MD5(const char* str) {
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
  explicit MD5(const void* blob) {
    q[0] = ntohq(((const uint64_t*)blob)[0]);
    q[1] = ntohq(((const uint64_t*)blob)[1]);
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

  uint64_t hash() const {
    // hash_int64_pair does way more work than necessary; all the bits here
    // are fantastically good.
    return q[0];
  }
};

} // HPHP
#endif

