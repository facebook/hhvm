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

#ifndef incl_HPHP_TEST_FASTCGI_PROTOCOL_H_
#define incl_HPHP_TEST_FASTCGI_PROTOCOL_H_

#include "hphp/test/ext/test_fastcgi.h"
#include "hphp/test/ext/test_base.h"
#include "hphp/util/base.h"

////////////////////////////////////////////////////////////////////////////////

class TestFastCGIProtocol : public TestCodeRun {
public:
  TestFastCGIProtocol();

  virtual bool RunTests(const std::string &which);

protected:
  bool LoadExchangeFromJson(const std::string& path,
                            TestMessageExchange&,
                            const char* file, int line);
  bool AddFile(const std::string& path, const char* file, int line);
  bool VerifyExchange(const TestMessageExchange& messages,
                      int reps,
                      const char* file,
                      int line);

  bool TestSanity();
  bool TestBeginRequest();
  bool TestDoubleBeginRequest();
  bool TestAbortRequest();
  bool TestEndRequest();
  bool TestParams();
  bool TestStdIn();
  bool TestStdOut();
  bool TestStdErr();
  bool TestData();
  bool TestGetValues();
  bool TestUnknownType();
  bool TestIgnoreNonActive();
  bool TestInvalidType();
};

////////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_FASTCGI_PROTOCOL_H_
