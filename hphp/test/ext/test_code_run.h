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

#ifndef incl_HPHP_TEST_CODE_RUN_H_
#define incl_HPHP_TEST_CODE_RUN_H_

#include "hphp/test/ext/test_base.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * Testing PHP -> C++ -> execution.
 */
class TestCodeRun : public TestBase {
 public:
  TestCodeRun();

  virtual bool preTest();
  virtual bool postTest();
  virtual bool RunTests(const std::string &which) = 0;

 protected:
  bool CleanUp();
  bool RecordMulti(const char *input, const char *output, const char *file,
                   int line, bool nowarnings, bool fileoutput);

  bool VerifyCodeRun(const char *input, const char *output,
                     const char *file = "", int line = 0,
                     bool nowarnings = false, bool fileoutput = false);

  bool m_perfMode;
  int m_test;
};

///////////////////////////////////////////////////////////////////////////////
// macros

#define VCR(a)                                                          \
  if (!Count(VerifyCodeRun(a,nullptr,__FILE__,__LINE__,false))) return false;

#define VCRO(a, b)                                                      \
  if (!Count(VerifyCodeRun(a,b,__FILE__,__LINE__,false))) return false;

#define VCRNW(a)                                                        \
  if (!Count(VerifyCodeRun(a,nullptr,__FILE__,__LINE__,true))) return false;

// Multi VCR
#define MVCR(a)                                                         \
  if (!RecordMulti(a,nullptr,__FILE__,__LINE__,false,false)) return false;

#define MVCRO(a, b)                                                     \
  if (!RecordMulti(a, b, __FILE__,__LINE__,false,false)) return false;

#define MVCROF(a, b)                                                     \
  if (!RecordMulti(a, b, __FILE__,__LINE__,false,true)) return false;

#define MVCRNW(a)                                                       \
  if (!RecordMulti(a,nullptr,__FILE__,__LINE__,true,false)) return false;

#define MVCRONW(a,b)                                                     \
  if (!RecordMulti(a,b,__FILE__,__LINE__,true,false)) return false;

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_CODE_RUN_H_
