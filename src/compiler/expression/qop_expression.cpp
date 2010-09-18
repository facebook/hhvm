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

#include <compiler/expression/qop_expression.h>
#include <compiler/analysis/code_error.h>
#include <runtime/base/complex_types.h>

using namespace HPHP;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

QOpExpression::QOpExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr condition, ExpressionPtr expYes, ExpressionPtr expNo)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES),
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
      ASSERT(false);
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
      m_condition = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_expYes = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    case 2:
      m_expNo = boost::dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
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
  if (getActualType() && getActualType()->is(Type::KindOfString) &&
      m_expYes->isLiteralString() != m_expNo->isLiteralString()) {
    setActualType(Type::Variant);
    setExpectedType(Type::String);
    m_expYes->setExpectedType(Type::Variant);
    m_expNo->setExpectedType(Type::Variant);
  }
  return ExpressionPtr();
}

TypePtr QOpExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                  bool coerce) {
  m_condition->inferAndCheck(ar, Type::Boolean, false);

  TypePtr typeYes = m_expYes->inferAndCheck(ar, Type::Some, coerce);
  TypePtr typeNo = m_expNo->inferAndCheck(ar, Type::Some, coerce);
  if (Type::SameType(typeYes, typeNo)
   && m_expYes->isLiteralString() == m_expNo->isLiteralString()) {
    // already the same type, no coercion needed
    // special case on literal string since String is slower than Variant
    m_expYes->inferAndCheck(ar, typeYes, false);
    m_expNo->inferAndCheck(ar, typeYes, false);
    return typeYes;
  }
  else {
    return Type::Variant;
  }
}

ExpressionPtr QOpExpression::unneededHelper(AnalysisResultPtr ar) {
  bool yesEffect = m_expYes->getContainedEffects();
  bool noEffect = m_expNo->getContainedEffects();

  if (!yesEffect && !noEffect) {
    return Expression::unneededHelper(ar);
  }

  m_expNo = m_expNo->unneeded(ar);
  m_expYes = m_expYes->unneeded(ar);
  return static_pointer_cast<Expression>(shared_from_this());
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void QOpExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  m_condition->outputPHP(cg, ar);
  cg_printf(" ? ");
  m_expYes->outputPHP(cg, ar);
  cg_printf(" : ");
  m_expNo->outputPHP(cg, ar);
}

bool QOpExpression::preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                                 int state) {
  if (!hasEffect()) {
    return Expression::preOutputCPP(cg, ar, state);
  }

  bool fix_condition = m_condition->preOutputCPP(cg, ar, 0);
  if (!ar->inExpression()) {
    return fix_condition ||
      m_expYes->preOutputCPP(cg, ar, 0) ||
      m_expNo->preOutputCPP(cg, ar, 0);
  }

  ar->setInExpression(false);
  bool fix_yes = m_expYes->preOutputCPP(cg, ar, 0);
  bool fix_no = m_expNo->preOutputCPP(cg, ar, 0);
  ar->setInExpression(true);

  if (fix_yes || fix_no) {
    ar->wrapExpressionBegin(cg);
    std::string tmp = genCPPTemp(cg, ar);

    TypePtr typeYes = m_expYes->getActualType();
    TypePtr typeNo = m_expNo->getActualType();
    TypePtr type =
      typeYes && typeNo && Type::SameType(typeYes, typeNo) &&
      !typeYes->is(Type::KindOfVariant) &&
      m_expYes->isLiteralString() == m_expNo->isLiteralString() ?
      typeYes : Type::Variant;

    type->outputCPPDecl(cg, ar);
    cg_printf(" %s;\n", tmp.c_str());
    cg_printf("if (");
    m_condition->outputCPP(cg, ar);
    cg_indentBegin(") {\n");
    m_expYes->preOutputCPP(cg, ar, 0);
    cg_printf("%s = (", tmp.c_str());
    m_expYes->outputCPP(cg, ar);
    cg_indentEnd(");\n");
    cg_indentBegin("} else {\n");
    m_expNo->preOutputCPP(cg, ar, 0);
    cg_printf("%s = (", tmp.c_str());
    m_expNo->outputCPP(cg, ar);
    cg_printf(");\n");
    cg_indentEnd("}\n");
    m_cppValue = tmp;
  } else if (state & FixOrder) {
    preOutputStash(cg, ar, state);
    return true;
  }
  return false;
}

bool QOpExpression::outputCPPUnneeded(CodeGenerator &cg, AnalysisResultPtr ar) {
  return m_cppValue.empty() && Expression::outputCPPUnneeded(cg, ar);
}

static void outputUnneededExpr(CodeGenerator &cg, AnalysisResultPtr ar,
                               ExpressionPtr exp) {
  cg_printf("(");
  if (exp->outputCPPUnneeded(cg, ar)) {
    cg_printf(",");
  }
  cg_printf("false)");
}

void QOpExpression::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (!m_cppValue.empty()) {
    cg_printf("%s", m_cppValue.c_str());
  } else {
    bool wrapped = !isUnused();
    if (wrapped) {
      cg_printf("(");
    }
    m_condition->outputCPP(cg, ar);
    if (isUnused()) {
      cg_printf(" ? ");
      outputUnneededExpr(cg, ar, m_expYes);
      cg_printf(" : ");
      outputUnneededExpr(cg, ar, m_expNo);
    } else {
      TypePtr typeYes = m_expYes->getActualType();
      TypePtr typeNo = m_expNo->getActualType();
      const char *castType =
        typeYes && typeNo && Type::SameType(typeYes, typeNo) &&
        !typeYes->is(Type::KindOfVariant) &&
        m_expYes->isLiteralString() == m_expNo->isLiteralString()
        ? "" : "(Variant)";

      cg_printf(" ? (%s(", castType);
      m_expYes->outputCPP(cg, ar);
      cg_printf(")) : (%s(", castType);
      m_expNo->outputCPP(cg, ar);
      cg_printf("))");
    }
    if (wrapped) {
      cg_printf(")");
    }
  }
}
