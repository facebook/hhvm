/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <stdint.h>

namespace HPHP {

typedef struct {
  uint32_t state[4];          /* state (ABCD) */
  uint32_t count[2];          /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64]; /* input buffer */
} PHP_MD5_CTX;

void PHP_MD5Init(PHP_MD5_CTX *context);

void PHP_MD5Update(PHP_MD5_CTX * context, const unsigned char *input,
              unsigned int inputLen);

void PHP_MD5Final(unsigned char digest[16], PHP_MD5_CTX * context);

}
