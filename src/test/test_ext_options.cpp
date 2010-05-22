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

#include <test/test_ext_options.h>
#include <runtime/ext/ext_options.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtOptions::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_assert_options);
  RUN_TEST(test_assert);
  RUN_TEST(test_dl);
  RUN_TEST(test_extension_loaded);
  RUN_TEST(test_get_loaded_extensions);
  RUN_TEST(test_get_extension_funcs);
  RUN_TEST(test_get_cfg_var);
  RUN_TEST(test_get_current_user);
  RUN_TEST(test_get_defined_constants);
  RUN_TEST(test_get_include_path);
  RUN_TEST(test_restore_include_path);
  RUN_TEST(test_set_include_path);
  RUN_TEST(test_get_included_files);
  RUN_TEST(test_get_magic_quotes_gpc);
  RUN_TEST(test_get_magic_quotes_runtime);
  RUN_TEST(test_get_required_files);
  RUN_TEST(test_getenv);
  RUN_TEST(test_getlastmod);
  RUN_TEST(test_getmygid);
  RUN_TEST(test_getmyinode);
  RUN_TEST(test_getmypid);
  RUN_TEST(test_getmyuid);
  RUN_TEST(test_getopt);
  RUN_TEST(test_getrusage);
  RUN_TEST(test_clock_getres);
  RUN_TEST(test_clock_gettime);
  RUN_TEST(test_clock_settime);
  RUN_TEST(test_ini_alter);
  RUN_TEST(test_ini_get_all);
  RUN_TEST(test_ini_get);
  RUN_TEST(test_ini_restore);
  RUN_TEST(test_ini_set);
  RUN_TEST(test_memory_get_peak_usage);
  RUN_TEST(test_memory_get_usage);
  RUN_TEST(test_php_ini_scanned_files);
  RUN_TEST(test_php_logo_guid);
  RUN_TEST(test_php_sapi_name);
  RUN_TEST(test_php_uname);
  RUN_TEST(test_phpcredits);
  RUN_TEST(test_phpinfo);
  RUN_TEST(test_phpversion);
  RUN_TEST(test_putenv);
  RUN_TEST(test_set_magic_quotes_runtime);
  RUN_TEST(test_set_time_limit);
  RUN_TEST(test_sys_get_temp_dir);
  RUN_TEST(test_version_compare);
  RUN_TEST(test_zend_logo_guid);
  RUN_TEST(test_zend_thread_id);
  RUN_TEST(test_zend_version);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtOptions::test_assert_options() {
  f_assert_options(1);
  return Count(true);
}

bool TestExtOptions::test_assert() {
  f_assert(true);
  try {
    f_assert(false);
  } catch (Assertion e) {
    return Count(true);
  }
  return Count(true);
}

bool TestExtOptions::test_dl() {
  VS(f_dl(""), 0);
  return Count(true);
}

bool TestExtOptions::test_extension_loaded() {
  VERIFY(f_extension_loaded("phpmcc"));
  VERIFY(f_extension_loaded("bcmath"));
  VERIFY(f_extension_loaded("spl"));
  VERIFY(f_extension_loaded("curl"));
  VERIFY(f_extension_loaded("simplexml"));
  VERIFY(f_extension_loaded("mysql"));
  return Count(true);
}

bool TestExtOptions::test_get_loaded_extensions() {
  VERIFY(!f_get_loaded_extensions().empty());
  return Count(true);
}

bool TestExtOptions::test_get_extension_funcs() {
  try {
    f_get_extension_funcs("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_get_cfg_var() {
  try {
    f_get_cfg_var("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_get_current_user() {
  f_get_current_user();
  return Count(true);
}

bool TestExtOptions::test_get_defined_constants() {
  try {
    f_get_defined_constants(true);
    return Count(false);
  } catch (NotSupportedException e) {
    return Count(true);
  }
  f_get_defined_constants();
  return Count(true);
}

bool TestExtOptions::test_get_include_path() {
  f_get_include_path();
  return Count(true);
}

bool TestExtOptions::test_restore_include_path() {
  f_restore_include_path();
  return Count(true);
}

bool TestExtOptions::test_set_include_path() {
  f_set_include_path("");
  return Count(true);
}

bool TestExtOptions::test_get_included_files() {
  VS(f_get_included_files(), Array::Create());
  return Count(true);
}

bool TestExtOptions::test_get_magic_quotes_gpc() {
  VS(f_get_magic_quotes_gpc(), 0);
  return Count(true);
}

bool TestExtOptions::test_get_magic_quotes_runtime() {
  try {
    f_get_magic_quotes_runtime();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_get_required_files() {
  try {
    f_get_required_files();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_getenv() {
  VS(f_getenv("NOTTHERE"), false);
  return Count(true);
}

bool TestExtOptions::test_getlastmod() {
  try {
    f_getlastmod();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_getmygid() {
  f_getmygid();
  return Count(true);
}

bool TestExtOptions::test_getmyinode() {
  try {
    f_getmyinode();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_getmypid() {
  VERIFY(f_getmypid());
  return Count(true);
}

bool TestExtOptions::test_getmyuid() {
  f_getmyuid();
  return Count(true);
}

bool TestExtOptions::test_getopt() {
  f_getopt("");
  return Count(true);
}

bool TestExtOptions::test_getrusage() {
  VERIFY(!f_getrusage().empty());
  return Count(true);
}

bool TestExtOptions::test_clock_getres() {
  Variant sec, nsec;
  VERIFY(f_clock_getres(k_CLOCK_THREAD_CPUTIME_ID, ref(sec), ref(nsec)));
  VS(sec, 0);
  VS(nsec, 1);
  return Count(true);
}

bool TestExtOptions::test_clock_gettime() {
  Variant sec, nsec;
  VERIFY(f_clock_gettime(k_CLOCK_THREAD_CPUTIME_ID, ref(sec), ref(nsec)));
  return Count(true);
}

bool TestExtOptions::test_clock_settime() {
  f_clock_settime(k_CLOCK_THREAD_CPUTIME_ID, 100, 100);
  return Count(true);
}

bool TestExtOptions::test_ini_alter() {
  try {
    f_ini_alter("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_ini_get_all() {
  try {
    f_ini_get_all();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_ini_get() {
  VS(f_ini_get(""), "");
  f_ini_set("memory_limit", 50000000);
  VS(f_ini_get("memory_limit"), "50000000");
  f_set_time_limit(30);
  VS(f_ini_get("max_execution_time"), "30");
  f_ini_set("max_execution_time", 40);
  VS(f_ini_get("max_execution_time"), "40");
  return Count(true);
}

bool TestExtOptions::test_ini_restore() {
  f_ini_restore("");
  return Count(true);
}

bool TestExtOptions::test_ini_set() {
  f_ini_set("", "");
  return Count(true);
}

bool TestExtOptions::test_memory_get_peak_usage() {
  f_memory_get_peak_usage();
  return Count(true);
}

bool TestExtOptions::test_memory_get_usage() {
  f_memory_get_usage();
  return Count(true);
}

bool TestExtOptions::test_php_ini_scanned_files() {
  try {
    f_php_ini_scanned_files();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_php_logo_guid() {
  try {
    f_php_logo_guid();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_php_sapi_name() {
  f_php_sapi_name();
  return Count(true);
}

bool TestExtOptions::test_php_uname() {
  VERIFY(!f_php_uname().empty());
  return Count(true);
}

bool TestExtOptions::test_phpcredits() {
  try {
    f_phpcredits();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_phpinfo() {
  //f_phpinfo();
  return Count(true);
}

bool TestExtOptions::test_phpversion() {
  VS(f_phpversion(), "5.2.5.hiphop");
  return Count(true);
}

bool TestExtOptions::test_putenv() {
  VERIFY(f_putenv("FOO=bar"));
  VERIFY(!f_putenv("FOO"));
  return Count(true);
}

bool TestExtOptions::test_set_magic_quotes_runtime() {
  f_set_magic_quotes_runtime(false);
  return Count(true);
}

bool TestExtOptions::test_set_time_limit() {
  f_set_time_limit(2);
  return Count(true);
}

bool TestExtOptions::test_sys_get_temp_dir() {
  VERIFY(f_sys_get_temp_dir() == "/tmp");
  return Count(true);
}

bool TestExtOptions::test_version_compare() {
  VERIFY(!f_version_compare("1.3.0.dev", "1.1.2", "<"));
  return Count(true);
}

bool TestExtOptions::test_zend_logo_guid() {
  try {
    f_zend_logo_guid();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_zend_thread_id() {
  try {
    f_zend_thread_id();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtOptions::test_zend_version() {
  try {
    f_zend_version();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}
