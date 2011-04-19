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

#ifndef __HPHP_HASH_H__
#define __HPHP_HASH_H__

#include <stdint.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * "64 bit Mix Functions", from Thomas Wang's "Integer Hash Function."
 * http://www.concentric.net/~ttwang/tech/inthash.htm
 */
inline long long hash_int64(long long key) {
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ ((unsigned long long)key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ ((unsigned long long)key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ ((unsigned long long)key >> 28);
  key = key + (key << 31);
  return key < 0 ? -key : key;
}

/*
 * http://murmurhash.googlepages.com/ (64bit, seed = 0)
 *
 * The case-insensitive version converts 8 bytes of lowercased characters
 * uppercase at once. This should work as identifiers usually only contain
 * alphanumeric characters and the underscore. Although PHP allows higher
 * ASCII characters (> 127) in an identifier, they should be very rare, and
 * do not change the correctness.
 */

inline long long hash_string_cs(const char *arKey, int nKeyLength) {
  const long long m = 0xc6a4a7935bd1e995LL;
  const int r = 47;

  register unsigned long long h = 0;

  const unsigned long long * data = (const unsigned long long *)arKey;
  const unsigned long long * end = data + (nKeyLength / 8);

  while (data != end) {
    unsigned long long k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  const unsigned char * data2 = (const unsigned char*)data;

  switch (nKeyLength & 7) {
  case 7: h ^= (unsigned long long)(data2[6]) << 48;
  case 6: h ^= (unsigned long long)(data2[5]) << 40;
  case 5: h ^= (unsigned long long)(data2[4]) << 32;
  case 4: h ^= (unsigned long long)(data2[3]) << 24;
  case 3: h ^= (unsigned long long)(data2[2]) << 16;
  case 2: h ^= (unsigned long long)(data2[1]) << 8;
  case 1: h ^= (unsigned long long)(data2[0]);
          h *= m;
  };

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h & 0x7fffffffffffffffLL;
}

inline long long hash_string_i(const char *arKey, int nKeyLength) {
  const unsigned long long m = 0xc6a4a7935bd1e995ULL;
  const int r = 47;

  register unsigned long long h = 0;

  const unsigned long long * data = (const unsigned long long *)arKey;
  const unsigned long long * end = data + (nKeyLength / 8);

  while (data != end) {
    unsigned long long k = *data++;
    k &= 0xdfdfdfdfdfdfdfdfULL; // a-z => A-Z

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  const unsigned char * data2 = (const unsigned char*)data;

  switch (nKeyLength & 7) {
  case 7: h ^= (unsigned long long)(data2[6] & 0xdf) << 48;
  case 6: h ^= (unsigned long long)(data2[5] & 0xdf) << 40;
  case 5: h ^= (unsigned long long)(data2[4] & 0xdf) << 32;
  case 4: h ^= (unsigned long long)(data2[3] & 0xdf) << 24;
  case 3: h ^= (unsigned long long)(data2[2] & 0xdf) << 16;
  case 2: h ^= (unsigned long long)(data2[1] & 0xdf) << 8;
  case 1: h ^= (unsigned long long)(data2[0] & 0xdf);
          h *= m;
  };

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h & 0x7fffffffffffffffULL;
}

inline long long hash_string(const char *arKey, int nKeyLength) {
  return hash_string_i(arKey, nKeyLength);
}

/**
 * We probably should get rid of this, so to detect code generation errors,
 * where a binary string is treated as a NULL-terminated literal. Do we ever
 * allow binary strings as array keys or symbol names?
 */
inline long long hash_string(const char *arKey) {
  return hash_string(arKey, strlen(arKey));
}
inline long long hash_string_i(const char *arKey) {
  return hash_string_i(arKey, strlen(arKey));
}

// This function returns true and sets the res parameter if arKey
// is a non-empty string that matches one of the following conditions:
//   1) The string is "0".
//   2) The string starts with a non-zero digit, followed by at most
//      18 more digits, and is less than or equal to 9223372036854775806.
//   3) The string starts with a negative sign, followed by a non-zero
//      digit, followed by at most 18 more digits, and is greater than
//      or equal to -9223372036854775807.
inline bool is_strictly_integer(const char* arKey, size_t nKeyLength,
                                long long& res) {
  if (nKeyLength == 0 || arKey[0] > '9')
    return false;
  if (nKeyLength <= 19 ||
      (arKey[0] == '-' && nKeyLength == 20)) {
    unsigned long long num = 0;
    bool neg = false;
    unsigned i = 0;
    if (arKey[0] == '-') {
      neg = true;
      i = 1;
      // The string "-" is NOT strictly an integer
      if (nKeyLength == 1)
        return false;
      // A string that starts with "-0" is NOT strictly an integer
      if (arKey[1] == '0')
        return false;
    } else if (arKey[0] == '0') {
      // The string "0" is strictly an integer
      if (nKeyLength == 1) {
        res = 0;
        return true;
      }
      // A string that starts with "0" followed by at least one digit
      // is NOT strictly an integer
      return false;
    }
    bool good = true;
    for (; i < nKeyLength; ++i) {
      if (arKey[i] >= '0' && arKey[i] <= '9') {
        num = 10*num + arKey[i] - '0';
      }
      else {
        good = false;
        break;
      }
    }
    if (good) {
      if (num <= 0x7FFFFFFFFFFFFFFEULL || (neg && num == 0x7FFFFFFFFFFFFFFFULL)) {
        res = neg ? 0 - num : (long long)num;
        return true;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HASH_H__
