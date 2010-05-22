/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_ext_session.h>
#include <runtime/ext/ext_session.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtSession::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_session_set_cookie_params);
  RUN_TEST(test_session_get_cookie_params);
  RUN_TEST(test_session_name);
  RUN_TEST(test_session_module_name);
  RUN_TEST(test_session_set_save_handler);
  RUN_TEST(test_session_save_path);
  RUN_TEST(test_session_id);
  RUN_TEST(test_session_regenerate_id);
  RUN_TEST(test_session_cache_limiter);
  RUN_TEST(test_session_cache_expire);
  RUN_TEST(test_session_encode);
  RUN_TEST(test_session_decode);
  RUN_TEST(test_session_start);
  RUN_TEST(test_session_destroy);
  RUN_TEST(test_session_unset);
  RUN_TEST(test_session_commit);
  RUN_TEST(test_session_write_close);
  RUN_TEST(test_session_register);
  RUN_TEST(test_session_unregister);
  RUN_TEST(test_session_is_registered);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSession::test_session_set_cookie_params() {
  f_session_set_cookie_params(10240, "ppp", "ddd", true, true);
  VS(f_session_get_cookie_params(),
     CREATE_MAP5("lifetime", 10240,
                 "path", "ppp",
                 "domain", "ddd",
                 "secure", true,
                 "httponly", true));
  return Count(true);
}

bool TestExtSession::test_session_get_cookie_params() {
  // tested in test_session_set_cookie_params()
  return Count(true);
}

bool TestExtSession::test_session_name() {
  VS(f_session_name("name1"), "PHPSESSID");
  VS(f_session_name("name2"), "name1");
  return Count(true);
}

bool TestExtSession::test_session_module_name() {
  return Count(true);
}

bool TestExtSession::test_session_set_save_handler() {
  return Count(true);
}

bool TestExtSession::test_session_save_path() {
  return Count(true);
}

bool TestExtSession::test_session_id() {
  return Count(true);
}

bool TestExtSession::test_session_regenerate_id() {
  return Count(true);
}

bool TestExtSession::test_session_cache_limiter() {
  return Count(true);
}

bool TestExtSession::test_session_cache_expire() {
  return Count(true);
}

bool TestExtSession::test_session_encode() {
  return Count(true);
}

bool TestExtSession::test_session_decode() {
  return Count(true);
}

bool TestExtSession::test_session_start() {
  f_session_start();
  f_session_destroy();
  return Count(true);
}

bool TestExtSession::test_session_destroy() {
  return Count(true);
}

bool TestExtSession::test_session_unset() {
  return Count(true);
}

bool TestExtSession::test_session_commit() {
  return Count(true);
}

bool TestExtSession::test_session_write_close() {
  return Count(true);
}

bool TestExtSession::test_session_register() {
  try {
    f_session_register(0, 0);
  } catch (NotSupportedException &e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtSession::test_session_unregister() {
  try {
    f_session_unregister("foo");
  } catch (NotSupportedException &e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtSession::test_session_is_registered() {
  try {
    f_session_is_registered("foo");
  } catch (NotSupportedException &e) {
    return Count(true);
  }
  return Count(false);
}
