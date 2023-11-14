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

#include "hphp/test/ext/test_ext_server.h"
#include <vector>
#include "hphp/runtime/ext/server/ext_server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::RunTests(const std::string &which) {
  bool ret = true;

  DECLARE_TEST_FUNCTIONS("");

  std::string root = std::string(HHVM_FN(getcwd)().toString().c_str()) +
                     "/test/ext/";

  RuntimeOption::SourceRoot = root;
  RuntimeOption::PageletServerThreadCount = 10;
  PageletServer::Restart();

  RuntimeOption::XboxServerThreadCount = 10;
  RuntimeOption::XboxServerInfoReqInitDoc = root + "test_xbox_init.php";
  XboxServer::Restart();

  RUN_TEST(test_pagelet_server_task_start);
  RUN_TEST(test_pagelet_server_task_status);
  RUN_TEST(test_pagelet_server_task_result);
  RUN_TEST(test_xbox_task_start);
  RUN_TEST(test_xbox_task_status);
  RUN_TEST(test_xbox_task_result);

  return ret;
}


///////////////////////////////////////////////////////////////////////////////
// Pagelet Server unit test

bool TestExtServer::test_pagelet_server_task_start() {
  // tested in test_pagelet_server_task_result()
  return Count(true);
}

bool TestExtServer::test_pagelet_server_task_status() {
  // tested in test_pagelet_server_task_result()
  return Count(true);
}

bool TestExtServer::test_pagelet_server_task_result() {
  const int TEST_SIZE = 20;

  String baseurl("ext/pageletserver?getparam=");
  String baseheader("MyHeader: ");
  String basepost("postparam=");

  std::vector<OptResource> tasks;
  tasks.reserve(TEST_SIZE);
  for (int i = 0; i < TEST_SIZE; ++i) {
    String url = baseurl + String(i);
    String header = baseheader + String(i);
    String post = basepost + String(i);
    OptResource task = HHVM_FN(pagelet_server_task_start)(url,
      make_vec_array(header), post);
    tasks.push_back(task);
  }

  for (int i = 0; i < TEST_SIZE; ++i) {
    HHVM_FN(pagelet_server_task_status)(tasks[i]);
  }

  // Calls that time out (try 1 ms) should return a status code of -1
  for (int i = 0; i < TEST_SIZE; ++i) {
    Array headers;
    int64_t code;
    VS("", HHVM_FN(pagelet_server_task_result)(tasks[i], headers, code, 1));
    VS(code, -1);
  }

  for (int i = 0; i < TEST_SIZE; ++i)  {
    String expected = "pagelet postparam: postparam=";
    expected += String(i);
    expected += "pagelet getparam: ";
    expected += String(i);
    expected += "pagelet header: ";
    expected += String(i);

    // A timeout of 0 indicates an infinite timeout that blocks.
    Array headers;
    int64_t code;
    VS(expected, HHVM_FN(pagelet_server_task_result)(tasks[i], headers,
                                                     code, 0));
    VS(code, 200);

    bool hasResponseHeader = false;
    auto const expectedHeader = String("ResponseHeader: okay");

    for (int headerIdx = 0; headerIdx < headers.size(); headerIdx++) {
      if (headers[headerIdx].toString() == expectedHeader) {
        hasResponseHeader = true;
        break;
      }
    }
    VERIFY(hasResponseHeader);
    VS(expected, HHVM_FN(pagelet_server_task_result)(tasks[i], headers,
                                                     code, 1));
    VS(code, 200);
  }

  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::test_xbox_task_start() {
  // tested in test_xbox_task_result()
  return Count(true);
}

bool TestExtServer::test_xbox_task_status() {
  // tested in test_xbox_task_result()
  return Count(true);
}

bool TestExtServer::test_xbox_task_result() {
  OptResource task = HHVM_FN(xbox_task_start)("hello");
  HHVM_FN(xbox_task_status)(task);
  Variant ret;
  VS(HHVM_FN(xbox_task_result)(task, 0, ret), 200);
  VS(ret, "olleh");
  return Count(true);
}
