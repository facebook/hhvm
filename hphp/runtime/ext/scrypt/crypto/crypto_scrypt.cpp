/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/scrypt/crypto/crypto_scrypt.h"

#include <sodium.h>

int crypto_scrypt(const uint8_t* password, size_t pwlen, const uint8_t* salt,
                  size_t saltlen, uint64_t N, uint32_t r, uint32_t p,
                  uint8_t* buf, size_t buflen) {
  return crypto_pwhash_scryptsalsa208sha256_ll(password, pwlen, salt, saltlen,
                                               N, r, p, buf, buflen);
}
