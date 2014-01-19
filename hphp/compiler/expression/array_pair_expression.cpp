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

#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/code_model_enums.h"
#include "hphp/parser/hphp.tab.hpp"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayPairExpression::ArrayPairExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr name, ExpressionPtr value, bool ref,
 bool collection /* = false */)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ArrayPairExpression)),
    m_name(name), m_value(value), m_ref(ref), m_collection(collection) {
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
  return isScalarArrayPair();
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

bool ArrayPairExpression::isScalarArrayPair() const {
  if (!m_value->isScalar()) return false;
  if (!m_name) return true;
  if (!m_name->isScalar()) return false;
  if (m_name->is(KindOfUnaryOpExpression) &&
      static_pointer_cast<UnaryOpExpression>(m_name)->getOp() == T_ARRAY) {
    return false;
  }
  return true;
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
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ArrayPairExpression::getKidCount() const {
  return 2;
}

void ArrayPairExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_name = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_value = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

TypePtr ArrayPairExpression::inferTypes(AnalysisResultPtr ar, TypePtr type,
                                        bool coerce) {
  if (m_name) {
    m_name->inferAndCheck(ar, Type::Some, false);
  }
  m_value->inferAndCheck(ar, Type::Some, false);
  return type;
}

bool ArrayPairExpression::canonCompare(ExpressionPtr e) const {
  if (!Expression::canonCompare(e)) return false;
  ArrayPairExpressionPtr a =
    static_pointer_cast<ArrayPairExpression>(e);

  return m_ref == a->m_ref;
}

///////////////////////////////////////////////////////////////////////////////

void ArrayPairExpression::outputCodeModel(CodeGenerator &cg) {
  if (m_name) {
    cg.printObjectHeader("BinaryOpExpression", 4);
    cg.printPropertyHeader("expression1");
    m_name->outputCodeModel(cg);
    cg.printPropertyHeader("expression2");
    cg.printExpression(m_value, m_ref);
    cg.printPropertyHeader("operation");
    cg.printValue(PHP_ARRAY_PAIR);
    cg.printPropertyHeader("sourceLocation");
    cg.printLocation(this->getLocation());
    cg.printObjectFooter();
  } else {
    cg.printExpression(m_value, m_ref);
  }
  return;
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ArrayPairExpression::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (m_name) {
    m_name->outputPHP(cg, ar);
    cg_printf(" => ");
  }
  if (m_ref) cg_printf("&");
  m_value->outputPHP(cg, ar);
}
