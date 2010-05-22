/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_HASH_SNEFRU_H__
#define __EXT_HASH_SNEFRU_H__

#include <runtime/ext/hash/hash_engine.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class hash_snefru : public HashEngine {
public:
  hash_snefru();

  virtual void hash_init(void *context);
  virtual void hash_update(void *context, const unsigned char *buf,
                           unsigned int count);
  virtual void hash_final(unsigned char *digest, void *context);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_HASH_SNEFRU_H__
