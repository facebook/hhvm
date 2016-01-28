/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_HASH_KECCAK_H_
#define incl_HPHP_EXT_HASH_KECCAK_H_

#include "hphp/runtime/ext/hash/hash_engine.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class hash_keccak : public HashEngine {
 public:
  hash_keccak(uint32_t capacity, uint32_t digestlen);

  void hash_init(void *context) override;
  void hash_update(void *context, const unsigned char *buf,
                           unsigned int count) override;
  void hash_final(unsigned char *digest, void *context) override;

 private:
  // Only supporting FIPS 202 - SHA3 fixed output modes
  const uint8_t SUFFIX = 0x06;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_HASH_KECCAK_H_
