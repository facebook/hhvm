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
     ASSERT(strlen(str) == 32);
     const int kQWordAsciiLen = 16;
     char buf[kQWordAsciiLen + 1];
     buf[kQWordAsciiLen] = 0;
     memcpy(buf, str, kQWordAsciiLen);
     ASSERT(strlen(buf) == 16);
     q[0] = strtoull(buf, NULL, 16);

     memcpy(buf, str + kQWordAsciiLen, 16);
     ASSERT(strlen(buf) == 16);
     q[1] = strtoull(buf, NULL, 16);
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
     return q[0] < r.q[0] || q[1] < r.q[1];
   }
   bool operator>(const MD5& r) const {
     return q[0] > r.q[0] || q[1] > r.q[1];
   }
   bool operator!=(const MD5& r) const {
     return !operator==(r);
   }

   uint64 hash() const {
     // hash_int64_pair does more work than necessary; both qwords already
     // are full of entropy. Flip a bit in q[0] to make it order-dependent.
     return q[0] ^ q[1];
   }
};

} // HPHP
#endif

