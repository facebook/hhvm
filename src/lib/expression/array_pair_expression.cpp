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

#include <lib/expression/array_pair_expression.h>
#include <lib/expression/scalar_expression.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayPairExpression::ArrayPairExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr name, ExpressionPtr value, bool ref)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_name(name), m_value(value), m_ref(ref) {
  if (m_ref) {
    m_value->setContext(Expression::RefValue);
  }
}

ExpressionPtr ArrayPairExpression::clone() {
  ArrayPairExpressionPtr exp(new ArrayPairExpression(*this));
  Expression::deepCopy(exp);
  exp->m_name = Clone(m_name);
  exp->m_value = Clone(m_value);
  return exp;
}

bool ArrayPairExpression::isScalar() const {
  return (!m_name || m_name->isScalar()) && m_value->isScalar();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

bool ArrayPairExpression::isScalarArrayPair() const {
  return (!m_name || m_name->isScalar()) && m_value->isScalar();
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

bool ArrayPairExpression::containsDynamicConstant(AnalysisResultPtr ar) const {
  return (m_name && m_name->containsDynamicConstant(ar)) ||
    m_value->containsDynamicConstant(ar);
}

void ArrayPairExpression::analyzeProgram(AnalysisResultPtr ar) {
  if (m_name) m_name->analyzeProgram(ar);
  m_value->analyzeProgram(ar);
}

ConstructPtr ArrayPairExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_name;
    case 1:
      return m_value;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int ArrayPairExpression::getKidCount() const {
  return 2;
}

int ArrayPairExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_name = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 1:
      m_value = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

ExpressionPtr ArrayPairExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_name);
  ar->preOptimize(m_value);
  return ExpressionPtr();
}

ExpressionPtr ArrayPairExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_name);
  ar->postOptimize(m_value);
  return ExpressionPtr();
}

TypePtr ArrayPairExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  if (m_name) {
    m_name->inferAndCheck(ar, NEW_TYPE(Primitive), false);
  }
  m_value->inferAndCheck(ar, NEW_TYPE(Some), false);
  return type;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ArrayPairExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_name) {
    m_name->outputPHP(cg, ar);
    cg.printf(" => ");
  }
  if (m_ref) cg.printf("&");
  m_value->outputPHP(cg, ar);
}

void ArrayPairExpression::outputCPPImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar) {
  cg.printf("NEW(ArrayElement)(");
  if (m_name) {
    m_name->outputCPP(cg, ar);
    cg.printf(", ");
  }
  m_value->outputCPP(cg, ar);
  if (m_name) {
    ScalarExpressionPtr sc = dynamic_pointer_cast<ScalarExpression>(m_name);
    if (sc) {
      int64 hash = sc->getHash();
      if (hash >= 0) {
        cg.printf(", 0x%016llXLL", hash);
      }
    }
  }
  cg.printf(")");
}

void ArrayPairExpression::outputCPPControlledEval(CodeGenerator &cg,
                                                  AnalysisResultPtr ar,
                                                  int temp) {
  cg.printf("NEW(ArrayElement)(");
  if (m_name) {
    m_name->outputCPP(cg, ar);
    cg.printf(", ");
  }
  cg.printf("%s%d", Option::EvalOrderTempPrefix, temp);
  if (m_name) {
    ScalarExpressionPtr sc = dynamic_pointer_cast<ScalarExpression>(m_name);
    if (sc) {
      int64 hash = sc->getHash();
      if (hash >= 0) {
        cg.printf(", 0x%016llXLL", hash);
      }
    }
  }
  cg.printf(")");
}
