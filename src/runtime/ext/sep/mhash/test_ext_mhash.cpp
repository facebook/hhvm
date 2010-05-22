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

#include "test_ext_mhash.h"
#include "ext_mhash.h"

IMPLEMENT_SEP_EXTENSION_TEST(Mhash);
///////////////////////////////////////////////////////////////////////////////

bool TestExtMhash::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_mhash);
  RUN_TEST(test_mhash_get_hash_name);
  RUN_TEST(test_mhash_count);
  RUN_TEST(test_mhash_get_block_size);
  RUN_TEST(test_mhash_keygen_s2k);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMhash::test_mhash() {
  return Count(true);
}

bool TestExtMhash::test_mhash_get_hash_name() {
  return Count(true);
}

bool TestExtMhash::test_mhash_count() {
  return Count(true);
}

bool TestExtMhash::test_mhash_get_block_size() {
  return Count(true);
}

bool TestExtMhash::test_mhash_keygen_s2k() {
  return Count(true);
}
