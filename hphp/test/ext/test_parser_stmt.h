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

#ifndef incl_HPHP_TEST_PARSER_STMT_H_
#define incl_HPHP_TEST_PARSER_STMT_H_

#include "hphp/test/ext/test_parser.h"

///////////////////////////////////////////////////////////////////////////////

class TestParserStmt : public TestParser {
 public:
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
  bool TestGotoStatement();
  bool TestYieldStatement();
  bool TestUseTraitStatement();
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_PARSER_STMT_H_
