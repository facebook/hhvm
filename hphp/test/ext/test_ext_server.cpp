/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/ext_server.h"
#include "hphp/runtime/server/pagelet-server.h"
#include "hphp/runtime/server/xbox-server.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/ext_file.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::RunTests(const std::string &which) {
  bool ret = true;

  DECLARE_TEST_FUNCTIONS("");

  std::string root = std::string(f_getcwd().toString().c_str()) + "/test/ext/";

  RuntimeOption::SourceRoot = root;
  RuntimeOption::PageletServerThreadCount = 10;
  PageletServer::Restart();

  RuntimeOption::XboxServerThreadCount = 10;
  RuntimeOption::XboxServerInfoReqInitDoc = root + "test_xbox_init.php";
  XboxServer::Restart();

  RUN_TEST(test_dangling_server_proxy_old_request);
  RUN_TEST(test_dangling_server_proxy_new_request);
  RUN_TEST(test_pagelet_server_task_start);
  RUN_TEST(test_pagelet_server_task_status);
  RUN_TEST(test_pagelet_server_task_result);
  RUN_TEST(test_xbox_send_message);
  RUN_TEST(test_xbox_post_message);
  RUN_TEST(test_xbox_task_start);
  RUN_TEST(test_xbox_task_status);
  RUN_TEST(test_xbox_task_result);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::test_dangling_server_proxy_old_request() {
  return Count(true);
}

bool TestExtServer::test_dangling_server_proxy_new_request() {
  return Count(true);
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

  std::vector<Resource> tasks;
  for (int i = 0; i < TEST_SIZE; ++i) {
    String url = baseurl + String(i);
    String header = baseheader + String(i);
    String post = basepost + String(i);
    Resource task = f_pagelet_server_task_start(url, make_packed_array(header),
                                                post);
    tasks.push_back(task);
  }

  for (int i = 0; i < TEST_SIZE; ++i) {
    f_pagelet_server_task_status(tasks[i]);
  }

  // Calls that time out (try 1 ms) should return a status code of -1
  for (int i = 0; i < TEST_SIZE; ++i) {
    Variant code, headers;
    VS("", f_pagelet_server_task_result(tasks[i], ref(headers), ref(code), 1));
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
    Variant code, headers;
    VS(expected, f_pagelet_server_task_result(tasks[i], ref(headers),
                                              ref(code), 0));
    VS(code, 200);
    VS(headers[1], "ResponseHeader: okay");

    VS(expected, f_pagelet_server_task_result(tasks[i], ref(headers),
                                              ref(code), 1));
    VS(code, 200);
  }

  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::test_xbox_send_message() {
  static const StaticString
    s_code("code"),
    s_response("response");
  Variant ret;
  VERIFY(f_xbox_send_message("hello", ref(ret), 5000));
  VS(ret[s_code], 200);
  VS(ret[s_response], "olleh");
  return Count(true);
}

bool TestExtServer::test_xbox_post_message() {
  VERIFY(f_xbox_post_message("hello"));
  return Count(true);
}

bool TestExtServer::test_xbox_task_start() {
  // tested in test_xbox_task_result()
  return Count(true);
}

bool TestExtServer::test_xbox_task_status() {
  // tested in test_xbox_task_result()
  return Count(true);
}

bool TestExtServer::test_xbox_task_result() {
  Resource task = f_xbox_task_start("hello");
  f_xbox_task_status(task);
  Variant ret;
  VS(f_xbox_task_result(task, 0, ref(ret)), 200);
  VS(ret, "olleh");
  return Count(true);
}
