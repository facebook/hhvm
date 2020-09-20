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

#ifndef incl_HPHP_TEST_EXT_SERVER_H_
#define incl_HPHP_TEST_EXT_SERVER_H_

#include "hphp/test/ext/test_cpp_ext.h"

///////////////////////////////////////////////////////////////////////////////

struct TestExtServer final : TestCppExt {
  bool RunTests(const std::string& which) override;

  bool test_pagelet_server_task_start();
  bool test_pagelet_server_task_status();
  bool test_pagelet_server_task_result();
  bool test_xbox_send_message();
  bool test_xbox_post_message();
  bool test_xbox_task_start();
  bool test_xbox_task_status();
  bool test_xbox_task_result();
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_EXT_SERVER_H_
