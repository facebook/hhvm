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

#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/runtime/base/complex-types.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

QOpExpression::QOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, ExpressionPtr expYes, ExpressionPtr expNo)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(QOpExpression)),
    m_condition(condition), m_expYes(expYes), m_expNo(expNo) {
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
  if (m_expYes) {
    m_expYes->analyzeProgram(ar);
  }
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
      assert(false);
      break;
  }
  return ConstructPtr();
}

int QOpExpression::getKidCount() const {
  return 3;
}

void QOpExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_condition = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_expYes = dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_expNo = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

ExpressionPtr QOpExpression::preOptimize(AnalysisResultConstPtr ar) {
  Variant value;
  if (m_condition->getScalarValue(value)) {
    if (value.toBoolean()) {
      if (m_expYes) {
        return m_expYes;
      }
      return m_condition;
    }
    return m_expNo;
  }

  return ExpressionPtr();
}

ExpressionPtr QOpExpression::postOptimize(AnalysisResultConstPtr ar) {
  if (getActualType() && getActualType()->is(Type::KindOfString) &&
      m_expYes && m_expYes->isLiteralString() != m_expNo->isLiteralString()) {
    setActualType(Type::Variant);
    setExpectedType(Type::String);
    m_expYes->setExpectedType(Type::Variant);
    m_expNo->setExpectedType(Type::Variant);
  }
  return ExpressionPtr();
}

TypePtr QOpExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  if (m_expYes) {
    m_condition->inferAndCheck(ar, Type::Boolean, false);
    TypePtr typeYes = m_expYes->inferAndCheck(ar, Type::Some, coerce);
    TypePtr typeNo = m_expNo->inferAndCheck(ar, Type::Some, coerce);
    if (Type::SameType(typeYes, typeNo) &&
        m_expYes->isLiteralString() == m_expNo->isLiteralString()) {
      // already the same type, no coercion needed
      // special case on literal string since String is slower than Variant
      m_expYes->inferAndCheck(ar, typeYes, false);
      m_expNo->inferAndCheck(ar, typeYes, false);
      return typeYes;
    }
  } else {
    TypePtr typeYes = m_condition->inferAndCheck(ar, Type::Some, coerce);
    TypePtr typeNo = m_expNo->inferAndCheck(ar, Type::Some, coerce);
    if (Type::SameType(typeYes, typeNo) &&
        m_condition->isLiteralString() == m_expNo->isLiteralString()) {
      // already the same type, no coercion needed
      // special case on literal string since String is slower than Variant
      m_condition->inferAndCheck(ar, typeYes, false);
      m_expNo->inferAndCheck(ar, typeYes, false);
      return typeYes;
    }
  }

  return Type::Variant;
}

ExpressionPtr QOpExpression::unneededHelper() {
  bool yesEffect = false;
  if (m_expYes) {
    yesEffect = m_expYes->getContainedEffects();
  }
  bool noEffect = m_expNo->getContainedEffects();

  if (!yesEffect && !noEffect) {
    return Expression::unneededHelper();
  }

  m_expNo = m_expNo->unneeded();
  if (m_expYes) {
    m_expYes = m_expYes->unneeded();
  }
  return static_pointer_cast<Expression>(shared_from_this());
}

///////////////////////////////////////////////////////////////////////////////

void QOpExpression::outputCodeModel(CodeGenerator &cg) {
  if (m_expYes == nullptr) {
    cg.printObjectHeader("ValueIfNullExpression", 3);
    cg.printPropertyHeader("expression");
    m_condition->outputCodeModel(cg);
    cg.printPropertyHeader("valueIfNull");
  } else {
    cg.printObjectHeader("ConditionalExpression", 4);
    cg.printPropertyHeader("condition");
    m_condition->outputCodeModel(cg);
    cg.printPropertyHeader("valueIfTrue");
    m_expYes->outputCodeModel(cg);
    cg.printPropertyHeader("valueIfFalse");
  }
  m_expNo->outputCodeModel(cg);
  cg.printPropertyHeader("sourceLocation");
  cg.printLocation(this->getLocation());
  cg.printObjectFooter();
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void QOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_condition->outputPHP(cg, ar);
  cg_printf(" ? ");
  if (m_expYes) {
    m_expYes->outputPHP(cg, ar);
  }
  cg_printf(" : ");
  m_expNo->outputPHP(cg, ar);
}

