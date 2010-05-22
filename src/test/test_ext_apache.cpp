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

#include <test/test_ext_apache.h>
#include <runtime/ext/ext_apache.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtApache::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_apache_child_terminate);
  RUN_TEST(test_apache_get_modules);
  RUN_TEST(test_apache_get_version);
  RUN_TEST(test_apache_getenv);
  RUN_TEST(test_apache_lookup_uri);
  RUN_TEST(test_apache_note);
  RUN_TEST(test_apache_request_headers);
  RUN_TEST(test_apache_reset_timeout);
  RUN_TEST(test_apache_response_headers);
  RUN_TEST(test_apache_setenv);
  RUN_TEST(test_ascii2ebcdic);
  RUN_TEST(test_ebcdic2ascii);
  RUN_TEST(test_getallheaders);
  RUN_TEST(test_virtual);
  RUN_TEST(test_apache_get_config);
  RUN_TEST(test_apache_get_scoreboard);
  RUN_TEST(test_apache_get_rewrite_rules);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtApache::test_apache_child_terminate() {
  try {
    f_apache_child_terminate();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_get_modules() {
  try {
    f_apache_get_modules();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_get_version() {
  try {
    f_apache_get_version();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_getenv() {
  try {
    f_apache_getenv("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_lookup_uri() {
  try {
    f_apache_lookup_uri("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_note() {
  f_apache_note("blarb", "foo");
  VS(f_apache_note("blarb", "smurf"), "foo");
  VS(f_apache_note("blarb"), "smurf");
  return Count(true);
}

bool TestExtApache::test_apache_request_headers() {
  f_apache_request_headers();
  return Count(true);
}

bool TestExtApache::test_apache_reset_timeout() {
  try {
    f_apache_reset_timeout();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_response_headers() {
  f_apache_response_headers();
  return Count(true);
}

bool TestExtApache::test_apache_setenv() {
  f_apache_setenv("", "");
  return Count(true);
}

bool TestExtApache::test_ascii2ebcdic() {
  try {
    f_ascii2ebcdic("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_ebcdic2ascii() {
  try {
    f_ebcdic2ascii("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_getallheaders() {
  f_getallheaders();
  return Count(true);
}

bool TestExtApache::test_virtual() {
  try {
    f_virtual("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApache::test_apache_get_config() {
  f_apache_get_config();
  return Count(true);
}

bool TestExtApache::test_apache_get_scoreboard() {
  f_apache_get_scoreboard();
  return Count(true);
}

bool TestExtApache::test_apache_get_rewrite_rules() {
  try {
    f_apache_get_rewrite_rules();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}
