/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TEST_FASTCGI_H_
#define incl_HPHP_TEST_FASTCGI_H_

#include "hphp/test/ext/test_code_run.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/types.h"

#include <tuple>
#include <vector>
#include <map>

///////////////////////////////////////////////////////////////////////////////

struct TestMessage {
  enum Command {
    SEND,
    RECV,
    CLOSE,
    RECV_CLOSE,
    CALL
  };
  typedef std::vector<uint8_t> Body;

  bool bodyFromStr(const String& input);
  bool fromJson(const Variant& json);

  Command m_command;
  Body m_body;
  Body m_mask;
  std::map<std::string, Variant> m_args;
};

struct TestMessageExchange {
  bool fromJson(const Variant& json);
  TestMessageExchange aggregate() const;
  TestMessageExchange split() const;

  std::vector<TestMessage> m_messages;
};

/**
 * Testing FastCGI server.
 */
class TestFastCGIServer : public TestCodeRun {
public:
  TestFastCGIServer();

  virtual bool RunTests(const std::string &which);

  bool TestNginxHelloWorld();
  bool TestNginxFileUpload();
  bool TestNginxNotFound();
  bool TestApacheHelloWorld();
  bool TestApacheNotFound();

protected:
  void RunServer();
  void StopServer();
  void SetupServerPorts();
  int FindFreePort(const char* host, int port_min, int port_max);
  void WaitForOpenPort(const char* host, int port);
  void WaitForClosedPort(const char* host, int port);

  bool LoadExchangeFromJson(const std::string& path,
                            TestMessageExchange&,
                            const char* file, int line);
  bool AddFile(const std::string& path, const char* file, int line);
  bool VerifyExchange(const TestMessageExchange& messages,
                      int reps,
                      const char* file,
                      int line);

  int m_serverPort;
  int m_adminPort;
  int m_rpcPort;
  int m_inheritFd;
};

//////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_FASTCGI_H_

