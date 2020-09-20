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

#include <cinttypes>

#include "hphp/util/assertions.h"
#include "hphp/util/mutex.h"
#include "hphp/util/lock.h"
#include "hphp/zend/crypt-blowfish.h"

#include <folly/portability/Unistd.h>

#if defined(_MSC_VER) || defined(__APPLE__)
# include "hphp/zend/php-crypt_r.h"
# define USE_PHP_CRYPT_R 1
#else
# include <crypt.h>
#endif

namespace HPHP {

int string_copy(char *dst, const char *src, int siz) {
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0)
        break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0';    /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return(s - src - 1);  /* count does not include NUL */
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

int string_ncmp(const char *s1, const char *s2, int len) {
  for (int i = 0; i < len; i++) {
    char c1 = s1[i];
    char c2 = s2[i];
    if (c1 > c2) return 1;
    if (c1 < c2) return -1;
  }
  return 0;
}

static int compare_right(char const **a, char const *aend,
                         char const **b, char const *bend) {
  int bias = 0;

  /* The longest run of digits wins.  That aside, the greatest
     value wins, but we can't know that it will until we've scanned
     both numbers to know that they have the same magnitude, so we
     remember it in BIAS. */
  for(;; (*a)++, (*b)++) {
    if ((*a == aend || !isdigit((int)(unsigned char)**a)) &&
        (*b == bend || !isdigit((int)(unsigned char)**b)))
      return bias;
    else if (*a == aend || !isdigit((int)(unsigned char)**a))
      return -1;
    else if (*b == bend || !isdigit((int)(unsigned char)**b))
      return +1;
    else if (**a < **b) {
      if (!bias)
        bias = -1;
    } else if (**a > **b) {
      if (!bias)
        bias = +1;
    }
  }

  return 0;
}

static int compare_left(char const **a, char const *aend,
                        char const **b, char const *bend) {
  /* Compare two left-aligned numbers: the first to have a
     different value wins. */
  for(;; (*a)++, (*b)++) {
    if ((*a == aend || !isdigit((int)(unsigned char)**a)) &&
        (*b == bend || !isdigit((int)(unsigned char)**b)))
      return 0;
    else if (*a == aend || !isdigit((int)(unsigned char)**a))
      return -1;
    else if (*b == bend || !isdigit((int)(unsigned char)**b))
      return +1;
    else if (**a < **b)
      return -1;
    else if (**a > **b)
      return +1;
  }

  return 0;
}

int string_natural_cmp(char const *a, size_t a_len,
                       char const *b, size_t b_len, int fold_case) {
  char ca, cb;
  char const *ap, *bp;
  char const *aend = a + a_len, *bend = b + b_len;
  int fractional, result;

  if (a_len == 0 || b_len == 0)
    return a_len - b_len;

  ap = a;
  bp = b;
  while (1) {
    ca = *ap; cb = *bp;

    /* skip over leading spaces or zeros */
    while (isspace((int)(unsigned char)ca))
      ca = *++ap;

    while (isspace((int)(unsigned char)cb))
      cb = *++bp;

    /* process run of digits */
    if (isdigit((int)(unsigned char)ca)  &&  isdigit((int)(unsigned char)cb)) {
      fractional = (ca == '0' || cb == '0');

      if (fractional)
        result = compare_left(&ap, aend, &bp, bend);
      else
        result = compare_right(&ap, aend, &bp, bend);

      if (result != 0)
        return result;
      else if (ap == aend && bp == bend)
        /* End of the strings. Let caller sort them out. */
        return 0;
      else {
        /* Keep on comparing from the current point. */
        ca = *ap; cb = *bp;
      }
    }

    if (fold_case) {
      ca = toupper((int)(unsigned char)ca);
      cb = toupper((int)(unsigned char)cb);
    }

    if (ca < cb)
      return -1;
    else if (ca > cb)
      return +1;

    ++ap; ++bp;
    if (ap >= aend && bp >= bend)
      /* The strings compare the same.  Perhaps the caller
         will want to call strcmp to break the tie. */
      return 0;
    else if (ap >= aend)
      return -1;
    else if (bp >= bend)
      return 1;
  }
}


//////////////////////////////////////////////////////////////////////

void string_translate(char *str, int len, const char *str_from,
                      const char *str_to, int trlen) {
  int i;
  unsigned char xlat[256];

  if ((trlen < 1) || (len < 1)) {
    return;
  }

  for (i = 0; i < 256; xlat[i] = i, i++);
  for (i = 0; i < trlen; i++) {
    xlat[(unsigned char) str_from[i]] = str_to[i];
  }

  for (i = 0; i < len; i++) {
    str[i] = xlat[(unsigned char) str[i]];
  }
}

char *string_rot13(const char *input, int len) {
  assert(input);

  static char rot13_from[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static char rot13_to[] =
    "nopqrstuvwxyzabcdefghijklmNOPQRSTUVWXYZABCDEFGHIJKLM";

  if (len == 0) {
    return nullptr;
  }
  char *ret = string_duplicate(input, len);
  string_translate(ret, len, rot13_from, rot13_to, 52);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// crc32

/*
 * This code implements the AUTODIN II polynomial
 * The variable corresponding to the macro argument "crc" should
 * be an unsigned long.
 * Original code  by Spencer Garrett <srg@quick.com>
 */

#define CRC32(crc, ch) (crc = (crc >> 8) ^ crc32tab[(crc ^ (ch)) & 0xff])

/* generated using the AUTODIN II polynomial
 *  x^32 + x^26 + x^23 + x^22 + x^16 +
 *  x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + 1
 */
static const unsigned int crc32tab[256] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
  0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
  0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
  0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
  0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
  0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
  0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
  0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
  0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
  0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
  0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
  0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
  0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
  0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
  0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
  0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
  0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
  0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

int string_crc32(const char *p, int len) {
  uint32_t crcinit = 0;
  register int32_t crc = crcinit ^ 0xFFFFFFFF;
  for (; len--; ++p) {
    crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32tab[(crc ^ (*p)) & 0xFF];
  }
  return crc ^ 0xFFFFFFFF;
}

///////////////////////////////////////////////////////////////////////////////
// crypt

#ifdef USE_PHP_CRYPT_R

char* php_crypt_r(const char* key, const char* salt) {
  if (salt[0] == '$' && salt[1] == '1' && salt[2] == '$') {
    char output[MD5_HASH_MAX_LEN], *out;

    out = php_md5_crypt_r(key, salt, output);
    return out ? strdup(out) : nullptr;
  } else if (salt[0] == '$' && salt[1] == '6' && salt[2] == '$') {
    char output[PHP_MAX_SALT_LEN + 1];

    char* crypt_res = php_sha512_crypt_r(key, salt, output, PHP_MAX_SALT_LEN);
    if (!crypt_res) {
      SECURE_ZERO(output, PHP_MAX_SALT_LEN + 1);
      return nullptr;
    } else {
      char* result = strdup(output);
      SECURE_ZERO(output, PHP_MAX_SALT_LEN + 1);
      return result;
    }
  } else if (salt[0] == '$' && salt[1] == '5' && salt[2] == '$') {
    char output[PHP_MAX_SALT_LEN + 1];

    char* crypt_res = php_sha256_crypt_r(key, salt, output, PHP_MAX_SALT_LEN);
    if (!crypt_res) {
      SECURE_ZERO(output, PHP_MAX_SALT_LEN + 1);
      return nullptr;
    } else {
      char* result = strdup(output);
      SECURE_ZERO(output, PHP_MAX_SALT_LEN + 1);
      return result;
    }
  } else if (
    salt[0] == '$' &&
    salt[1] == '2' &&
    salt[3] == '$') {
    char output[PHP_MAX_SALT_LEN + 1];

    memset(output, 0, PHP_MAX_SALT_LEN + 1);

    char* crypt_res = php_crypt_blowfish_rn(key, salt, output, sizeof(output));
    if (!crypt_res) {
      SECURE_ZERO(output, PHP_MAX_SALT_LEN + 1);
      return nullptr;
    } else {
      char* result = strdup(output);
      SECURE_ZERO(output, PHP_MAX_SALT_LEN + 1);
      return result;
    }
  } else if (salt[0] == '*' && (salt[1] == '0' || salt[1] == '1')) {
    return nullptr;
  } else {
    struct php_crypt_extended_data buffer;
    /* DES Fallback */
    memset(&buffer, 0, sizeof(buffer));
    _crypt_extended_init_r();

    char* crypt_res = _crypt_extended_r(key, salt, &buffer);
    if (!crypt_res || (salt[0] == '*' && salt[1] == '0')) {
      return nullptr;
    } else {
      return strdup(crypt_res);
    }
  }
}

#endif

static unsigned char itoa64[] =
  "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static void ito64(char *s, long v, int n) {
  while (--n >= 0) {
    *s++ = itoa64[v&0x3f];
    v >>= 6;
  }
}

char *string_crypt(const char *key, const char *salt) {
  assert(key);
  assert(salt);

  char random_salt[12];
  if (!*salt) {
    memcpy(random_salt,"$1$",3);
    ito64(random_salt+3,rand(),8);
    random_salt[11] = '\0';
    return string_crypt(key, random_salt);
  }

  if ((strlen(salt) > sizeof("$2X$00$")) &&
    (salt[0] == '$') &&
    (salt[1] == '2') &&
    (salt[2] >= 'a') && (salt[2] <= 'z') &&
    (salt[3] == '$') &&
    (salt[4] >= '0') && (salt[4] <= '3') &&
    (salt[5] >= '0') && (salt[5] <= '9') &&
    (salt[6] == '$')) {
    // Bundled blowfish crypt()
    char output[61];
    if (php_crypt_blowfish_rn(key, salt, output, sizeof(output))) {
      return strdup(output);
    }

  } else {
    // System crypt() function
#ifdef USE_PHP_CRYPT_R
    return php_crypt_r(key, salt);
#else
    static Mutex mutex;
    Lock lock(mutex);
    char *crypt_res = crypt(key,salt);

    if (crypt_res) {
      return strdup(crypt_res);
    }
#endif
  }

  return ((salt[0] == '*') && (salt[1] == '0'))
                  ? strdup("*1") : strdup("*0");
}

//////////////////////////////////////////////////////////////////////

char *string_bin2hex(const char *input, int len, char* result) {
  static char hexconvtab[] = "0123456789abcdef";

  assert(input);
  if (len == 0) {
    return nullptr;
  }

  int i, j;
  for (i = j = 0; i < len; i++) {
    result[j++] = hexconvtab[(unsigned char)input[i] >> 4];
    result[j++] = hexconvtab[(unsigned char)input[i] & 15];
  }
  result[j] = '\0';
  return result;
}

char *string_bin2hex(const char *input, int &len) {
  if (!len) return nullptr;
  int inLen = len;
  int outLen = inLen * 2;
  char* result = (char*)malloc(outLen + 1);
  len = outLen;
  return string_bin2hex(input, inLen, result);
}

//////////////////////////////////////////////////////////////////////

}
