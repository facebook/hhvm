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

#include "hphp/compiler/expression/yield_expression.h"
#include "hphp/compiler/analysis/function_scope.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

YieldExpression::YieldExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr keyExp, ExpressionPtr valExp)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(YieldExpression)),
    m_keyExp(keyExp), m_valExp(valExp), m_label(this) {
}

ExpressionPtr YieldExpression::clone() {
  YieldExpressionPtr exp(new YieldExpression(*this));
  Expression::deepCopy(exp);
  exp->m_keyExp = Clone(m_keyExp);
  exp->m_valExp = Clone(m_valExp);
  exp->m_label.setExpression(exp.get());
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions


///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void YieldExpression::analyzeProgram(AnalysisResultPtr ar) {
  assert(getFunctionScope() && getFunctionScope()->isGenerator());
  if (m_keyExp) {
    m_keyExp->analyzeProgram(ar);
  }
  m_valExp->analyzeProgram(ar);

  m_label.setNew();
}

ConstructPtr YieldExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_keyExp;
    case 1:
      return m_valExp;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int YieldExpression::getKidCount() const {
  return 2;
}

void YieldExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_keyExp = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_valExp = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

TypePtr YieldExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                    bool coerce) {
  if (m_keyExp) {
    m_keyExp->inferAndCheck(ar, Type::Some, false);
  }
  m_valExp->inferAndCheck(ar, Type::Some, false);
  return Type::Variant;
}

///////////////////////////////////////////////////////////////////////////////

void YieldExpression::outputCodeModel(CodeGenerator &cg) {
  cg.printObjectHeader("YieldExpression", m_keyExp != nullptr ? 3 : 2);
  cg.printPropertyHeader("expression");
  if (m_keyExp != nullptr) {
    cg.printPropertyHeader("key");
    m_keyExp->outputCodeModel(cg);
  }
  cg.printPropertyHeader("value");
  m_valExp->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void YieldExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_printf("yield ");
  if (m_keyExp) {
    m_keyExp->outputPHP(cg, ar);
    cg_printf(" => ");
  }
  m_valExp->outputPHP(cg, ar);
}
