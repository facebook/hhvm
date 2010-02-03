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

#ifndef __TEST_TYPE_INFERENCE_H__
#define __TEST_TYPE_INFERENCE_H__

#include <test/test_transformer.h>

///////////////////////////////////////////////////////////////////////////////

class TestTypeInference : public TestTransformer {
 public:
  TestTypeInference();

 public:
  virtual bool RunTests(const std::string &which);

  bool TestLocalVariable();
  bool TestDynamicLocalVariable();
  bool TestClassConstant();
  bool TestClassVariable();
  bool TestDynamicClassVariable();
  bool TestGlobalConstant();
  bool TestGlobalVariable();
  bool TestDynamicGlobalVariable();
  bool TestFunctionReturn();
  bool TestFunctionParameter();
  bool TestMethodParameter();
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_TYPE_INFERENCE_H__
