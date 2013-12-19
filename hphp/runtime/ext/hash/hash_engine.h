/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_HASH_ENGINE_H_
#define incl_HPHP_EXT_HASH_ENGINE_H_

#include "hphp/util/base.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define L64(x) (x ## ULL)

DECLARE_BOOST_TYPES(HashEngine);
class HashEngine {
public:
  HashEngine(int digest_size_, int block_size_, int context_size_)
    : digest_size(digest_size_), block_size(block_size_),
      context_size(context_size_) {}
  virtual ~HashEngine() {}

  virtual void hash_init(void *context) = 0;
  virtual void hash_update(void *context, const unsigned char *buf,
                           unsigned int count) = 0;
  virtual void hash_final(unsigned char *digest, void *context) = 0;
  virtual void hash_copy(void *new_context, void *old_context) {
    assert(new_context != nullptr);
    assert(old_context != nullptr);
    assert(context_size >= 0);
    memcpy(new_context, old_context, context_size);
  }

  int digest_size;
  int block_size;
  int context_size;
};

typedef std::map<std::string, HashEnginePtr> HashEngineMap;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_HASH_ENGINE_H_
