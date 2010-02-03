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

#ifndef __TEST_TRANSFORMER_STMT_H__
#define __TEST_TRANSFORMER_STMT_H__

#include <test/test_transformer.h>

///////////////////////////////////////////////////////////////////////////////

class TestTransformerStmt : public TestTransformer {
 public:
  TestTransformerStmt();

  virtual bool RunTests(const std::string &which);

  bool TestFunctionStatement();
  bool TestClassStatement();
  bool TestInterfaceStatement();
  bool TestClassVariable();
  bool TestClassConstant();
  bool TestMethodStatement();
  bool TestStatementList();
  bool TestBlockStatement();
  bool TestIfBranchStatement();
  bool TestIfStatement();
  bool TestWhileStatement();
  bool TestDoStatement();
  bool TestForStatement();
  bool TestSwitchStatement();
  bool TestCaseStatement();
  bool TestBreakStatement();
  bool TestContinueStatement();
  bool TestReturnStatement();
  bool TestGlobalStatement();
  bool TestStaticStatement();
  bool TestEchoStatement();
  bool TestUnsetStatement();
  bool TestExpStatement();
  bool TestForEachStatement();
  bool TestCatchStatement();
  bool TestTryStatement();
  bool TestThrowStatement();
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_TRANSFORMER_STMT_H__
