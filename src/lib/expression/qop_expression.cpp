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

#include <lib/expression/qop_expression.h>
#include <lib/analysis/code_error.h>
#include <cpp/base/type_variant.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

QOpExpression::QOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, ExpressionPtr expYes, ExpressionPtr expNo)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
    m_condition(condition), m_expYes(expYes), m_expNo(expNo), m_effect(false) {
  m_effect = (m_condition->hasEffect() ||
              m_expYes->hasEffect() ||
              m_expNo->hasEffect());
}

ExpressionPtr QOpExpression::clone() {
  QOpExpressionPtr exp(new QOpExpression(*this));
  Expression::deepCopy(exp);
  exp->m_condition = Clone(m_condition);
  exp->m_expYes = Clone(m_expYes);
  exp->m_expNo = Clone(m_expNo);
  return exp;
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void QOpExpression::analyzeProgram(AnalysisResultPtr ar) {
  m_condition->analyzeProgram(ar);
  m_expYes->analyzeProgram(ar);
  m_expNo->analyzeProgram(ar);
}

ConstructPtr QOpExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_condition;
    case 1:
      return m_expYes;
    case 2:
      return m_expNo;
    default:
      return ConstructPtr();
  }
  ASSERT(0);
}

int QOpExpression::getKidCount() const {
  return 3;
}

int QOpExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 1:
      m_expYes = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    case 2:
      m_expNo = boost::dynamic_pointer_cast<Expression>(cp);
      return 1;
    default:
      return 0;
  }
  ASSERT(0);
}

ExpressionPtr QOpExpression::preOptimize(AnalysisResultPtr ar) {
  ar->preOptimize(m_condition);
  ar->preOptimize(m_expYes);
  ar->preOptimize(m_expNo);
  Variant value;
  if (m_condition->getScalarValue(value)) {
    if (value.toBoolean()) return m_expYes; else return m_expNo;
  } else {
    return ExpressionPtr();
  }
}

ExpressionPtr QOpExpression::postOptimize(AnalysisResultPtr ar) {
  ar->postOptimize(m_condition);
  ar->postOptimize(m_expYes);
  ar->postOptimize(m_expNo);
  return ExpressionPtr();
}

TypePtr QOpExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  m_condition->inferAndCheck(ar, Type::Boolean, false);

  TypePtr typeSome = Type::CreateType(Type::KindOfSome);
  TypePtr typeYes = m_expYes->inferAndCheck(ar, typeSome, coerce);
  TypePtr typeNo = m_expNo->inferAndCheck(ar, typeSome, coerce);
  if (Type::SameType(typeYes, typeNo)
   && !m_expYes->isLiteralString() && !m_expNo->isLiteralString()) {
    // already the same type, no coercion needed
    // special case on literal string since String is slower than Variant
    return typeYes;
  }
  else {
    return Type::Variant;
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void QOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_condition->outputPHP(cg, ar);
  cg.printf(" ? ");
  m_expYes->outputPHP(cg, ar);
  cg.printf(" : ");
  m_expNo->outputPHP(cg, ar);
}

void QOpExpression::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_condition->outputCPP(cg, ar);
  TypePtr typeYes = m_expYes->getActualType();
  TypePtr typeNo = m_expNo->getActualType();
  const char *castType = 
    typeYes && typeNo && Type::SameType(typeYes, typeNo) && 
    !typeYes->is(Type::KindOfVariant) && 
    m_expYes->isLiteralString() == m_expNo->isLiteralString()
      ? "" : "(Variant)";
  cg.printf(" ? (%s(", castType); 
  m_expYes->outputCPP(cg, ar);
  cg.printf(")) : (%s(", castType);
  m_expNo->outputCPP(cg, ar);
  cg.printf("))");
}
