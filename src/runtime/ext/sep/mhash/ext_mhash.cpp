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

#include "ext_mhash.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_mhash(int64 hash, CStrRef data, CStrRef key /* = null_string */) {
  throw NotImplementedException(__func__);
}

Variant f_mhash_get_hash_name(int64 hash) {
  throw NotImplementedException(__func__);
}

int64 f_mhash_count() {
  throw NotImplementedException(__func__);
}

Variant f_mhash_get_block_size(int64 hash) {
  throw NotImplementedException(__func__);
}

Variant f_mhash_keygen_s2k(int64 hash, CStrRef password, CStrRef salt, int64 bytes) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////
}
