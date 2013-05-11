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

#include <compiler/expression/yield_expression.h>
#include <compiler/analysis/function_scope.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

YieldExpression::YieldExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr exp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(YieldExpression)),
    m_exp(exp), m_label(-1) {
}

ExpressionPtr YieldExpression::clone() {
  YieldExpressionPtr exp(new YieldExpression(*this));
  Expression::deepCopy(exp);
  exp->m_exp = Clone(m_exp);
  exp->m_label = m_label;
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions


///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void YieldExpression::analyzeProgram(AnalysisResultPtr ar) {
  assert(getFunctionScope() && getFunctionScope()->isGenerator());
  m_exp->analyzeProgram(ar);
  if (m_label == -1) {
    setLabel(getFunctionScope()->allocYieldLabel());
  }
}

ConstructPtr YieldExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_exp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int YieldExpression::getKidCount() const {
  return 1;
}

void YieldExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_exp = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

TypePtr YieldExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                    bool coerce) {
  m_exp->inferAndCheck(ar, Type::Some, false);
  return Type::Variant;
}


///////////////////////////////////////////////////////////////////////////////
// code generation functions

void YieldExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("yield ");
  m_exp->outputPHP(cg, ar);
}
