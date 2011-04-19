/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_memcache.h>
#include <runtime/ext/ext_memcache.h>

IMPLEMENT_SEP_EXTENSION_TEST(Memcache);
///////////////////////////////////////////////////////////////////////////////

bool TestExtMemcache::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_memcache_connect);
  RUN_TEST(test_memcache_pconnect);
  RUN_TEST(test_memcache_add);
  RUN_TEST(test_memcache_set);
  RUN_TEST(test_memcache_replace);
  RUN_TEST(test_memcache_get);
  RUN_TEST(test_memcache_delete);
  RUN_TEST(test_memcache_increment);
  RUN_TEST(test_memcache_decrement);
  RUN_TEST(test_memcache_close);
  RUN_TEST(test_memcache_debug);
  RUN_TEST(test_memcache_get_version);
  RUN_TEST(test_memcache_flush);
  RUN_TEST(test_memcache_setoptimeout);
  RUN_TEST(test_memcache_get_server_status);
  RUN_TEST(test_memcache_set_compress_threshold);
  RUN_TEST(test_memcache_get_stats);
  RUN_TEST(test_memcache_get_extended_stats);
  RUN_TEST(test_memcache_set_server_params);
  RUN_TEST(test_memcache_add_server);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMemcache::test_memcache_connect() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_pconnect() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_add() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_set() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_replace() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_get() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_delete() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_increment() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_decrement() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_close() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_debug() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_get_version() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_flush() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_setoptimeout() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_get_server_status() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_set_compress_threshold() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_get_stats() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_get_extended_stats() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_set_server_params() {
  return Count(true);
}

bool TestExtMemcache::test_memcache_add_server() {
  return Count(true);
}
