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

#ifndef __TEST_CODE_ERROR_H__
#define __TEST_CODE_ERROR_H__

#include <test/test_base.h>
#include <compiler/analysis/code_error.h>

///////////////////////////////////////////////////////////////////////////////

class TestCodeError : public TestBase {
 public:
  TestCodeError();

  virtual bool RunTests(const std::string &which);

#define CODE_ERROR_ENTRY(x) bool Test ## x();
#include "../compiler/analysis/core_code_error.inc"
#undef CODE_ERROR_ENTRY

 private:
  bool Verify(HPHP::CodeError::ErrorType type, const char *src,
              const char *file, int line, bool exists);
};

///////////////////////////////////////////////////////////////////////////////
// macros

#define VE(type, src)                                                   \
  if (!Count(Verify(CodeError::type, src, __FILE__, __LINE__, true)))   \
    return false;                                                       \

#define VEN(type, src)                                                  \
  if (!Count(Verify(CodeError::type, src, __FILE__, __LINE__, false)))  \
    return false;                                                       \

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_CODE_ERROR_H__
