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

#pragma once

#include "hphp/util/assertions.h"

#include <map>
#include <memory>
#include <cstring>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct HashEngine {
  HashEngine(int digest_size_, int block_size_, int context_size_)
    : digest_size(digest_size_), block_size(block_size_),
      context_size(context_size_) {}
  virtual ~HashEngine() {}

  virtual void hash_init(void *context) = 0;
  virtual void hash_update(void *context, const unsigned char *buf,
                           unsigned int count) = 0;
  virtual void hash_final(unsigned char *digest, void *context) = 0;
  virtual void hash_copy(void *new_context, void *old_context) {
    assertx(new_context != nullptr);
    assertx(old_context != nullptr);
    assertx(context_size >= 0);
    memcpy(new_context, old_context, context_size);
  }

  int digest_size;
  int block_size;
  int context_size;
};

using HashEngineMap = std::map<std::string,std::shared_ptr<HashEngine>>;

///////////////////////////////////////////////////////////////////////////////
}
