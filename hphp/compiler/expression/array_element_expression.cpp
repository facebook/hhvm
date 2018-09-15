/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/parser/hphp.tab.hpp"

#include "hphp/runtime/base/execution-context.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

ArrayElementExpression::ArrayElementExpression
(EXPRESSION_CONSTRUCTOR_PARAMETERS,
 ExpressionPtr variable, ExpressionPtr offset)
  : Expression(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(ArrayElementExpression)),
    m_variable(variable), m_offset(offset), m_global(false),
    m_dynamicGlobal(false) {
  m_variable->setContext(Expression::AccessContext);

  if (m_variable->is(Expression::KindOfSimpleVariable)) {
    auto var = dynamic_pointer_cast<SimpleVariable>(m_variable);
    if (var->getName() == "GLOBALS") {
      m_global = true;
      m_dynamicGlobal = true;
      if (m_offset && m_offset->is(Expression::KindOfScalarExpression)) {
        auto offset = dynamic_pointer_cast<ScalarExpression>(m_offset);

        if (offset->isLiteralString()) {
          m_globalName = offset->getIdentifier();
          if (!m_globalName.empty()) {
            m_dynamicGlobal = false;
          }
        }
      }
    }
  }
}

ExpressionPtr ArrayElementExpression::clone() {
  ArrayElementExpressionPtr exp(new ArrayElementExpression(*this));
  Expression::deepCopy(exp);
  exp->m_variable = Clone(m_variable);
  exp->m_offset = Clone(m_offset);
  return exp;
}

void ArrayElementExpression::setContext(Context context) {
  m_context |= context;
  switch (context) {
    case Expression::LValue:
      if (!hasContext(Expression::UnsetContext)) {
        m_variable->setContext(Expression::LValue);
      }
      break;
    case Expression::ExistContext:
    case Expression::UnsetContext:
    case Expression::DeepReference:
      m_variable->setContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_variable->setContext(DeepReference);
      break;
    case Expression::InvokeArgument:
      m_variable->setContext(context);
    default:
      break;
  }
}

void ArrayElementExpression::clearContext(Context context) {
  m_context &= ~context;
  switch (context) {
    case Expression::LValue:
    case Expression::UnsetContext:
    case Expression::DeepReference:
      m_variable->clearContext(context);
      break;
    case Expression::InvokeArgument:
      m_variable->clearContext(context);
      break;
    case Expression::RefValue:
    case Expression::RefParameter:
      m_variable->clearContext(DeepReference);
      break;
    default:
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

bool ArrayElementExpression::appendClass(ExpressionPtr cls,
                                         AnalysisResultConstRawPtr ar,
                                         FileScopePtr file) {
  if (m_variable->is(Expression::KindOfArrayElementExpression)) {
    return dynamic_pointer_cast<ArrayElementExpression>(m_variable)
      ->appendClass(cls, ar, file);
  }
  if (m_variable->is(Expression::KindOfSimpleVariable) ||
      m_variable->is(Expression::KindOfDynamicVariable)) {
    StaticMemberExpressionPtr sme(
      new StaticMemberExpression(
        m_variable->getScope(), m_variable->getRange(),
        cls, m_variable));
    sme->onParse(ar, file);
    m_variable = sme;
    m_global = m_dynamicGlobal = false;
    m_globalName.clear();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

ConstructPtr ArrayElementExpression::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_variable;
    case 1:
      return m_offset;
    default:
      assert(false);
      break;
  }
  return ConstructPtr();
}

int ArrayElementExpression::getKidCount() const {
  return 2;
}

void ArrayElementExpression::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_variable = dynamic_pointer_cast<Expression>(cp);
      break;
    case 1:
      m_offset = dynamic_pointer_cast<Expression>(cp);
      break;
    default:
      assert(false);
      break;
  }
}

/**
 * ArrayElementExpression comes from:
 *
 * reference_variable[|expr]
 * ->object_dim_list[|expr]
 * encaps T_VARIABLE[expr]
 * encaps ${T_STRING[expr]}
 */

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void ArrayElementExpression::outputPHP(CodeGenerator &cg,
                                       AnalysisResultPtr ar) {
  m_variable->outputPHP(cg, ar);
  cg_printf("[");
  if (m_offset) m_offset->outputPHP(cg, ar);
  cg_printf("]");
}
