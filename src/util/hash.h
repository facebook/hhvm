/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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
 * How to toggle hash functions: comment/uncomment the following macro
 * definition, build hphp, make -C system, and then rebuild hphp.
 */
#define USE_MURMUR 1

#ifdef USE_MURMUR

/*
 * http://murmurhash.googlepages.com/ (64bit, seed = 0)
 *
 * The case-insensitive version converts 8 bytes of lowercased characters
 * uppercase at once. This should work as identifiers usually only contain
 * alphanumeric characters and the underscore. Although PHP allows higher
 * ASCII characters (> 127) in an identifier, they should be very rare, and
 * do not change the correctness.
 */

inline long long hash_string(const char *arKey, int nKeyLength) {
  const long long m = 0xc6a4a7935bd1e995;
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

  return h & 0x7fffffffffffffff;
}

inline long long hash_string_i(const char *arKey, int nKeyLength) {
  const unsigned long long m = 0xc6a4a7935bd1e995;
  const int r = 47;

  register unsigned long long h = 0;

  const unsigned long long * data = (const unsigned long long *)arKey;
  const unsigned long long * end = data + (nKeyLength / 8);

  while (data != end) {
    unsigned long long k = *data++;
    k &= 0xdfdfdfdfdfdfdfdf; // a-z => A-Z

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

  return h & 0x7fffffffffffffff;
}

#else /* not using murmur hashing */

/*
 * DJBX33A (Daniel J. Bernstein, Times 33 with Addition)
 *
 * This is Daniel J. Bernstein's popular `times 33' hash function as
 * posted by him years ago on comp.lang.c. It basically uses a function
 * like ``hash(i) = hash(i-1) * 33 + str[i]''. This is one of the best
 * known hash functions for strings. Because it is both computed very
 * fast and distributes very well.
 *
 * The magic of number 33, i.e. why it works better than many other
 * constants, prime or not, has never been adequately explained by
 * anyone. So I try an explanation: if one experimentally tests all
 * multipliers between 1 and 256 (as RSE did now) one detects that even
 * numbers are not useable at all. The remaining 128 odd numbers
 * (except for the number 1) work more or less all equally well. They
 * all distribute in an acceptable way and this way fill a hash table
 * with an average percent of approx. 86%.
 *
 * If one compares the Chi^2 values of the variants, the number 33 not
 * even has the best value. But the number 33 and a few other equally
 * good numbers like 17, 31, 63, 127 and 129 have nevertheless a great
 * advantage to the remaining numbers in the large set of possible
 * multipliers: their multiply operation can be replaced by a faster
 * operation based on just one shift plus either a single addition
 * or subtraction operation. And because a hash function has to both
 * distribute good _and_ has to be very fast to compute, those few
 * numbers should be preferred and seems to be the reason why Daniel J.
 * Bernstein also preferred it.
 *
 *
 *                  -- Ralf S. Engelschall <rse@engelschall.com>
 */

inline long long hash_string(const char *arKey, int nKeyLength) {
  register unsigned long long hash = 5381;

  /* variant with the hash unrolled eight times */
  for (; nKeyLength >= 8; nKeyLength -= 8) {
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
  }
  switch (nKeyLength) {
  case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
  case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
  case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
  case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
  case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
  case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
  case 1: hash = ((hash << 5) + hash) + *arKey++; break;
  case 0: break;
  default:
    ASSERT(false);
    break;
  }
  long long ret = hash;
  return ret < 0 ? -ret : ret;
}

// Case insensitive version. Hash will be equivalent to hash_string of lower
inline long long hash_string_i(const char *arKey, int nKeyLength) {
  register unsigned long long hash = 5381;
  char c;
  char diff = 'a' - 'A';
  /* variant with the hash unrolled eight times */
  for (; nKeyLength >= 8; nKeyLength -= 8) {
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  }
  switch (nKeyLength) {
  case 7:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 6:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 5:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 4:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 3:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 2:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 1:
    c = *arKey++;
    if (c >= 'A' && c <= 'Z') c += diff;
    hash = ((hash << 5) + hash) + c;
  case 0: break;
  default:
    ASSERT(false);
    break;
  }
  long long ret = hash;
  return ret < 0 ? -ret : ret;
}

#endif /* USE_MURMUR */

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
                                int64& res) {
  if (nKeyLength == 0 || arKey[0] > '9')
    return false;
  if (nKeyLength <= 19 ||
      (arKey[0] == '-' && nKeyLength == 20)) {
    uint64 num = 0;
    bool neg = false;
    uint32 i = 0;
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
      if (num <= 0x7FFFFFFFFFFFFFFE || (neg && num == 0x7FFFFFFFFFFFFFFF)) {
        res = neg ? 0 - num : (int64)num;
        return true;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HASH_H__
