/* $Id$ */
/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2015 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Pierre Alain Joye  <pajoye@php.net                          |
   +----------------------------------------------------------------------+
 */

#ifndef incl_HPHP_UTIL_PHP_CRYPT_R_H_
#define incl_HPHP_UTIL_PHP_CRYPT_R_H_

#include "crypt-blowfish.h"
#include "crypt-freesec.h"

#define MD5_HASH_MAX_LEN 120
#define PHP_MAX_SALT_LEN 123

/* Macros specific to the portability of the crypto implementations taken from
 * PHP. If these needed to be more generally used, they could be moved to
 * util/portability.h but for now they are just here. */
#ifdef _MSC_VER
#define SECURE_ZERO(var, size) RtlSecureZeroMemory((var), (size))
#define STRTOUL(s0, s1, base) _strtoui64((s0), (s1), (base))
#else
#define SECURE_ZERO(var, size) memset((var), 0, (size))
#define STRTOUL(s0, s1, base) strtoull((s0), (s1), (base))
#endif

namespace HPHP {

void php_init_crypt_r();
void php_shutdown_crypt_r();

extern void _crypt_extended_init_r(void);

extern char * php_md5_crypt_r(const char *pw, const char *salt, char *out);
extern char * php_sha512_crypt_r (const char *key, const char *salt, char *buffer, int buflen);
extern char * php_sha256_crypt_r (const char *key, const char *salt, char *buffer, int buflen);

}

#endif /* incl_HPHP_UTIL_PHP_CRYPT_R_H_ */
