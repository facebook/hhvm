/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/expression/null_coalesce_expression.h"
#include "hphp/compiler/analysis/code_error.h"

#include "hphp/runtime/base/type-variant.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

NullCoalesceExpression::NullCoalesceExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr expFirst, ExpressionPtr expSecond)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(NullCoalesceExpression)),
    m_expFirst(expFirst), m_expSecond(expSecond) {
}

ExpressionPtr NullCoalesceExpression::clone() {
  NullCoalesceExpressionPtr exp(new NullCoalesceExpression(*this));
  Expression::deepCopy(exp);
  exp->m_expFirst = Clone(m_expFirst);
  exp->m_expSecond = Clone(m_expSecond);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void NullCoalesceExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_expFirst->analyzeProgram(ar);
  m_expSecond->analyzeProgram(ar);
}

ConstructPtr NullCoalesceExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_expFirst;
    case 1:
      return m_expSecond;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int NullCoalesceExpression::getKidCount() const {
  return 2;
}

void NullCoalesceExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_expFirst = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_expSecond = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

ExpressionPtr NullCoalesceExpression::preOptimize(AnalysisResultConstPtr ar) {
  Variant value;
  if (m_expFirst->getScalarValue(value)) {
    return value.isNull() ? m_expSecond : m_expFirst;
  }
  return ExpressionPtr();
}

ExpressionPtr NullCoalesceExpression::unneededHelper() {
  if (!m_expFirst->getContainedEffects() &&
      !m_expSecond->getContainedEffects()) {
    return Expression::unneededHelper();
  }
  m_expFirst = m_expFirst->unneeded();
  m_expSecond = m_expSecond->unneeded();
  return static_pointer_cast<Expression>(shared_from_this());
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void NullCoalesceExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_expFirst->outputPHP(cg, ar);
  cg_printf(" ?? ");
  m_expSecond->outputPHP(cg, ar);
}
