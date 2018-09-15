/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_TEST_EXT_CURL_H_
#define incl_HPHP_TEST_EXT_CURL_H_

#include "hphp/test/ext/test_cpp_ext.h"

///////////////////////////////////////////////////////////////////////////////

struct TestExtCurl : TestCppExt {
  bool RunTests(const std::string& which) override;

  bool test_curl_init();
  bool test_curl_copy_handle();
  bool test_curl_version();
  bool test_curl_setopt();
  bool test_curl_setopt_array();
  bool test_curl_exec();
  bool test_curl_getinfo();
  bool test_curl_errno();
  bool test_curl_error();
  bool test_curl_close();
  bool test_curl_multi_init();
  bool test_curl_multi_add_handle();
  bool test_curl_multi_remove_handle();
  bool test_curl_multi_exec();
  bool test_curl_multi_select();
  bool test_curl_multi_getcontent();
  bool test_curl_multi_info_read();
  bool test_curl_multi_close();
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_EXT_CURL_H_
