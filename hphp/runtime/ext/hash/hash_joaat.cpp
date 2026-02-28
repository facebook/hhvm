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

#include "hphp/runtime/ext/hash/hash_joaat.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  struct PhpJooatCtx {
    unsigned int state;
  };

} // anon namespace


hash_joaat::hash_joaat() :
  HashEngine(4, 4, sizeof(PhpJooatCtx)) {
}

void hash_joaat::hash_init(void *context) {
  unsigned int &state = ((PhpJooatCtx *)context)->state;
  state = 0;
}

void hash_joaat::hash_update(void *context, const unsigned char *buf,
                               unsigned int count) {
  unsigned int &state = ((PhpJooatCtx *)context)->state;
  for (unsigned int i = 0; i < count; i++) {
      state += buf[i];
      state += (state << 10);
      state ^= (state >> 6);
  }

  state += (state << 3);
  state ^= (state >> 11);
  state += (state << 15);
}

void hash_joaat::hash_final(unsigned char *digest, void *context_) {
  PhpJooatCtx *context = (PhpJooatCtx*)context_;
#ifdef WORDS_BIGENDIAN
  memcpy(digest, &context->state, 4);
#else
  int i = 0;
  unsigned char *c = (unsigned char *) &context->state;

  for (i = 0; i < 4; i++) {
      digest[i] = c[3 - i];
  }
#endif
  context->state = 0;
}

///////////////////////////////////////////////////////////////////////////////
}
