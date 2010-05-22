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

#include <test/test_ext_server.h>
#include <runtime/ext/ext_server.h>
#include <runtime/base/server/pagelet_server.h>
#include <runtime/base/server/xbox_server.h>
#include <runtime/base/runtime_option.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::RunTests(const std::string &which) {
  bool ret = true;

  RuntimeOption::PageletServerThreadCount = 10;
  PageletServer::Restart();

  RuntimeOption::XboxServerThreadCount = 10;
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

  String baseurl("pageletserver?getparam=");
  String baseheader("MyHeader: ");
  String basepost("postparam=");

  std::vector<Object> tasks;
  for (int i = 0; i < TEST_SIZE; ++i) {
    String url = baseurl + String(i);
    String header = baseheader + String(i);
    String post = basepost + String(i);
    Object task = f_pagelet_server_task_start(url, CREATE_VECTOR1(header),
                                              post);
    tasks.push_back(task);
  }

  for (int i = 0; i < TEST_SIZE; ++i) {
    f_pagelet_server_task_status(tasks[i]);
  }

  for (int i = 0; i < TEST_SIZE; ++i)  {
    String expected = "pagelet postparam: ";
    expected += String(i);
    expected += "pagelet getparam: ";
    expected += String(i);
    expected += "pagelet header: ";
    expected += String(i);

    Variant code, headers;
    VS(expected, f_pagelet_server_task_result(tasks[i], ref(headers),
                                              ref(code)));
    VS(code, 200);
    VS(headers[1], "ResponseHeader: okay");
  }

  return Count(true);
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtServer::test_xbox_send_message() {
  Variant ret;
  VERIFY(f_xbox_send_message("hello", ref(ret), 5000));
  VS(ret["code"], 200);
  VS(ret["response"], "olleh");
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
  Object task = f_xbox_task_start("hello");
  f_xbox_task_status(task);
  Variant ret;
  VS(f_xbox_task_result(task, 0, ref(ret)), 200);
  VS(ret, "olleh");
  return Count(true);
}
