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

#ifndef __TEST_PARSER_EXPR_H__
#define __TEST_PARSER_EXPR_H__

#include <test/test_parser.h>

///////////////////////////////////////////////////////////////////////////////

class TestParserExpr : public TestParser {
 public:
  virtual bool RunTests(const std::string &which);

  bool TestExpressionList();
  bool TestAssignmentExpression();
  bool TestSimpleVariable();
  bool TestDynamicVariable();
  bool TestStaticMemberExpression();
  bool TestArrayElementExpression();
  bool TestStringOffsetExpression();
  bool TestDynamicFunctionCall();
  bool TestSimpleFunctionCall();
  bool TestScalarExpression();
  bool TestObjectPropertyExpression();
  bool TestObjectMethodExpression();
  bool TestListAssignment();
  bool TestNewObjectExpression();
  bool TestUnaryOpExpression();
  bool TestBinaryOpExpression();
  bool TestQOpExpression();
  bool TestArrayPairExpression();
  bool TestClassConstantExpression();
  bool TestParameterExpression();
  bool TestModifierExpression();
  bool TestConstant();
  bool TestEncapsListExpression();
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_PARSER_EXPR_H__
