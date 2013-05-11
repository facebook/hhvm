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

#ifndef incl_HPHP_TEST_DEBUGGER_H_
#define incl_HPHP_TEST_DEBUGGER_H_

#include <test/test_base.h>
#include <runtime/base/complex_types.h>

///////////////////////////////////////////////////////////////////////////////

/**
 * Testing HTTP server.
 */
class TestDebugger : public TestBase {
public:
  TestDebugger();

  virtual bool RunTests(const std::string &which);

  // test test harness
  bool TestSanity();
  bool TestBasic();
  bool TestBreak();
  bool TestFlow();
  bool TestStack();
  bool TestEval();
  bool TestException();
  bool TestInfo();

  bool TestWebRequest();

private:
  int m_serverPort;
  int m_adminPort;
  int m_debugPort;
  std::string m_fname;

  void runServer();
  void stopServer();

  bool getResponse(const std::string& path, std::string& result, int port = -1,
                   const std::string& host = "");
  bool recvFromTests(char& c);

  void testWebRequestHelper();
  void testWebRequestHelperSignal();
  bool m_tempResult;
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_DEBUGGER_H_
