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

#ifndef _CRYPT_WIHN32_H_
#define _CRYPT_WIHN32_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "crypt-freesec.h"

void php_init_crypt_r();
void php_shutdown_crypt_r();

extern void _crypt_extended_init_r(void);

#define MD5_HASH_MAX_LEN 120
#define PHP_MAX_SALT_LEN 123

#include "crypt-blowfish.h"

extern char * php_md5_crypt_r(const char *pw, const char *salt, char *out);
extern char * php_sha512_crypt_r (const char *key, const char *salt, char *buffer, int buflen);
extern char * php_sha256_crypt_r (const char *key, const char *salt, char *buffer, int buflen);

#ifdef __cplusplus
}
#endif

#endif /* _CRYPT_WIHN32_H_ */
