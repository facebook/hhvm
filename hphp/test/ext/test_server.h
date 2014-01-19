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

#ifndef incl_HPHP_TEST_SERVER_H_
#define incl_HPHP_TEST_SERVER_H_

#include "hphp/test/ext/test_code_run.h"
#include "hphp/runtime/base/complex-types.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * Testing HTTP server.
 */
class TestServer : public TestCodeRun {
public:
  TestServer() = delete;
  explicit TestServer(const std::string serverType);

  virtual bool RunTests(const std::string &which);

  // test test harness
  bool TestSanity();

  // test $_ variables
  bool TestServerVariables();
  // test things that need more than one request
  bool TestInteraction();
  bool TestGet();
  bool TestPost();
  bool TestCookie();

  // test transport related extension functions
  bool TestResponseHeader();
  bool TestSetCookie();

  // test multithreaded request processing
  bool TestRequestHandling();
  bool TestSimpleServer();

  // test inheriting server fd
  virtual bool TestInheritFdServer();

  // test takeover
  virtual bool TestTakeoverServer();

  // test HttpClient class that proxy server uses
  bool TestHttpClient();

  // test RPCServer
  bool TestRPCServer();

  // test XboxServer
  bool TestXboxServer();

  // test PageletServer
  bool TestPageletServer();

protected:
  void RunServer();
  void StopServer();
  bool VerifyServerResponse(const char *input, const char *output,
                            const char *url, const char *method,
                            const char *header, const char *postdata,
                            bool responseHeader,
                            const char *file = "", int line = 0,
                            int port = 0) {
    return VerifyServerResponse(input, &output, &url, 1,
                                method, header, postdata, responseHeader,
                                file, line, port);
  }

  bool VerifyServerResponse(const char *input, const char **outputs,
                            const char **urls, int nUrl, const char *method,
                            const char *header, const char *postdata,
                            bool responseHeader,
                            const char *file = "", int line = 0,
                            int port = 0);
  bool PreBindSocket();
  void CleanupPreBoundSocket();

  const std::string m_serverType;
};

class TestLibEventServer : public TestServer {
  public:
  TestLibEventServer() : TestServer("libevent") {}
};

///////////////////////////////////////////////////////////////////////////////
// macros

#define VSR(input, output)                                              \
  if (!Count(VerifyServerResponse(input, output, "string", "GET", nullptr, \
                                  nullptr, false, __FILE__,__LINE__)))  \
    return false;

#define VSR2(input, output) do {                                        \
    const char* urls[2] = { "string", "string" };                       \
    const char* outputs[2] = { output, output };                        \
    if (!Count(VerifyServerResponse(input, outputs, urls, 2, "GET",     \
                                    nullptr, nullptr, false,            \
                                    __FILE__,__LINE__)))                \
      return false;                                                     \
  } while (false)

#define VSRES(input, output)                                            \
  if (!Count(VerifyServerResponse(input, output, "string", "GET", nullptr, \
                                  nullptr, true, __FILE__,__LINE__)))      \
    return false;

#define VSGET(input, output, url)                                       \
  if (!Count(VerifyServerResponse(input, output, url, "GET", nullptr,      \
                                  nullptr, false, __FILE__,__LINE__)))     \
    return false;

#define VSGETP(input, output, url, port)                                \
  if (!Count(VerifyServerResponse(input, output, url, "GET", nullptr,      \
                                  nullptr, false, __FILE__,__LINE__,       \
                                  port)))                               \
    return false;

#define VSPOST(input, output, url, postdata)                            \
  if (!Count(VerifyServerResponse(input, output, url, "POST", nullptr,     \
                                  postdata, false, __FILE__,__LINE__))) \
    return false;

#define VSRX(input, output, url, method, header, postdata)              \
  if (!Count(VerifyServerResponse(input, output, url, method, header,   \
                                  postdata, false, __FILE__,__LINE__))) \
    return false;

#define WITH_PREBOUND_SOCKET(action) \
  if (!PreBindSocket()) \
    return false; \
  action \
  CleanupPreBoundSocket();

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_SERVER_H_
