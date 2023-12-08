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

#include "hphp/runtime/ext/hash/hash_adler32.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PhpAdler32Ctx {
  unsigned int state;
};

hash_adler32::hash_adler32(bool invert /*= false */) :
  HashEngine(4, 4, sizeof(PhpAdler32Ctx)),
  m_invert(invert) {
}

void hash_adler32::hash_init(void *context) {
  unsigned int &state = ((PhpAdler32Ctx *)context)->state;
  state = 1;
}

void hash_adler32::hash_update(void *context, const unsigned char *buf,
                               unsigned int count) {
  unsigned int &state = ((PhpAdler32Ctx *)context)->state;
  unsigned int s[2];
  s[0] = state & 0xffff;
  s[1] = (state >> 16) & 0xffff;
  for (unsigned int i = 0; i < count; ++i) {
    s[0] = (s[0] + buf[i]) % 65521;
    s[1] = (s[1] + s[0]) % 65521;
  }
  state = s[0] + (s[1] << 16);
}

void hash_adler32::hash_final(unsigned char *digest, void *context) {
  unsigned int &state = ((PhpAdler32Ctx *)context)->state;

  // This was a bug in PHP, see PHP bug #48284
  // We currently rely on the old behaviour
  if (m_invert) {
    digest[3] = (unsigned char)((state >> 24) & 0xff);
    digest[2] = (unsigned char)((state >> 16) & 0xff);
    digest[1] = (unsigned char)((state >> 8) & 0xff);
    digest[0] = (unsigned char)(state & 0xff);
  } else {
    digest[0] = (unsigned char)((state >> 24) & 0xff);
    digest[1] = (unsigned char)((state >> 16) & 0xff);
    digest[2] = (unsigned char)((state >> 8) & 0xff);
    digest[3] = (unsigned char)(state & 0xff);
  }

  state = 0;
}

///////////////////////////////////////////////////////////////////////////////
}
